/* Pinout:
Arduino UNO/Nano
NRF24L01: CE-7 CSN-8 SCK-13 MOSI-11 MISO-12 VCC-3.3V
Buzzer-Digital pin(+) GND(-)
BMP280: SCL-A5 SDA-A4 VCC-5V
NeoGPS 6m: Rx-Rx Tx-Tx Vcc-5/3.3V
*/
/* ESP32
BMP280: SCL-D22 SDA-D21 VCC-3.3V/5V
*/

// Imports
#include <GyverBME280.h>    //BMP280 Library
#include <math.h>
// #include <preferences.h>
// Objects
GyverBME280 bme;    //Crating object bme for BMP280

// Variables
float humidity, temp, pressure;

// Setup
void setup() {
  j
  Serial.begin(115200); //Activating serial port (115200 baud) for ESP32
  if (!bme.begin(0x76)) {  // Check if BMP280 connected
    Serial.println("Could not find a valid BMP280 sensor, check wiring!");
    while (1);
  }
}
// just prints temperature, Humidity, Pressure.
void weatherData(){
  Serial.print(bme.readTemperature());
  Serial.println(" Celsius");
  Serial.print(bme.readHumidity());
  Serial.println("% Humidity");
  Serial.print(bme.readPressure());
  Serial.println(" Pa");
}
// vertical speed function
float verticalSpeed(){
  // version 1.0 (idk about accuracy but it seems to work) дальше в будущих версиях проверка точности показания скорости с помощью акселлерометра (наличие ускорения)
  float alt1, alt2, dalt, vertical_speed;
  alt1=rounding(pressureToAltitude(bme.readPressure()) , 2 ); //Reading altitude #1 
  delay(200); //Waiting 200ms 
  alt2=rounding(pressureToAltitude(bme.readPressure()) , 2 ); //Reading altitude #2
  dalt=alt2-alt1; //difference between altitudes 2 and 1
  vertical_speed=dalt/0.2; //calculating speed gained in 200ms
  return vertical_speed;
}

double rounding(double num, int decimal_places) { //функция округления значений с несколькими знаками после запятой. rounding(число, знаков после запятой)
  double multiplier = pow(10, decimal_places); 
  return round(num * multiplier) / multiplier; //возвращает округленное число 
}
void loop() {

}
