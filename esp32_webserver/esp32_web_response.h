void web_power(String value) {
    if (value != "on" && value != "off") {
        server.send(400, "application/json", "{ \"status\": false, \"reason\": \"Allowed value are on|off\" }");
        return;
    }
    bool power_value = (value == "on");
    if (deskLight.set_power(power_value)) {
      server.send(200, "application/json", "{ \"status\": true }");
    } else {
      server.send(501, "application/json", "{ \"status\": false, \"reason\": \"not implemented\" }");
    }
}

void web_light_dim(String name, int value) {
    Light *l = deskLight.light(name);
    if (l == 0) {
      server.send(404, "application/json", "{ \"status\": false, \"reason\": \"unknown light " + name + "\" }");
      return;
    }
    if (value < 0 || value > 255) {
      server.send(400, "application/json", "{ \"status\": false, \"reason\": \"unvalid value [0;255], value " + String(value) + "\" }");
      return;
    }
    
    if (l->set_dim(value)) {
      server.send(200, "application/json", "{ \"status\": true }");
    } else {
      server.send(501, "application/json", "{ \"status\": false, \"reason\": \"not implemented\" }");
    }
}

void web_light_hsv(String name, int hue_color, int hue_saturation) {
    Light *l = deskLight.light(name);
    if (l == 0) {
      server.send(404, "application/json", "{ \"status\": false, \"reason\": \"unknown light " + name + "\" }");
      return;
    }
    if (hue_color < 0 || hue_color > 255 || hue_saturation < 0 || hue_saturation > 255) {
      server.send(400, "application/json", "{ \"status\": false, \"reason\": \"unvalid value [0;255], hue: " + String(hue_color) + ", saturation: " + String(hue_saturation) + "\" }");
      return;
    }
    if (l->set_colour_hsv(hue_color, hue_saturation)) {
      server.send(200, "application/json", "{ \"status\": true }");
    } else {
      server.send(501, "application/json", "{ \"status\": false, \"reason\": \"not implemented\" }");
    }
}

void web_status() {
  server.send(200, "application/json", deskLight.status());
}
