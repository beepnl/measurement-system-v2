EESchema Schematic File Version 4
EELAYER 26 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title "Beep PCB holder v1.4"
Date "2018-09-20"
Rev "4"
Comp "BEEP"
Comment1 "Dragino LoRa Mini v1.4"
Comment2 "Wisen Whisper board LoRa"
Comment3 "Ideetron Nexus low power"
Comment4 "Can contain 3 Arduino-LoRa boards"
$EndDescr
$Comp
L Device:R R1
U 1 1 5B588E29
P 9775 2775
F 0 "R1" V 9982 2775 50  0000 C CNN
F 1 "R" V 9891 2775 50  0000 C CNN
F 2 "Resistors_Universal:Resistor_SMD+THTuniversal_1206_RM10_HandSoldering" V 9705 2775 50  0001 C CNN
F 3 "" H 9775 2775 50  0001 C CNN
	1    9775 2775
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x02_MountingPin J1
U 1 1 5B58A53C
P 9250 975
F 0 "J1" H 9338 798 50  0000 L CNN
F 1 "Power" H 9338 889 50  0000 L CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x02_Pitch2.00mm" H 9250 975 50  0001 C CNN
F 3 "" H 9250 975 50  0001 C CNN
	1    9250 975 
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J6
U 1 1 5B58A5BA
P 7625 2000
F 0 "J6" V 7900 1850 50  0000 L CNN
F 1 "Weight B" V 7775 1925 50  0000 C CNN
F 2 "Connectors:Grove_1x04" H 7625 2000 50  0001 C CNN
F 3 "" H 7625 2000 50  0001 C CNN
	1    7625 2000
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J3
U 1 1 5B58ACFD
P 7625 1300
F 0 "J3" V 7925 1150 50  0000 L CNN
F 1 "Weight A/ALL" V 7775 975 50  0000 L CNN
F 2 "Connectors:Grove_1x04" H 7625 1300 50  0001 C CNN
F 3 "" H 7625 1300 50  0001 C CNN
	1    7625 1300
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J7
U 1 1 5B58AED1
P 7625 2725
F 0 "J7" V 7900 2675 50  0000 C CNN
F 1 "Weight C" V 7775 2650 50  0000 C CNN
F 2 "Connectors:Grove_1x04" H 7625 2725 50  0001 C CNN
F 3 "" H 7625 2725 50  0001 C CNN
	1    7625 2725
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J4
U 1 1 5B58AED8
P 7625 3425
F 0 "J4" V 7875 3300 50  0000 C CNN
F 1 "Weight D" V 7759 3295 50  0000 C CNN
F 2 "Connectors:Grove_1x04" H 7625 3425 50  0001 C CNN
F 3 "" H 7625 3425 50  0001 C CNN
	1    7625 3425
	1    0    0    -1  
$EndComp
$Comp
L Iconize:HX711 U3
U 1 1 5B58B61D
P 4825 1950
F 0 "U3" V 5400 2725 50  0000 R CNN
F 1 "HX711 A (Front)" V 5825 3000 50  0000 R CNN
F 2 "HX711:HX711" H 5675 2200 50  0001 C CNN
F 3 "" H 5675 2200 50  0001 C CNN
	1    4825 1950
	0    -1   -1   0   
$EndComp
$Comp
L Iconize:HX711 U2
U 1 1 5B58B999
P 6600 1950
F 0 "U2" V 7150 2625 50  0000 L CNN
F 1 "HX711 B (Back)" V 7600 2400 50  0000 L CNN
F 2 "HX711:HX711" H 7450 2200 50  0001 C CNN
F 3 "" H 7450 2200 50  0001 C CNN
	1    6600 1950
	0    -1   -1   0   
$EndComp
$Comp
L Connector:Conn_01x06_Male J5
U 1 1 5B5B7B53
P 4150 3750
F 0 "J5" V 4303 3362 50  0000 R CNN
F 1 "3V3 TTL FTDI" V 4275 4525 50  0000 R CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x06_Pitch2.54mm" H 4150 3750 50  0001 C CNN
F 3 "" H 4150 3750 50  0001 C CNN
	1    4150 3750
	0    -1   -1   0   
