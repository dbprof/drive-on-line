/**************************************************************
drive-on-line.ino
BlackBug Engineering
01.05.2017
https://github.com/dbprof/drive-on-line
***************************************************************/

// Подключаем библиотеки таймера
#include <Event.h>
#include <Timer.h>

// Подключаем библиотеки управления по bluetooth RemoteXY
// http://remotexy.com
#define REMOTEXY_MODE__SOFTSERIAL
#include <SoftwareSerial.h>
#include <RemoteXY.h>

// Подключаем библиотеку для работы с I2C
#include <Wire.h>

// Подключаем библиотеку шилда моторов
#include <Adafruit_MotorShield.h>

// Подключаем библиотеку драйвера моторов Adafruit
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Подключаем нестандартную библиотеку работы с сервоприводами с переменной скоростью
// http://forum.arduino.cc/index.php?topic=61586.0
#include <VarSpeedServo.h>

// Подключаем библиотеку для работы с 4-значным экраном
#include "TM1637.h"

////////////////////////////////////////////ЭКРАН////////////////////////////////////////////////////////
// Пины подключения экрана
const int TM1637_CLK_PIN = 12;
const int TM1637_DIO_PIN = 13;

// Создаем объект экрана
TM1637 Disp4d(TM1637_CLK_PIN,TM1637_DIO_PIN);

/////////////////////////////////////////////ШИЛД//////////////////////////////////////////////////////
// Создаем объект шилда моторов с адресом I2C по умолчанию
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 

// Создаем объекты правого и левого моторов
// Правый мотор подключен к выходу 2
Adafruit_DCMotor *RightMotor = AFMS.getMotor(2);
// Левый мотор подключен к выходу 1
Adafruit_DCMotor *LeftMotor = AFMS.getMotor(1);

/////////////////////////////////////////////BLUETOOTH//////////////////////////////////////////////////////
// настройки соединения 
#define REMOTEXY_SERIAL_RX 2
#define REMOTEXY_SERIAL_TX 3
#define REMOTEXY_SERIAL_SPEED 9600

// конфигурация интерфейса Android
#pragma pack(push, 1)
uint8_t RemoteXY_CONF[] =
  { 255,3,0,0,0,35,0,6,0,0,
  2,0,33,11,37,19,2,79,78,0,
  79,70,70,0,1,1,33,36,18,17,
  2,60,0,1,1,54,36,17,17,2,
  62,0 };

// структура определяет все переменные интерфейса управления 
struct {
  uint8_t switch_run; // =1 если переключатель включен и =0 если отключен 
  uint8_t button_left; // =1 если кнопка нажата, иначе =0 
  uint8_t button_right; // =1 если кнопка нажата, иначе =0 
  uint8_t connect_flag;  // =1 if wire connected, else =0 
} RemoteXY;
#pragma pack(pop)


//////////////////////////////////////////////ДАТЧИК ЛИНИИ//////////////////////////////////////////////////////
// Левый датчик линии по направлению движения
#define L_LINE_SENSOR_PIN 9
int iLLS;
// Центральный датчик линии по направлению движения
#define C_LINE_SENSOR_PIN 10
int iCLS;
// Правый датчик линии по направлению движения
#define R_LINE_SENSOR_PIN 11
int iRLS;
int iLCR;

// Объявляем переменную признака последнего движения (влево, вправо, прямо)
bool isLast1Left;
bool isLast1Right;
bool isLast1Forward;
bool isLast1Back;

// Объявляем и устанавливаем переменные задержек при движении
int iFDelay;
int iRDelay;
int iLDelay;
int iUDelay;

// Создаем переменную для команд Bluetooth
char vcmd;

// Создаем ограничение максимальной скорости для каждого мотора
int iMaxSpeedRM = 55; //55
int iMaxSpeedLM = 50; //50

// Создаем объект таймера
Timer t;

void setup() {
  // Открываем последовательный порт
  //Serial.begin(9600);

  t.every(1, readBTSignal);
  t.every(10, doMoving);

  // Инициализация управления Bluetooth-Android
  RemoteXY_Init(); 

  // Создаем объект шилда мотор моторов на частоте по умолчанию 1.6KHz
  AFMS.begin();

  // Объявляем переменную признака последнего движения (влево, вправо, прямо)
  isLast1Left = false;
  isLast1Right = false;
  isLast1Forward = false;
  isLast1Back = false;

  // Объявляем и устанавливаем переменные задержек при движении
  iFDelay = 150;
  iRDelay = 100;
  iLDelay = 100;
  iUDelay = 50;

  //Устанавливаем яркость экрана и инициируем его
  Disp4d.set(5);
  Disp4d.init(D4056A);
}


//////////////////////////////////////////////ПРИМИТИВНЫЕ ДВИЖЕНИЯ//////////////////////////////////////////////////////
// Установка максимальной разрешенной скорости
void setMaxSpeed() {
  RightMotor->setSpeed(iMaxSpeedRM);
  LeftMotor->setSpeed(iMaxSpeedLM);
}

// Установка скорости с правым приоритетом
void setMaxRSpeed() {
  RightMotor->setSpeed(iMaxSpeedRM);
  LeftMotor->setSpeed(iMaxSpeedLM/2);
}

