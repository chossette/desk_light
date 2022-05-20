#ifndef __LIGHTMANAGER_H__
#define __LIGHTMANAGER_H__

class Light {
  public:
    String   _name;
    uint8_t  _colour_hsv_hue;
    uint8_t  _colour_hsv_saturation;
    uint8_t  _dim;
    uint8_t  _program;

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

    inline uint8_t colour_hsv_hue(void)         { return _colour_hsv_hue; }
    inline uint8_t colour_hsv_saturation(void)  { return _colour_hsv_saturation; }
    inline uint8_t dim(void)                    { return _dim; }
    
    virtual bool set_colour_hsv(uint8_t hue, uint8_t saturation) {
      _colour_hsv_hue = hue;
      _colour_hsv_saturation = saturation;
      return true;    
    }
    
    virtual bool set_dim(uint8_t dimv) {
      _dim = dimv;
      return true;
    }
    
    virtual bool set_program(uint8_t prg) {
      _program = prg;
      if (_program == 0) set_dim(0);
      return false;
    }

    virtual inline uint8_t program(void) {
      return _program;
    }
    
    virtual inline uint8_t program_count(void) {
      return 0;
    }

    virtual void loop() {
      ;
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
    
    bool set_power(bool value) {
      uint8_t dim = value ? 255 : 0;
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->set_dim(dim);
      }
      return true;
    }
    
    bool set_colour_hsv(uint8_t hue, uint8_t saturation) {
      for (uint8_t i = 0; i < light_count; ++i) {
        if (!lights[i]) continue;
        lights[i]->set_colour_hsv(hue, saturation);
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