$EndComp
$Comp
L power:GND #PWR0101
U 1 1 5B5E983E
P 9875 1125
F 0 "#PWR0101" H 9875 875 50  0001 C CNN
F 1 "GND" H 9880 952 50  0000 C CNN
F 2 "" H 9875 1125 50  0001 C CNN
F 3 "" H 9875 1125 50  0001 C CNN
	1    9875 1125
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_2_Bridged JP1
U 1 1 5B66A10C
P 1825 2000
F 0 "JP1" H 1900 1850 50  0000 R CNN
F 1 "Sensor switchable power" H 2200 2125 50  0000 R CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Bridged_RoundedPad1.0x1.5mm" H 1825 2000 50  0001 C CNN
F 3 "" H 1825 2000 50  0001 C CNN
	1    1825 2000
	-1   0    0    1   
$EndComp
$Comp
L Jumper:SolderJumper_2_Open JP2
U 1 1 5B73E13E
P 1825 1475
F 0 "JP2" H 1875 1325 50  0000 R CNN
F 1 "Sensor continuous power" H 2150 1600 50  0000 R CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_RoundedPad1.0x1.5mm" H 1825 1475 50  0001 C CNN
F 3 "" H 1825 1475 50  0001 C CNN
	1    1825 1475
	-1   0    0    1   
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J8
U 1 1 5B7B75B7
P 8500 1700
F 0 "J8" V 8775 1600 50  0000 L CNN
F 1 "Grove 1=DHT22, 2=NC/D5 Dragino" V 8650 1425 50  0000 L CNN
F 2 "Connectors:Grove_1x04" H 8500 1700 50  0001 C CNN
F 3 "~" H 8500 1700 50  0001 C CNN
	1    8500 1700
	1    0    0    -1  
$EndComp
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J2
U 1 1 5B8AF04B
P 8500 3425
F 0 "J2" H 8587 3341 50  0000 L CNN
F 1 "Grove I2C, 1=SDA, 2=SCL" H 8587 3250 50  0000 L CNN
F 2 "Connectors:Grove_1x04" H 8500 3425 50  0001 C CNN
F 3 "~" H 8500 3425 50  0001 C CNN
	1    8500 3425
	1    0    0    -1  
$EndComp
$Comp
L Beep:Dragino-LoRa-Mini U5
U 1 1 5B725428
P 9650 5225
F 0 "U5" H 9550 5150 50  0000 C CNN
F 1 "Dragino-LoRa-Mini" H 9525 4950 50  0000 C CNN
F 2 "Nexus:Dragino LoRa Mini" H 9650 5225 50  0001 C CNN
F 3 "" H 9650 5225 50  0001 C CNN
	1    9650 5225
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_2_Open JP3
U 1 1 5B86C75D
P 5550 2250
F 0 "JP3" V 5504 2318 50  0000 L CNN
F 1 "A4 on DT" V 5595 2318 50  0000 L CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_RoundedPad1.0x1.5mm" H 5550 2250 50  0001 C CNN
F 3 "~" H 5550 2250 50  0001 C CNN
	1    5550 2250
	0    1    1    0   
$EndComp
$Comp
L Jumper:SolderJumper_2_Open JP4
U 1 1 5B86C7FC
P 6150 2250
F 0 "JP4" V 6104 2318 50  0000 L CNN
F 1 "A5 on SCK" V 6195 2318 50  0000 L CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Open_RoundedPad1.0x1.5mm" H 6150 2250 50  0001 C CNN
F 3 "~" H 6150 2250 50  0001 C CNN
	1    6150 2250
	0    1    1    0   
$EndComp
$Comp
L Jumper:SolderJumper_2_Bridged JP5
U 1 1 5BA3A858
P 1850 4025
F 0 "JP5" H 1875 4200 50  0000 C CNN
F 1 "VIN TTL = VCC" H 1850 3900 50  0000 C CNN
F 2 "Jumper:SolderJumper-2_P1.3mm_Bridged_RoundedPad1.0x1.5mm" H 1850 4025 50  0001 C CNN
F 3 "~" H 1850 4025 50  0001 C CNN
	1    1850 4025
	1    0    0    -1  
