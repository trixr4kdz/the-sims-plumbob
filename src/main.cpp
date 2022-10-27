#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ESPAsyncTCP.h>
#include <FastLED.h>
#include <FS.h>

#include "constants.h"
#include "helpers.h"

// TODO: separate html/css/js into data/ dir and use SPIFFS

AsyncWebServer server(80);

char ssid[] = AP_SSID;
char password[] = AP_PSK;

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;
uint8_t connections = 0;

// TODO: add effects? (like spinning diamond)
// TODO: add label for number of clients

uint8_t color_val = 0;
uint8_t num_clients = 0;
uint8_t brightness = 100;

bool poweredOn = true;
bool unsupervised = false;
uint8_t unsupervisedDurationInMin;;
uint8_t colorChangePerSec;

CRGBArray<LED_COUNT> leds;

unsigned long currentTime = millis();

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
  if (DEBUG_MODE) {
    Serial.print("poweredOn: ");
    Serial.println(poweredOn); 
  }

  req->send(200, "text/plain", "OK");
}

void handleColor(AsyncWebServerRequest *req) {
  if (req->hasParam("value")) {
    color_val = req->getParam("value")->value().toInt();
    if (poweredOn) {
      powerOn();
    }
    unsupervised = false;
  }

  if (DEBUG_MODE) {
    Serial.println(color_val);
  }
  req->send(200, "text/plain", "OK");
}

void handleBrightness(AsyncWebServerRequest *req) {
  if (req->hasParam("value")) {
    brightness = req->getParam("value")->value().toInt();
    LEDS.setBrightness(brightness);
    LEDS.show();
  }

  if (DEBUG_MODE) {
    Serial.println(brightness);
  }
  req->send(200, "text/plain", "OK"); 
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
  connections++;
  Serial.print("Station connected: ");
  Serial.println(macToString(evt.mac));
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
  connections--;
  Serial.print("Station disconnected: ");
  Serial.println(macToString(evt.mac));
}

void setup() {
  pinMode(CONTROL_PIN, OUTPUT);
  WiFi.softAP(AP_SSID, AP_PSK, 1, false, 2);

  if (DEBUG_MODE) {
    Serial.begin(9600);

    Serial.print("Access Point \"");
    Serial.print(ssid);
    Serial.println("\" started");

    Serial.print("IP address: \t");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.begin(115200);
  }

  LEDS.setBrightness(brightness);
  LEDS.addLeds<WS2812B, CONTROL_PIN, GRB>(leds, LED_COUNT);
  LEDS.show();
  delay(500);

  // Call "onStationConnected" each time a station connects
  stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
  // Call "onStationDisconnected" each time a station disconnects
  stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);

  server.onNotFound(handleNotFound);

  // int connections = WiFi.softAPgetStationNum();
  // Serial.println(connections);

  // Set up routes
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *req) {
    req->send_P(200, "text/html", index_html);
  });

  server.on("/power", HTTP_GET, handlePowerState);
  server.on("/color", HTTP_GET, handleColor);
  server.on("/brightness", HTTP_GET, handleBrightness);

  server.on("/unsupervised", HTTP_GET, [](AsyncWebServerRequest *req){
    if (req->hasParam("duration")) {
      unsupervisedDurationInMin = req->getParam("duration")->value().toInt();
      unsupervised = true;
      colorChangePerSec = (uint8_t) ceil(255 / (unsupervisedDurationInMin * 60));

      // reset color_val
      color_val = 0;
      powerOn();
    }

    if(DEBUG_MODE) {
      Serial.print("unsupervised duration: ");
      Serial.println(unsupervisedDurationInMin);
    }
    req->send(200, "text/plain", "OK");
  });

  server.begin();
  powerOn();

  if (DEBUG_MODE) {
   Serial.println("HTTP server started\n");
  }
}

void loop() {
  if (unsupervised) {
    if (DEBUG_MODE) {
      Serial.println("Currently unsupervised...");
      Serial.println("currentTime: " + currentTime);
      Serial.println("millis(): " + millis());
    }
    if (millis() - currentTime >= 1000) {
      color_val = constrain(color_val + colorChangePerSec, 0, 255);
      powerOn();
      currentTime = millis();
    }
  }
}

// Ref: https://randomnerdtutorials.com/esp8266-nodemcu-async-web-server-espasyncwebserver-library/