#include <Adafruit_GFX.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_ILI9341.h>
#include <SPI.h>
#include <GyverBME280.h>  // BMP280

// Define TFT display pins for ESP32
#define TFT_CS 15
#define TFT_RST 4
#define TFT_DC 2
#define TFT_MOSI 19
#define TFT_SCK 18
#define TFT_MISO 23

// Create TFT display object
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_RST);
GyverBME280 bme;

#define GREY 0x8c71
void setup() {
tft.begin();
  tft.setRotation(0);
  tft.fillScreen(ILI9341_BLACK);
  // drawAltitudeIndicator(0, 60, 20, 100, 300); // x=60, y=20, width=100, height=300
}

void loop() {
  // int currentAltitude = pressureToAltitude(bme.readPressure()); // This should be replaced by your function call to get the current altitude
  // // Update the altitude display periodically
  // // currentAltitude = getAltitude(); // Call your function here
  // drawAltitudeIndicator(); // Adjust position and size as needed
  // delay(1000); // Update every second (adjust as needed)
  drawVariable(10,10,"Altitude", 1200);
}

void box_Variable(int x, int y, const char* text, int variable) {
  int rectHeight = 60;
  int rectWidth = 108;
  // Нарисовать прямоугольник и заполнить его часть серым цветом
  tft.drawRect(x, y, rectWidth, rectHeight, ILI9341_WHITE);
  tft.fillRect(x, y, rectWidth, 15, ILI9341_DARKGREY);
  // Установка размера текста и цвета
  tft.setTextSize(1); // Размер текста для серого прямоугольника
  tft.setTextColor(ILI9341_BLACK);
  // Вычисление размеров текста
  int16_t x1, y1;
  uint16_t textWidth, textHeight;
  tft.getTextBounds(text, 0, 0, &x1, &y1, &textWidth, &textHeight);
  // Вычисление координат для центрирования текста по оси X в сером прямоугольнике
  int xCText = x + (rectWidth - textWidth) / 2;
  // Установка курсора и вывод текста в сером прямоугольнике
  tft.setCursor(xCText, y + 3); // Устанавливаем y чуть ниже верхнего края серого прямоугольника
  tft.print(text);
  // Преобразование переменной в строку
  char variableStr[10];
  sprintf(variableStr, "%d", variable);
  // Установка размера текста и вычисление размеров текста переменной
  tft.setTextSize(3); // Размер текста для белого 
  tft.getTextBounds(variableStr, 0, 0, &x1, &y1, &textWidth, &textHeight);
  // Вычисление координат для центрирования текста по оси X в белом прямоугольнике
  int xCVariable = x + (rectWidth - textWidth) / 2;
  // Установка курсора и вывод текста переменной в белом прямоугольнике
  tft.setTextColor(ILI9341_WHITE);
  tft.setCursor(xCVariable, y + 25); // Устанавливаем y как 25 пикселей от верхнего края квадрата
  tft.print(variableStr);
}