$EndComp
$Comp
L Jumper:SolderJumper_3_Bridged12 JP6
U 1 1 5B82EEBA
P 1825 2875
F 0 "JP6" H 1825 3080 50  0000 C CNN
F 1 "Nexus BAT to V+(solder)/3V3+ " H 1825 2989 50  0000 C CNN
F 2 "Jumper:SolderJumper-3_P1.3mm_Bridged12_RoundedPad1.0x1.5mm" H 1825 2875 50  0001 C CNN
F 3 "~" H 1825 2875 50  0001 C CNN
	1    1825 2875
	-1   0    0    1   
$EndComp
$Comp
L Jumper:SolderJumper_3_Bridged12 JP7
U 1 1 5B7920ED
P 1825 4575
F 0 "JP7" H 1575 4425 50  0000 L CNN
F 1 "Nexus Batt -> 3V3/PWR" H 1375 4700 50  0000 L CNN
F 2 "Jumper:SolderJumper-3_P1.3mm_Bridged2Bar12_RoundedPad1.0x1.5mm" H 1825 4575 50  0001 C CNN
F 3 "~" H 1825 4575 50  0001 C CNN
	1    1825 4575
	1    0    0    -1  
$EndComp
Text GLabel 3500 4400 0    50   Input ~ 0
GND
Text GLabel 10450 4825 2    50   Input ~ 0
GND
Text GLabel 6025 5225 0    50   Input ~ 0
DHT22
Text GLabel 8500 5425 0    50   Input ~ 0
GND
Text GLabel 6300 1250 2    50   Input ~ 0
GND
Text GLabel 8300 3325 0    50   Input ~ 0
SDA
Text GLabel 8300 3425 0    50   Input ~ 0
SCL
Text GLabel 8300 3525 0    50   Input ~ 0
SV+
Text GLabel 8300 3625 0    50   Input ~ 0
GND
$Comp
L Connector_Generic_MountingPin:Conn_01x04_MountingPin J9
U 1 1 5B658B47
P 8500 2575
F 0 "J9" V 8800 2450 50  0000 L CNN
F 1 "Grove 1=Mic, 2=DS18B20" V 8650 2025 50  0000 L CNN
F 2 "Connectors:Grove_1x04" H 8500 2575 50  0001 C CNN
F 3 "~" H 8500 2575 50  0001 C CNN
	1    8500 2575
	1    0    0    -1  
$EndComp
$Comp
L Iconize:Nexus U1
U 1 1 5B5E973F
P 6125 6075
F 0 "U1" H 6600 5325 50  0000 C CNN
F 1 "Nexus" H 6625 5200 50  0000 C CNN
F 2 "Nexus:Nexus" H 6575 5325 50  0001 C CNN
F 3 "https://webshop.ideetron.nl/Nexus" H 6575 5325 50  0001 C CNN
	1    6125 6075
	1    0    0    1   
