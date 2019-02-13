/*******************************************************************************
 * BEEP LoRa code, adjusted for Dragino pinout
 *
 * Many thanks to Ideetron, Maarten Westenberg, Thomas Telkamp and Matthijs Kooijman
 * porting the LMIC stack to Arduino IDE and Gerben den Hartog for his tiny
 * stack implementation with the AES library that we used in the LMIC stack.
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with sensor values read.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1,
 *  0.1% in g2).
 *
 * Change DevAddr to a unique address for your node
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Define you own LoRa connection details in lorawan_def.h (from row 238):
 * struct.activation_method -> OVER_THE_AIR_ACTIVATION, or ACTIVATION_BY_PERSONALISATION
 * struct.back_end        -> The_Things_Network, KPN (NL)
 * struct.Session, or OTAA (depending on the chosen activation_method)
 *
 * v7 added:
 *     - w_fl, w_fr, w_bl, w_br
 *     - H   -> *2 (range 0-200)
 *     - T   -> -10 -> +40 range (+10, *5), so 0-250 is /5, -10
 *     - W_E -> -20 -> +80 range (/2, +10, *5), so 0-250 is /5, -10, *2
 *
 * v8 added:
 *     - Measurement timing can be set from sensor_sample_sec (min) to max_send_timeout (max)
 *
 * v9 added:
 *     - T/H DHT22 based on SimpleDHT.h, because std DHT.h has timing issue after getBandgap()
 *
 *******************************************************************************/

// Enable debug prints to serial monitor
#define MY_DEBUG_NO // MY_DEBUG_NO (for production), MY_DEBUG_SENS, MY_DEBUG

#if defined(__AVR__)
#include <avr/pgmspace.h>
#include <Arduino.h>
#elif defined(ARDUINO_ARCH_ESP8266)
#include <ESP.h>
#elif defined(__MKL26Z64__)
#include <Arduino.h>
#else
#error Unknown architecture in aes.cpp
#endif

#include <avr/power.h>
#include <avr/sleep.h>
#include <stdbool.h>

#include <LowPower.h>

#include <SPI.h>
#include <EEPROM.h>

#include "AES-128.h"
#include "Encrypt.h"
#include "Nexus_LoRaWAN.h"
#include "spi_nexus.h" // for RFM_sleep function
#include "RFM95.h"
#include "LoRaMAC.h"
#include "timers.h"
#include "lorawan_def.h"

//#include "DHT.h"   // has timing problems in reltion with battery voltage read script
#include <SimpleDHT.h> // so use SimpleDHT.h in stead.
#include "HX711.h"
#include <DS18B20.h> // much smaller than DallasTemperature
//#include <stdlib.h>

/******************************************************************************************
                  PIN DEFINITIONS
******************************************************************************************/

// Audio samples
// Use A2 ADC in free running mode
#define MIC_SENSE_PIN A2
#define SENSOR_POWER A0
#define LOG_OUT 1 // use the logarithmic output function
#define FHT_N 128 // set to 128 point fht (in stead of 256)
#define DIVIDE_FREQ_BY 60 // max freq is 18kHz at 16MHz processor, so factor 60 is max 300Hz sampling, so each band is 5Hz (@FHT_N=128)
#define MEASURE_THRESHOLD 5 // 0-100 threshold for counting values
#include <FHT.h>

// DHT22
#define DHTPIN 5
//#define DHTTYPE DHT22

// Weight sensors
#define DAT_FRONT A1
#define CLK_FRONT A3
#define DAT_BACK 3
#define CLK_BACK 4

// One wire temp DS18B20, does not work on A6/A7 (analog only)
#define ONEW A4

/******************************************************************************************
                  GLOBAL VARIABLES
******************************************************************************************/

bool large_diff = false;

