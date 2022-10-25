#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <FastLED.h>

AsyncWebServer server(80);

// Set Access Point creds
#ifndef AP_SSID
#define AP_SSID "trix-sims"
#define AP_PSK "plumbob-69420"
#endif

char ssid[] = AP_SSID;
char password[] = AP_PSK;

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

#define CONTROL_PIN D1

// TODO: add effects? (like spinning diamond)
// TODO: add label for number of clients

const int LED_COUNT = 13;

uint8_t color_val = 0;
uint8_t num_clients = 0;
uint8_t brightness = 100;

bool poweredOn = true;

CRGBArray<LED_COUNT> leds;

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
    leds[i] = CRGB(0 + color_val, 255 - color_val, 0);
  }  
  FastLED.show();
  delay(50);
}

void powerOff() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB::Black;
  }  
  FastLED.show();
  delay(50);
}

void handlePowerState(AsyncWebServerRequest *req) {
  int powerVal;
  if (req->hasParam("state")) {
    powerVal = req->getParam("state")->value().toInt();
    poweredOn = powerVal == 1;
    if (!poweredOn) {
      powerOff();
    } else {
      powerOn();
    }
  }
  Serial.print("poweredOn: ");
  Serial.println(poweredOn);
  req->send(200, "text/plain", "OK");
}

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
  <head>
    <title>The Sims Plumbob LED Controller</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
      html { font-family: Arial; display: inline-block; text-align: center; }
      body { margin: 0px auto; padding-bottom: 15px; }
      .slider { flex-grow: 1; }
    </style>
  </head>
  <body>
    <h2>The Sims Plumbob LED Controller</h2>
    <div align="center">
      <div id="num_clients"></div>
      <label for="num_clients">Clients: </label><br>

      <input type="checkbox" id="powerState" name="powerState" checked onchange="togglePower(this)" />
      <label for="powerState">Power: </label><br>

      <input type="range" class="slider" name="slider" min="0" max="255" step="5" value="0" onchange="handleSlider(this)" />
      <label for="slider">Slider</label><br>

      <input type="range" class="slider" name="brightness" min="0" max="100" value="100" onchange="handleBrightness(this)" />
      <label for="brightness">Brightness</label><br>
    </div>
    <script>
      function togglePower(e) {
        let xhr = new XMLHttpRequest();
        if (e.checked) {
          xhr.open("GET", "/power?state=1", true);
        } else {
          xhr.open("GET", "/power?state=0", true);
        }
        xhr.send();
      }

      function handleSlider(e) {
        let xhr = new XMLHttpRequest();
        xhr.open("GET", "/color?value=" + e.value, true);
        xhr.send();
        e.preventDefault();
      }

      function handleBrightness(e) {
        let xhr = new XMLHttpRequest();
        xhr.open("GET", "/brightness?value=" + e.value, true);
        xhr.send();
        e.preventDefault();
      }
    </script>
  </body>
</html>)rawliteral";
 
String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  Serial.print("Station connected: ");
  Serial.println(macToString(evt.mac));
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  Serial.print("Station disconnected: ");
  Serial.println(macToString(evt.mac));
}

void setup() {
  pinMode(CONTROL_PIN, OUTPUT);
  Serial.begin(115200);
  delay(500);

  LEDS.setBrightness(brightness);
  LEDS.addLeds<WS2812B, CONTROL_PIN, GRB>(leds, LED_COUNT);
  LEDS.show();
  Serial.println('\n');

  WiFi.softAP(AP_SSID, AP_PSK, 1, false, 2);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address: \t");
  Serial.println(WiFi.softAPIP());

  // Call "onStationConnected" each time a station connects
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  // Call "onStationDisconnected" each time a station disconnects
  stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started\n");
  powerOn();

  // int connections = WiFi.softAPgetStationNum();
  // Serial.println(connections);

  // Set up routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", index_html);
    Serial.println("landing");
  });

  server.on("/power", HTTP_GET, handlePowerState);

  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *req){
    if (req->hasParam("value")) {
      color_val = req->getParam("value")->value().toInt();
      Serial.println(color_val);
      powerOn(); 
    }
    req->send(200, "text/plain", "OK");
  });

  server.on("/brightness", HTTP_GET, [](AsyncWebServerRequest *req){
    if (req->hasParam("value")) {
      brightness = req->getParam("value")->value().toInt();
      Serial.println(brightness);
      LEDS.setBrightness(brightness);
      LEDS.show();
    }
    req->send(200, "text/plain", "OK");
  });
}

void loop() {}

// Ref: https://randomnerdtutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/