$EndComp
Text GLabel 3950 3550 1    50   Input ~ 0
RST
Text GLabel 4050 3550 1    50   Input ~ 0
Tx
Text GLabel 9050 975  0    50   Input ~ 0
GND
Text GLabel 9050 1075 0    50   Input ~ 0
BAT+
Text GLabel 1825 2725 1    50   Input ~ 0
BAT+
Text GLabel 2025 2875 2    50   Input ~ 0
3V3+
Text GLabel 1625 2875 0    50   Input ~ 0
V+
Text GLabel 7175 5625 2    50   Input ~ 0
V+
Text GLabel 4150 3550 1    50   Input ~ 0
Rx
Text GLabel 4250 3550 1    50   Input ~ 0
3V3TTL
Text GLabel 4450 3550 1    50   Input ~ 0
GND
Text GLabel 7175 5725 2    50   Input ~ 0
GND
Text GLabel 4525 1250 2    50   Input ~ 0
GND
Text GLabel 4525 1550 2    50   Input ~ 0
SV+
Text GLabel 6300 1550 2    50   Input ~ 0
SV+
Text GLabel 6300 1350 2    50   Input ~ 0
DT2
Text GLabel 6300 1450 2    50   Input ~ 0
SCK2
Text GLabel 4525 1350 2    50   Input ~ 0
DT1
Text GLabel 4525 1450 2    50   Input ~ 0
SCK1
Text GLabel 5550 2100 1    50   Input ~ 0
SCK2
Text GLabel 6150 2100 1    50   Input ~ 0
DT2
Text GLabel 6150 2400 3    50   Input ~ 0
SCL
Text GLabel 5550 2400 3    50   Input ~ 0
SDA
Text GLabel 4900 5100 2    50   Input ~ 0
SDA
Text GLabel 4900 5000 2    50   Input ~ 0
SCL
Text GLabel 7175 4925 2    50   Input ~ 0
SDA
Text GLabel 7175 5025 2    50   Input ~ 0
SCL
Text GLabel 10450 5425 2    50   Input ~ 0
SDA
Text GLabel 10450 5325 2    50   Input ~ 0
SCL
Text GLabel 10450 5125 2    50   Input ~ 0
DS18b20
Text GLabel 8300 2575 0    50   Input ~ 0
DS18b20
Text GLabel 7175 5225 2    50   Input ~ 0
DS18b20
Text GLabel 10450 5625 2    50   Input ~ 0
MIC
Text GLabel 7175 4625 2    50   Input ~ 0
MIC
Text GLabel 4900 5400 2    50   Input ~ 0
MIC
Text GLabel 4900 5500 2    50   Input ~ 0
SVin+
Text GLabel 7175 4525 2    50   Input ~ 0
SVin+
Text GLabel 10450 5825 2    50   Input ~ 0
SVin+
Text GLabel 3500 5200 0    50   Input ~ 0
DS18b20
Text GLabel 7175 4725 2    50   Input ~ 0
DT1
Text GLabel 7175 4825 2    50   Input ~ 0
SCK1
Text GLabel 3500 6000 0    50   Input ~ 0
BAT+
Text GLabel 7175 5125 2    50   Input ~ 0
BAT+
Text GLabel 7175 5525 2    50   Input ~ 0
3V3+
Text GLabel 4900 4900 2    50   Input ~ 0
BAT+
Text GLabel 2000 4025 2    50   Input ~ 0
BAT+
Text GLabel 8500 5625 0    50   Input ~ 0
BAT+
Text GLabel 10450 5225 2    50   Input ~ 0
BAT+
Text GLabel 10450 5525 2    50   Input ~ 0
SCK1
Text GLabel 10450 5725 2    50   Input ~ 0
DT1
Text GLabel 8500 5525 0    50   Input ~ 0
D5
Text GLabel 8300 1700 0    50   Input ~ 0
D5
Text GLabel 8300 1600 0    50   Input ~ 0
DHT22
Text GLabel 1675 2000 0    50   Input ~ 0
SV+
Text GLabel 1975 2000 2    50   Input ~ 0
SVin+
$Comp
L Whisper_Node_AVR:WhisperAVR U4
U 1 1 5B58862E
P 2650 4950
F 0 "U4" H 4225 4950 50  0000 C CNN
F 1 "Whisper LoRa" H 4200 4775 50  0000 C CNN
F 2 "Whisper Lora:Whisper_Lora" H 4550 3700 50  0001 C CNN
F 3 "" H 4550 3700 50  0001 C CNN
	1    2650 4950
	1    0    0    -1  
