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
TFT display 3.2 240 x 340px: SDO(MOSI)-D19 LED-1KOhm 3.3V SCK-D18 SDI-D23 D/C-D2 RESET-D4 CS-D15 GND-GND VCC-3.3V
*/

// Libraries
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <TinyGPS++.h> // GPS neo-6m
#include <GyverBME280.h> // BMP280 
#include <Adafruit_MPU6050.h> // MPU6500

// Variables
struct weatherstruct {
  float humidity, temp, pressure;
};
struct gyroscope_offset_value{
  float x,y,z;
};

// Defining pin numbers
#define RXD2 16
#define TXD2 17

// Objects
GyverBME280 bme; // Creating object bme for BMP280  
weatherstruct weather; // Object for weather variables (ex: weather.humidity=....) 
gyroscope_offset_value gyroOffset;
HardwareSerial neogps(1); // Creating serial port for GPS
TinyGPSPlus gps; // Object for GPS module 
Adafruit_MPU6050 mpu; // Object for accelerometer & gyroscope

class gyroClass {
  public:
    // Gyroscope calibration offset
    gyroClass() {
      gyroOffset.x = 0;
      gyroOffset.y = 0;
      gyroOffset.z = 0;
    }

    void Calibration() {
      mpu.setFilterBandwidth(MPU6050_BAND_44_HZ); // Increasing frequency of sensor
      float total_x = 0, total_y = 0, total_z = 0;
      int counter = 500;
      sensors_event_t a, g, temp;

      for (int i = 1; i <= counter; i++) {
        mpu.getEvent(&a, &g, &temp);
        total_x += g.gyro.x;
        total_y += g.gyro.y;
        total_z += g.gyro.z;
        delay(2); // Small delay to stabilize readings
      }

      gyroOffset.x = total_x / counter;
      gyroOffset.y = total_y / counter;
      gyroOffset.z = total_z / counter;
    }

    float x() {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      return g.gyro.x - gyroOffset.x;
    }

    float y() {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      return g.gyro.y - gyroOffset.y;
    }

    float z() {
      sensors_event_t a, g, temp;
      mpu.getEvent(&a, &g, &temp);
      return g.gyro.z - gyroOffset.z;
    }
}; 

gyroClass gyroscope; // Creating object gyroscope for using MPU6050's gyroscope

// Initialization of MPU6500 sensor
void mpuInitialization() {
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G); // 2_G/4_G/8_G/16_G
  mpu.setGyroRange(MPU6050_RANGE_500_DEG); // 250_DEG/500_DEG/1000_DEG/2000_DEG
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ); // 5_HZ/10_HZ/21_HZ/44_HZ/94_HZ/184_HZ/260_HZ
}

void weatherData() {
  weather.humidity = bme.readHumidity(); // Reading humidity from BMP280
  weather.temp = bme.readTemperature(); // Reading temperature from BMP280
  weather.pressure = bme.readPressure(); // Reading pressure from BMP280
  Serial.print(weather.temp);
  Serial.println(" Celsius");
  Serial.print(weather.humidity);
  Serial.println("% Humidity");
  Serial.print(weather.pressure);
  Serial.println(" Pa");
}

// Function returns vertical speed in float format
float verticalSpeed() {
  // Version 1.0: Future versions will include acceleration check for accuracy
  float alt1, alt2, dalt, vertical_speed;
  alt1 = rounding(pressureToAltitude(bme.readPressure()), 3); // Reading altitude #1 
  delay(200); // Waiting 200ms 
  alt2 = rounding(pressureToAltitude(bme.readPressure()), 3); // Reading altitude #2
  dalt = alt2 - alt1; // Difference between altitudes 2 and 1
  vertical_speed = dalt / 0.2; // Calculating speed gained in 200ms
  return vertical_speed; 
}

double rounding(double num, int decimal_places) { // Function to round values with specified decimal places
  double multiplier = pow(10, decimal_places); 
  return round(num * multiplier) / multiplier; // Returning rounded number 
}

void variometer() {
  if (verticalSpeed() < 0) {
    // Add your code here for handling negative vertical speed
  }
}

boolean gpsChecker() { // Function returns true if GPS module is reachable
  boolean newData = false;
  for (unsigned long start = millis(); millis() - start < 1000;) {
    while (neogps.available()) {
      if (gps.encode(neogps.read())) {
        newData = true;
      }
    }
  }
  return newData;
}

// Reading RAW data from MPU6050
void mpuFastRead() {
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
void gpsFastRead() {
  Serial.print("Latitude: ");
  Serial.println(gps.location.lat());
  Serial.print("Longitude: ");
  Serial.println(gps.location.lng());
  Serial.print("Altitude: ");
  Serial.println(gps.altitude.meters());
  Serial.print("Satellites: ");
  Serial.println(gps.satellites.value());
  Serial.print("Speed: ");
  Serial.println(gps.speed.kmph());
}

// Print all data from sensors
void allDataRead() {
  Serial.println("GPS data");
  gpsFastRead();
  Serial.println("\nMPU6500 data");
  mpuFastRead();
  Serial.println("\nBMP280 data");
  weatherData();
  Serial.println("------------------------------------");
}

// Setup
void setup() {
  Serial.begin(115200); // Activating serial port (115200 baud) for ESP32
  delay(2000);
  // Trying to connect BMP280
  if (!bme.begin(0x76)) { // Check if BMP280 connected
    Serial.println("Could not find a valid BMP280 sensor");
  } else {
    Serial.println("BMP280 has started");
  }
  // Trying to connect GPS module
  neogps.begin(9600, SERIAL_8N1, RXD2, TXD2);
  if (!gpsChecker()) {
    Serial.println("Couldn't find GPS module");
  } else {
    Serial.println("GPS has started!");
  }
  // Starting MPU6500
  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU6500 sensor");
  } else {
    mpuInitialization();
    gyroscope.Calibration();
    Serial.println("MPU6500 has started");
  }
}

// MAIN LOOP
void loop() {
  allDataRead();
  delay(1000);
}
