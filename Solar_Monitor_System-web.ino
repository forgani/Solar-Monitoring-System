/*******************************************************************************
 ESP-12 based Solar Panel Monitoring System
 This system helps us to remotely monitor the power of the solar panels, batteries and the DC load  from anywhere.
 Forgani, Forghanain
 https://www.forgani.com/electronics-projects/solar-monitoring-system/
 
 Last update 20 Jan. 2020
 ******************************************************************************/

#define BLYNK_PRINT Serial
#include <Wire.h>
#include <OneWire.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <Adafruit_ADS1015.h>
#include <Adafruit_Sensor.h>
#include <DallasTemperature.h>
#include <math.h>

Adafruit_ADS1115 ads48(0x48);    // address pin is pulled to GND
Adafruit_ADS1115 ads49(0x49);    // address pin is pulled to VCC -- reserve

#define vPIN_VOLTAGE_BTY0    V0    
#define vPIN_VOLTAGE_BTY4    V4   // negative terminal of solar panel
#define vPIN_VOLTAGE_BTY5    V5   // positive terminal of battery 1

#define vPIN_VOLTAGE_BTY10   V10  // reserve
#define vPIN_VOLTAGE_BTY11   V11  // reserve
#define vPIN_VOLTAGE_BTY12   V12  // positive terminal of battery 2
#define vPIN_VOLTAGE_BTY13   V13  // reserve

#define vPIN_CURRENT_LOAD    V6
#define vPIN_CURRENT_SOLAR   V7

#define vPIN_DHT_TEMP        V8
#define vPIN_DHT_HUM         V9
#define vPIN_DS_TEMP         V15

#define vPIN_RELAY_1         V20  // battery 1/2 relay switch
#define vPIN_RELAY_2         V21  // reserve
#define vPIN_RELAY_PUSH      V22  // disconnect the solar panel from the regulator to measure open circuit voltage

#define  VPIN_RESET          V30

// GPIO02 GPIO04 GPIO05 GPIO12 GPIO13 GPIO14
// SDA -> D1 on NodeMCU GPIO05 and SCL -> D2 on NodeMCU GPIO04 
#define dhtPin D4       // D4 on NodeMCU  GPIO02   
#define relayPin1 D5    // D5 on NodeMCU  GPIO14
#define relayPin2 D6    // D6 on NodeMCU  GPIO12
#define relayPinPush D7 // D7 on NodeMCU  GPIO13
#define ONE_WIRE_BUS D3 // GPIO00 . do not use D8 & D0 GPIO15/16

#define DHTTYPE DHT11  
DHT dht(dhtPin, DHTTYPE);
BlynkTimer timer;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds(&oneWire);

char auth[] = "x";
char ssid[] = "x"; 
char pass[] = "x"; 
IPAddress BlynkServerIP(127,0,0,1);  // blynk IP address
int port = xxx;

float multiplier = 0.000125F; /* ADS1115  @ +/- 4.096V gain_1 (16-bit results) */

/*---------- SYNC ALL SETTINGS ON BOOT UP ----------*/
bool isFirstConnect = true;

BLYNK_CONNECTED() {
  if (isFirstConnect) {
    Blynk.syncAll();
    isFirstConnect = false;
  }
}

BLYNK_WRITE(VPIN_RESET) { // Reset
  if (param.asInt()==1) {
    delay(100);
    ESP.restart(); //ESP.reset();
  } 
}

BLYNK_WRITE(vPIN_RELAY_1) {  // D5  GPIO14
  int varP1 = digitalRead(relayPin1);
  int varP2 = digitalRead(relayPin2);
  int rlyState1 = param.asInt();
  if (rlyState1 == 1 && varP2 == HIGH) {
    Blynk.virtualWrite(vPIN_RELAY_2, 0);
    Blynk.virtualWrite(vPIN_RELAY_1, 1);
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin1, HIGH);
    Serial.println("The relay1 is ON.");
    Serial.println("The relay2 is OFF!");
  } else if (rlyState1 == 0 && varP2 == LOW){
     digitalWrite(relayPin2, LOW);
     digitalWrite(relayPin1, HIGH);
     Blynk.virtualWrite(vPIN_RELAY_1, 1);
     Serial.println("The relay1 is ON default!");
  }
}
 
BLYNK_WRITE(vPIN_RELAY_2) {  // D6  GPIO12
  int rlyState2 = param.asInt();
  if (rlyState2 == 1)  {
    Blynk.virtualWrite(vPIN_RELAY_1, 0);
    digitalWrite(relayPin1, LOW);
    digitalWrite(relayPin2, HIGH);
    Serial.println("The relay1 is OFF!");
    Serial.println("The relay2 is ON.");
  } else {
    digitalWrite(relayPin2, LOW);
    digitalWrite(relayPin1, HIGH);
    Blynk.virtualWrite(vPIN_RELAY_1, 1);
    Serial.println("The relay1 is ON.");
    Serial.println("The relay2 is OFF!");
  }
}

BLYNK_WRITE(vPIN_RELAY_PUSH) {
  if (param.asInt()==1)  {
    digitalWrite(relayPinPush, HIGH);
    Serial.println("The relayPush is On!");
  } else {
    digitalWrite(relayPinPush, LOW);
    Serial.println("The relayPinPush is Off!");
  }
}

