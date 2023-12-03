// Version 12
// v.12
//    1) В строке дисплея добавлен знак градуса
//    2) В конце 2-ой строки добавлен символ стрелка вверх (нагрев) или стрелка вниз (без нагрева)
// v.11
//    1) Добавлена константа LCD_SAVER_ENABLE, которая разрешает по умолчанию saver для LCD дисплея
//    2) Добавлена константа LCD_ON_HEATING и функционал, который позволяет включать дисплей в начале нагрева, при этом функционал saver LCD отключается и включается после окончания нагрева
//    3) Изменена константа допустимого падения температуры TDROP_PERCENT с 10% на 7% (выключение при отображаемой температуре (если макс установлено 60): (60 - 7%) - 3 = 52,8
//    4) В progress лампочек изменен алгоритм расчета горящих лампочек. Округление выполяентся в большую сторону

#include <Wire.h>
#include <TaskScheduler.h>
#include <microDS18B20.h>
#include "avr/wdt.h"

#include "nLeds.h"
#include "nButton.h"

#include "nPotentiometer.h"
#include "nRelayIN.h"
#include "nLCD_1602.h"

#define LED_HEATER1_PIN  3
#define LED_HEATER2_PIN  4
#define LED_HEATER3_PIN  5
#define LED_HEATER4_PIN  6
#define BTNLCD_PIN  8
#define RELAY_PIN 9
#define TSENSOR_PIN 10
#define PMETER_PIN A1

#define LCD_PORT 0x27
#define LCD_COL 16
#define LCD_ROW 2
#define LCD_SAVER_ENABLE true
#define LCD_SAVER_TIME 300000 // 5 minutes = 5*60*1000
#define LCD_ON_HEATING true // Включать дисплей при нагреве (функция saver отключается в процессе нагрева)
#define TMAX 80
#define TMIN 30
#define TMAX_SHIFT 3.0 // Сдвиг предела нагрева. На дисплее сдвиг максимальной температуры не отображается. Нагревательный тен находится близко к датчику и температура сразу падает
#define TSHIFT 0.0 // Сдвиг температуры датчика. Коррекция датчика температуры
#define TSHOW_SHIFT -3.0 // Сдвиг отображения измеряемой температуры
#define TMULTIPLE 1.0 // Точность установки максимальной температуры
#define TDROP_PERCENT 7 // Допустимое падение температуры в %
#define TINC 1.0 // Порядок изменения температуры при повороте ручки
#define MANUAL_CONTROL_DELAY 3000 // Задержка после ручного изменения максимальной температуры
#define TSENSOR_FREQ 1000 // Частота опроса датчика температуры
#define PMETER_FREQ 200 // Частота опроса ручки регулятора максимальной температуры

#define DEBUG_ON false

void blinkLedError();
void loop_TSensor();
void loop_PMeters();
void loop_lcdSaver();
void heaterTurnOn();
void heaterTurnOff();
void handlerButtonLcdClicked();
void handlerTSensorError(boolean _sensorError, boolean _sensorErrorLast);
void handlerSetTMax(NPMeter* _pmeter);
void setTMax(float _value);
void setTMaxUp();
void setTMaxDown();
void printTMax();
void printTCur(float _temp);
void ledHManualCtrlOn();
void ledHManualCtrlOff();
void lcd_initChar();

unsigned long wdt_timer;
float         mTMax         = TMAX;
boolean       mHeaterOn     = false;
uint32_t      mTmrLedHManualCtlrOffPause = 0;
float         mTemperature  = 0;
boolean       mTSensorError = false;

NRelayIN mRelay("R1", RELAY_PIN);
NLCD1602 mLcd(LCD_PORT, LCD_COL, LCD_ROW);

MicroDS18B20<10> mTSensor;
NLeds           *mLeds;
NLedScenarios   *mLedScenas;
NLedProgress    *mLedProgress;
NButton         *mButtonLed;
NPMeter         *mPMeter;

Scheduler tScheduler;
Task tTSensor(TSENSOR_FREQ, TASK_FOREVER, &loop_TSensor);
Task tPMeters(PMETER_FREQ, TASK_FOREVER, &loop_PMeters);
Task tLcdSaver(5000, TASK_FOREVER, &loop_lcdSaver);

void lcd_initChar() {
  uint8_t *customChar;
  
  customChar = new uint8_t[8]{
    B00000,
    B00100,
    B01110,
    B10101,
    B00100,
    B00100,
    B00100,
    B00000
  };
  mLcd.createChar(0, customChar);
  delete []customChar;

  customChar = new uint8_t[8]{
    B00000,
    B00100,
    B00100,
    B00100,
    B10101,
    B01110,
    B00100,
    B00000
  };
  mLcd.createChar(1, customChar);
  delete []customChar;
}