uint8_t  battery_dV        = 0;  // 30 = 3.0V
uint16_t times_not_sent    = 0;  // times not sent counter
uint16_t max_send_timeout  = 3600; // 10800 = 3 hours, 3600 = 1 hour. Has to be a multiple of 8, max 65536 = 1092 min = 18 hour
uint16_t sensor_sample_sec = 904; // 3600 = 1 hour, 904 = 15 min, 296 = 5 min. Has to be a multiple of 8, max 65536 = 1092 min = 18 hour

  // Interrupt variables.
  volatile bool LORA_SEND;

  double read_supply_voltage (void);

  sAPP        app;    //  Application variables
  sLoRaWAN    lora;   //  See the Nexus_Lorawan.h file for the settings of this structure.

  // Initialize the LoRaWAN stack.
  LORAMAC     lorawan (&lora);

/******************************************************************************************
                  INTERRUPTS
******************************************************************************************/
/*
  @Brief  Interrupt vector for the alarm of the MCP7940 Real Time Clock.
*/
ISR(INT1_vect)
{
  // Set the boolean to true to indicate that the RTC alarm has occurred. Do not use I2C functions or long delays here, but handle that in the main loop.
  LORA_SEND = true;
}

/*
  @Brief  Interrupt vector for Timer1 which is used to time the Join and Receive windows for timeslot 1 and timelsot 2
*/
ISR(TIMER1_COMPA_vect)
{
  // Increment the timeslot counter variable for timing the JOIN and receive delays.
  lora.timeslot++;
}


/******************************************************************************************
                  SENSOR
******************************************************************************************/

// FHT
uint16_t sound_total    = 0;
uint16_t sound_bee_fan4 = 0;
uint16_t sound_bee_fan6 = 0;
uint16_t sound_bee_fan9 = 0;
uint16_t sound_bee_fly  = 0;

float w_fl_offset = 0;
float w_fr_offset = 0;
float w_bl_offset = 0;
float w_br_offset = 0;

uint8_t w_e_hist[] = {0,0,0};
uint8_t t_e_hist[] = {0,0,0};
uint8_t t_i_hist[] = {0,0,0};
uint8_t h_e_hist[] = {0,0,0};
uint8_t s_t_hist[] = {0,0,0};
uint8_t b_v_hist[] = {0,0,0};

// HX711
int16_t weight_calib = 5000; // -5000 for red to E+ and black to E-, else 5000
HX711 weight_front(DAT_FRONT, CLK_FRONT);
HX711 weight_back(DAT_BACK, CLK_BACK);

// DHT 22
SimpleDHT22 dht(DHTPIN);
//DHT dht(DHTPIN, DHTTYPE);

// One wire
OneWire oneWire(ONEW);
DS18B20 sensor(&oneWire);

/******************************************************************************************
                  FUNCTIONS
******************************************************************************************/

// big == true defines if arr is uint16_t (otherwise uint8_t)
uint8_t shiftAndCalculatePercDiff(uint8_t arr[], uint8_t value)
{
  int i;
  float    ave = 0;
  uint16_t sum = 0;
  uint8_t  cnt = 0;

  #ifdef MY_DEBUG_SENS
    Serial.print(F("["));
    for (i = 0; i < 3; i++)
    {
      Serial.print(arr[i]);
      Serial.print(F(","));
    }
    Serial.print(F("]"));
  #endif

  // calculate average
  for (i = 0; i < 3; i++)
  {
    uint8_t val = arr[i];
    if (val > 0)
    {
      sum += val;
      cnt += 1;
    }
  }
  ave = sum / max(1, cnt);

  // push and shift value
  for (i = 2; i > -1; i--)
  {
    if (i > 0)
    {
      arr[i] = arr[i-1]; // 2->*, 1->2, 0->1
    }
    else // push first array value
    {
      arr[i] = value;
    }
  }

  float  dif = value - ave;
  float  fra = (ave == 0) ? dif : (dif / ave);
  int8_t inc = min(127, max(-127, round(100 * fra) ));

  #ifdef MY_DEBUG_SENS
    Serial.print(F(", ave="));
    Serial.print(ave);
    Serial.print(F(", val="));
    Serial.print(value);
    Serial.print(F(", dif="));
    Serial.print(dif);
    Serial.print(F(", inc="));
    Serial.print(inc);
    Serial.print(F("%"));
    Serial.println();
  #endif

  return abs(inc);
}

