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
#include <TinyGPS++.h>         // GPS neo-6m
#include <GyverBME280.h>       // BMP280
#include <Adafruit_MPU6050.h>  // MPU6500
#include <Adafruit_GFX.h>      //Graphics
#include <Adafruit_ILI9341.h>  //TFT display

// Variables
struct weatherstruct {
  float humidity, temp, pressure;
};
struct gyroscope_offset_value {
  float x, y, z;
};

// Defining pin numbers
#define RXD2 16
#define TXD2 17
#define buzzer 5
// TFT display pins
#define TFT_CS 15
#define TFT_RST 4
#define TFT_DC 2

// Define custom colors
#define CUSTOM_BROWN 0xA145
#define CUSTOM_BLUE 0x1ab2


// Objects
GyverBME280 bme;        // Creating object bme for BMP280
weatherstruct weather;  // Object for weather variables (ex: weather.humidity=....)
gyroscope_offset_value gyroOffset;
HardwareSerial neogps(1);                                          // Creating serial port for GPS
TinyGPSPlus gps;                                                   // Object for GPS module
Adafruit_MPU6050 mpu;                                              // Object for accelerometer & gyroscope
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);  //tft display

class gyroClass {
public:
  // Gyroscope calibration offset
  gyroClass() {
    gyroOffset.x = 0;
    gyroOffset.y = 0;
    gyroOffset.z = 0;
  }

