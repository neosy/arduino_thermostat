#include "nLeds.h"

// Library NLed
// Author: Neosy <neosy.dev@gmail.com>
//
//==================================
// Version 0.1
//==================================

// ********************************* Functions for class NLed ****************************
NLed::NLed(const char *_name, uint8_t _pin) {
  this->name = new char[strlen(_name) + 1];
  strcpy(this->name, _name);
  this->pin   = _pin;
  this->prev  = NULL;
  this->next  = NULL;
  this->isOn = LOW;
  
  pinMode(this->pin, OUTPUT);
}

NLed::~NLed() {
  delete []name;
}

char *NLed::name_get() {
  return this->name;
}

boolean NLed::isOn_get() {
  return this->isOn;
}

boolean NLed::switchLight(boolean _isOn) {
  if (_isOn != this->isOn) {
    this->isOn = _isOn;
    digitalWrite(this->pin, this->isOn);
  }    
  return this->isOn;
}

void NLed::switchLight() {
  this->switchLight(!this->isOn);
}

void NLed::on() {
  this->switchLight(HIGH);
}

void NLed::off() {
  this->switchLight(LOW);
}

// ********************************* Functions for class NLeds ****************************
NLeds::NLeds() {
  this->first = NULL;
  this->last = NULL;
}

NLeds::~NLeds() {
  NLed  *cur = this->first;
  NLed  *tmp;

  if (cur != NULL) {
    while (cur) {
      tmp = cur;
      cur = cur->next;
      delete tmp;
    }
  }
}

/*NLeds::NLeds(unsigned char _size) {
  this->ledArr = new NLed*[_size];
}*/

NLed *NLeds::add(NLed *_led) {
  NLed  *ret = NULL;

  this->number++;

  if (_led != NULL) {
    if (this->first == NULL) {
      _led->prev  = NULL;
      this->first = _led;
    } else {
      _led->prev  = last;
      last->next  = _led;  
    }
    _led->next  = NULL;
    this->last  = _led;
    ret = this->last;
  }    

  return ret;
}

NLed *NLeds::add(const char *_name, uint8_t _pin) {
  return this->add(new NLed(_name, _pin));
}

NLed *NLeds::led(uint8_t _num) {
  NLed          *cur = this->first;
  unsigned char i   = 0;

  while (cur) {
    if (i == _num) {
      break;
    }
    i++;
    cur = cur->next;
  }

  return cur;
}

NLed *NLeds::led(const char *_name) {
  NLed  *cur = this->first;

  while (cur) {
    if (strcmp(cur->name, _name) == 0) {
      break;
    }
    cur = cur->next;
  }
  
  return cur;
}

// ********************************* Functions for class NLedProgress ****************************
NLedProgress::NLedProgress(NLedScena *_ledScena) {
  this->first     = NULL;
  this->last      = NULL;
  this->ledScena  = _ledScena;
}

NLedProgress::~NLedProgress() {
  Led *cur = this->first;
  Led *tmp;

  if (cur != NULL) {
    while (cur) {
      tmp = cur;
      cur = cur->next;
      delete tmp;
    }
  }
}

NLed *NLedProgress::add(NLed *_led) {
  Led *ledStruct = new Led();

  ledStruct->led  = _led;

  this->number++;

  if (this->first == NULL) {
    ledStruct->prev  = NULL;
    this->first = ledStruct;
  } else {
    ledStruct->prev  = last;
    last->next  = ledStruct;  
  }
  ledStruct->next  = NULL;
  this->last  = ledStruct;

  return this->last->led;
}

NLed *NLedProgress::led(uint8_t _num) {
  Led               *cur = this->first;
  unsigned char i   = 0;

  while (cur) {
    if (i == _num) {
      break;
    }
    i++;
    cur = cur->next;
  }

  return cur->led;
}

NLed *NLedProgress::ledIncomplete() {
  return this->led(numIncomplete);
}

void NLedProgress::init(uint32_t _total) {
  this->total = _total;
  this->count = 0;
}

void NLedProgress::init(uint32_t _total, long _count) {
  this->total = _total;
  this->count = (_count < 0) ? 0 : (uint32_t)_count;
}

