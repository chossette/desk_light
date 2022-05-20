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
      TStripLightProgramFunction loopFunc;
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
      _programs.push_back({ "Fire", 20, &striplight_fire });
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
      _leds[pixel].nscale8(dim());
      
      return true;    
    }
    
    virtual inline uint8_t program_count(void) {
      return static_cast<uint8_t>(_programs.size());
    }

     virtual void loop() {
      // run the progra
      if ((millis() - _last_refresh) > _programs[_program].refresh_delay) {
        _programs[_program].loopFunc(this);
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
  // context
  static uint8_t led_count = 0;
  static uint8_t* heats = 0;
  if (led_count == 0)
  {
    led_count = sl->led_count();
    heats = new uint8_t[led_count];
    memset(heats, 100, led_count * sizeof(uint8_t));
  }
  
  // effect parameters
  static int cooling_speed = 5;
  int hue_shift = 80;
  // shift hue based on heat (cold to left, hot to right)
  uint8_t sparkle_trigger_min_heat = 50;
  uint8_t sparkle_trigger_random_threshold = 90;
  uint8_t sparkle_new_min_heat = 80;

  // set effect
  for (uint16_t i = 0; i < led_count; ++i)
  {
    // 1. cool down
    int heat = qsub8(heats[i], random(cooling_speed));
    
    // 2. sparkle a little !
    if (heat < sparkle_trigger_min_heat && random(100) > sparkle_trigger_random_threshold) {
      heat = random(sparkle_new_min_heat, 100);
    }
    
    // 3. diffuse left and right
    uint8_t lled = (i == 0) ? led_count-1 : i-1;
    uint8_t rled = (i == led_count-1) ? 0 : i+1;
    heat = (heats[lled] + heats[rled] + 2*heat) / 4;
    uint8_t val = ((uint16_t)heat * sl->dim() / 100);
    
    // 4. shift a bit the hue, based on the heat value
    int hue_shift_value = 0;
    int hue = sl->colour_hsv_hue();
    // rescale
    int heat_scale = ((heat - 50) / 50.) * hue_shift;
    hue = hue + heat_scale;
    if (hue < 0) hue = 255 - hue;
    if (hue > 255) hue = hue - 255;

    // update
    heats[i] = heat;
    sl->set_colour_hsv_pixel(i, hue, sl->colour_hsv_saturation(), val);
  }  
}

#endif
