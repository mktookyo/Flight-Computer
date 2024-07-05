/* Pinout:
Arduino UNO/Nano
NRF24L01: CE-7 CSN-8 SCK-13 MOSI-11 MISO-12 VCC-3.3V
Buzzer-Digital pin(+) GND(-)
BMP280: SCL-A5 SDA-A4 VCC-5V
NeoGPS 6m: Rx-Rx Tx-Tx Vcc-5/3.3V
*/
/* ESP32
BMP280: SCL-D22 SDA-D21 VCC-3.3V/5V
NeoGPS 6m: TX-RXD2 RX-TXD2 vcc-3.3/5v
*/

// libraries
#include <Wire.h>
#include <TinyGPS++.h> //GPS neo-6m
#include <GyverBME280.h>    //BMP280 
// #include <math.h> //maybe in future ;-;
// #include <preferences.h>

// Variables
struct weatherstruct {
  float humidity, temp, pressure;
};
// defining pin numbers
#define RXD2 16
#define TXD2 17
#define buzzer 18

// Objects
GyverBME280 bme;    //Crating object bme for BMP280  
weatherstruct weather; //object for weather variables (ex: weather.humidity=....) 
HardwareSerial neogps(1); //creating serial port for gps
TinyGPSPlus gps; //object for gps module 

void UART_raw_read (){
  byte data = neogps.read();
  Serial.write(data);
}

boolean gpsChecker(){ //функция возвращает true если до модуля gps можно дозвониться
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;)
  {
    while (neogps.available())
    {
      if (gps.encode(neogps.read()))
      {
        newData = true;
      }
    }
  }
  return newData;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(buzzer, OUTPUT);
  Serial.begin(115200); //Activating serial port (115200 baud) for ESP32
  if (!bme.begin(0x76)) {  // Check if BMP280 connected
    Serial.println("Could not find a valid BMP280 sensor");
    // while (1);
  }
  else{
    Serial.println("bmp280 has started");
  }
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  if (gpsChecker()!=true){
    Serial.println("Couldn't find GPS module");
    // while(1);
  }
  else{
    Serial.println("GPS has started!");
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  tone(buzzer, 1000);
  delay(300);
  noTone(buzzer);
  delay(300);
  tone(buzzer, 500);
  delay(300);
  noTone(buzzer);
  delay(300);
  tone(buzzer, 300);
  delay(300);
  noTone(buzzer);
  delay(300);
}
