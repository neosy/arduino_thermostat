#ifndef nDHT_h
#define nDHT_h

#include "Arduino.h" 
#include "DHT.h"

#define DHT_DEBUG false

class NDHT: public DHT {
  protected:
    boolean debugState = DHT_DEBUG;
    float   t_last, h_last;
    boolean dhtError = false;
  
  public:
    void (*callBackDHTError)(boolean _errorState, boolean _errorStateLast);
  
    NDHT(uint8_t _pin, uint8_t _type, uint8_t _count = 6);
    void setCallBackDHTError(void (*_callBack)(boolean _state, boolean _errorStateLast));
    float getTemperature();
    float getHumidity();
    boolean getErrorState();
    boolean debugMode(boolean _debugState);
    boolean debugMode();
    void loop_DHT();
};

#endif
