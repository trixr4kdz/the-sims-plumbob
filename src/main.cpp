#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <FastLED.h>
#include <ESP8266WebServer.h>
#include <WiFiClient.h>

#ifndef AP_SSID
#define AP_SSID "trix-sims"
#define AP_PSK "plumbob-69420"
#endif

#define LED D0
#define LED_COUNT 14
#define CONTROL_PIN D1

// TODO: send slider input from Blynk App? to esp32
// TODO: create qr code for connecting to hidden AP on esp32
// TODO: set up webserver on nodemcu
// TODO: set up client for phones

char ssid[] = AP_SSID;
char password[] = AP_PSK;

ESP8266WebServer server(80);

const uint8_t slider_val = 255;

IPAddress Ip(192,168,7,2);
IPAddress Gateway(192,168,7,1);
IPAddress Subnet(255,255,255,0);

CRGBArray<LED_COUNT> leds;
uint8_t brightness = 50;

void handleHome() {
  digitalWrite(CONTROL_PIN, HIGH);
  server.send(200, "text/plain", "Welcome!\r\n");
  digitalWrite(CONTROL_PIN, LOW);
}

void handleNotFound() {
  digitalWrite(CONTROL_PIN, HIGH);
  String msg = "File Not Found\n\n";
  msg += "URI: ";
  msg += server.uri();
  msg += "\nMethod: ";
  msg += (server.method() == HTTP_GET) ? "GET" : "POST";
  msg += "\nArguments: ";
  msg += server.args();
  msg += "\n";
  for (uint8_t i = 0; i < server.args(); i++) { 
    msg += " " + server.argName(i) + ": " + server.arg(i) + "\n"; 
  }
  server.send(404, "text/plain", msg);
  digitalWrite(CONTROL_PIN, LOW);
}
 
void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  LEDS.setBrightness(brightness);
  LEDS.addLeds<WS2812B, CONTROL_PIN, GRB>(leds, LED_COUNT);
  LEDS.show();
  delay(10);
  Serial.println('\n');

  WiFi.softAP(AP_SSID, AP_PSK, 1, false, 2);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address: \t");
  Serial.println(WiFi.softAPIP());

  server.on("/", handleHome);
  server.on("/test", []() {
    server.send(200, "text/plain", "test");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started\n");
}

void loop() {
  digitalWrite(LED, HIGH);
  delay(1000);
  digitalWrite(LED, LOW);
  delay(1000);

  for (int i = 0; i++; i < LED_COUNT) {
    leds[i] = CRGB(0 + slider_val, 255 - slider_val, 0);
    FastLED.show();
    delay(20);
  }

  server.handleClient();
  int connections = WiFi.softAPgetStationNum();
  Serial.println(connections);
  delay(500);
}