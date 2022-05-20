#ifndef __ROTARYHANDLER_H__
#define __ROTARYHANDLER_H__

#include <Button2.h>

class RotaryHandler
{
  public:
    typedef std::function<void(void)> THandlerFunction;
    
    // long press configuration
    uint32_t _long_press_timeout = 500;
    uint32_t _very_long_press_timeout = 3000;
    
    RotaryHandler(uint8_t button_pin,
                  uint8_t rotary_clock_pin,
                  uint8_t rotary_data_pin): _pin_button(button_pin),
                                            _pin_rotary_clock(rotary_clock_pin),
                                            _pin_rotary_data(rotary_data_pin),
                                            _prevNextCode(0) {
    }

    void setup(void) {
      pinMode(_pin_button, INPUT);
      pinMode(_pin_button, INPUT_PULLUP);
      pinMode(_pin_rotary_clock, INPUT);
      pinMode(_pin_rotary_clock, INPUT_PULLUP);
      pinMode(_pin_rotary_data, INPUT);
      pinMode(_pin_rotary_data, INPUT_PULLUP);

      _button.begin(_pin_button);
      _button.setLongClickTime(_long_press_timeout);
      _button.setClickHandler(std::bind(&RotaryHandler::_button2_ontap, this, std::placeholders::_1));
      _button.setLongClickHandler(std::bind(&RotaryHandler::_button2_onlongclick, this, std::placeholders::_1));
    }

    void onPress(THandlerFunction t) {
      _onPressFunc = t;  
    };
    void onLongPress(THandlerFunction t) {
      _onLongPressFunc = t;  
    };
    void onVeryLongPress(THandlerFunction t) {
      _onVeryLongPressFunc = t;  
    };

    void onRotaryLeft(THandlerFunction t) {
      _onRotaryLeftFunc = t;
    };
    
    void onRotaryRight(THandlerFunction t) {
      _onRotaryRightFunc = t;
    };

    void handle() {
      // check rotary action
      if ( read_rotary() ) {
        if ( (_prevNextCode&0x0f)==0x0b) {
          _onRotaryLeft();    
        }
        if ( (_prevNextCode&0x0f)==0x07) {
          _onRotaryRight();
        }
      }

      _button.loop();
    }
    
  private:
    // hardware definition
    uint8_t _pin_button;
    uint8_t _pin_rotary_clock;
    uint8_t _pin_rotary_data;

    // hardware reading
    uint8_t _prevNextCode;
    Button2  _button;


    // user callback
    THandlerFunction _onPressFunc;
    THandlerFunction _onLongPressFunc;
    THandlerFunction _onVeryLongPressFunc;
    THandlerFunction _onRotaryLeftFunc;
    THandlerFunction _onRotaryRightFunc;

    // redirect the Button2 class callback
    void _button2_ontap(Button2&) {
      _onPress();
    }
    void _button2_onlongclick(Button2& btn) {
      if (btn.wasPressedFor() >= _very_long_press_timeout)
        _onVeryLongPress();
      else
        _onLongPress();
    }
    
    // action feedback
    void _onPress() {
      if (_onPressFunc) {
        _onPressFunc();
      }
    }
    void _onLongPress() {
      if (_onLongPressFunc) {
        _onLongPressFunc();
      }
    }
    void _onVeryLongPress() {
      if (_onVeryLongPressFunc) {
        _onVeryLongPressFunc();
      }
    }
    void _onRotaryLeft() {
      if (_onRotaryLeftFunc) {
        _onRotaryLeftFunc();
      }
    }
    void _onRotaryRight() {
      if (_onRotaryRightFunc) {
        _onRotaryRightFunc();
      }
    }
    
    // rotary data reading
    int8_t read_rotary() {
      static int8_t rot_enc_table[] = {0,1,1,0,1,0,0,1,1,0,0,1,0,1,1,0};
    
      _prevNextCode <<= 2;
      if (digitalRead(_pin_rotary_data)) _prevNextCode |= 0x02;
      if (digitalRead(_pin_rotary_clock)) _prevNextCode |= 0x01;
      _prevNextCode &= 0x0f;
    
      return ( rot_enc_table[( _prevNextCode & 0x0f )]);
    }
};

#endif
