# Solar-Monitoring-System
### ESP-12 based Solar Panel Monitoring System

This system helps you to remotely monitor the power of  your solar panels, batteries and the DC load with a smartphone from anywhere.<br>
It’s based on the Nodemcu ESP-12 WiFi module and Blynk application. The battery and solar voltage with the temperature will be displayed. <br>
It’s also possible by using buttons on installed app to decide witch batteries should be charged.<br>

The charger is positive ground controller. (And this provide me a lot problems with negative grounded ESP.)<br>
To measure the currents I’m using two ACS712 modules (hall effect device).<br>
The ACS712 has to be fed with 5V and the analog outputs voltage is proportional to measured current on the sensing terminals.<br>
This sensor is bi-directional so it will give 2.5V output even if not current is flowing through it. 
The module gives a sensitivity of 0.1V/A in 20A and available in 5A, 20A and 30A.<br>

I’m using ADS1115 in two channel and in GAIN_ONE mode.<br>
The ADS1115 is a 16-bit analog-to-digital converter. It can be addressed with one of four I2C addresses. (0x48, 0x49, 0x4a, 0x4b).<br>
In GAIN_ONE mode the voltage can be measured up to maximum 4V and provides a resolution of 0.125mV per bit.  (4.096 volts / 32767 bits)<br>

![image](https://user-images.githubusercontent.com/25223934/142388200-35c7ebca-dd0c-4da4-ada8-330876fd0463.png)

### Visualizing the data on a Blynk Android App
The system is accessible through a Blynk app.<br>
The switching the batteries will sent from app by using virtual pins and the data can be easily access on app.<br>

![image](https://user-images.githubusercontent.com/25223934/142388135-af128c20-dd6f-4c97-84f8-925f4f0b792e.png)         ![image](https://user-images.githubusercontent.com/25223934/142388652-3759402f-e5e2-4be5-9f18-613273506198.png)


For more infos: [solar-monitoring-system](https://www.forgani.com/electronics-projects/solar-monitoring-system/)