// Saves the value to the history array and calculates the diff % with
// Sensor char 'w', 't', 'h', 'i', 's', 'b'
uint8_t saveValueAndReturnDiff(char sensor, uint16_t value)
{
  uint8_t diff_perc = 0;

  #ifdef MY_DEBUG_SENS
    Serial.print(F(", "));
    Serial.print(sensor);
  #endif

  if (value == 0 && (sensor != 'w' && sensor != 's')) // do not place 0 value in history, because it is an error value
  {
    Serial.println(F(" ZERO VALUE"));
    return 0;
  }

  switch(sensor){
    case 'w':
      diff_perc = shiftAndCalculatePercDiff(w_e_hist, value);
      if (diff_perc > 2)
        large_diff = true;
      break;
    case 't':
      diff_perc = shiftAndCalculatePercDiff(t_e_hist, value);
      if (diff_perc > 5)
        large_diff = true;
      break;
    case 'h':
      diff_perc = shiftAndCalculatePercDiff(h_e_hist, value);
      break;
      if (diff_perc > 15)
        large_diff = true;
    case 'i':
      diff_perc = shiftAndCalculatePercDiff(t_i_hist, value);
      if (diff_perc > 3)
        large_diff = true;
      break;
    case 's':
      diff_perc = shiftAndCalculatePercDiff(s_t_hist, value);
      if (diff_perc > 50)
        large_diff = true;
      break;
    case 'b':
      diff_perc = shiftAndCalculatePercDiff(b_v_hist, value);
      if (diff_perc > 3)
        large_diff = true;
      break;
  }

  return diff_perc;
}


// Sensor range -10 to +61 => 0x00 == invalid to low, 0xFF is invalid too high
byte tempify(float val)
{
  if(val <= -10)
    return 0;

  if(val >= 41)
    return 255;

  return round(5 * (val + 10));
}

// Sensor range 0 to +655.35 => 0x00 == invalid to low, 0xFFFF is invalid too high
uint16_t floatify(float val)
{
  if(val < 0)
    return 0;

  if(val > 655.35)
    return 65535;

  return round(val * 100);
}

// Sensor range 0 to +218.45 => 0x00 == invalid to low, 0xFFFF is invalid too high
uint16_t weightify(float val)
{
  return floatify(val * 3);
}



// FHT Audio

void sumSounds()
{
  sound_bee_fan9 += normFhtLog(39) + normFhtLog(40);
  sound_bee_fan6 += normFhtLog(47) + normFhtLog(48);
  sound_bee_fly  += normFhtLog(52) + normFhtLog(53);
  sound_bee_fan4 += normFhtLog(59) + normFhtLog(60);

  for (byte i = 7 ; i < FHT_N/2 ; i++) // max bands: FHT_N/2, first 2 to 7 bands are always full
  {
    int add = normFhtLog(i);
    if (sound_total + add < 65535)
      sound_total += add;
  }

}

int normFhtLog(byte i)
{
  return (byte)min(9, fht_log_out[i] * MEASURE_THRESHOLD / 255);
}

void emptyLog()
{
  sound_bee_fan9 = 0;
  sound_bee_fan6 = 0;
  sound_bee_fly  = 0;
  sound_bee_fan4 = 0;
  sound_total    = 0;
}

