/* Pinout:
Arduino UNO/Nano
NRF24L01: CE-7 CSN-8 SCK-13 MOSI-11 MISO-12 VCC-3.3V
Buzzer-Digital pin(+) GND(-)
BMP280: SCL-A5 SDA-A4 VCC-5V
*/










// Imports
#include <GyverBME280.h>    //BMP280 Library
#include <math.h>
// #include <preferences.h>
// Objects
GyverBME280 bme;    //Crating object bme for BMP280

// Variables
float humidity, temp, pressure;
void setup() {
    Serial.begin(9600);
    FILTER_COEF_4;
    bme.begin(); 
}
void weatherData(){
  Serial.print(bme.readTemperature());
  Serial.println(" Celsius");
  Serial.print(bme.readHumidity());
  Serial.println("% Humidity");
  Serial.print(bme.readPressure());
  Serial.println(" Pa");
}
void variometer(){
  float alt1, alt2, dalt, acceleration;
  alt1=round(pressureToAltitude(bme.readPressure()));
  // double 
  // Serial.println(alt1);
  delay(200);
  alt2=roundf(pressureToAltitude(bme.readPressure()));
  // Serial.println(alt2);
  dalt=alt2-alt1;
  acceleration=dalt/0.2;
  if(dalt!=0){
    Serial.println(dalt);
    Serial.println(acceleration);
  }
  Serial.println("//////////////////////////////////////////////////////////");
}
double rounding(double num, int decimal_places) { //функция округления значений с несколькими знаками после запятой. rounding(число, знаков после запятой)
  double multiplier = pow(10, decimal_places);
  return round(num * multiplier) / multiplier; //возвращает округленное число 
}
void loop() {
  // variometer();
  Serial.print(bme.readTemperature());
  Serial.println(" Celsius");
  Serial.print(bme.readHumidity());
  Serial.println("% Humidity");
  Serial.print(bme.readPressure());
  Serial.println(" Pa");
  Serial.println(pressureToAltitude(bme.readPressure()));
  // Serial.println(pressureToAltitude(101325));
  delay(1000);
}
