#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FastLED.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>

AsyncWebServer server(80);

// Set Access Point creds
#ifndef AP_SSID
#define AP_SSID "trix-sims"
#define AP_PSK "plumbob-69420"
#endif

char ssid[] = AP_SSID;
char password[] = AP_PSK;

#define CONTROL_PIN D1

// TODO: send slider input from Blynk App? to esp32
// TODO: set up client for phones
// TODO: add button for power on/off
// TODO: get slider input from slider component
// TODO: add brightness input
// TODO: add effects? (like spinning diamond)

const int LED_COUNT = 14;

uint8_t slider_val = 120;
uint8_t num_clients = 0;

const char* SLIDER_INPUT = "slider";
bool powered_on = true;

CRGBArray<LED_COUNT> leds;
uint8_t brightness = 100;

void handleHome(AsyncWebServerRequest *req) {
  digitalWrite(CONTROL_PIN, HIGH);
  req->send(200, "text/plain", "Welcome!\r\n");
  digitalWrite(CONTROL_PIN, LOW);
}

void handleNotFound(AsyncWebServerRequest *req) {
  digitalWrite(CONTROL_PIN, HIGH);
  req->send(404, "text/plain", "Page does not exist");
  digitalWrite(CONTROL_PIN, LOW);
}

void powerOn() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB(0 + slider_val, 255 - slider_val, 0);
  }  
  FastLED.show();
  delay(500);
}

void powerOff() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB::Black;
  }  
  FastLED.show();
  delay(500);
}

void handlePowerState(AsyncWebServerRequest *req) {
  // TODO: turn on LEDs via button
  if (!powered_on) {
    powerOff();
  } else {
    // TODO: Turn off LEDs via button
    powerOn();
  }
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>The Sims Plumbob LED Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
  </head>
  <body>
    <h2>The Sims Plumbob LED Controller</h2>
    <form action="/get">
      <input type="range" name="slider" min="0" max="255" value="0">
      <label for="slider">Slider</label><br>

      <span name="num_clients"></span>
      <label for="num_clients">Clients: </label>
    </form><br>
  <script>
    var xhr = new XMLHttpRequest();
  </script>
  </body>
</html>)rawliteral";
 
void setup() {
  pinMode(CONTROL_PIN, OUTPUT);
  Serial.begin(9600);
  LEDS.setBrightness(brightness);
  LEDS.addLeds<WS2812B, CONTROL_PIN, GRB>(leds, LED_COUNT);
  LEDS.show();
  delay(500);
  Serial.println('\n');

  WiFi.softAP(AP_SSID, AP_PSK, 1, false, 2);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address: \t");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", index_html);
  });

  server.on("/power", handlePowerState);

  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *req){
    // slider_val = req->getParam("slider")->value().toInt();
    String val;
    String param;
    val = req->getParam(SLIDER_INPUT)->value();
    param = SLIDER_INPUT;

    Serial.println(val);
    Serial.println(param);
  });

  server.on("/test", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send(200, "text/plain", "test");
    Serial.print("slider_val: ");
    Serial.println(slider_val);
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started\n");
}

void loop() {
  powerOn();   

  int connections = WiFi.softAPgetStationNum();
  Serial.println(connections);
  Serial.println(slider_val);
  delay(1000);
}

// Ref: https://randomnerdtutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/