boolean NLedProgress::parmBlinkLast() {
  return this->blinkLast;
}

boolean NLedProgress::parmBlinkLast(boolean _blinkLast) {
  this->blinkLast = _blinkLast;
  return this->blinkLast;
}

uint8_t NLedProgress::number_get(){
 return this->number; 
}

uint32_t NLedProgress::incCount(uint32_t _v) {
  this->count += _v;
  return this->count;
}

uint32_t NLedProgress::incCount() {
  return this->incCount(1);
}

void NLedProgress::setCount(long _count) {
  this->count = (_count < 0) ? 0 : (uint32_t)_count;
}

void NLedProgress::setTotal(uint32_t _total) {
  this->total = _total;
}

void NLedProgress::reset() {
  this->count = 0;
  this->update();
}

void NLedProgress::update(boolean _force) {
  float   exactCntLedOn = this->calcExactCntLedOn();
  uint8_t cntLedOn = ceil(exactCntLedOn);

  if (PROGRESS_LED_ROUND_MORE == false) {
    cntLedOn = round(exactCntLedOn);
  }

  this->numIncomplete = 0;
  if (cntLedOn < exactCntLedOn) {
    this->numIncomplete = cntLedOn;
  } else if (cntLedOn > 0) {
    this->numIncomplete = cntLedOn - 1;
  }

  //Serial.println("cntLedOn: " + String(cntLedOn));
  //Serial.println("this->cntLedOn: " + String(this->cntLedOn));
  //Serial.println("numIncomplete: " + String(numIncomplete));

  if (_force || this->cntLedOn != cntLedOn) {
    this->cntLedOn = cntLedOn;

    if (cntLedOn > 0) {
      this->ledScena->clear();
      for (uint8_t i=0; i<this->number; i++) {
        if (blinkLast && i == numIncomplete) {
          this->ledScena->add(this->led(i), &NLed::switchLight);
          this->ledScena->addDelay(PROGRESS_BLINK_DELAY);
        } else {
          if (i+1 <= cntLedOn) {
            this->ledScena->add(this->led(i), &NLed::on);
          } else {
            this->ledScena->add(this->led(i), &NLed::off);
          }
        }
      }
      this->ledScena->reset();
      this->ledScena->run();
    } else {
      this->ledScena->clear();    
      if (blinkLast) {
        this->ledScena->add(this->led(0), &NLed::switchLight);
        this->ledScena->addDelay(PROGRESS_BLINK_DELAY);
      }
    }
  }
}

void NLedProgress::hideAll() {
  for (uint8_t i=0; i<this->number; i++) {
    this->led(i)->off();    
  }
}

float NLedProgress::calcExactCntLedOn() {
  float     exactCntLedOn = 0;
  uint32_t  total = this->total;
  uint32_t  count = (this->count < total) ? this->count : total;

  if (total == 0) {
    exactCntLedOn = this->number;
  } else if (this->number != 0) {
    exactCntLedOn = (float)count/((float)total/(float)this->number);
  }

  //Serial.print(count);
  //Serial.print(" ");
  //Serial.print(total);
  //Serial.print(" ");
  //Serial.print(this->number);
  //Serial.print(" ");
  //Serial.println(exactCntLedOn);

  return exactCntLedOn;
}

// ********************************* Functions for class NLedScena ****************************
NLedScena::NLedScena(const char *_name) {
  this->name = new char[strlen(_name) + 1];
  strcpy(this->name, _name);
  this->prev    = NULL;
  this->next    = NULL;
  this->init();
}

NLedScena::NLedScena(const char *_name, uint16_t _iterations):NLedScena(_name) {
  this->iterations = _iterations;
}

/*NLedScena::NLedScena(const char *_name, uint16_t _iterations, uint16_t _startDelay):NLedScena(_name, _iterations) {
  this->startDelay  = _startDelay;  
}*/

