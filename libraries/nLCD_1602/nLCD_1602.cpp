#include "nLCD_1602.h"

// Library NLCD1602
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 0.1
//==================================

NLCD1602::NLCD1602(uint8_t lcd_Addr, uint8_t lcd_cols, uint8_t lcd_rows):LiquidCrystal_I2C(lcd_Addr, lcd_cols, lcd_rows) {
  this->cols = lcd_cols;
  this->rows = lcd_rows;
}

boolean NLCD1602::backlightSwitch(boolean _backlightOn) {
  this->backlightOn  = _backlightOn;

  if (this->backlightOn) {
    this->backlight();
    this->saverReset();
  } else {
    this->noBacklight();
  }
  
  return this->backlightOn;
}

boolean NLCD1602::backlightSwitch() {
  return this->backlightOn;
}

boolean NLCD1602::saverOn_set(boolean _saverOn) {
  this->saverOn = _saverOn;
  return this->saverOn;
}

boolean NLCD1602::saverOn_get() {
  return this->saverOn;
}

uint32_t NLCD1602::saverTime_set(uint32_t _time) {
  this->saverTime = _time;
  return this->saverTime;
}

uint32_t NLCD1602::saverTime_get() {
  return this->saverTime;
}

void NLCD1602::saverReset() {
  if (this->saverOn) {
    this->tmrSaver = millis();
  }
}

void NLCD1602::loop_Saver() {
  if (this->saverOn && this->backlightOn) {
    if (millis() - this->tmrSaver >= this->saverTime) {
      this->saverReset();
      this->backlightSwitch(false);
    }
  }
}

void NLCD1602::printRow(byte _row, String _text) {
  String text = _text;

  for (byte i=1; i<=text.length() - this->cols; i++) {
    text += " "; 
  }
  
  this->setCursor(0, _row);
  this->print(text);
}
