void web_power(String sname, String value) {
    if (value != "on" && value != "off") {
        server.send(400, "application/json", "{ \"status\": false, \"reason\": \"Allowed value are on|off\" }");
        return;
    }
    bool power_value = (value == "on");

    Light *l = 0;
    if (sname.length() != 0) {
      l = deskLight.light(sname);
      if (l == 0) {
        server.send(404, "application/json", "{ \"status\": false, \"reason\": \"unknown light " + sname + "\" }");
        return;
      }
    }

    bool power_res = true;
    if (l) {
      power_res = l->set_power(power_value);
    }
    else {
      power_res = deskLight.set_power(power_value);
    }
    if (power_res) {
      server.send(200, "application/json", "{ \"status\": true }");
    } else {
      server.send(501, "application/json", "{ \"status\": false, \"reason\": \"not implemented\" }");
    }
}

void web_light(String sname, String shue, String sdim, String ssat) {
  Light *l = 0;
  if (sname.length() != 0) {
    l = deskLight.light(sname);
    if (l == 0) {
      server.send(404, "application/json", "{ \"status\": false, \"reason\": \"unknown light " + sname + "\" }");
      return;
    }
  }

  String response;
  // setting hue
  if (shue.length() != 0) {
    uint8_t hue = shue.toInt();
    if (l) {
      l->set_colour_hue(hue);
    }
    else {
      deskLight.set_colour_hue(hue);
    }
    response += "\"hue\": " + String(hue);
  }
  
  // setting saturation
  if (ssat.length() != 0) {
    uint8_t sat = ssat.toInt();
    if (l) {
      l->set_colour_saturation(sat);
    }
    else {
      deskLight.set_colour_saturation(sat);
    }
    if (response.length() != 0) {
      response += ", ";
    }
    response += "\"saturation\": " + String(sat);
  }
  
  // setting dim (brightness)
  if (sdim.length() != 0) {
    uint8_t dim = sdim.toInt();
    if (l) {
      l->set_dim(dim);
    }
    else {
      deskLight.set_dim(dim);
    }
    if (response.length() != 0) {
      response += ", ";
    }
    response += "\"dim\": " + String(dim);
  }

  // add status
  if (response.length() != 0) {
    response += ", ";
  }
  response += "\"status\": true";
  Serial.println(String("Response: ") + response);
  server.send(200, "application/json", String("{ ") + response + " }");
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