NLedScena::~NLedScena() {
  Step *curStep = this->first;
  Step *tmpStep;

  if (curStep != NULL) {
    while (curStep) {
      tmpStep = curStep;
      curStep = curStep->next;
      if (tmpStep->ledFn != NULL) {
        delete tmpStep->ledFn;
      }        
      delete tmpStep;
    }
  }

  delete []name;
}

void NLedScena::init() {
  this->first           = NULL;
  this->last            = NULL;
  this->lastRun         = NULL;
  this->numberSteps     = 0;
  this->iterationCount  = 0;
}

NLedScena::Step::Step() {
  this->prev  = NULL;
  this->next  = NULL;
  this->ledFn = NULL;
}

NLedScena::Step::~Step() {
}

char *NLedScena::name_get() {
  return this->name;
}

boolean NLedScena::enabled_get() {
  return this->enabled;
}

void NLedScena::reset() {
  Step  *cur  = this->first;

  this->lastRun         = NULL;
  this->iterationCount  = 0;
  this->hideAll();

  while (cur) {
    cur->tmrTime = millis();
    
    cur = cur->next;
  }  
}

void NLedScena::enable() {
  if (this->enabled == false) {
    this->enabled = true;
    this->reset();
  }    
}

void NLedScena::disable(boolean _hideAll) {
  Step  *cur  = this->first;
  
  if (this->enabled) {
    this->enabled = false;
    if (_hideAll) {
      this->hideAll();
    }      
    
    while (cur) {
      cur->tmrTime = 0;
      
      cur = cur->next;
    }  
  }    
}

void NLedScena::disable() {
    this->disable(true);
}

void NLedScena::add(NLed *_led, void (NLed::*_fn)()) {
  NLedScena::Step         *step   = new Step();
  NLedScena::Step::LedFn  *ledFn  = new Step::LedFn();
  NLedScena::Step         *cur    = NULL;

  if (_led != NULL) {
    ledFn->led  = _led;
    ledFn->fn   = _fn;
    
    step->type                  = Step::Type::Led;
    step->ledFn                 = ledFn;
    step->delayTime             = 0;
    step->delayTimeAccumulated  = 0;
  
    this->numberSteps++;
  
    if (this->first == NULL) {
      step->prev  = NULL;
      this->first = step;
    } else {
      step->prev  = last;
      last->next  = step;  
    }
    step->next  = NULL;
    this->last  = step;
    
    cur = this->first;
    while (cur) {
      this->last->delayTimeAccumulated += cur->delayTime;
      cur = cur->next;
    }
  }    
}

void NLedScena::addDelay(uint16_t _time) {
  NLedScena::Step         *step = new Step();
  NLedScena::Step         *cur  = NULL;

  step->type                  = NLedScena::Step::Type::Delay;
  step->ledFn                 = NULL;
  step->delayTime             = _time;
  step->delayTimeAccumulated  = 0;

  this->numberSteps++;
  
  if (this->first == NULL) {
    step->prev  = NULL;
    this->first = step;
  } else {
    step->prev  = last;
    last->next  = step;  
  }
  step->next  = NULL;
  this->last  = step;
  
  cur = this->first;
  while (cur) {
    this->last->delayTimeAccumulated += cur->delayTime;
    cur = cur->next;
  }
}

void NLedScena::clear() {
  Step *curStep = this->first;
  Step *tmpStep;

  this->hideAll();
  
  while (curStep) {
    tmpStep = curStep;
    curStep = curStep->next;
    if (tmpStep->ledFn != NULL) {
      delete tmpStep->ledFn;
    }        
    delete tmpStep;
  }

  this->init();
}

void NLedScena::hideAll() {
  Step *cur = this->first;
  
  while (cur) {
    if (cur->type == Step::Type::Led) {
      cur->ledFn->led->off();
    }      
    
    cur = cur->next;
  }  
}

