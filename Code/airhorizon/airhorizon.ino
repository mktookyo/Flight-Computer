#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// TFT display pins
#define TFT_CS    15
#define TFT_RST   4
#define TFT_DC    2
#define TFT_MOSI  19
#define TFT_SCK   18
#define TFT_MISO  23
#define TFT_LED   5  // Use a PWM pin if you want to control brightness

// I2C pins for MPU6050
#define MPU6050_SDA  21
#define MPU6050_SCL  22

int naviCounter = 0;
// Create display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);

// Create MPU6050 object
Adafruit_MPU6050 mpu;

// Function to initialize the TFT display
void setupDisplay() {
  Serial.println("Initializing TFT display...");
  tft.begin();
  tft.setRotation(0);  // Adjust the rotation as needed
  tft.fillScreen(ILI9341_BLACK);
  Serial.println("TFT display initialized.");
}

// Function to initialize the MPU6050
void setupMPU6050() {
  Serial.println("Initializing MPU6050...");
  mpu.begin();
  mpu.setAccelerometerRange(MPU6050_RANGE_8_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  Serial.println("MPU6050 initialized.");
}

// Function to draw the artificial horizon box
void drawHorizonBox(int x, int y, int w, int h, float pitch, float roll) {
  if (naviCounter<1){
    // Clear previous content
    tft.fillRect(x, y, w, h, ILI9341_BLACK);
    naviCounter+=1;
  }

  // Calculate horizon line position
  int horizonY = y + map(pitch, -90, 90, 0, h);

  // Draw ground and sky
  tft.fillRect(x, horizonY, w, y + h - horizonY, ILI9341_ORANGE);  // Ground
  tft.fillRect(x, y, w, horizonY - y, ILI9341_BLUE);  // Sky

  // Draw roll indicator
  int centerX = x + w / 2;
  int centerY = y + h / 2;
  tft.drawRect(x, y, w, h, ILI9341_WHITE);
  tft.drawCircle(centerX, centerY, 50, ILI9341_WHITE);

  // Draw the roll line across the entire circle diameter
  int lineX1 = centerX + 50 * sin((roll + 90) * DEG_TO_RAD);
  int lineY1 = centerY + 50 * cos((roll + 90) * DEG_TO_RAD);
  int lineX2 = centerX + 50 * sin((roll - 90) * DEG_TO_RAD);
  int lineY2 = centerY + 50 * cos((roll - 90) * DEG_TO_RAD);
  tft.drawLine(lineX1, lineY1, lineX2, lineY2, ILI9341_WHITE);
}

// Function to read angles from MPU6050
void getAngles(float &pitch, float &roll) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  pitch = atan2(a.acceleration.y, sqrt(a.acceleration.x * a.acceleration.x + a.acceleration.z * a.acceleration.z)) * 180 / PI;
  roll = atan2(-a.acceleration.x, a.acceleration.z) * 180 / PI;
}

void setup() {
  Serial.begin(115200);
  setupDisplay();
  setupMPU6050();

  // // LED brightness (optional)
  // pinMode(TFT_LED, OUTPUT);
  // analogWrite(TFT_LED, 128);  // Adjust brightness (0-255)
}

void loop() {
  float pitch, roll;
  getAngles(pitch, roll);
  Serial.print("Pitch: ");
  Serial.print(pitch);
  Serial.print(" Roll: ");
  Serial.println(roll);

  // Draw horizon box with specified size and position
  int boxX = 50;
  int boxY = 60;
  int boxWidth = 140;
  int boxHeight = 140;
  drawHorizonBox(boxX, boxY, boxWidth, boxHeight, pitch, roll);

  delay(100);  // Update rate, adjust as needed
}
