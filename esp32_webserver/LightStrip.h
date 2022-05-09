#ifndef __LIGHTSTRIP_H__
#define __LIGHTSTRIP_H__

#include "LightManager.h"
#include <FastLED.h>
#include <vector>

#define STRIPLIGHT_DEFAULT_LED_UPDATE_TIMEOUT 10

class StripLight;
void striplight_fire(StripLight *sl);
void striplight_static(StripLight *sl);

// StripLight (ws2812)
class StripLight : public Light {
  public:
    uint8_t   _led_pin;
    uint8_t   _led_count;
    uint32_t  _last_refresh;
    uint32_t  _led_update_timeout;

    CRGB*   _leds;

    // program handling
    typedef std::function<void(StripLight*)> TStripLightProgramFunction;
    struct StripLightProgram {
      String name;
      int refresh_delay;
      TStripLightProgramFunction exec;
    };
    std::vector<struct StripLightProgram> _programs;
    
    // init !
    StripLight(uint8_t pin, uint16_t count) : _led_pin(pin),
                                              _led_count(count),
                                              _last_refresh(0),
                                              _led_update_timeout(STRIPLIGHT_DEFAULT_LED_UPDATE_TIMEOUT),
                                              Light("StripLight") {
    }

    void setup(void) {
      pinMode(_led_pin, OUTPUT);
      _leds = new CRGB[_led_count];

      switch (_led_pin)
      {
        case 35:
          FastLED.addLeds<WS2812, 35, RGB>(_leds, _led_count);
          break;
        case 13:
          FastLED.addLeds<WS2812, 13, RGB>(_leds, _led_count);
          break;
        default:
          // unsupported pin implementation for this strip
          Serial.println("Unsupported pin for StripLight");
          break;
      }
      set_dim(0);

      // register program
      _programs.push_back({ "Static", 20, &striplight_static });
      _programs.push_back({ "Fire", 15, &striplight_fire });
    }

    inline uint8_t led_count() {
      return _led_count;
    }

    // set a specific led colour by hsv
    inline bool set_colour_hsv_pixel(uint8_t pixel, uint8_t h, uint8_t s, uint8_t b) {
      if (pixel >= _led_count) {
        return false;
      }
      _leds[pixel] = CHSV(h, s, b);
      
      return true;    
    }
    
    // set a specific led colour by rgb
    inline bool set_colour_rgb_pixel(uint8_t pixel, uint8_t r, uint8_t g, uint8_t b) {
      if (pixel >= _led_count) {
        return false;
      }
      _leds[pixel] = CRGB(r, g, b);
      
      return true;    
    }

     virtual void loop() {
      // run the progra
      if ((millis() - _last_refresh) > _programs[_program].refresh_delay) {
        FastLED.show();
        _last_refresh = millis();
      }        
    }
};

void striplight_static(StripLight *sl) {
  static uint8_t led_count = 0;
  if (led_count == 0)
  {
    led_count = sl->led_count();
  }
  
  // set color to all led
  for (uint16_t i = 0; i < led_count; ++i)
  {
    sl->set_colour_hsv_pixel(i, sl->colour_hsv_hue(), sl->colour_hsv_saturation(), sl->dim());
  }    
}

void striplight_fire(StripLight *sl) {
  // get from https://www.tweaking4all.com/hardware/arduino/adruino-led-strip-effects/
  static int cooling = 55;
  static int sparking = 120;
  
  static byte *heat = 0;
  static uint8_t led_count = 0;
  if (heat == 0) {
    led_count = sl->led_count();
    heat = new byte[led_count];
  }
  
  int cooldown = 0;
  // Step 1.  Cool down every cell a little
  for( int i = 0; i < led_count; i++) {
    cooldown = random(0, ((cooling * 10) / led_count) + 2);
    if(cooldown>heat[i]) {
      heat[i]=0;
    } else {
      heat[i]=heat[i]-cooldown;
    }
  }
 
  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for( int k= led_count - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2]) / 3;
  }
   
  // Step 3.  Randomly ignite new 'sparks' near the bottom
  if( random(255) < sparking ) {
    int y = random(7);
    heat[y] = heat[y] + random(160,255);
  }

  // Step 4.  Convert heat to LED colors
  for( int j = 0; j < led_count; j++) {
    int temperature = heat[j];
    // Scale 'heat' down from 0-255 to 0-191
    byte t192 = round((temperature/255.0)*191);
    
    // calculate ramp up from
    byte heatramp = t192 & 0x3F; // 0..63
    heatramp <<= 2; // scale up to 0..252
    
    // figure out which third of the spectrum we're in:
    if( t192 > 0x80) {                     // hottest
      sl->set_colour_rgb_pixel(j, 255, 255, heatramp);
    } else if( t192 > 0x40 ) {             // middle
      sl->set_colour_rgb_pixel(j, 255, heatramp, 0);
    } else {                               // coolest
      sl->set_colour_rgb_pixel(j, heatramp, 0, 0);
    }
  }
}

#endif
