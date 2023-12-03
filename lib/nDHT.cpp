#include "nDHT.h"

NDHT::NDHT(uint8_t _pin, uint8_t _type, uint8_t _count):DHT(_pin, _type, _count) {
}

void NDHT::setCallBackDHTError(void (*_callBack)(boolean _errorState, boolean _errorStateLast)) {
  this->callBackDHTError = _callBack;
}

float NDHT::getTemperature() {
  return this->t_last;
}

float NDHT::getHumidity() {
  return this->h_last;
}

boolean NDHT::getErrorState() {
  return this->dhtError;
}

boolean NDHT::debugMode(boolean _debugState) {
  this->debugState = _debugState;

  return this->debugState;
}

boolean NDHT::debugMode() {
  return this->debugState;
}

void NDHT::loop_DHT() {
  boolean errorState;
  boolean lastDhtError = this->dhtError;
  float t = this->readTemperature();
  float h = this->readHumidity();

  // ошибочные данные с датчика!
  if (isnan(h)) {
    errorState = true;
    
    if (this->debugState) {
      Serial.println("Failed to read from DHT");
    }     

    this->t_last = 0;
    this->h_last = 0;
  } else {
    errorState = false;
    
    if (this->debugState) {
      Serial.print("Temperature: ");
      Serial.print(t);
      Serial.print("; ");
      Serial.print("Humidity: ");
      Serial.println(h);
    }
    
    this->t_last = t;
    this->h_last = h;
  }    
  
  this->dhtError = errorState;
  
  if (errorState != lastDhtError) {
    this->dhtError = errorState;
    (*this->callBackDHTError)(errorState, lastDhtError);
  }    
}