void sample_audio()
{
  if (battery_dV < 24) // do not measure if voltage is below 2.4V
    return;

  //pinMode(MIC_SENSE_PIN, INPUT);

  ADCSRA = 0xe5; // set the adc to free running mode

  if (MIC_SENSE_PIN == A0)
  {
    ADMUX  = 0x40; // use adc0 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00000001; // turn off the digital input for adc0 of adc 8->0
  }
  else if (MIC_SENSE_PIN == A1)
  {
    ADMUX  = 0x41; // use adc1 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00000010; // turn off the digital input for adc1 of adc 8->0
  }
  else if (MIC_SENSE_PIN == A2)
  {
    ADMUX  = 0x42; // use adc2 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00000100; // turn off the digital input for adc2 of adc 8->0
  }
  else if (MIC_SENSE_PIN == A3)
  {
    ADMUX  = 0x43; // use adc3 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00001000; // turn off the digital input for adc3 of adc 8->0
  }
  else if (MIC_SENSE_PIN == A4)
  {
    ADMUX  = 0x44; // use adc4 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00010000; // turn off the digital input for adc4 of adc 8->0
  }
  else if (MIC_SENSE_PIN == A5)
  {
    ADMUX  = 0x45; // use adc5 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00100000; // turn off the digital input for adc5 of adc 8->0
  }

  //printDebug();
  delay(500);

  cli();  // UDRE interrupt slows this way down on arduino1.0

  TIMSK0 = 0; // turn off timer0 for lower jitter
  for (int i = 0 ; i < FHT_N ; i++) { // save 256 samples

    ADCSRA = 0xf5; // restart adc

    // Cut down freq to Freq_max / DIVIDE_FREQ_BY (10 ~= 900Hz)
    long adc_l = 0;
    long adc_h = 0;
    for ( int8_t inner = 0; inner < DIVIDE_FREQ_BY; inner++)
    {
      ADCSRA |= (1<<ADSC);
      while(!(ADCSRA & 0x10));
      adc_l = adc_l + ADCL;
      adc_h = adc_h + ADCH;
    }
    adc_l = adc_l / DIVIDE_FREQ_BY;
    adc_h = adc_h / DIVIDE_FREQ_BY;

    // while(!(ADCSRA & 0x10)); // wait for adc to be ready
    byte m = max(0,min(255,adc_l)); // fetch adc data
    byte j = max(0,min(255,adc_h));

    //byte m = ADCL; // fetch adc data
    //byte j = ADCH;
    int k = (j << 8) | m; // form into an int
    k -= 0x0200; // form into a signed int
    k <<= 6; // form into a 16b signed int
    fht_input[i] = k; // put real data into bins
  }
  TIMSK0 = 1; // turn on timer0

  fht_window(); // window the data for better frequency response
  fht_reorder(); // reorder the data before doing the fht
  fht_run(); // process the data in the fht
  fht_mag_log(); // take the output of the fht

  sumSounds();

  sei(); // enable interrupt again

  ADMUX  = 0x00; // use user defined ADC
  ADCSRA = 0x87; // release ADC -> Analog comparator

  #ifdef MY_DEBUG_SENS
    Serial.print(F(", s9="));
    Serial.print(sound_bee_fan9);
    Serial.print(F(", fan6="));
    Serial.print(sound_bee_fan6);
    Serial.print(F(", fan4="));
    Serial.print(sound_bee_fan4);
    Serial.print(F(", fly="));
    Serial.print(sound_bee_fly);
    Serial.print(F(", sound="));
    Serial.print(sound_total);
  #endif

  lora.TX.Data[6]= sound_total;
  lora.TX.Data[7]= sound_bee_fan4;
  lora.TX.Data[8]= sound_bee_fan6;
  lora.TX.Data[9]= sound_bee_fan9;
  lora.TX.Data[10]= sound_bee_fly;

  emptyLog(); // empy sound log

  saveValueAndReturnDiff('s', sound_total);
}