// Установка скорости с левым приоритетом
void setMaxLSpeed() {
  RightMotor->setSpeed(iMaxSpeedRM/2);
  LeftMotor->setSpeed(iMaxSpeedLM);
}

// Движение вперед
void moveF() {
  RightMotor->run(FORWARD);
  LeftMotor->run(FORWARD);
}

// Движение назад
void moveB() {
  RightMotor->run(BACKWARD);
  LeftMotor->run(BACKWARD);
}

// Движение влево
void moveL() {
  RightMotor->run(FORWARD);
  LeftMotor->run(BACKWARD);
}

// Движение вправо
void moveR() {
  RightMotor->run(BACKWARD);
  LeftMotor->run(FORWARD);
}

// Остановка движения
void moveS() {
  RightMotor->run(RELEASE);
  LeftMotor->run(RELEASE);
}

//////////////////////////////////////////////ПРЕДИДУЩЕЕ ДВИЖЕНИЕ//////////////////////////////////////////////////////
void setLast1Right(){
  isLast1Left = false;
  isLast1Right = true;
  isLast1Forward = false;
  isLast1Back = false;
}

void setLast1Left(){
  isLast1Left = true;
  isLast1Right = false;
  isLast1Forward = false;
  isLast1Back = false;
}

void setLast1Forward(){
  isLast1Left = false;
  isLast1Right = false;
  isLast1Forward = true;
  isLast1Back = false;
}

void setLast1Back(){
  isLast1Left = false;
  isLast1Right = false;
  isLast1Forward = false;
  isLast1Back = true;
}


void loop() 
{
  t.update();
}

void readBTSignal() 
{
  RemoteXY_Handler();
}

void doMoving() 
{
  Disp4d.display(RemoteXY.button_left*100 + RemoteXY.switch_run*10 + RemoteXY.button_right);
  
    if (RemoteXY.switch_run)
    {
      // 0 - датчик на линии, 1 - датчик вне линии
      iLLS = digitalRead(L_LINE_SENSOR_PIN);
      iCLS = digitalRead(C_LINE_SENSOR_PIN);
      iRLS = digitalRead(R_LINE_SENSOR_PIN);
      
      //  Определяем действия в соответствии с текущим положением датчиков
      if (iLLS == 1 && iCLS == 1 && iRLS == 1)
      {
        // 111 - Если все датчики находятся вне линии - действуем на основе предидущего движения
        setMaxSpeed();
        if (isLast1Right)
        {
          // Если предидущее движение было направо - поворачиваем правее
          moveR();
          setLast1Right();
        }
        else if (isLast1Left)
        {
          // Если предидущее движение было налево - поворачиваем левее
          moveL();
          setLast1Left();
        }
        else if (isLast1Forward)
        {
          // Если предидущее движение было прямо - поворачиваем правее
          moveR();
          setLast1Right();
        }
        else
        {
          // Если предидущее движение было назад или не было определено - поворачиваем правее
          moveR();
          setLast1Right();
        }
        delay(iUDelay);
      }
      else if (iLLS == 1 && iCLS == 0 && iRLS == 1)
      {
        // 101 - Если только центральный датчик находится на линии - продолжаем движение прямо
        setMaxSpeed();
        moveF();
        setLast1Forward();
        delay(iFDelay);
      }
      else if (iLLS == 0 && iCLS == 1 && iRLS == 1)
      {
        // 011 - Если только левый датчик находится на линии - продолжаем движение прямо левее
        setMaxRSpeed();
        setLast1Left();
        moveF();
        delay(iLDelay);
      }
      else if (iLLS == 0 && iCLS == 0 && iRLS == 1)
      {
        // 001 - Если левый и центральный датчики находятся на линии - продолжаем движение прямо левее
        setMaxRSpeed();
        moveF();
        setLast1Left();
        delay(iLDelay);
      }
      else if (iLLS == 1 && iCLS == 1 && iRLS == 0)
      {
        // 110 - Если только правый датчик находится на линии - продолжаем движение прямо правее
        setMaxLSpeed(); 
        moveF();
        setLast1Right();
        delay(iRDelay);
      }
      else if (iLLS == 1 && iCLS == 0 && iRLS == 0)
      {
        // 100 - Если правый и центральный датчики находятся на линии - продолжаем движение прямо правее
        setMaxLSpeed(); 
        moveF();
        setLast1Right();
        delay(iRDelay);
      }
      else
      {
        // 000 010 - Если правый и левый датчики находятся на линии - действуем на основе предидущего движения
        setMaxSpeed();
        if (isLast1Right)
        {
          // Если предидущее движение было направо - поворачиваем правее
          moveR();
          setLast1Right();
        }
        else if (isLast1Left)
        {
          // Если предидущее движение было налево - поворачиваем левее
          moveL();
          setLast1Left();
        }
        else if (isLast1Forward)
        {
          // Если предидущее движение было прямо - поворачиваем правее
          moveR();
          setLast1Right();
        }
        else
        {
          // Если предидущее движение было назад или не было определено - назад
          moveB();
          setLast1Right();
        }
        delay(iUDelay);  
      }
    }
    else
    {
      moveS();
    }
}
