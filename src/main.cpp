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

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;

#define CONTROL_PIN D1

// TODO: send slider input from Blynk App? to esp32
// TODO: set up client for phones
// TODO: add button for power on/off
// TODO: get slider input from slider component
// TODO: add brightness input
// TODO: add effects? (like spinning diamond)
// TODO: add label for number of clients

const int LED_COUNT = 14;

uint8_t slider_val = 0;
uint8_t num_clients = 0;

const char* SLIDER_INPUT = "slider";
const char* POWER_INPUT = "powerState";
bool poweredOn = true;

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
  delay(100);
}

void powerOff() {
  for (int i = 0; i < LED_COUNT; i++) {
    leds[i] = CRGB::Black;
  }  
  FastLED.show();
  delay(100);
}

void handlePowerState(AsyncWebServerRequest *req) {
  // TODO: Fix the crashing when toggling
  int powerVal;
  if (req->hasParam(POWER_INPUT)) {
    powerVal = req->getParam(POWER_INPUT)->value().toInt();
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
  </head>
  <body>
    <h2>The Sims Plumbob LED Controller</h2>
    <p>
      <input type="checkbox" id="powerState" name="powerState" checked onchange="togglePower(this)" />
      <label for="powerState">Power: </label>
    </p><br>
    <input type="range" name="slider" min="0" max="255" step="5" value="0" onchange="handleSlider(this)" />
    <label for="slider">Slider</label><br>

    <span name="num_clients"></span>
    <label for="num_clients">Clients: </label>
    <br>
    <script>
      function togglePower(e) {
        var xhr = new XMLHttpRequest();
        if (e.checked) {
          xhr.open("GET", "/power?powerState=1", true);
        } else {
          xhr.open("GET", "/power?powerState=0", true);
        }
        xhr.send();
      }

      function handleSlider(e) {
        var xhr = new XMLHttpRequest();
        xhr.open("GET", "/color?slider=" + e.value, true);
        xhr.send();
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
  Serial.begin(9600);
  LEDS.setBrightness(brightness);
  LEDS.addLeds<WS2812B, CONTROL_PIN, GRB>(leds, LED_COUNT);
  LEDS.show();
  delay(1000);
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
  // delay(1000);

  // Set up routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", index_html);
    Serial.println("landing");
  });

  server.on("/power", HTTP_GET, handlePowerState);

  server.on("/color", HTTP_GET, [](AsyncWebServerRequest *req){
    // TODO: Fix this slider changing multiple times...
    slider_val = req->getParam(SLIDER_INPUT)->value().toInt();
    Serial.println(slider_val);
    powerOn();
  });
}

void loop() {}

// Ref: https://randomnerdtutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/