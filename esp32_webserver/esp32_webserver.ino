#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <uri/UriBraces.h>
#include <uri/UriRegex.h>

// TODO
// - implement a serial protocol to configure ssid/password/hostname
//   and avoid hardcoding of these


const char* ap_ssid     = "MyLightWelcome";
const char* ap_password = "hellow_world";
const char* hostname    = "DeskLight"; 

// const char* ssid     = "Livebox-C18E";          // 12 caracteres  up to 32 
// const char* password = "pDVWCXJoAor5c5wgdF";    // 18 caracteres  up to 64


WebServer server(80);

#include "LightStrip.h"
#include "LightSimple.h"
#include "RotaryHandler.h"
#include "config_setup.h"

Config config;

#define STRIP_PIN 13
#define STRIP_LED_COUNT 35
DeskLight deskLight;

void init_lights(void) {
  deskLight.lights[0] = new StripLight(STRIP_PIN, STRIP_LED_COUNT);
  deskLight.lights[1] = new SimpleLight(14);
}

#define ROTARY_MODE_COUNT 3
uint8_t rotary_dim_mode = 0;

#define ROTARY_INCREMENT  20
#define ROTARY_PIN_BUTTON 23
#define ROTARY_PIN_CLK    19
#define ROTARY_PIN_DATA   21
RotaryHandler rotary(ROTARY_PIN_BUTTON, ROTARY_PIN_CLK, ROTARY_PIN_DATA);

#include "esp32_web_response.h"

void wifi_disconnect(void) {
  if (WiFi.status() == WL_CONNECTED)
    WiFi.disconnect();
}
bool wifi_ap(String ssid, String password) {
  wifi_disconnect();
  
  Serial.print("Setting wifi access point: ");
  Serial.println(ssid);
  
  if (!WiFi.softAP(ssid.c_str(), password.c_str())) return false;
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  return true;
}

bool wifi_connect(String ssid, String password, String hostname, uint32_t timeout) {
  wifi_disconnect();
  
  Serial.print("Connecting to wifi access point: ");
  Serial.println(ssid);
    
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid.c_str(), password.c_str());
    
  // Wait for connection
  uint64_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    if (millis() - start > timeout) {
      Serial.println("TIMEOUT");
      return false;
    }
    delay(500);
    Serial.print(".");
  }
  
  if (MDNS.begin(hostname.c_str())) {
    Serial.println("MDNS responder started");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  
  return true;
}


  /* Attempt to get http server performance
   * By running on the other unused core
TaskHandle_t webServerLoopHandle;
void webServerLoop(void *params);
*/

void setup(void) {
  Serial.begin(115200);
  
  delay(500);

  // light setup
  Serial.println("Init lights");
  init_lights();  
  deskLight.setup();

  // rotary setup
  rotary.setup();

  // configuration setup
  config.setup();
  config.read();

  // wifi setup
  if (config.is_init()) {
    wifi_connect(config.wifi_ssid(), config.wifi_password(), config.wifi_hostname(), 3000);
  }
  // if connection failed, then switch to AccessPoint
  if (WiFi.status() != WL_CONNECTED) {
    wifi_ap(ap_ssid, ap_password);
  }

  // webserver setup
  // Futur web page for interacting
  server.on(F("/"), []() {
    server.send(200, "text/plain", "Make the light ! IP is " + String(WiFi.localIP()));
  });

  // return status
  server.on(F("/status"), web_status);

  // Route for action
  // wifi configuration
  server.on("/wifi", HTTP_GET, []() {
    // get ssif and password arguments
    String wifi_ssid = server.arg("ssid");
    String wifi_password = server.arg("password");
    
    // and check validity of parameters
    if (wifi_ssid.length() == 0 || wifi_password.length() == 0) {
      server.send(400, "text/plain", "Bad request: ssid and password args must be defined");
      return;
    }
    
    if ( wifi_connect(wifi_ssid, wifi_password, hostname, 3000) ) {
      server.send(200, "application/json", "{ \"status\": true }");
      config.set_wifi_ssid(wifi_ssid);
      config.set_wifi_password(wifi_password);
      config.write();
    }
    else
    {
      Serial.println("FAILED, fallback to access point");
      wifi_ap(ap_ssid, ap_password);
      server.send(404, "application/json", "{ \"status\": false, \"reason\": \"Unable to connect to " + String(wifi_ssid) + "\" }");
    }
  });

  // power on/off
  server.on(UriBraces("/power/{}"), []() {
    // supported args:
    // - opt name: for a specific light
    web_power(server.arg("name"), server.pathArg(0));
  });

  // adjust light
  server.on("/light", []() {
    // supported args:
    // - opt name: for a specific light
    // - opt hue: to define hue
    // - opt dim: to define brightness
    // - opt sat: to define saturation
    // at least one of hue/dim/sat shall be defined
    String name = server.arg("name");
    String hue = server.arg("hue");
    String dim = server.arg("dim");
    String sat = server.arg("sat");

    // badly formed request
    if (name.length() == 0 && hue.length() == 0 && dim.length() == 0 && sat.length() == 0) {
      server.send(400, "text/plain", "Bad request: hue, dim or sat must be defined (at least one of them)");
    }

    web_light(name, hue, dim, sat);
  });
  
  // start serving
  server.begin();
  Serial.println("HTTP Server started");
  
  rotary.onPress([&deskLight]() -> void { 
    deskLight.set_power(!deskLight.power()); 
  });
  rotary.onLongPress([&deskLight]() -> void {
    Serial.print("Rotary_dim_mode: ");
    Serial.print((short)rotary_dim_mode);
    rotary_dim_mode = (rotary_dim_mode + 1) % ROTARY_MODE_COUNT;
    Serial.print(" -> ");
    Serial.println((short)rotary_dim_mode);
  });
  rotary.onVeryLongPress([&deskLight]() -> void {
    // reset wifi configuration, move to access point
    Serial.println("Switch to access point mode");
    wifi_ap(ap_ssid, ap_password);
  });
  rotary.onRotaryLeft([&deskLight]() -> void {
    Light* l = deskLight.light("StripLight");
    if (rotary_dim_mode == 0)
    {
      uint8_t new_dim = l->dim() <= ROTARY_INCREMENT ? 0 : l->dim() - ROTARY_INCREMENT;
      l->set_dim(new_dim);
    }
    else if (rotary_dim_mode == 1) {
      uint8_t new_hue = l->colour_hsv_hue() - ROTARY_INCREMENT;
      l->set_colour_hsv(new_hue, l->colour_hsv_saturation());
    }
    else if (rotary_dim_mode == 2) {
      deskLight.program_inc(-1);
    }
  });
  rotary.onRotaryRight([&deskLight]() -> void {
    Light* l = deskLight.light("StripLight");
    if (rotary_dim_mode == 0)
    {
      uint8_t new_dim = l->dim() >= (255-ROTARY_INCREMENT) ? 255 : l->dim() + ROTARY_INCREMENT;
      l->set_dim(new_dim);
    }
    else if (rotary_dim_mode == 1)
    {
      uint8_t new_hue = l->colour_hsv_hue() + ROTARY_INCREMENT;
      l->set_colour_hsv(new_hue, l->colour_hsv_saturation());
    }
    else if (rotary_dim_mode == 2) {
      deskLight.program_inc(+1);
    }
  });
}

void loop(void) {
  server.handleClient();
  rotary.handle();
  deskLight.loop();
  delay(1);//allow the cpu to switch to other tasks
}
