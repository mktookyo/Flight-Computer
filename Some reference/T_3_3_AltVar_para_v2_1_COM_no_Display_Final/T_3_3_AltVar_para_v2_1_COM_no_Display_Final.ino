/*
Альтиметр-вариометр AltVar+ с возможностью подачи 
звуковых и световых предупреждающих сигналов
версия без дисплея для параплана V2.1.0
 
 АППАРАТНАЯ ЧАСТЬ
 
* Gy-68  Pro Mini      Значение датчика
 1          VCC            VIN  (питание)
 2          GND            GND  (земля) 
 3        analog 5         SCL  (тактирование)
 4        analog 4         SDA  (данные)
 
 * Пищалка Pro Mini       Значение
 1        digital 8        конт + 
 2        digital 9        конт – (через резистор 330-1К)
 
 *Красный LED  Pro Mini    Значение
 1        digital 13        контакт + (анод)
 2          GND             контакт - (катод через резистор 1К-4.7К)
 
 * Питание - от одного литий-полимерного элемента (3.7 вольт).
  LiPO 3.7V                     Pro Mini
 чёрный провод  (-)             GND
 красный провод (+)             RAW (допускается питание до 12 вольт)

*/ 
// ПРОГРАММНАЯ ЧАСТЬ \\

#include <Wire.h>                      //i2c библиотека интерфейса
#include <BMP085.h>                    //bmp085 библиотека датчика давления
#include <Tone.h>                      //tone библиотека пьезодинамика
//#define OLED_RESET 4

//Adafruit_SSD1306 display(OLED_RESET);

short speaker_pin1 = 8;                //arduino speaker output -
short speaker_pin2 = 9;                //arduino speaker output +

float vario_down = -1.1;               // Установка СНИЖЕНИЯ
float vario_up; 
float alt[51];
float tim[51];
float beep;
float Beep_period;
float mux;
float Altitude;
float Alt0;                 // нулевая высота в момент включения + !
float correctBMP180 = 0;    // коррекция бародатчика в гектапаскалях /индивидуальна для каждого датчика/

const float p0 = 101325;

long Pressure = 101325;
unsigned long bounseInput4P = 0UL;

int samples=40;
int maxsamples=50;
int countPressVal = 0;

bool tmp1 = 0;
bool countPress = 0;
bool bounseInput4S = 0;
bool bounseInput4O = 0;

BMP085   bmp085 = BMP085();
Tone     tone_out1;
Tone     tone_out2;
boolean  thermalling = false;

void play_beep()                        //звуковой сигнал
{
  for (int aa=100;aa<=800;aa=aa+100)
  {
    tone_out1.play(aa,200);       
    tone_out2.play(aa+3,200);
    delay(50);              
  }
}


void play_welcome_beep()                 //звук приветствия
{
    for (int aa=100;aa<=800;aa=aa+100)
    {
        tone_out1.play(aa,200);       
        tone_out2.play(aa+3,200);
        delay(50);
    }
}

void setup()
{
    Wire.begin();                   // инициализация i2c
    pinMode(4, INPUT);
    digitalWrite(4, HIGH);
    pinMode(13, OUTPUT);             // выход на светодиод
    digitalWrite(13, LOW);           // светодиод выключен

    
    Serial.begin (9600);               // открывает последовательный порт + !
                                       // задаѐт скорость обмена 9600
    
    
    bounseInput4O =  digitalRead(4);// подтягивающий резистор
    tone_out1.begin(speaker_pin1);  // выход динамика pin8 -
    tone_out2.begin(speaker_pin2);  // выход динамика pin9 +
    bmp085.init(MODE_ULTRA_HIGHRES, p0, false); // чувствительность датчика давления
    bmp085.calcTruePressure(&Pressure);
    Alt0 = (float)44330 * (1 - pow(((float)Pressure/p0), 0.190295)); // полючение нулевой высоты в метрах в момент включения +!
    play_welcome_beep();
    
    ////////////////////////////////////////
      // Информационая загрузка программы
  
  digitalWrite(13,HIGH);     // светодиод включен
  delay(200);                // ждем 0.2 секунды
  digitalWrite(13,LOW);     // светодиод выключен    
  delay(300);                // ждем 0.3 секунды
//////////////////////////////////////////////////////////
    
}

