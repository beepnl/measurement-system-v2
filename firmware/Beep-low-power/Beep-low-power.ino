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
 * struct.back_end 		    -> The_Things_Network, KPN (NL)
 * struct.Session, or OTAA (depending on the chosen activation_method)
 * 
 *******************************************************************************/

// Enable debug prints to serial monitor
#define MY_DEBUG_SENS // MY_DEBUG_NO (for production), MY_DEBUG_SENS, MY_DEBUG

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

#include "DHT.h"
#include "HX711.h"
#include <DS18B20.h> // much smaller than DallasTemperature

/******************************************************************************************
                  PIN DEFINITIONS
******************************************************************************************/

// Audio samples
// Use ADC in free running mode
#define MIC_SENSE_PIN A2
#define SENSOR_POWER A0
#define LOG_OUT 1 // use the logarithmic output function
#define FHT_N 128 // set to 128 point fht (in stead of 256)
#define DIVIDE_FREQ_BY 60 // max freq is 18kHz at 16MHz processor, so factor 60 is max 300Hz sampling, so each band is 5Hz (@FHT_N=128)
#define MEASURE_THRESHOLD 6 // 0-100 threshold for counting values, depends on microphone gain
#include <FHT.h>

// DHT
#define DHTPIN 5
#define DHTTYPE DHT22

// Weight sensors
#define DAT_FRONT A1
#define CLK_FRONT A3
#define DAT_BACK 3
#define CLK_BACK 4

// One wire temp DS18B20
#define ONEW A4

/******************************************************************************************
                  GLOBAL VARIABLES
******************************************************************************************/

uint16_t lora_send_sec = 296; // 296 = 5 min. Has to be a multiple of 8, max 65536 = 1092 sec = 18 min 

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


// HX711
int16_t weight_calib = 5000; // -5000 for red to E+ and black to E-, else 5000
HX711 weight_front(DAT_FRONT, CLK_FRONT);
HX711 weight_back(DAT_BACK, CLK_BACK);

// DHT 22
DHT dht(DHTPIN, DHTTYPE);

// One wire
OneWire oneWire(ONEW);
DS18B20 sensor(&oneWire);

/******************************************************************************************
                  FUNCTIONS
******************************************************************************************/
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
  ADCSRA = 0xe5; // set the adc to free running mode
  if (MIC_SENSE_PIN == A0)
  {
    ADMUX  = 0x40; // use adc1 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00000001; // turn off the digital input for adc2 of adc 8->0 (bit 3 should be 1)
  }
  if (MIC_SENSE_PIN == A1)
  {
    ADMUX  = 0x41; // use adc1 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00000010; // turn off the digital input for adc2 of adc 8->0 (bit 3 should be 1)
  }
  if (MIC_SENSE_PIN == A2)
  {
    ADMUX  = 0x42; // use adc1 (adc0 = 0x40, adc1=0x41, adc1=0x42)
    DIDR0  = B00000100; // turn off the digital input for adc2 of adc 8->0 (bit 3 should be 1)
  }

  delay(50);

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

    byte m = max(0,min(255,adc_l)); // fetch adc data
    byte j = max(0,min(255,adc_h));

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

  ADMUX  = 0x00; // use adc0 (adc0 = 0x40, adc1=0x41)
  ADCSRA = 0x87; // release ADC -> Analog comparator

  #ifdef MY_DEBUG_SENS
    Serial.print(F("sound_bee_fan9: "));
    Serial.println(sound_bee_fan9);
    Serial.print(F("sound_bee_fan6: "));
    Serial.println(sound_bee_fan6);
    Serial.print(F("sound_bee_fan4: "));
    Serial.println(sound_bee_fan4);
    Serial.print(F("sound_bee_fly: "));
    Serial.println(sound_bee_fly);
    Serial.print(F("sound_total: "));
    Serial.println(sound_total);
  #endif

}


void get_sensor_weight()
{

  weight_front.power_up();
  weight_front.set_scale(weight_calib);
  weight_front.power_down();

  weight_back.power_up();
  weight_back.set_scale(weight_calib);
  weight_back.power_down();

  byte weight_channel_a = 64;
  byte weight_channel_b = 32;

  uint16_t weight_max = 10000; // max value of weight
  uint16_t w_a = 0;
  uint16_t w_b = 0;
  uint16_t w_c = 0;
  uint16_t w_d = 0;
  uint16_t w_e = 0;

  float w_val = 0;

  weight_front.power_up();

  weight_front.set_gain(weight_channel_a);
  w_val = weight_front.get_units(1);
  if (abs(w_val) < weight_max)
    w_a = abs(round(w_val));

  weight_front.set_gain(weight_channel_b);
  w_val = weight_front.get_units(1);
  if (abs(w_val) < weight_max)
    w_b = abs(round(w_val));

  weight_front.power_down();
  weight_back.power_up();

  weight_back.set_gain(weight_channel_a);
  w_val = weight_back.get_units(1);
  if (abs(w_val) < weight_max)
    w_c = abs(round(w_val));

  weight_back.set_gain(weight_channel_b);
  w_val = weight_back.get_units(1);
  if (abs(w_val) < weight_max)
    w_d = abs(round(w_val));

  weight_back.power_down();

  w_val = 0;
  w_val += w_a;
  w_val += w_b;
  w_val += w_c;
  w_val += w_d;
  w_val /= 4;

  if (abs(w_val) < weight_max)
    w_e = round(w_val);

  #ifdef MY_DEBUG_SENS
    Serial.print(F("W_A: "));
    Serial.println(w_a);
    Serial.print(F("W_B: "));
    Serial.println(w_b);
    Serial.print(F("W_C: "));
    Serial.println(w_c);
    Serial.print(F("W_D: "));
    Serial.println(w_d);
    Serial.print(F("W_E: "));
    Serial.println(w_e);
  #endif

  lora.TX.Data[2] = w_e;

}