void get_sensor_weight()
{
  if (battery_dV < 27) // do not measure if voltage is below 2.6V
    return;

  //delay(500);

  weight_front.power_up();
  weight_front.set_scale(weight_calib);
  //weight_front.power_down();

  weight_back.power_up();
  weight_back.set_scale(weight_calib);
  //weight_back.power_down();

  byte weight_channel_a = 64;
  byte weight_channel_b = 32;

  uint16_t weight_max = 1000; // max value of weight
  uint16_t w_fl = 0;
  uint16_t w_fr = 0;
  uint16_t w_bl = 0;
  uint16_t w_br = 0;
  uint16_t w_e = 0;

  float w_fl_f = 0;
  float w_fr_f = 0;
  float w_bl_f = 0;
  float w_br_f = 0;

  float w_val = 0;

  bool first_time = false;

  if (w_fl_offset == 0 && w_fr_offset == 0 && w_bl_offset == 0 && w_br_offset == 0)
    first_time = true;

  //weight_front.power_up();

  delay(100);

  byte oversample = 3; // 1 time is fast, more becomes slow

  weight_front.set_gain(weight_channel_a);
  w_val = weight_front.get_units(oversample);
  if (w_fl_offset == 0)
    w_fl_offset = w_val;

  w_fl_f = abs(w_val - w_fl_offset);
  w_fl   = weightify(w_fl_f);

  weight_front.set_gain(weight_channel_b);
  w_val = weight_front.get_units(oversample) * 2;  // multiply by 2 because channel gain is 2x as low
  if (w_fr_offset == 0)
    w_fr_offset = w_val;

  w_fr_f = abs(w_val - w_fr_offset);
  w_fr   = weightify(w_fr_f);

  //weight_back.power_up();

  //delay(100);

  weight_back.set_gain(weight_channel_a);
  w_val = weight_back.get_units(oversample);
  if (w_bl_offset == 0)
    w_bl_offset = w_val;

  w_bl_f = abs(w_val - w_bl_offset);
  w_bl   = weightify(w_bl_f);

  weight_back.set_gain(weight_channel_b);
  w_val = weight_back.get_units(oversample) * 2;  // multiply by 2 because channel gain is 2x as low
  if (w_br_offset == 0)
    w_br_offset = w_val;

  w_br_f = abs(w_val - w_br_offset);
  w_br   = weightify(w_br_f);

  weight_front.power_down();
  weight_back.power_down();


  int count = 0;
  if (w_fl_f > 1)
    count++;

  if (w_fr_f > 1)
    count++;

  if (w_bl_f > 1)
    count++;

  if (w_br_f > 1)
    count++;

  w_val = 0;
  w_val += w_fl_f;
  w_val += w_fr_f;
  w_val += w_bl_f;
  w_val += w_br_f;
  w_val /= max(1, count);

  w_e = min(254, round(w_val));

  #ifdef MY_DEBUG_SENS
    if (first_time)
    {
      Serial.print(F(", W_FL_OFS="));
      Serial.print(w_fl_offset);
      Serial.print(F(", W_FR_OFS="));
      Serial.print(w_fr_offset);
      Serial.print(F(", W_BL_OFS="));
      Serial.print(w_bl_offset);
      Serial.print(F(", W_BR_OFS="));
      Serial.print(w_br_offset);
    }
    else
    {
      Serial.print(F(", W_FL="));
      Serial.print(w_fl_f);
      Serial.print(F(", W_FR="));
      Serial.print(w_fr_f);
      Serial.print(F(", W_BL="));
      Serial.print(w_bl_f);
      Serial.print(F(", W_BR="));
      Serial.print(w_br_f);
      Serial.print(F(", W_E="));
      Serial.print(w_val);
    }
  #endif

  saveValueAndReturnDiff('w', w_e);

  lora.TX.Data[2]  = w_e;
  lora.TX.Data[11] = highByte(w_fl);
  lora.TX.Data[12] = lowByte(w_fl);
  lora.TX.Data[13] = highByte(w_fr);
  lora.TX.Data[14] = lowByte(w_fr);
  lora.TX.Data[15] = highByte(w_bl);
  lora.TX.Data[16] = lowByte(w_bl);
  lora.TX.Data[17] = highByte(w_br);
  lora.TX.Data[18] = lowByte(w_br);

}