$EndComp
Text GLabel 4900 5700 2    50   Input ~ 0
3V3+
Text GLabel 4900 5800 2    50   Input ~ 0
3V3+
Text GLabel 4900 5900 2    50   Input ~ 0
3V3+
Text GLabel 4900 6000 2    50   Input ~ 0
3V3+
Text GLabel 8300 1800 0    50   Input ~ 0
SV+
Text GLabel 8300 1900 0    50   Input ~ 0
GND
Text GLabel 8300 2675 0    50   Input ~ 0
SV+
Text GLabel 1675 1475 0    50   Input ~ 0
SV+
Text GLabel 1975 1475 2    50   Input ~ 0
3V3+
Text GLabel 8500 5225 0    50   Input ~ 0
DT2
Text GLabel 8500 5325 0    50   Input ~ 0
SCK2
Text GLabel 8500 4825 0    50   Input ~ 0
RST
Text GLabel 3900 6200 3    50   Input ~ 0
RST
Text GLabel 4000 6200 3    50   Input ~ 0
Rx
Text GLabel 4100 6200 3    50   Input ~ 0
Tx
Text GLabel 4200 6200 3    50   Input ~ 0
3V3TTL
Text GLabel 4400 6200 3    50   Input ~ 0
GND
Text GLabel 8500 4925 0    50   Input ~ 0
Tx
Text GLabel 8500 5025 0    50   Input ~ 0
Rx
Text GLabel 6850 6175 3    50   Input ~ 0
RST
Text GLabel 6750 6175 3    50   Input ~ 0
Rx
Text GLabel 6650 6175 3    50   Input ~ 0
Tx
Text GLabel 6550 6175 3    50   Input ~ 0
3V3TTL
Text GLabel 6350 6175 3    50   Input ~ 0
GND
Text GLabel 5450 1150 0    50   Input ~ 0
E+B
Text GLabel 7425 2825 0    50   Input ~ 0
E+B
Text GLabel 7425 3525 0    50   Input ~ 0
E+B
Text GLabel 5450 1250 0    50   Input ~ 0
E-B
Text GLabel 7425 3625 0    50   Input ~ 0
E-B
Text GLabel 7425 2925 0    50   Input ~ 0
E-B
Text GLabel 5450 1350 0    50   Input ~ 0
A-B
Text GLabel 5450 1450 0    50   Input ~ 0
A+B
Text GLabel 5450 1550 0    50   Input ~ 0
B-B
Text GLabel 5450 1650 0    50   Input ~ 0
B+B
Text GLabel 7425 3325 0    50   Input ~ 0
B+B
Text GLabel 7425 3425 0    50   Input ~ 0
B-B
Text GLabel 7425 2725 0    50   Input ~ 0
A-B
Text GLabel 7425 2625 0    50   Input ~ 0
A+B
Text GLabel 3675 1150 0    50   Input ~ 0
E+A
Text GLabel 3675 1250 0    50   Input ~ 0
E-A
Text GLabel 3675 1350 0    50   Input ~ 0
A-A
Text GLabel 3675 1450 0    50   Input ~ 0
A+A
Text GLabel 3675 1550 0    50   Input ~ 0
B-A
Text GLabel 3675 1650 0    50   Input ~ 0
B+A
Text GLabel 7425 1400 0    50   Input ~ 0
E+A
Text GLabel 7425 2100 0    50   Input ~ 0
E+A
Text GLabel 7425 2200 0    50   Input ~ 0
E-A
Text GLabel 7425 1500 0    50   Input ~ 0
E-A
Text GLabel 7425 1300 0    50   Input ~ 0
A-A
Text GLabel 7425 1200 0    50   Input ~ 0
A+A
Text GLabel 7425 2000 0    50   Input ~ 0
B-A
Text GLabel 7425 1900 0    50   Input ~ 0
B+A
Text GLabel 9775 2625 1    50   Input ~ 0
DS18b20
Text GLabel 9775 2925 3    50   Input ~ 0
SV+
Text GLabel 9875 1125 1    50   Input ~ 0
GND
Text GLabel 8300 2475 0    50   Input ~ 0
MIC
Text GLabel 8300 2775 0    50   Input ~ 0
GND
Text GLabel 4900 4400 2    50   Input ~ 0
GND
Text GLabel 4900 5600 2    50   Input ~ 0
GND
Text GLabel 3500 5800 0    50   Input ~ 0
GND
Text GLabel 3500 4700 0    50   Input ~ 0
DT1
Text GLabel 3500 4800 0    50   Input ~ 0
DHT22
Text GLabel 3500 4900 0    50   Input ~ 0
SCK1
Text GLabel 1700 4025 0    50   Input ~ 0
3V3TTL
Text GLabel 1825 4725 3    50   Input ~ 0
BAT+
Text GLabel 1625 4575 0    50   Input ~ 0
3V3+
Text GLabel 2025 4575 2    50   Input ~ 0
V+
$EndSCHEMATC