  void Calibration() {
    mpu.setFilterBandwidth(MPU6050_BAND_44_HZ);  // Increasing frequency of sensor
    float total_x = 0, total_y = 0, total_z = 0;
    int counter = 500;
    sensors_event_t a, g, temp;

    for (int i = 1; i <= counter; i++) {
      mpu.getEvent(&a, &g, &temp);
      total_x += g.gyro.x;
      total_y += g.gyro.y;
      total_z += g.gyro.z;
      delay(2);  // Small delay to stabilize readings
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
gyroClass gyroscope;  // Creating object gyroscope for using MPU6050's gyroscope

class Display {
public:
  Display()
    : tft(TFT_CS, TFT_DC, TFT_RST), naviCounter(0) {}

  void setup() {
    Serial.println("Initializing TFT display...");
    tft.begin();
    tft.setRotation(0);  // Adjust the rotation as needed
    tft.fillScreen(ILI9341_BLACK);
    Serial.println("TFT display initialized.");
  }

  void update() {
    int altitude = pressureToAltitude(bme.readPressure());
    int speed = gps.speed.kmph();
    float pitch, roll;
    getAngles(pitch, roll);
    box_Variable(10, 10, "Altitude", altitude);
    box_Variable(122, 10, "Speed", speed);
    drawHorizonBox(50, 150, 140, 150, pitch, roll, 45);
    delay(20);  // Update rate, adjust as needed
  }

private:
  Adafruit_ILI9341 tft;
  int naviCounter = 0;
  void drawHorizonBox(int x, int y, int w, int h, float pitch, float roll, float pitchViewAngle) {  //position: x,y  size: width, height   angles: pitch, roll   angle of view in Horizon box: pitchViewAngle
    if (naviCounter < 1) {
      // Clear previous content
      tft.fillRect(x, y, w, h, ILI9341_BLACK);
      naviCounter += 1;
    }

    // Calculate horizon line position
    int centerX = x + w / 2;
    int centerY = y + h / 2;
    int horizonY = centerY + map(pitch, -pitchViewAngle, pitchViewAngle, -h / 2, h / 2);

    // Constrain horizonY to be within the box
    horizonY = constrain(horizonY, y, y + h);

    // Draw ground and sky
    tft.fillRect(x, horizonY, w, y + h - horizonY, CUSTOM_BROWN);  // Ground
    tft.fillRect(x, y, w, horizonY - y, CUSTOM_BLUE);              // Sky

    // Draw roll indicator
    tft.drawRect(x, y, w, h, ILI9341_WHITE);

    // Draw the roll line across the entire circle diameter
    int lineX1 = centerX + 50 * cos((roll + 180) * DEG_TO_RAD);
    int lineY1 = centerY + 50 * sin((roll + 180) * DEG_TO_RAD);
    int lineX2 = centerX + 50 * cos(roll * DEG_TO_RAD);
    int lineY2 = centerY + 50 * sin(roll * DEG_TO_RAD);

    // Constrain lines to be within the box
    lineX1 = constrain(lineX1, x, x + w);
    lineY1 = constrain(lineY1, y, y + h);
    lineX2 = constrain(lineX2, x, x + w);
    lineY2 = constrain(lineY2, y, y + h);

    // Draw thicker roll line
    tft.drawLine(lineX1, lineY1, lineX2, lineY2, ILI9341_WHITE);
    tft.drawLine(lineX1, lineY1 + 1, lineX2, lineY2 + 1, ILI9341_WHITE);
    tft.drawLine(lineX1, lineY1 - 1, lineX2, lineY2 - 1, ILI9341_WHITE);

    // Draw fixed horizontal lines indicating normal position at the center
    int horizontalLineLength = 20;  // Length of the horizontal lines
    int offsetFromCenter = 50;      // Offset from the center for the horizontal lines

    tft.drawLine(centerX - offsetFromCenter - horizontalLineLength / 2, centerY, centerX - offsetFromCenter + horizontalLineLength / 2, centerY, ILI9341_WHITE);
    tft.drawLine(centerX + offsetFromCenter - horizontalLineLength / 2, centerY, centerX + offsetFromCenter + horizontalLineLength / 2, centerY, ILI9341_WHITE);
    tft.drawLine(centerX - offsetFromCenter - horizontalLineLength / 2, centerY + 1, centerX - offsetFromCenter + horizontalLineLength / 2, centerY + 1, ILI9341_WHITE);
    tft.drawLine(centerX + offsetFromCenter - horizontalLineLength / 2, centerY + 1, centerX + offsetFromCenter + horizontalLineLength / 2, centerY + 1, ILI9341_WHITE);
    tft.drawLine(centerX - offsetFromCenter - horizontalLineLength / 2, centerY - 1, centerX - offsetFromCenter + horizontalLineLength / 2, centerY - 1, ILI9341_WHITE);
    tft.drawLine(centerX + offsetFromCenter - horizontalLineLength / 2, centerY - 1, centerX + offsetFromCenter + horizontalLineLength / 2, centerY - 1, ILI9341_WHITE);
  }

  void getAngles(float &pitch, float &roll) {
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    pitch = atan2(a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;
    roll = atan2(-a.acceleration.x, a.acceleration.z) * 180 / PI;
  }
  void box_Variable(int x, int y, const char *text, int variable) {
    int rectHeight = 60;
    int rectWidth = 108;
    // Нарисовать прямоугольник и заполнить его часть серым цветом
    tft.drawRect(x, y, rectWidth, rectHeight, ILI9341_WHITE);
    tft.fillRect(x, y, rectWidth, 15, ILI9341_DARKGREY);
    // Установка размера текста и цвета
    tft.setTextSize(1);  // Размер текста для серого прямоугольника
    tft.setTextColor(ILI9341_BLACK);
    // Вычисление размеров текста
    int16_t x1, y1;
    uint16_t textWidth, textHeight;
    tft.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
    // Вычисление координат для центрирования текста по оси X в сером прямоугольнике
    int xCText = x + (rectWidth - textWidth) / 2;
    // Установка курсора и вывод текста в сером прямоугольнике
    tft.setCursor(xCText, y + 3);  // Устанавливаем y чуть ниже верхнего края серого прямоугольника
    tft.print(text);
    // Преобразование переменной в строку
    char variableStr[10];
    sprintf(variableStr, "%d", variable);
    // Установка размера текста и вычисление размеров текста переменной
    tft.setTextSize(3);  // Размер текста для белого
    tft.getTextBounds(variableStr, 0, 0, &x1, &y1, &textWidth, &textHeight);
    // Вычисление координат для центрирования текста по оси X в белом прямоугольнике
    int xCVariable = x + (rectWidth - textWidth) / 2;
    // Установка курсора и вывод текста переменной в белом прямоугольнике
    tft.fillRect(x + 1, y + 15, rectWidth - 2, rectHeight - 16, ILI9341_BLACK);
    tft.setTextColor(ILI9341_WHITE);
    tft.setCursor(xCVariable, y + 25);  // Устанавливаем y как 25 пикселей от верхнего края квадрата
    tft.print(variableStr);
  }
};
// Create Display object
Display display;

// Initialization of MPU6500 sensor
void mpuInitialization() {
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);  // 2_G/4_G/8_G/16_G
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);       // 250_DEG/500_DEG/1000_DEG/2000_DEG
  mpu.setFilterBandwidth(MPU6050_BAND_5_HZ);     // 5_HZ/10_HZ/21_HZ/44_HZ/94_HZ/184_HZ/260_HZ
}

void weatherData() {
  weather.humidity = bme.readHumidity();  // Reading humidity from BMP280
  weather.temp = bme.readTemperature();   // Reading temperature from BMP280
  weather.pressure = bme.readPressure();  // Reading pressure from BMP280
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
  alt1 = rounding(pressureToAltitude(bme.readPressure()), 3);  // Reading altitude #1
  delay(200);                                                  // Waiting 200ms
  alt2 = rounding(pressureToAltitude(bme.readPressure()), 3);  // Reading altitude #2
  dalt = alt2 - alt1;                                          // Difference between altitudes 2 and 1
  vertical_speed = dalt / 0.2;                                 // Calculating speed gained in 200ms
  return vertical_speed;
}

double rounding(double num, int decimal_places) {  // Function to round values with specified decimal places
  double multiplier = pow(10, decimal_places);
  return round(num * multiplier) / multiplier;  // Returning rounded number
}

void variometer() {
  if (verticalSpeed() < 0) {
  }
}

boolean gpsChecker() {  // Function returns true if GPS module is reachable
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
  Serial.begin(115200);  // Activating serial port (115200 baud) for ESP32
  delay(2000);
  display.setup();
  // Trying to connect BMP280
  if (!bme.begin(0x76)) {  // Check if BMP280 connected
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
  mpu.begin();
  mpuInitialization();
  gyroscope.Calibration();
  Serial.println("MPU6500 has started");
}

// MAIN LOOP
void loop() {
  display.update();
}