void get_sensor_dht()
{
  // NB: Does not work below 3V
  dht.begin();
  delay(500);

  uint16_t t_e;
  uint16_t h_e;

  t_e = dht.readTemperature();
  h_e = dht.readHumidity();

  #ifdef MY_DEBUG_SENS
    Serial.print(F("T_E: "));
    Serial.println(t_e);
    Serial.print(F("H_E: "));
    Serial.println(h_e);
  #endif

  lora.TX.Data[0] = round(t_e);
  lora.TX.Data[1] = round(h_e);
}

void get_sensor_one_wire()
{
  uint16_t t_i;

  sensor.begin(); // One wire
  delay(2000);
  sensor.setResolution(9);
  sensor.requestTemperatures();
  t_i = sensor.getTempC();

  #ifdef MY_DEBUG_SENS
    Serial.print(F("T_I: "));
    Serial.println(t_i);
  #endif

  lora.TX.Data[3] = round(t_i);
}


void printDebug()
{
  #ifdef MY_DEBUG
    Serial.print(F("ADMUX: "));
    Serial.println(String(ADMUX,HEX));
    Serial.print(F("DIDR0: "));
    Serial.println(String(DIDR0,HEX));
    Serial.print(F("DIDR1: "));
    Serial.println(String(DIDR1,HEX));
    Serial.print(F("ADCSRA: "));
    Serial.println(String(ADCSRA,HEX));
  #endif
}

uint16_t readBat()
{
  #ifdef MY_DEBUG_SENS
  Serial.println(F("readBat"));
  #endif
  /*
    A modification to the Arduino analogRead function is required!! See the warning statement above!
  */
  delay(50);

  // Sample the reference voltage with the modified analog read function
  int vcc = getBandgap();
  //double vcc = 1126.4 / (double) (analogRead(0x8E));

  #ifdef MY_DEBUG_SENS
    int voltage = (int)vcc/10;
    Serial.print(F("B_V (x10): "));
    Serial.println(voltage);
  #endif

  lora.TX.Data[5] = (int)vcc/10;
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


void read_sensors() {

  #ifdef MY_DEBUG_SENS
    Serial.println(F("Sens pw on"));
  #endif
  pinMode(SENSOR_POWER, OUTPUT);
  digitalWrite(SENSOR_POWER, HIGH);

  delay(50);   // XXX delay is added for Serial and for sensor stabilization after power up

  // Read sensor values and multiply by 100 to effectively keep 2 decimals, uint16_t: 0 up to 65535
  get_sensor_dht(); // t_e, h_e (0,1)
  get_sensor_weight(); // w_e (2)
  get_sensor_one_wire(); // t_i (3)
  lora.TX.Data[4] = 0; // a_i (4)
  readBat(); //(5)
  sample_audio();

  #ifdef MY_DEBUG_SENS
    Serial.println(F("Sens pw off"));
  #endif
  digitalWrite(SENSOR_POWER, LOW);

  lora.TX.Data[6]= sound_total;
  lora.TX.Data[7]= sound_bee_fan4;
  lora.TX.Data[8]= sound_bee_fan6;
  lora.TX.Data[9]= sound_bee_fan9;
  lora.TX.Data[10]= sound_bee_fly;

  lora.TX.Count = 11;

  printStringAndHex("LoRa TX Data: ", &(lora.TX.Data[0]), lora.TX.Count);

  emptyLog(); // empy sound log
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

  // Power on delay for the RFM module
  delay(3000);

  lorawan.init();

  #ifdef MY_DEBUG_SENS
  Serial.println(F("End setup"));
  #endif
}


void loop()
{
  printStringAndHex("LoRa DEV EUI: ", &(lora.OTAA.DevEUI[0]), 8);

  // Connect to the back-end with OTAA when OTAA is selected as the activation method.
  lorawan.OTAA_connect();

  // Set the alarm to off
  LORA_SEND = true;

  // Super loop
  while(1)
  {

    // Catch the minute alarm from the RTC.
    if(LORA_SEND == true)
    {
      // Measure
      #ifdef MY_DEBUG_SENS
      Serial.println();
      Serial.println(F("Read sensors"));
      #endif
      read_sensors();

      // Transmit the created message as a confirmed up message type and then receive the back-end reply.
      #ifdef MY_DEBUG_SENS
      Serial.println(F("LoRa send"));
      #endif

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
      // Clear the boolean.
      LORA_SEND = false;
    }
    else
    {
      // Go to POWER_DOWN_MODE to reduce power consumption when the Arduino has nothing to do.
      sleep(lora_send_sec);
    }
  }//While(1)
}


void sleep(int seconds)
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
  RFM_Wakeup();

  // wake up again
  LORA_SEND = true;
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

  delay(100);
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
      lora_send_sec = (int)lora.RX.Data[0];
      #ifdef MY_DEBUG_SENS
      Serial.print(F("LoRaWAN_message_interval: "));
      Serial.println(lora_send_sec);
      #endif
    }
  }
}