void get_sensor_simple_dht()
{
  if (battery_dV < 30) // do not measure if voltage is below 3.0V
    return;

  float t_e = 0;
  float h_e = 0;
  int err = SimpleDHTErrSuccess;
  if ((err = dht.read2(&t_e, &h_e, NULL)) != SimpleDHTErrSuccess) {
    #ifdef MY_DEBUG_SENS
      Serial.println(F("DHT read error"));
    #endif
    return;
  }
  
  #ifdef MY_DEBUG_SENS
    Serial.print(F(", T_E="));
    Serial.print(t_e);
    Serial.print(F(", H_E="));
    Serial.print(h_e);
  #endif

  uint8_t t = 0;
  uint8_t h = 0;

  if (!isnan(t_e))
    t = tempify(t_e);

  if (!isnan(h_e))
    h = round(h_e * 2);

  saveValueAndReturnDiff('t', t);
  saveValueAndReturnDiff('h', h);

  lora.TX.Data[0] = t;
  lora.TX.Data[1] = h;
}

void get_sensor_one_wire()
{
  if (battery_dV < 30) // do not measure if voltage is below 3.0V
    return;

  float t_i = 0;

  sensor.begin(); // One wire
  sensor.setResolution(11);
  sensor.requestTemperatures();

  bool       err = false;
  uint32_t milis = millis();

  while (!sensor.isConversionComplete())
  {
    if (millis() - milis > 1000)
    {
      err = true;
      break;
    }
  };

  t_i = sensor.getTempC();

  uint8_t t = 0;
  if (err || t_i < -10 || t_i > 41)
    t = 0;
  else
    t = tempify(t_i);

  #ifdef MY_DEBUG_SENS
    Serial.print(F(", T_I="));
    Serial.println(t_i);

    if (err)
      Serial.print(F(" (ERR)"));
  #endif

  saveValueAndReturnDiff('i', t);

  lora.TX.Data[3] = t;
}


void printDebug()
{
  #ifdef MY_DEBUG_SENS
    Serial.print(F(", ADMUX="));
    Serial.print(String(ADMUX,HEX));
    Serial.print(F(", DIDR0="));
    Serial.print(String(DIDR0,BIN));
    Serial.print(F(", DIDR1="));
    Serial.print(String(DIDR1,BIN));
    Serial.print(F(", ADCSRA="));
    Serial.println(String(ADCSRA,HEX));
  #endif
}

int getBandgap(void) // Returns actual value of Vcc (x 100)
{

  #if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    // For mega boards
    const long InternalReferenceVoltage = 1115L;  // Adjust this value to your boards specific internal BG voltage x1000
       // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc reference
       // MUX4 MUX3 MUX2 MUX1 MUX0  --> 11110 1.1V (VBG)         -Selects channel 30, bandgap voltage, to measure
    ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR)| (0<<MUX5) | (1<<MUX4) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  #else
    // For 168/328 boards
    const long InternalReferenceVoltage = 1056L;  // Adjust this value to your boards specific internal BG voltage x1000
       // REFS1 REFS0          --> 0 1, AVcc internal ref. -Selects AVcc external reference
       // MUX3 MUX2 MUX1 MUX0  --> 1110 1.1V (VBG)         -Selects channel 14, bandgap voltage, to measure
    ADMUX = (0<<REFS1) | (1<<REFS0) | (0<<ADLAR) | (1<<MUX3) | (1<<MUX2) | (1<<MUX1) | (0<<MUX0);

  #endif

  delay(50);  // Let mux settle a little to get a more stable A/D conversion
     // Start a conversion
  ADCSRA |= _BV( ADSC );
     // Wait for it to complete
  while( ( (ADCSRA & (1<<ADSC)) != 0 ) );
     // Scale the value
  int results = (((InternalReferenceVoltage * 1024L) / ADC) + 5L) / 10L; // calculates for straight line value
  return results;

}