void setup(void) {
  Serial.begin(115200);
  delay(10);
  Serial.print("Connecting to ");  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("WiFi connected."); Serial.print("IP address:"); Serial.println(WiFi.localIP());

  timer.setInterval(8000L, CheckConnection); // check if still connected every 11s
  Blynk.config(auth, BlynkServerIP, port);
  Blynk.connect();
  Serial.println("Connected to Blynk server.");

  delay(10);
  // SCL -> D2=GPIO04 SDA -> D1=GPIO05
  Wire.begin(4,5); //SDA, SCL
  
  pinMode(relayPin1, OUTPUT);
  pinMode(relayPin2, OUTPUT);
  digitalWrite(relayPin1, HIGH);
  digitalWrite(relayPin2, LOW);
  pinMode(relayPinPush, OUTPUT);
  digitalWrite(relayPinPush, LOW);

  ads48.setGain(GAIN_ONE);
  ads48.begin();
  timer.setInterval(5000L, uploadAds48);  // ads48
  
  ads49.setGain(GAIN_ONE);
  ads49.begin();
  timer.setInterval(5000L, uploadAds49);  // ads48

  Serial.println("Initialize DHT11.");
  dht.begin();
  timer.setInterval(10000L, sensorDHTRead); 
  
  Serial.println("Initialize DS18B20.");
  ds.begin();
  timer.setInterval(10000L, sensorDSRead); // 4.7k an VCC
}

void loop(void) {
  /*
  byte error, address;
  int nDevices;
  Serial.println("Scanning...");
  nDevices = 0;
  for(address = 1; address < 127; address++ ) {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      Serial.print("I2C device found at address 0x");
      if (address < 16)
        Serial.print("0");
      Serial.print(address, HEX); Serial.println("  !");
      nDevices++;
    } else if (error==4) {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
  */
  if (Blynk.connected()) 
    Blynk.run();
  timer.run();
  delay(500);
}

void CheckConnection(){ // check every 11s if connected to Blynk server
  if(!Blynk.connected()){
    Serial.println("Not connected to Blynk server");
    Blynk.connect(); // try to connect to server with default timeout
  } else {
    Serial.println("Connected to Blynk server");
  }
} 

void uploadAds48() {
  // 30A current sensor ACS712 has sensitivity of 66mV/A
  // 10k/(4.7k+10k)  1/0.68=1.47   max.(3.3Vx1.47) 
  // no current by 2.5V/1.47=1.74V => 13920 bit
  int16_t adcCurrVoltage = ads48.readADC_SingleEnded(3);
  float current = ((((adcCurrVoltage - 13980) * multiplier) * 1.47) / 0.066); // Ampere
  current = current + 0.30;
  if (current < 0.01) {current = 0.00;}
  Serial.print("solar current(pin1):"); Serial.println(current);
  Blynk.virtualWrite(vPIN_CURRENT_SOLAR, current);

  adcCurrVoltage = ads48.readADC_SingleEnded(2);
  current = ((((adcCurrVoltage - 13980) * multiplier) * 1.47) / 0.066); // Ampere
  if (current < 0.01) {current = 0.00;}
  Serial.print("load current(pin2):"); Serial.println(current);
  Blynk.virtualWrite(vPIN_CURRENT_LOAD, current);
  
  // 10k/(10k+120k)  1/0.0769=13  max. 42.9V (3.3Vx13)  (connected to the battary posivive pol)
  int16_t adc = ads48.readADC_SingleEnded(0);  
  float volt4 = (adc * multiplier) * 13;
  Serial.print("battery0 voltage(pin3):"); Serial.print(volt4); Serial.println(" V");
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY4, volt4);
  
  // 20k/(20k+68k)   1/0.2272=4.4  max. 14.5v (3.3Vx4.4) (connected to the Solar panel negative pol)
  adc = ads48.readADC_SingleEnded(1);
  float volt5 = (adc * multiplier) * 4.4;
  Serial.print("battery1 voltage(pin4):"); Serial.print(volt5); Serial.println(" V");
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY5, volt5);
  
  float volt0 = volt5 - volt4;
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY0, volt0);
}

void uploadAds49() {
  // 10k/(10k+220k)  1/0.0769=13  max. 42.9V (3.3Vx13)
  int16_t adc = ads49.readADC_SingleEnded(0); 
  float volt20 = (adc * multiplier) * 23;
  Serial.print("solar voltage(pin20):"); Serial.print(volt20); Serial.println(" V");
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY10, volt20);
  
  adc = ads49.readADC_SingleEnded(1);
  float volt21 = adc * multiplier * 4.4;
  Serial.print("solar voltage(pin21):"); Serial.print(volt21); Serial.println(" V");
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY11, volt21);
  
  // 10k/(10k+68k)  1/0.0769=13  max. 42.9V (3.3Vx13)
  adc = ads49.readADC_SingleEnded(2);
  float volt22 = adc * multiplier * 7.8;
  Serial.print("solar voltage(pin22):"); Serial.print(volt22); Serial.println(" V");
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY12, volt22);
  
  adc = ads49.readADC_SingleEnded(3);
  Blynk.virtualWrite(vPIN_VOLTAGE_BTY13, adc * multiplier);
  Serial.print("solar voltage(pin23):"); Serial.print(adc * multiplier); Serial.println(" V");
}

void sensorDHTRead() {
  float t, h;
  t = dht.readTemperature();
  h = dht.readHumidity();
  Serial.print("DHT Temperature:"); Serial.print(t,1); Serial.println(" ºC");
  Serial.print("DHT Humidity:"); Serial.print(int(h)); Serial.println(" %");
  Blynk.virtualWrite(vPIN_DHT_TEMP, t);
  Blynk.virtualWrite(vPIN_DHT_HUM, h);
}

void sensorDSRead() {
  ds.requestTemperatures(); 
  double dsTempe = ds.getTempCByIndex(0);
  Serial.print("DS Temperature:"); Serial.print(dsTempe,1); Serial.println(" ºC");
  Blynk.virtualWrite(vPIN_DS_TEMP, dsTempe);
}
