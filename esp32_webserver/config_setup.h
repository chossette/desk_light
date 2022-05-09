#include <EEPROM.h>

// EEPROM STORAGE (512 bytes max)
// WIFI SSID     32 bytes max length: https://serverfault.com/questions/45439/what-is-the-maximum-length-of-a-wifi-access-points-ssid
// WIFI PASSWORD 64 bytes max length (WPA2-PSK)
// WIFI HOSTNAME 96 bytes max (why not ?)
#define EEPROM_SIZE                 512
#define EEPROM_MAGIC_VALUE          0xDEAD
#define EEPROM_WIFI_SSID_LENGTH     32
#define EEPROM_WIFI_PASSWORD_LENGTH 64
#define EEPROM_WIFI_HOSTNAME_LENGTH 96

class Config {
  private:
    struct Eeprom_Config {
      uint16_t magic;
      char wifi_ssid[EEPROM_WIFI_SSID_LENGTH];
      char wifi_password[EEPROM_WIFI_PASSWORD_LENGTH];
      char wifi_hostname[EEPROM_WIFI_HOSTNAME_LENGTH];
    } _config;

    bool _init;
 
  public:
    Config(void): _init(false) {
    }

    void setup(void) {
      // init the eeprom
      EEPROM.begin(EEPROM_SIZE);
    }

    inline bool is_init(void) {
      return (_config.magic == EEPROM_MAGIC_VALUE);
    }

    void reset(void) {
      _config.magic = 0x00;
      EEPROM.put(0, _config);
      EEPROM.commit();
      _init = false;
    }
    
    void read(void) {
      EEPROM.get(0, _config);
    }

    void write(void) {
      _config.magic = EEPROM_MAGIC_VALUE;
      EEPROM.put(0, _config);
      EEPROM.commit();
      _init = true;
    }

    inline bool set_wifi_ssid(String s) {
      if (s.length() >= EEPROM_WIFI_SSID_LENGTH) return false;
      memcpy((void*)_config.wifi_ssid, (void*)s.c_str(), s.length());
      _config.wifi_ssid[s.length()] = '\0'; // set string terminator
    }
    inline String wifi_ssid(void) {
      if (! is_init()) return String();        
      return String(_config.wifi_ssid);
    }

    inline bool set_wifi_password(String s) {
      if (s.length() >= EEPROM_WIFI_PASSWORD_LENGTH) return false;
      memcpy((void*)_config.wifi_password, (void*)s.c_str(), s.length());
      _config.wifi_password[s.length()] = '\0'; // set string terminator
    }
    inline String wifi_password(void) {
      if (! is_init()) return String();        
      return String(_config.wifi_password);
    }
    
    inline bool set_wifi_hostname(String s) {
      if (s.length() >= EEPROM_WIFI_HOSTNAME_LENGTH) return false;
      memcpy((void*)_config.wifi_hostname, (void*)s.c_str(), s.length());
      _config.wifi_hostname[s.length()] = '\0'; // set string terminator
    }
    inline String wifi_hostname(void) {
      if (! is_init()) return String();        
      return String(_config.wifi_hostname);
    }
};