void readBat()
{
  // Sample the reference voltage with the modified analog read function
  int vcc = getBandgap();

  #ifdef MY_DEBUG_SENS
    int voltage = (int)vcc/10;
    Serial.print(F("B_V="));
    Serial.print(voltage);
  #endif

  battery_dV = round(vcc/10);

  saveValueAndReturnDiff('b', battery_dV);

  lora.TX.Data[5] = battery_dV;
}

void read_sensors()
{
  #ifdef MY_DEBUG_SENS
    Serial.println(F("Sens pw on"));
  #endif

  large_diff = false;

  pinMode(SENSOR_POWER, OUTPUT);
  digitalWrite(SENSOR_POWER, HIGH);

  delay(50); // delay is added for Serial and for sensor stabilization after power up

  // Read sensor values and multiply by 100 to effectively keep 2 decimals, uint16_t: 0 up to 65535
  readBat(); //(5)
  sample_audio();
  get_sensor_weight(); // w_e (2)
  get_sensor_simple_dht(); // t_e, h_e (0,1)
  get_sensor_one_wire(); // t_i (3)
  lora.TX.Count = 19; // w_br => [17+18]

  #ifdef MY_DEBUG_SENS
    Serial.println(F("Sens pw off"));
  #endif
  digitalWrite(SENSOR_POWER, LOW);
}


void send_lora_msg()
{
  // Transmit the created message as a confirmed up message type and then receive the back-end reply.
  #ifdef MY_DEBUG_SENS
  Serial.println(F("LoRa send"));
  printStringAndHex("LoRa TX Data: ", &(lora.TX.Data[0]), lora.TX.Count);
  #endif

  RFM_Wakeup();

  lorawan.LORA_Send_Data(); // if using LORA_send_and_receive(); the power comsumptions stays at about 46mA after sending

  // Check whether a reply was received or not.

  if(lora.RX.retVal == RX_MESSAGE)
  {
    // Print the received data and handle the data
    printStringAndHex("Rx data:", lora.RX.Data, lora.RX.Count);
    handle_reply();
  }
  else
  {
    // Print the return value as to why no data is received.
    printStringAndHex("Retval: ", (uint8_t *)&(lora.RX.retVal), 1);
  }

  RFM_Sleep();
}



void setup() {

  Serial.begin(9600);

  #ifdef MY_DEBUG_SENS
    Serial.println(F("Starting"));
  #endif

  //Initialize the SPI port FOR LoRa communication
  SPI.begin();
  SPI.beginTransaction(SPISettings(4000000, MSBFIRST, SPI_MODE0));

  //Initialize I/O pins for the RFM95, DS2401 and MCP
  pinMode(RFM_DIO0, INPUT);
  pinMode(RFM_DIO1, INPUT);
  pinMode(RFM_DIO5, INPUT);
  pinMode(RFM_DIO2, INPUT);
  pinMode(RFM_NSS,  OUTPUT);

  #ifdef MY_DEBUG_SENS
    Serial.println(F("Weight offset"));
  #endif
  pinMode(SENSOR_POWER, OUTPUT);
  digitalWrite(SENSOR_POWER, HIGH);
  delay(500);
  get_sensor_weight();
  digitalWrite(SENSOR_POWER, LOW);

  // Power on delay for the RFM module
  delay(2000);

  lorawan.init();
  read_sensors();

  #ifdef MY_DEBUG_SENS
    Serial.println(F("End setup"));
  #endif
}


void loop()
{
  printStringAndHex("LoRa DEV EUI: ", &(lora.OTAA.DevEUI[0]), 8);

  lorawan.OTAA_connect();
  RFM_Sleep();

  // Super loop
  while(1)
  {

    // Measure
    read_sensors();

    if (battery_dV > 18) // do not measure if voltage is below 1.8V
    {
      if (large_diff || (times_not_sent * sensor_sample_sec) >= max_send_timeout )
      {
        send_lora_msg();
        times_not_sent = 0;
        large_diff     = false;
      }
      else
      {
        times_not_sent++;
      }
    }
    // Go to POWER_DOWN_MODE to reduce power consumption when the Arduino has nothing to do.
    sleep(sensor_sample_sec);

  }//While(1)
}


