// Подключаем библиотеку для работы с I2C
#include <Wire.h>

// Подключаем библиотеку шилда моторов
#include <Adafruit_MotorShield.h>

// Подключаем библиотеку драйвера моторов Adafruit
#include "utility/Adafruit_MS_PWMServoDriver.h"

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

// Объявляем переменную признака авторежима движения
bool isAuto;

// Объявляем переменную признака движения по линии
bool isLine;

// Создаем переменную для команд Bluetooth
char vcmd;

// Создаем ограничение максимальной скорости для каждого мотора
int iMaxSpeedRM = 55; //55
int iMaxSpeedLM = 50; //50

void setup() {
  // Открываем последовательный порт
  Serial.begin(9600);

  // Создаем объект шилда мотор моторов на частоте по умолчанию 1.6KHz
  AFMS.begin();

  // Устанавливаем переменную признака авторежима
  isAuto = false;

  // Устанавливаем переменную признака движения по линии
  isLine = false;

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


void loop() 
{
  if (Serial.available())
  {
    //Управление программой на Android: Bluetooth RC Controller
    vcmd = (char)Serial.read();
    //F – вперед
    //B – назад
    //L – влево
    //R – вправо
    //G – прямо и влево
    //I – прямо и вправо
    //H – назад и влево
    //J – назад и вправо
    //S – стоп 
    //W – передняя фара включена
    //w – передняя фара выключена
    //U – задняя фара включена
    //u – задняя фара выключена
    //V – звуковой сигнал включен
    //v – звуковой сигнал выключен
    //X – сигнал “аварийка” включен
    //x - сигнал “аварийка” выключен
    //0 – скорость движения 0%
    //1 – скорость движения 10%
    //2 – скорость движения 20%
    //3 – скорость движения 30%
    //4 – скорость движения 40%
    //5 – скорость движения 50%
    //6 – скорость движения 60%
    //7 – скорость движения 70%
    //8 – скорость движения 80%
    //9 – скорость движения 90%
    //q – скорость движения 100% 
    
    // Включить автопилот
    if (vcmd == 'X')
    {
      isAuto = true;
      Disp4d.display(1);
    }
    // Выключить автопилот
    if (vcmd == 'x')
    {
      isAuto = false;
      Disp4d.display(-1);
    }

    // Включить движение по линии
    if (vcmd == 'W')
    {
      isLine = true;
      Disp4d.display(2);
    }
    // Выключить движение по линии
    if (vcmd == 'w')
    {
      isLine = false;
      Disp4d.display(-2);
    }

    if (isLine)
    {
      // 0 - датчик на линии, 1 - датчик вне линии
      iLLS = digitalRead(L_LINE_SENSOR_PIN);
      iCLS = digitalRead(C_LINE_SENSOR_PIN);
      iRLS = digitalRead(R_LINE_SENSOR_PIN);

      Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
      
      //  Определяем действия в соответствии с текущим положением датчиков
      if (iLLS == 1 && iCLS == 1 && iRLS == 1)
      {
        // 111 - Если все датчики находятся вне линии - двигаемся назад
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        setMaxSpeed();
        moveB();
        delay(50);
      }
      else if (iLLS == 1 && iCLS == 0 && iRLS == 1)
      {
        // 101 - Если только центральный датчик находится на линии - продолжаем движение прямо
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        setMaxSpeed();
        moveF();
        delay(150);
      }
      else if (iLLS == 0 && iCLS == 1 && iRLS == 1)
      {
        // 011 - Если только левый датчик находится на линии - поворачиваем влево
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        //setMaxSpeed();
        //moveL();
        setMaxRSpeed();
        moveF();
        delay(100);
      }
      else if (iLLS == 0 && iCLS == 0 && iRLS == 1)
      {
        // 001 - Если левый и центральный датчики находятся на линии - продолжаем движение прямо левее
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        setMaxRSpeed();
        moveF();
        delay(100);
      }
      else if (iLLS == 1 && iCLS == 1 && iRLS == 0)
      {
        // 110 - Если только правый датчик находится на линии - поворачиваем вправо
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        //setMaxSpeed();
        //moveR();
        setMaxLSpeed(); 
        moveF();
        delay(100);
      }
      else if (iLLS == 1 && iCLS == 0 && iRLS == 0)
      {
        // 100 - Если правый и центральный датчики находятся на линии - продолжаем движение прямо правее
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        setMaxLSpeed(); 
        moveF();
        delay(100);
      }
      else
      {
        // 000 - Если правый и левый датчики находятся на линии - двигаемся назад
        Disp4d.display(iLLS*100 + iCLS*10 + iRLS);
        //setMaxSpeed();
        //moveR();
        setMaxLSpeed(); 
        moveF();
        delay(50);  
      }
      goto EndOfMoving;
    }
    
    // Вперед
    if (vcmd == 'F') 
    {
      setMaxSpeed();
      moveF();
    }
    // Назад
    if (vcmd == 'B')
    {
      setMaxSpeed();
      moveB();
    }
    // Влево
    if (vcmd == 'L')
    {
      setMaxSpeed();
      moveL();
    }    
    // Вправо
    if (vcmd == 'R')
    {
      setMaxSpeed();
      moveR();
    }
    
    // Вперед и влево
    if (vcmd == 'G') 
    {
      setMaxRSpeed();
      moveF();
    }
    // Вперед и вправо
    if (vcmd == 'I')
    {
      setMaxLSpeed();
      moveF();
    }
    // Назад и влево
    if (vcmd == 'H')
    {
      setMaxRSpeed();
      moveB();
    }
    // Назад и вправо
    if (vcmd == 'J')
    {
      setMaxLSpeed();
      moveB();
    }
    // Стоп
    if (vcmd == 'S')
    {
      moveS();
    }
    EndOfMoving:;
  } //if
} //loop