//Действие при нажатии кнопки включение\выключение дисплея
void handlerButtonLcdClicked() {
  mLcd.backlightSwitch(!mLcd.backlightSwitch());
}

//Включить нагрев
void heaterTurnOn() {
  if (!mHeaterOn && mTSensorError == false) {
    mHeaterOn = true;
    if (LCD_ON_HEATING) {
      mLcd.saverOn_set(false);
      mLcd.backlightSwitch(true);
    }
    mLedProgress->parmBlinkLast(mHeaterOn);
    mLedProgress->update(true);
    mRelay.switchIN(HIGH);
  }    
}

//Выключить нагрев
void heaterTurnOff() {
  if (mHeaterOn) {
    mHeaterOn  = false;
    if (LCD_ON_HEATING) {
      mLcd.saverOn_set(LCD_SAVER_ENABLE);
      mLcd.saverReset();
    }
    mLedProgress->parmBlinkLast(mHeaterOn);
    mLedProgress->update(true);
    mRelay.switchIN(LOW);
  }    
}

void updateHeater() {
  if (mTemperature != 0) {
    if (mTemperature < mTMax - round(mTMax*(float)(TDROP_PERCENT)/100)) {
      heaterTurnOn();
    } else if (mTemperature >= (mTMax + TMAX_SHIFT)) {
      heaterTurnOff();
    }
  }    
}

//Установка пороговой максимальной температуры
void setTMax(float _value) {
  mTMax = round(_value);

  if (mTMax > TMAX) {
    mTMax = TMAX;
  }

  if (mTMax < TMIN) {
    mTMax = TMIN;
  }

  updateHeater();

  mLcd.backlightSwitch(true);
  printTMax();
  
  if (DEBUG_ON) {
    Serial.println("Temperature MAX: " + String(mTMax) + "\xF8");
  }    
}

//Установка пороговой максимальной температуры
void handlerSetTMax(NPMeter* _pmeter) {
  float value;

  value = _pmeter->valueConvert(TMIN, TMAX, TMULTIPLE);
  if (value != mTMax) {
    ledHManualCtrlOn();
    
    setTMax(value);
    
    if (DEBUG_ON)
      Serial.println(String(mTMax));
    
    mLedProgress->setCount(value - TMIN);
    mLedProgress->update();
    
    ledHManualCtrlOff();
  }    
}

//Увеличение пороговой максимальной температуры
void setTMaxUp() {
  setTMax(mTMax + TINC);
}

//Уменьшение пороговой максимальной температуры
void setTMaxDown() {
  setTMax(mTMax - TINC);
}

//Действие, если возникает ошибка датчика температуры
void handlerTSensorError(boolean _sensorError, boolean _sensorErrorLast) {
  if (_sensorError == true && _sensorErrorLast == false) {
      heaterTurnOff();
      mLedScenas->scena("LPr")->disable();
      mLedScenas->scena("TEr")->enable();
      mLcd.printRow(1, "T: Error sensor");
  }
  
  if (_sensorError == false && _sensorErrorLast == true) {
      mLedScenas->scena("TEr")->disable();
      mLedScenas->scena("LPr")->enable();
      mLedProgress->setCount(round(mTemperature + TSHOW_SHIFT - TMIN));
      mLedProgress->update(true);
      mLcd.printRow(1, "T cur: reading...");
  }
}

//Вывод на дисплей значения максимальной допустимой температуры
void printTMax() {
  mLcd.printRow(0, "T max: " + String(mTMax, 1) + char(223));
}

//Вывод на дисплей значения текущей температуры
void printTCur(float _t) {
  mLcd.printRow(1, "T cur: " + String(_t, 1) + char(223));
  mLcd.setCursor(15, 1);
  if (mHeaterOn) {
    mLcd.write(0); // Стрелка вверх
  } else {
    mLcd.write(1); // Стрелка вниз
  }
}

void ledHManualCtrlOn() {
  mLedProgress->parmBlinkLast(false);
}

void ledHManualCtrlOff() {
  mTmrLedHManualCtlrOffPause = millis();
}

void loop_ledHManualCtrlOff() {
  if (mTmrLedHManualCtlrOffPause != 0 && (millis() - mTmrLedHManualCtlrOffPause >= MANUAL_CONTROL_DELAY)) {
    mTmrLedHManualCtlrOffPause = 0;

    mLedProgress->parmBlinkLast(mHeaterOn);
    mLedProgress->setCount(round(mTemperature + TSHOW_SHIFT - TMIN));
    //Serial.println(String(mHeaterOn));
    //Serial.println(String(round(mTemperature - TMIN)));
    mLedProgress->update(true);
  }    
}