void sleep(uint16_t seconds)
{
  #ifdef MY_DEBUG_SENS
  Serial.print(F("zzz for "));
  Serial.print(seconds);
  Serial.println(F(" sec..."));
  Serial.flush();
  #endif

  RFM_Sleep();
  turn_off();
  for (int i=0; i<int(seconds/8); i++)
  {
    // Use library from https://github.com/rocketscream/Low-Power
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);
  }
  turn_on();
  RFM_Sleep();
}

void RFM_Sleep()
{
  // Set RFM to sleep mode
  SPI_Write(RFM_NSS, 0x01, 0x00);
  //Wait until RFM is in sleep mode
  delay(10);
}

void RFM_Wakeup()
{
  //Set RFM in LoRa mode
  SPI_Write(RFM_NSS, 0x01, 0x80);
  //Wait until RFM is powered up
  delay(250);
}


/*
  @brief
    Function which disables all IO's that can leak current through
*/
void turn_off (void)
{
  digitalWrite(SENSOR_POWER, LOW);

  // power off IO's
  digitalWrite(DHTPIN, LOW);
  digitalWrite(DAT_BACK, LOW);
  digitalWrite(CLK_BACK, LOW);
  digitalWrite(DAT_FRONT, LOW);
  digitalWrite(CLK_FRONT, LOW);
  digitalWrite(ONEW, LOW);

  pinMode(DHTPIN, OUTPUT);
  pinMode(DAT_BACK, OUTPUT);
  pinMode(CLK_BACK, OUTPUT);
  pinMode(DAT_FRONT, OUTPUT);
  pinMode(CLK_FRONT, OUTPUT);
  pinMode(ONEW, OUTPUT);

}

void turn_on (void)
{
  pinMode(DAT_BACK, INPUT);
  pinMode(DAT_FRONT, INPUT);
  pinMode(DHTPIN, INPUT);
}

/*
  @brief
    Function to print a string of text and an array of data in hexadecimal format.
  @parameters
    String  String of characters, must be zero terminated.
    data  Pointer to the array that must be printed in hexadecimal format
    n   Number of bytes that must be printed out in hexadecimal
*/
void printStringAndHex(const char *String, uint8_t *data, uint8_t n)
{
  #ifdef MY_DEBUG_SENS
    uint8_t i;

    Serial.print(String);
    Serial.flush();
    Serial.print(n, DEC);
    Serial.print(" bytes; ");

    // Print the data as a hexadecimal string
    for( i = 0 ; i < n ; i++)
    {
      // Print single nibbles, since the Hexadecimal format printed by the Serial.Print function does not print leading zeroes.
      Serial.print((unsigned char) ((data[i] & 0xF0) >> 4), HEX); // Print MSB first
      Serial.print((unsigned char) ((data[i] & 0x0F) >> 0), HEX); // Print LSB second
      Serial.print(' ');
      Serial.flush();
    }
    Serial.println();
  #endif
}


/*
  @brief
    Function to handle incoming data from an confirmed message up. If there is a single received byte and it's value is not zero, the
    value will be written to the LoRaWAN message interval and the RTC will be set to send a LoRaWAN message in the given number of minutes.
*/
void handle_reply (void)
{
  // Check if there is a single byte in the reply message from the back-end, if so retrieve it and copy it's contents to the LoRaWAN message interval
  if(lora.RX.Count == 1)
  {
    // Copy the received byte and reconfigure the LoRaWAN message interval when the received byte isn't zero.
    if(lora.RX.Data[0] != 0)
    {
      app.LoRaWAN_message_interval = lora.RX.Data[0];
      Serial.print(F("LoRaWAN_message_interval: "));
      Serial.println(app.LoRaWAN_message_interval);
      // Reconfigure the RTC for an interrupt on the next interval.
      // mcp7940_reset_minute_alarm(app.LoRaWAN_message_interval);
    }
  }
}
