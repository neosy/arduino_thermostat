#ifndef nPotentiometer_h
#define nPotentiometer_h

#include "Arduino.h" 

#define DEBUG_PMETER false

#define PMETER_SENSITIVITY 10
#define PMETER_MIN 20
#define PMETER_MAX 960

class NPMeter {
    private:
    NPMeter     *prev, *next;
    
  protected:
    boolean debugState = DEBUG_PMETER;
    char *name;
    byte pin;
    uint16_t sensitivity;
    uint16_t value;
    uint16_t realValue;
    uint16_t minValue, maxValue;
    void (*callBackChanged)(NPMeter* _pmeter);
  
  public:
    NPMeter(const char *_name, byte _pin, uint16_t _sensitivity = PMETER_SENSITIVITY);
    NPMeter(const char *_name, byte _pin, void (*_callBack)(NPMeter* _pmeter), uint16_t _sensitivity = PMETER_SENSITIVITY);
    ~NPMeter();
    void init();
    void setCallBackChanged(void (*_callBack)(NPMeter* _pmeter));
    char *name_get();
    uint16_t sensitivity_get();
    uint16_t value_get();
    uint16_t realValue_get();
    uint16_t minValue_get();
    uint16_t maxValue_get();
    uint16_t roundSens(uint16_t _value);
    float valueConvert(float _value, float _min, float _max, uint8_t _multiple); // _multiple - кратно
    float valueConvert(float _min, float _max, uint8_t _multiple = 0); // _multiple - кратно
    virtual void down();
    virtual void up();
    virtual void changed();
    boolean debugMode(boolean _debugState);
    boolean debugMode();
    void loop_run();
    
    friend class NPMeters;
};

class NPMeters {
  private:
    NPMeter     *first, *last;
    uint8_t     number = 0;

  public:
    NPMeters();
    ~NPMeters();
    NPMeter* add(NPMeter* _pmeter);
    NPMeter* add(const char *_name, byte _pin, uint16_t _sensitivity = PMETER_SENSITIVITY);
    NPMeter* add(const char *_name, byte _pin, void (*_callBack)(NPMeter* _pmeter), uint16_t _sensitivity = PMETER_SENSITIVITY);
    NPMeter* pmeter(uint8_t _num);
    NPMeter* pmeter(const char *_name);
    void loop_run();
};

#endif
