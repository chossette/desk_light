#ifndef __LIGHTMANAGER_H__
#define __LIGHTMANAGER_H__

class Light {
  public:
    String   _name;
    uint8_t  _colour_hsv_hue;
    uint8_t  _colour_hsv_saturation;
    uint8_t  _dim;
    uint8_t  _program;

    // transition from current stat to target value
    // for a defined time
    struct Transition {
      bool     active = false;
      uint32_t start_ms;
      uint32_t duration_ms = 150;
      uint8_t  v_start;
      uint8_t  v_target;
    };
    struct Transition _transition_hue;
    struct Transition _transition_sat;
    struct Transition _transition_dim;

  public:
    Light(String n) : _name(n),
                      _colour_hsv_hue(0), 
                      _colour_hsv_saturation(255),
                      _dim(0), 
                      _program(0) {
    }

    ~Light() {
      set_dim(0);
    }

    virtual void setup(void) {
      set_dim(0);      
    }

    void transition_stop(void) {
      _transition_hue.active = false;
      _transition_sat.active = false;
      _transition_dim.active = false;
    }

    inline uint8_t colour_hsv_hue(void)         { return _colour_hsv_hue; }
    inline uint8_t colour_hsv_saturation(void)  { return _colour_hsv_saturation; }
    inline uint8_t dim(void)                    { return _dim; }
    
    virtual inline bool set_colour_hue(uint8_t hue, bool fade = false) {
      if (fade) {
        set_transition(&_transition_hue, _colour_hsv_hue, hue);
      }
      else {
        _colour_hsv_hue = hue;
      }
      return true;    
    }
    virtual inline bool set_colour_saturation(uint8_t sat, bool fade = false) {
      if (fade) {
        set_transition(&_transition_sat, _colour_hsv_saturation, sat);
      }
      else {
        _colour_hsv_saturation = sat;
      }
      return true;    
    }
    
    virtual bool set_colour_hsv(uint8_t hue, uint8_t saturation, bool fade = false) {
      set_colour_hue(hue, fade);
      set_colour_saturation(saturation, fade);
      return true;    
    }
    
    virtual inline bool set_dim(uint8_t dimv, bool fade = false) {
      if (fade) {
        set_transition(&_transition_dim, _dim, dimv);
      }
      else {
        _dim = dimv;
      }
      return true;
    }
    
    virtual bool set_power(bool value, bool animated = false) {
      uint8_t dim = value ? 255 : 0;
      return set_dim(dim, animated);
    }
    
    virtual bool set_program(uint8_t prg) {
      // stop all transition when moving to a program
      transition_stop();
      return false;
    }

    virtual inline uint8_t program(void) {
      return _program;
    }
    
    virtual inline uint8_t program_count(void) {
      return 0;
    }

    void set_transition(struct Transition *t, uint8_t v_start, uint8_t v_target) {
        t->active = true;
        t->start_ms = millis();
        t->v_start = v_start;
        t->v_target = v_target;
    }

    void apply_transition(uint32_t now_ms, uint8_t* target_value, struct Transition *t) {      
      if (t->active) {
        // remaining time for transition
        int32_t percent_done = (now_ms - t->start_ms) * 100 / t->duration_ms;
        // no more time !
        if (percent_done >= 100) {
          t->active = false;
          *target_value = t->v_target;
        }
        else
        {
          // range
          int16_t percent_new_value = percent_done;
          if (t->v_target < t->v_start) {
            // move back, inverse percent done
            percent_new_value = 100 - percent_done;
          }
          int16_t shift = ((t->v_target - t->v_start) * percent_done) / 100;
          *target_value = (abs(t->v_target - t->v_start) * percent_new_value) / 100;
        }
      }
    }
    virtual void loop() {
      uint32_t now_ms = millis();
      apply_transition(now_ms, &_dim, &_transition_dim);
      apply_transition(now_ms, &_colour_hsv_hue, &_transition_hue);
      apply_transition(now_ms, &_colour_hsv_saturation, &_transition_sat);
    }

    String status(void) {
      String s = String("{");
      s += String("\"name\": \"") + String(_name) + String("\",");
      s += String("\"dim\": ") + String(_dim) + String(",");
      s += String("\"hsv\": { \"hue\": ") + String(_colour_hsv_hue) + String(", \"saturation\": ") + String(_colour_hsv_saturation) + String("},");
      s += String("\"program\": { \"id\": ") + String(_program) + String(", \"count\": ") + String(program_count()) + String("} }");
      return s;
    }
};

constexpr uint8_t light_count = 2;
class DeskLight {
  public:
    Light *lights[light_count] = { 0, 0 };

  public:
    DeskLight() {
    }

    ~DeskLight() {
      for (uint8_t i = 0; i < light_count; ++i) {
        delete lights[i];
        lights[i] = 0;
      }
    }

    void setup(void) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->setup();
      }      
    }

    bool power() {
      int powered_count = 0;
      for (uint8_t i = 0; i < light_count; ++i) {
        if (lights[i] && lights[i]->_dim > 0) ++powered_count;
      }
      return (powered_count > 0);
    }
    
    bool set_power(bool value, bool animated = false) {
      uint8_t dim = value ? 255 : 0;
      return set_dim(dim, animated);
    }
    
    bool set_dim(uint8_t dim, bool animated = false) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->set_dim(dim, animated);
      }
      return true;
    }
    
    bool set_colour_hue(uint8_t hue, bool animated = false) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->set_colour_hue(hue, animated);
      }
      return true;
    }
    
    bool set_colour_saturation(uint8_t saturation, bool animated = false) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->set_colour_saturation(saturation, animated);
      }
      return true;
    }
    
    bool set_colour_hsv(uint8_t hue, uint8_t saturation, bool animated = false) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->set_colour_hsv(hue, saturation, animated);
      }
      return true;
    }

    // return common program if same on all light
    // otherwise -1
    short program(void) {
      short common_program = -1;
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        if (common_program == -1)
        {
          common_program = lights[i]->program();
        }
        else if (lights[i]->program() != common_program)
        {
          return -1;
        }
        return common_program;
      }
    }

    // set same program on all light
    bool set_program(uint8_t prg) {
      for (uint8_t i = 0; i < light_count; ++i)
      {
        if (!lights[i]) continue;
        lights[i]->set_program(prg);
      }
      return true;
    }

    // increment program (and rotate)
    bool program_inc(short increment) {
      Serial.println(String("Program_inc " ) + increment);
      for (uint8_t i = 0; i < light_count; ++i)
      {
        if (!lights[i]) continue;
        short prg = lights[i]->program();
        short prg_cnt = lights[i]->program_count();
        if (prg_cnt > 0)
        {
          Serial.println(lights[i]->status());
          short prg_next = prg + increment;
          if (prg_next < 0) 
          {
            prg_next = prg_cnt + prg_next;
          }
          else if (prg_next >= prg_cnt)
          {
            prg_next = prg_next - prg_cnt; 
          }
          Serial.print("Increment is ");
          Serial.print(increment);
          Serial.print(" next is ");
          Serial.println(lights[i]->status());
          Serial.println(prg_next);
          lights[i]->set_program(prg_next);
          Serial.println(lights[i]->status());
        }
      }
      return true;
    }
    
    String status(void) {
      String status("{ \"lights\": [");
      uint16_t status_count = 0;
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        if (status_count) status += String(", ");
        status += lights[i]->status();
        ++status_count;
      }
      status += String("]}");
      return status; 
    }

    Light* light(String name) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        if (lights[i]->_name == name) return lights[i];
      }
      return 0;
    }

    void loop() {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->loop();
      }
    }
};

#endif
