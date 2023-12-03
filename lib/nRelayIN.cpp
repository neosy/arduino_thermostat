#include "nRelayIN.h"

// ********************************* Functions for class RelayIN ****************************
NRelayIN::NRelayIN(const char *_name, byte _pin) {
  this->name = new char[strlen(_name) + 1];
  strcpy(this->name, _name);
  this->pin = _pin;
  this->state = LOW;
  
  pinMode(this->pin, OUTPUT);
}

NRelayIN::~NRelayIN() {
  delete []name;
}

boolean NRelayIN::switchIN(boolean _state) {
  this->state = _state;
  digitalWrite(this->pin, this->state);
  return this->state;
}

boolean NRelayIN::switchIN() {
  this->state = !this->state;
  digitalWrite(this->pin, this->state);
  return this->state;
}
