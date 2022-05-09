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
DeskLight deskLight;

void init_lights(void) {
  deskLight.lights[0] = new StripLight(STRIP_PIN, 10);
  deskLight.lights[1] = new SimpleLight(14);
}

bool rotary_dim_mode = true;
#define ROTARY_INCREMENT  20
#define ROTARY_PIN_BUTTON 23
#define ROTARY_PIN_CLK    19
#define ROTARY_PIN_DATA   21
RotaryHandler rotary(ROTARY_PIN_BUTTON, ROTARY_PIN_CLK, ROTARY_PIN_DATA);

CRGB *leds = new CRGB[3];

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

void setup(void) {
  Serial.begin(115200);
  delay(2000);

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
  server.on(UriBraces("/wifi/ssid/{}/password/{}"), []() {
    // request connection to this wifi network
    String uri_ssid = server.pathArg(0);
    String uri_password = server.pathArg(1);
    Serial.println("Switch wifi to " + uri_ssid);
    if ( wifi_connect(uri_ssid, uri_password, hostname, 3000) ) {
      server.send(200, "application/json", "{ \"status\": true }");
      config.set_wifi_ssid(uri_ssid);
      config.set_wifi_password(uri_password);
      config.write();
    }
    else
    {
      Serial.println("FAILED, fallback to access point");
      wifi_ap(ap_ssid, ap_password);
      server.send(404, "application/json", "{ \"status\": false, \"reason\": \"Unable to connect to " + String(uri_ssid) + "\" }");
    }
  });
  // power on/off
  server.on(UriBraces("/power/{}"), []() {
    web_power(server.pathArg(0));
  });

  // Dim a specific light
  server.on(UriBraces("/light/{}/dim/{}"), []() {
    web_light_dim(server.pathArg(0), server.pathArg(1).toInt());
  });

  // Colour a specific light
  server.on(UriBraces("/light/{}/hsv/{}/{}"), []() {
    web_light_hsv(server.pathArg(0), server.pathArg(1).toInt(), server.pathArg(2).toInt());
  });
  
  // start serving
  server.begin();
  Serial.println("HTTP Server started");
  
  rotary.onPress([&deskLight]() -> void { 
    deskLight.set_power(!deskLight.power()); 
  });
  rotary.onLongPress([&deskLight]() -> void {
    rotary_dim_mode = !rotary_dim_mode;
  });
  rotary.onVeryLongPress([&deskLight]() -> void {
    // reset wifi configuration, move to access point
    Serial.println("Switch to access point mode");
    wifi_ap(ap_ssid, ap_password);
  });
  rotary.onRotaryLeft([&deskLight]() -> void {
    Light* l = deskLight.light("StripLight");
    if (rotary_dim_mode)
    {
      uint8_t new_dim = l->dim() <= ROTARY_INCREMENT ? 0 : l->dim() - ROTARY_INCREMENT;
      l->set_dim(new_dim);
    }
    else {
      uint8_t new_hue = l->colour_hsv_hue() - ROTARY_INCREMENT;
      l->set_colour_hsv(new_hue, l->colour_hsv_saturation());
    }
  });
  rotary.onRotaryRight([&deskLight]() -> void {
    Light* l = deskLight.light("StripLight");
    if (rotary_dim_mode)
    {
      uint8_t new_dim = l->dim() >= (255-ROTARY_INCREMENT) ? 255 : l->dim() + ROTARY_INCREMENT;
      l->set_dim(new_dim);
    }
    else {
      uint8_t new_hue = l->colour_hsv_hue() + ROTARY_INCREMENT;
      l->set_colour_hsv(new_hue, l->colour_hsv_saturation());
    }
  });
}

void loop(void) {
  server.handleClient();
  rotary.handle();
  deskLight.loop();
  delay(1);//allow the cpu to switch to other tasks
}