//Петля для опроса датчика температуры
void loop_TSensor() {
  float   t_last = mTemperature;  
  boolean last_TSensorError = mTSensorError;

  // запрос температуры
  mTSensor.requestTemp();
  
  if (t_last == 0) {
    if (mTSensor.readTemp())
      t_last = mTSensor.getTemp() + TSHIFT;
  }
  
  if (t_last != 0) {
    if (mTSensor.readTemp()) {
      mTSensorError = false;
      mTemperature = mTSensor.getTemp() + TSHIFT;
      mLedProgress->setCount(round(mTemperature + TSHOW_SHIFT - TMIN));
      if (DEBUG_ON) {
        Serial.println("Temp sensor: " + String(mTemperature, 1));
      }
      mLedProgress->update();
      
      printTCur(mTemperature + TSHOW_SHIFT);
  
      updateHeater();
    }        
    else {
      mTSensorError = true;
    }      

    if (mTSensorError != last_TSensorError) {
      handlerTSensorError(mTSensorError, last_TSensorError);
    }      
  }
}

//Петля для опроса потенциометра
void loop_PMeters() {
  mPMeter->loop_run();
}

//Петля для автоматического выключения экрана
void loop_lcdSaver() {
  mLcd.loop_Saver();
}

void setup() {
  if (DEBUG_ON) {
    Serial.begin(9600);
    Serial.println("Setup...");
  }

  wdt_reset();
  wdt_disable();
  wdt_enable(WDTO_8S);
  wdt_timer = millis();
  
  //Инициализация параметров дисплея
  mLcd.init();
  lcd_initChar();
  mLcd.saverOn_set(LCD_SAVER_ENABLE);
  mLcd.saverTime_set(LCD_SAVER_TIME);

  //Инициализация лампочек
  mLeds = new NLeds();
  mLeds->add("L1", LED_HEATER1_PIN);
  mLeds->add("L2", LED_HEATER2_PIN);
  mLeds->add("L3", LED_HEATER3_PIN);
  mLeds->add("L4", LED_HEATER4_PIN);

  //Инициализация сцен лампочек
  mLedScenas = new NLedScenarios();

  mLedScenas->add("LPr");

  mLedScenas->add("TEr");
  mLedScenas->scena("TEr")->add(mLeds->led("L1"), &NLed::switchLight);
  mLedScenas->scena("TEr")->add(mLeds->led("L2"), &NLed::switchLight);
  mLedScenas->scena("TEr")->add(mLeds->led("L3"), &NLed::switchLight);
  mLedScenas->scena("TEr")->add(mLeds->led("L4"), &NLed::switchLight);
  mLedScenas->scena("TEr")->addDelay(200);

  //Инициализация прогресса из лампочек
  mLedProgress = new NLedProgress(mLedScenas->scena("LPr"));
  mLedProgress->init(TMAX - TMIN);
  mLedProgress->add(mLeds->led("L1"));
  mLedProgress->add(mLeds->led("L2"));
  mLedProgress->add(mLeds->led("L3"));
  mLedProgress->add(mLeds->led("L4"));
  mLedProgress->update(true);

  //Инициализация кнопок
  mButtonLed = new NButton("BL", BTNLCD_PIN);
  mButtonLed->setCallBackClicked(handlerButtonLcdClicked);
  mButtonLed->debugMode(DEBUG_ON); //DEBUG_ON

  //Инициализация потенциометра
  mPMeter = new NPMeter("PM", PMETER_PIN, handlerSetTMax);
  mPMeter->init();
  mPMeter->debugMode(DEBUG_ON); // DEBUG_ON

  mLedScenas->scena("LPw")->enable();
  mLedScenas->scena("LPr")->enable();

  setTMax(mTMax);

  //Инициализация планировщика
  tScheduler.addTask(tTSensor);
  tTSensor.enable();
  tScheduler.addTask(tPMeters);
  tPMeters.enable();
  tScheduler.addTask(tLcdSaver);
  tLcdSaver.enable();

  //Включение света дисплея
  mLcd.backlightSwitch(true);
   
  if (DEBUG_ON) {
    Serial.println("freeMemory()=" + String(freeMemory()));
  }    
}

void loop() {
  // put your main code here, to run repeatedly:
  loop_ledHManualCtrlOff();
  tScheduler.execute();
  mLedScenas->loop_run();
  mButtonLed->loop_run();

  if (millis() - wdt_timer > 4000) {
    wdt_timer = millis();
    wdt_reset(); 
  }

  delay(10);
}
