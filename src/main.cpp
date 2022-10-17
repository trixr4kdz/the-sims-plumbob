#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FS.h>
#include <FastLED.h>

// TODO: determine minimum num of LEDs (15-16?)
// TODO: how to add PSK to AP
// TODO: send slider input from Blynk App? to esp32
// TODO: create qr code for conencting to hidden AP on esp32
// TODO: set up webserver on nodemcu
// TODO: set up client for phones

char ssid[] = "ESP8266 Access Point";
char password[] = "plumbob";

const uint8_t slider_val = 255;

IPAddress Ip(192,168,7,2);
IPAddress Gateway(192,168,7,1);
IPAddress Subnet(255,255,255,0);

#define LED D0
#define LED_COUNT 10
#define CONTROL_PIN D1

CRGBArray<LED_COUNT> leds;
uint8_t brightness = 50;
uint8_t slider_val = 0;
 
void setup() {
  pinMode(LED, OUTPUT);
  Serial.begin(9600);
  LEDS.setBrightness(brightness);
  LEDS.addLeds<WS2812B, CONTROL_PIN, GRB>(leds, LED_COUNT);
  LEDS.show();
  delay(10);
  Serial.println('\n');

  WiFi.softAP(ssid);
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");

  Serial.print("IP address: \t");
  Serial.println(WiFi.softAPIP());
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
}