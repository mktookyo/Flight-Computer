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
MPU6500: SCL-D22 SDA-D21 vcc-3.3V
*/

// libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h> //GPS neo-6m
#include <GyverBME280.h>    //BMP280 
#include <Adafruit_MPU6050.h> //MPU6500
// #include <math.h> //maybe in future ;-;
// #include <preferences.h>

// Variables
struct weatherstruct {
  float humidity, temp, pressure;
};
// defining pin numbers
#define RXD2 16
#define TXD2 17

// Objects
GyverBME280 bme;    //Crating object bme for BMP280  
weatherstruct weather; //object for weather variables (ex: weather.humidity=....) 
HardwareSerial neogps(1); //creating serial port for gps
TinyGPSPlus gps; //object for gps module 
Adafruit_MPU6050 mpu; //object for accelerometer & gyroscope

// Setup
void setup() {
  Serial.begin(115200); //Activating serial port (115200 baud) for ESP32
  delay(2000);
  // Trying to connect bmp280
  if (!bme.begin(0x76)) {  // Check if BMP280 connected
    Serial.println("Could not find a valid BMP280 sensor");
  }
  else{
    Serial.println("bmp280 has started");
  }
  // Trying to connect GPS module
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  if (gpsChecker()!=true){
    Serial.println("Couldn't find GPS module");
  }
  else{
    Serial.println("GPS has started!");
  }
  // Starting MPU6500
  mpu.begin();
  mpuInitilization();
  Serial.println("MPU6500 has started");
}
// MAIN LOOP
void loop() {
  allDataRead();
  delay(1000);
}

// just prints temperature, Humidity, Pressure.
void weatherData(){
  weather.humidity=bme.readHumidity(); //reading humidity from BMP280
  weather.temp=bme.readTemperature(); //reading temperature from BMP280
  weather.pressure=bme.readPressure(); //reading pressure from BMP280
  Serial.print(weather.temp);
  Serial.println(" Celsius");
  Serial.print(weather.humidity);
  Serial.println("% Humidity");
  Serial.print(weather.pressure);
  Serial.println(" Pa");
}
// function returns vertical speed in float format
float verticalSpeed(){
  // version 1.0  дальше в будущих версиях проверка точности показания скорости с помощью акселлерометра (наличие ускорения)
  float alt1, alt2, dalt, vertical_speed;
  alt1=rounding(pressureToAltitude(bme.readPressure()) , 3 ); //Reading altitude #1 
  delay(200); //Waiting 200ms 
  alt2=rounding(pressureToAltitude(bme.readPressure()) , 3 ); //Reading altitude #2
  dalt=alt2-alt1; //difference between altitudes 2 and 1
  vertical_speed=dalt/0.2; //calculating speed gained in 200ms
  return vertical_speed; 
}

double rounding(double num, int decimal_places) { //функция округления значений с несколькими знаками после запятой. rounding(число, знаков после запятой)
  double multiplier = pow(10, decimal_places); 
  return round(num * multiplier) / multiplier; //возвращает округленное число 
}
void variometer(){
  if (verticalSpeed()<0){
    
  }
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
// Initilization of MPU6500 sensor
void mpuInitilization(){
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // 2_G/4_G/8_G/16_G
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); //250_DEG/500_DEG/1000_DEG/2000_DEG
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ); // 5_HZ/10_HZ/21_HZ/44_HZ/94_HZ/184_HZ/260_HZ

}
void mpuFastRead(){
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);
  /* Print out the values */
  Serial.print("Acceleration X: ");
  Serial.print(a.acceleration.x);
  Serial.print(", Y: ");
  Serial.print(a.acceleration.y);
  Serial.print(", Z: ");
  Serial.print(a.acceleration.z);
  Serial.println(" m/s^2");

  Serial.print("Rotation X: ");
  Serial.print(g.gyro.x);
  Serial.print(", Y: ");
  Serial.print(g.gyro.y);
  Serial.print(", Z: ");
  Serial.print(g.gyro.z);
  Serial.println(" rad/s");
  Serial.println("");
}
// Fast printing for GPS data
void gpsFastRead(){
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat());
  Serial.print("Lontitude: ");
  Serial.println(gps.location.lng());
  Serial.print("Altitude: ");
  Serial.println(gps.altitude.meters());
  Serial.print("Satelites: ");
  Serial.println(gps.satellites.value());
  Serial.print("Speed: ");
  Serial.println(gps.speed.kmph());
}
// Print all data from sensors
void allDataRead(){
  Serial.println("GPS data");
  gpsFastRead();
  Serial.println("\n MPU6500 data");
  mpuFastRead();
  Serial.println("\n BMP280 data");
  weatherData();
  Serial.println("------------------------------------");
}

