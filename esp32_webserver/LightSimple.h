#ifndef __LIGHTSIMPLE_H__
#define __LIGHTSIMPLE_H__

#include "LightManager.h"

// simple class light only able to switch on or off
class SimpleLight : public Light {
  public:
    uint8_t _led_pin;
    
    SimpleLight(uint8_t pin) : _led_pin(pin),
                               Light("SimpleLight") {
    }

    void setup(void) {
      pinMode(_led_pin, OUTPUT);
      set_dim(0);
    }
    
    virtual bool set_colour_hsv(uint8_t hue, uint8_t saturation) {
      // not supported
      return false;
    }
    
    virtual bool set_dim(uint8_t dimv) {
      dimv = dimv > 128 ? 255 : 0;
      Light::set_dim(dimv);
      digitalWrite(_led_pin, _dim > 128 ? 1 : 0);
      return true;
    }
};

#endif
