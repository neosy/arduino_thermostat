#include "nPotentiometer.h"

// Library NPMeter
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 0.1
//==================================

// ********************************* Functions for class PMeter ****************************
NPMeter::NPMeter(const char *_name, byte _pin, uint16_t _sensitivity) {
  this->name = new char[strlen(_name) + 1];
  strcpy(this->name, _name);
  this->pin = _pin;
  this->value = 0;
  this->sensitivity = _sensitivity;
  this->minValue = this->roundSens(PMETER_MIN);
  this->maxValue = this->roundSens(PMETER_MAX);
}

NPMeter::NPMeter(const char *_name, byte _pin, void (*_callBack)(NPMeter* _pmeter), uint16_t _sensitivity): NPMeter(_name, _pin, _sensitivity) {
  this->setCallBackChanged(_callBack);
}

NPMeter::~NPMeter() {
  delete []name;
}

void NPMeter::setCallBackChanged(void (*_callBack)(NPMeter* _pmeter)){
  this->callBackChanged = _callBack;
}

void NPMeter::init() {
  int curValue = analogRead(this->pin);
  this->realValue = curValue;
  this->value = this->roundSens(curValue);
  this->changed();
}

uint16_t NPMeter::sensitivity_get() {
  return this->sensitivity;
}

char *NPMeter::name_get() {
  return this->name;
}

uint16_t NPMeter::value_get() {
  return this->value;
}

uint16_t NPMeter::realValue_get() {
  return this->realValue;
}

uint16_t NPMeter::minValue_get() {
  return this->minValue;
}

uint16_t NPMeter::maxValue_get() {
  return this->maxValue;
}

uint16_t NPMeter::roundSens(uint16_t _value) {
  float value = round((float)(_value / this->sensitivity))*this->sensitivity;

  if (value > PMETER_MAX) {
    value = PMETER_MAX;
  } else if (value < PMETER_MIN) {
    value = PMETER_MIN;
  }
  
  return value;
}

float NPMeter::valueConvert(float _value, float _min, float _max, uint8_t _multiple) {
  float newValue = 0;
  if ((this->maxValue - this->minValue != 0) && (_max - _min != 0)) {
    newValue = ((float)(_value - this->minValue)/(float)((this->maxValue - this->minValue)/(float)(_max - _min))) + _min;

    if (_multiple != 0) {
      newValue = round((float)(newValue / _multiple))*_multiple;
    }
  }    
  //Serial.println("value: " + String(_value) + " newValue: " + String(newValue) + " min: " + String(_min) + " max: " + String(_max) + " PMin: " + String(this->minValue) + " PMax: " + String(this->maxValue));
  return newValue;
}

float NPMeter::valueConvert(float _min, float _max, uint8_t _multiple) {
  return this->valueConvert(this->value, _min, _max, _multiple);
}

void NPMeter::down() {
  if (debugState)
    Serial.println("Potentiometer name \"" + String(this->name) + "\" Down; Value: " + String(this->value));
}

void NPMeter::up() {
  if (debugState)
    Serial.println("Potentiometer name \"" + String(this->name) + "\" Up; Value: " + String(this->value));
}

void NPMeter::changed() {
  if (this->callBackChanged != NULL) {
    (*this->callBackChanged)(this);
  }
}

boolean NPMeter::debugMode(boolean _debugState) {
  this->debugState = _debugState;

  return this->debugState;
}

boolean NPMeter::debugMode() {
  return this->debugState;
}

void NPMeter::loop_run() {
  uint16_t oldRealValue = this->realValue;
  uint16_t oldValue = this->value;
  uint16_t realValue = analogRead(this->pin);
  uint16_t curValue = realValue;

  //Serial.println("loop_PMeter() realValue: " + String(realValue) + "; oldRealValue: " + String(oldRealValue) + "; sens: " + String(this->sensitivity));

  this->realValue = realValue;

  if (abs((int)(realValue - oldRealValue)) >= this->sensitivity) {
    curValue = this->roundSens(curValue);
    if (abs((int)(curValue - oldValue)) >= this->sensitivity) {
      if (debugState) {
        Serial.println("loop_PMeter() curValue: " + String(curValue) + "; oldValue: " + String(oldValue) + "; sens: " + String(this->sensitivity));
      }
      this->value = curValue;
      if (curValue > oldValue) {
        this->up();
      } else {
        this->down();
      }
      this->changed();
    }
  }
}

// ********************************* Functions for class PMeters ****************************
NPMeters::NPMeters() {
  this->first = NULL;
  this->last  = NULL;
}

NPMeters::~NPMeters() {
  NPMeter *cur = this->first;
  NPMeter *tmp;

  if (cur != NULL) {
    while (cur) {
      tmp = cur;
      cur = cur->next;
      delete tmp;
    }
  }
}

NPMeter *NPMeters::add(NPMeter *_pmeter) {
  NPMeter  *ret = NULL;

  this->number++;

  if (_pmeter != NULL) {
    if (this->first == NULL) {
      _pmeter->prev  = NULL;
      this->first = _pmeter;
    } else {
      _pmeter->prev  = last;
      last->next  = _pmeter;  
    }
    _pmeter->next  = NULL;
    this->last  = _pmeter;
    ret = this->last;
  }    

  return ret;
}


NPMeter* NPMeters::add(const char *_name, byte _pin, uint16_t _sensitivity) {
  NPMeter* pmeter = this->add(new NPMeter(_name, _pin, _sensitivity));
  pmeter->init();
  
  return pmeter;
}

NPMeter* NPMeters::add(const char *_name, byte _pin, void (*_callBack)(NPMeter* _pmeter), uint16_t _sensitivity) {
  NPMeter* pmeter = this->add(new NPMeter(_name, _pin, _callBack, _sensitivity));
  pmeter->init();

  return pmeter;
}

NPMeter *NPMeters::pmeter(uint8_t _num) {
  NPMeter       *cur  = this->first;
  unsigned char i     = 0;

  while (cur) {
    if (i == _num) {
      break;
    }
    i++;
    cur = cur->next;
  }

  return cur;
}

NPMeter *NPMeters::pmeter(const char *_name) {
  NPMeter  *cur = this->first;

  while (cur) {
    if (strcmp(cur->name, _name) == 0) {
      break;
    }
    cur = cur->next;
  }
  
  return cur;
}

void NPMeters::loop_run() {
  for (uint8_t i=0; i<this->number; i++) {
    this->pmeter(i)->loop_run();
  }
}
