EESchema Schematic File Version 2
LIBS:power
LIBS:device
LIBS:transistors
LIBS:conn
LIBS:linear
LIBS:regul
LIBS:74xx
LIBS:cmos4000
LIBS:adc-dac
LIBS:memory
LIBS:xilinx
LIBS:microcontrollers
LIBS:dsp
LIBS:microchip
LIBS:analog_switches
LIBS:motorola
LIBS:texas
LIBS:intel
LIBS:audio
LIBS:interface
LIBS:digital-audio
LIBS:philips
LIBS:display
LIBS:cypress
LIBS:siliconi
LIBS:opto
LIBS:atmel
LIBS:contrib
LIBS:valves
LIBS:antenna-test-1-cache
EELAYER 25 0
EELAYER END
$Descr A4 11693 8268
encoding utf-8
Sheet 1 1
Title ""
Date ""
Rev ""
Comp ""
Comment1 ""
Comment2 ""
Comment3 ""
Comment4 ""
$EndDescr
$Comp
L L L1
U 1 1 599B2C47
P 5350 2450
F 0 "L1" V 5300 2450 50  0000 C CNN
F 1 "L" V 5425 2450 50  0000 C CNN
F 2 "NFC-antenna-test:nfc-test2" H 5350 2450 50  0001 C CNN
F 3 "" H 5350 2450 50  0001 C CNN
	1    5350 2450
	1    0    0    -1  
$EndComp
$Comp
L C C1
U 1 1 599B2CAB
P 5350 2050
F 0 "C1" H 5375 2150 50  0000 L CNN
F 1 "C" H 5375 1950 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 5388 1900 50  0001 C CNN
F 3 "" H 5350 2050 50  0001 C CNN
	1    5350 2050
	1    0    0    -1  
$EndComp
$Comp
L C C2
U 1 1 599B2CD4
P 5350 2850
F 0 "C2" H 5375 2950 50  0000 L CNN
F 1 "C" H 5375 2750 50  0000 L CNN
F 2 "Capacitors_SMD:C_0805" H 5388 2700 50  0001 C CNN
F 3 "" H 5350 2850 50  0001 C CNN
	1    5350 2850
	1    0    0    -1  
$EndComp
$Comp
L GND #PWR01
U 1 1 599B2D19
P 5350 1800
F 0 "#PWR01" H 5350 1550 50  0001 C CNN
F 1 "GND" H 5350 1650 50  0000 C CNN
F 2 "" H 5350 1800 50  0001 C CNN
F 3 "" H 5350 1800 50  0001 C CNN
	1    5350 1800
	-1   0    0    1   
$EndComp
$Comp
L GND #PWR02
U 1 1 599B2D3F
P 5350 3100
F 0 "#PWR02" H 5350 2850 50  0001 C CNN
F 1 "GND" H 5350 2950 50  0000 C CNN
F 2 "" H 5350 3100 50  0001 C CNN
F 3 "" H 5350 3100 50  0001 C CNN
	1    5350 3100
	1    0    0    -1  
$EndComp
$Comp
L CONN_01X03 J1
U 1 1 599B2DB3
P 4600 2450
F 0 "J1" H 4600 2650 50  0000 C CNN
F 1 "ANT" V 4700 2450 50  0000 C CNN
F 2 "Pin_Headers:Pin_Header_Straight_1x03_Pitch1.27mm" H 4600 2450 50  0001 C CNN
F 3 "" H 4600 2450 50  0001 C CNN
	1    4600 2450
	-1   0    0    1   
$EndComp
Wire Wire Line
	5350 1800 5350 1900
Wire Wire Line
	5350 2200 5350 2300
Wire Wire Line
	5350 2600 5350 2700
Wire Wire Line
	5350 3000 5350 3100
Wire Wire Line
	5350 2650 5050 2650
Wire Wire Line
	5050 2650 5050 2550
Wire Wire Line
	5050 2550 4800 2550
Connection ~ 5350 2650
Wire Wire Line
	4800 2350 5050 2350
Wire Wire Line
	5050 2350 5050 2250
Wire Wire Line
	5050 2250 5350 2250
Connection ~ 5350 2250
$Comp
L GND #PWR03
U 1 1 599B2E3A
P 4900 2450
F 0 "#PWR03" H 4900 2200 50  0001 C CNN
F 1 "GND" H 4900 2300 50  0000 C CNN
F 2 "" H 4900 2450 50  0001 C CNN
F 3 "" H 4900 2450 50  0001 C CNN
	1    4900 2450
	0    -1   -1   0   
$EndComp
Wire Wire Line
	4800 2450 4900 2450
$EndSCHEMATC
