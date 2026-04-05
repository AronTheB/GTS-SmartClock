#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
#include "config.h"

const char* WIFI_SSID = WIFI_SSID;
const char* WIFI_PASSWORD = WIFI_PASSWORD;

const long UTC_OFFSET_SEC = 7200;
const int DAYLIGHT_OFFSET_SEC = 0;
const char* NTP_SERVER = "pool.ntp.org";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to WiFi");
}

void syncNTP() {
  configTime(UTC_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.println("Syncing with NTP server");
  struct tm t;
  while (!getLocalTime(&t)) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("synced");
}

// put function declarations here:
int myFunction(int, int);

void setup() {
  Serial.begin(115200);
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 allocation failed"));
      for(;;);
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();

  connectToWiFi();
  syncNTP();
}


void loop() {
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    display.clearDisplay();
    display.setTextSize(2);
    display.setCursor(0, 0);
    display.printf("%02d:%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec);
    display.display();
  }
  delay(1000);
}
