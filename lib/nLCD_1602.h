#ifndef nLCD_1602_h
#define nLCD_1602_h

#include "Arduino.h" 
#include "LiquidCrystal_I2C.h"

#define LCD_SAVER_ENABLE true
#define LCD_SAVER_TIME 300000 // 5 minutes = 5*60*1000

class NLCD1602: public LiquidCrystal_I2C {
  protected:
    boolean   backlightOn = false;
    boolean   saverOn = LCD_SAVER_ENABLE;
    uint32_t  tmrSaver;
    uint8_t   cols, rows;
  
  public:
    uint32_t  saverTime = LCD_SAVER_TIME;
  
    NLCD1602(uint8_t lcd_Addr,uint8_t lcd_cols,uint8_t lcd_rows);
    boolean backlightSwitch(boolean _backlightOn);
    boolean backlightSwitch();
    boolean saverOn_set(boolean _saverOn);
    boolean saverOn_get();
    uint32_t saverTime_set(uint32_t _time);
    uint32_t saverTime_get();
    virtual void saverReset();
    void loop_Saver();
    void printRow(byte _row, String _text);
};

#endif
