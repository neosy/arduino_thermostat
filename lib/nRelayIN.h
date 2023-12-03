#ifndef nRelayIN_h
#define nRelayIN_h

#include <Arduino.h>

class NRelayIN {
  protected:
    boolean state = LOW;
  
  public:
    char *name;
    byte pin;

    NRelayIN(const char *_name, byte _pin);
    ~NRelayIN();
    boolean switchIN(boolean _state);
    boolean switchIN();
};

#endif