void NLedScena::run() {
  if (this->enabled == false) {
    return;
  }    
  
  NLedScena::Step *cur  = NULL;
  NLedScena::Step *cur2 = NULL;

  cur = (this->lastRun != NULL && this->lastRun->next != NULL) ? this->lastRun->next : this->first;
  while (cur) {
    if ((millis() - cur->tmrTime) >= cur->delayTimeAccumulated) {
      //Serial.println(String((cur->type == cur->Type::Led) ? cur->ledFn->led->name_get() : "Delay") + ": " + String(millis()) + " " + String(cur->tmrTime) + " " + cur->delayTimeAccumulated);

      cur->tmrTime  = millis();
      this->lastRun = cur;

      cur->delayTimeAccumulated  = 0;
      cur2 = this->first;
      while (cur2) {
        cur->delayTimeAccumulated += cur2->delayTime;   
        cur2 = cur2->next;
      }

      switch (cur->type) {
        case cur->Type::Led :
            ((cur->ledFn->led)->*(cur->ledFn->fn))();
          break;        
      }  

      if (this->iterations != SCENA_INFINITY)
      {
        if (cur->next == NULL) {
          this->iterationCount++;
        }

        if (this->iterationCount >= this->iterations) {
          this->disable(false);
          break;
        }
      }
    }
    cur = cur->next;
  }
}

// ********************************* Functions for class NLedScenarios ****************************
NLedScenarios::NLedScenarios() {
  this->number  = 0;
  this->first   = NULL;
  this->last    = NULL;
}

NLedScenarios::~NLedScenarios() {
  NLedScena *cur = this->first;
  NLedScena *tmp;

  if (cur != NULL) {
    while (cur) {
      tmp = cur;
      cur = cur->next;
      delete tmp;
    }
  }
}

uint8_t NLedScenarios::number_get() {
  return this->number;
}

NLedScena *NLedScenarios::add(NLedScena *_scena) {
  this->number++;

  if (this->first == NULL) {
    _scena->prev = NULL;
    this->first = _scena;
  } else {
    _scena->prev  = last;
    last->next    = _scena;  
  }
  _scena->next  = NULL;
  this->last    = _scena;

  return this->last;
}

NLedScena *NLedScenarios::add(const char *_name) {
  NLedScena *scena = new NLedScena(_name);
  return this->add(scena);
}

NLedScena *NLedScenarios::add(const char *_name, uint16_t _iterations) {
  NLedScena *scena = new NLedScena(_name, _iterations);
  return this->add(scena);
}

/*NLedScena *NLedScenarios::add(const char *_name, uint16_t _iterations, uint16_t _startDelay) {
  NLedScena *scena = new NLedScena(_name, _iterations, _startDelay);
  return this->add(scena);
}*/

void NLedScenarios::del(const char *_name) {
  NLedScena *cur = NULL;

  if (strcmp(this->first->name, _name) == 0)
  {
    if (this->last == this->first) {
      cur = this->first;
      this->first = NULL;
      this->last = NULL;
      delete cur;
      this->number--;
    } else if (this->first->next == this->last) {
      cur = this->first;
      this->first = this->last;
      this->first->prev = NULL;
      this->first->next = NULL;
      this->last = this->first;
      delete cur;
      this->number--;
    }
  } else if (strcmp(this->last->name, _name) == 0) {
    cur = this->last;
    cur->prev->next = NULL;
    this->last = cur->prev;
    delete cur;
    this->number--;
  } else {
    cur = this->first;
    while (cur) {
      if (strcmp(cur->name, _name) == 0) {
        cur->prev->next = cur->next;
        cur->next->prev = cur->prev;
        delete cur;
        this->number--;
        break;
      }
      cur = cur->next;
    }
  }    
}

NLedScena *NLedScenarios::scena(uint8_t _num) {
  NLedScena *cur = this->first;
  unsigned char i   = 0;

  while (cur) {
    if (i == _num) {
      break;
    }
    i++;
    cur = cur->next;
  }

  return cur;
}

NLedScena *NLedScenarios::scena(const char *_name) {
  NLedScena *cur = this->first;

  while (cur) {
    if (strcmp(cur->name, _name) == 0) {
      break;
    }
    cur = cur->next;
  }
  
  return cur;
}

void NLedScenarios::loop_run() {
  NLedScena *cur = this->first;
  
  while (cur) {
    if (cur->enabled) {
      cur->run();
    }
    cur = cur->next;
  }
}