void loop(void)
{
    bool  bounceTmp =  (digitalRead (4));
    
    if (bounseInput4S)          // защита от дребезга
    {
        if (millis() >= (bounseInput4P + 40))
        {bounseInput4O= bounceTmp; bounseInput4S=0;}
    }
    else{
        if (bounceTmp != bounseInput4O )
        {bounseInput4S=1; bounseInput4P = millis();}
    }
    if (!(bounseInput4O))
    {
        if (! countPress)
        {
            countPressVal++;
            
            for (int i = 0; i < countPressVal; i++)	// проиграть номер меню
            {
                tone_out2.play(800, 100); 
                delay(200);                
            }
            countPress = 1;
        }
    }
    else{
        countPress=0;
    }
    if (countPressVal < 0 ) countPressVal = 0;
    if (tmp1) countPressVal = 0;            // == МЕНЮ ЧУВСТВИТЕЛЬНОСТИ НА ПОДЪЁМ ==
    if((countPressVal) == 0) {mux = 0.5;}  // 4 сигнала
    if((countPressVal) == 1) {mux = 0.3;}  // 1 сигнал
    if((countPressVal) == 2) {mux = 0.35;} // 2 сигнала
    if((countPressVal) == 3) {mux = 0.4;}  // 3 сигнала
    tmp1 =  countPressVal  >=  4;
    vario_up = mux;
    
    
    float tempo=millis();
    float vario=0;
    float N1=0;
    float N2=0;
    float N3=0;
    float D1=0;
    float D2=0;
    bmp085.calcTruePressure(&Pressure);
    
    Altitude = (float)44330 * (1 - pow(((float)Pressure/p0), 0.190295)); // полючение новой высоты в метрах
    
    for(int cc=1;cc<=maxsamples;cc++){                                   // усреднитель
        alt[(cc-1)]=alt[cc];
        tim[(cc-1)]=tim[cc];
    };
    alt[maxsamples]=Altitude;
    tim[maxsamples]=tempo;
    float stime=tim[maxsamples-samples];
    for(int cc=(maxsamples-samples);cc<maxsamples;cc++){
        N1+=(tim[cc]-stime)*alt[cc];
        N2+=(tim[cc]-stime);
        N3+=(alt[cc]);
        D1+=(tim[cc]-stime)*(tim[cc]-stime);
        D2+=(tim[cc]-stime);
    };
    
    vario=1000*((samples*N1)-N2*N3)/(samples*D1-D2*D2); // рачёт звука
    
    // БЛОК ИНДИКАЦИИ ************************
  
  Serial.print("Alt:");
  Serial.println ((Altitude-Alt0),1);// отправляет значение высоты в метрах на COM
  Serial.print("Var:");
  Serial.println (vario,1);          // отправляет значение вертикальной скорости в м\сек на COM
  Serial.print("mmHg:");
  Serial.println (float((Pressure)*0.007501),1);// отправляет значение атм. давления в мм рт.ст. на СОМ
  Serial.println ();                            // пустая строка для разграничения
 
  if ((vario) >=0)
  {
   // lcd.print ("+^+ ");
    digitalWrite(13,LOW);               // светодиод выключен ******
  }
  if ((vario) <0)
  {
   // lcd.print ("-v- ");
    digitalWrite(13,HIGH);               // светодиод включен ******
  }
    
    if ((tempo-beep)>Beep_period)
    
    {
        beep=tempo;
        if (vario>vario_up && vario<15 )
        {
            Beep_period=350-(vario*5);
            tone_out1.play((1000+(100*vario)),300-(vario*5)); // звук на подъёме
            tone_out2.play((1003+(100*vario)),300-(vario*5));
            thermalling = true;
        }
        else if ((vario < 0 ) && (thermalling == true))
        {
            thermalling = false; 
         // tone_out2.play(200, 800); // звук предпоток (по-желанию )
            
        }
        else if (vario< vario_down){         // звук на сливе
            Beep_period=200;
            tone_out1.play((300-(vario)),340);
            tone_out2.play((303-(vario)),340);
            thermalling = false;
        }
    }
}
