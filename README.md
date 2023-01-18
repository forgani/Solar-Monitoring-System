# Solar-Monitoring-System
ESP-12 based Solar Panel Monitoring System

This system helps you to remotely monitor the power of  your solar panels, batteries and the DC load with a smartphone from anywhere.
It’s based on the Nodemcu ESP-12 WiFi module and Blynk application. The battery and solar voltage with the temperature will be displayed. It’s also possible by using buttons on installed app to decide witch batteries should be charged.

The solar panels output up to 35V with direct sun.
The solar panels charger is positive ground controller. (And this provide me a lot problems with negative grounded ESP.)
Two car batteries.

To measure the currents I’m using two ACS712 modules (hall effect device).
One sensor measures the current of battery charger and the other one the current of the load.
The ACS712 has to be fed with 5V and the analog outputs voltage is proportional to measured current on the sensing terminals.

The sensor is bi-directional so it will give 2.5V output even if not current is flowing through it.  (below my calculation to get current values).
The module gives a sensitivity of 0.1V/A in 20A and available in 5A, 20A and 30A.

The ADS1115 is a 16-bit analog-to-digital converter. It can be addressed with one of four I2C addresses. (0x48, 0x49, 0x4a, 0x4b).

Voltage Supply (Vdd): 2 ~ 5V
Measurement range: -300mV to Vdd+300mV
Data rate: 8 ~ 860 SPS
Active current: ~150uA (200uA max)

I’m using ADS1115 in two channel and in GAIN_ONE mode.
In GAIN_ONE mode the voltage can be measured up to maximum 4V and provides a resolution of 0.125mV per bit.  (4.096 volts / 32767 bits )

I calculated the ADC in gain_one program mode together with ACS as followed:


![image](https://user-images.githubusercontent.com/25223934/142387939-b8997d1a-4e8e-456d-99d6-f8d10b7dc193.png)

We can not connect up to a 3.3V voltage an ADC input!
So we using a voltage divider to bring the voltage between 0V and 3.3V.

R1 = 10k, R2 = 20k
R2/(R1 + R2) => 0.66 => 1/0.66=1.51
ADC(MaxVin) = 0.66 * 5V = 3.3V
The ACS sensor have zero current by 2.5V.  This corresponds to the voltage 1.65V at ADC and also 13200 bits.
So I can calculate the current as followed:
multiplier = 0.000125F; /* ADS1115 @ +/- 4.096V gain_1 (16-bit results) */
aMultiplier = multiplier * ( 1/0.66)
Now we read the input AIN2 digital value of 16 bits ADC

adc = ads.readADC_SingleEnded(2);   
current = (adc - 13200 ) * aMultiplier ) / (2.5/20)); // in Ampere

![image](https://user-images.githubusercontent.com/25223934/142388200-35c7ebca-dd0c-4da4-ada8-330876fd0463.png)

Visualizing the data on a Blynk Android App
The system is accessible through a Blynk app.
The switching the batteries will sent from app by using virtual pins and the data can be easily access on app.

![image](https://user-images.githubusercontent.com/25223934/142388135-af128c20-dd6f-4c97-84f8-925f4f0b792e.png)         ![image](https://user-images.githubusercontent.com/25223934/142388652-3759402f-e5e2-4be5-9f18-613273506198.png)


For more infos: https://www.forgani.com/electronics-projects/solar-monitoring-system/
