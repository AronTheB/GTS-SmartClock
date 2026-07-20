#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <time.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "config.h"


const long UTC_OFFSET_SEC = 7200;
const int DAYLIGHT_OFFSET_SEC = 0;
const char* NTP_SERVER = "pool.ntp.org";

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1
#define SCREEN_ADDRESS 0x3C
#define I2C_SDA_PIN D5
#define I2C_SCL_PIN D4
#define BUTTON_PIN D2

enum ScreenView { VIEW_CLOCK, VIEW_BUS };
ScreenView currentView = VIEW_CLOCK;
bool lastButtonState = HIGH;
time_t nextBusDepartureEpoch = 0;
unsigned long lastBkvFetch = 0;
const unsigned long BKV_FETCH_INTERVAL_MS = 60000;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void showStatus(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println(message);
  display.display();
}

void connectToWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  String dots = "";
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    dots += ".";
    if (dots.length() > 10) dots = "";
    showStatus(("Connecting to WiFi" + dots).c_str());
  }
  Serial.println("\nConnected to WiFi");
  showStatus("WiFi connected");
}

void syncNTP() {
  configTime(UTC_OFFSET_SEC, DAYLIGHT_OFFSET_SEC, NTP_SERVER);
  Serial.println("Syncing with NTP server");
  struct tm t;
  String dots = "";
  while (!getLocalTime(&t)) {
    delay(500);
    Serial.print(".");
    dots += ".";
    if (dots.length() > 10) dots = "";
    showStatus(("Syncing NTP" + dots).c_str());
  }
  Serial.println("synced");
  showStatus("NTP synced");
}

void bkvAPI(){
  if (WiFi.status() != WL_CONNECTED) return;
  
  WiFiClientSecure client;
  client.setInsecure();

  HTTPClient http;
  String url = String("https://futar.bkk.hu/api/query/v1/ws/otp/api/where/arrivals-and-departures-for-stop")
    + "?key=" + BKK_API_KEY
    + "&version=3&appVersion=1&stopId=" + BKK_STOP_ID
    + "&onlyDepartures=true&minutesAfter=60";

  if (!http.begin(client, url)) {
    Serial.println("bkvAPI: http.begin failed");
    return;
  }

  int code = http.GET();
  if (code != HTTP_CODE_OK) {
    Serial.printf("bkvAPI: HTTP error %d\n", code);
    http.end();
    return;
  }

  JsonDocument filter;
  filter["data"]["entry"]["stopTimes"][0]["tripId"] = true;
  filter["data"]["entry"]["stopTimes"][0]["stopHeadsign"] = true;
  filter["data"]["entry"]["stopTimes"][0]["predictedDepartureTime"] = true;
  filter["data"]["entry"]["stopTimes"][0]["departureTime"] = true;
  filter["data"]["references"]["trips"]["*"]["routeId"] = true;
  filter["data"]["references"]["trips"]["*"]["tripHeadsign"] = true;
  filter["data"]["references"]["routes"]["*"]["shortName"] = true;

  JsonDocument doc;
  DeserializationError err = deserializeJson(doc, http.getStream(), DeserializationOption::Filter(filter));
  http.end();

  if (err) {
    Serial.printf("bkvAPI: JSON parse failed: %s\n", err.c_str());
    return;
  }

  JsonArray stopTimes = doc["data"]["entry"]["stopTimes"];
  JsonObject trips = doc["data"]["references"]["trips"];
  JsonObject routes = doc["data"]["references"]["routes"];

  time_t now;
  time(&now);

  time_t earliest = 0;
  for (JsonObject st : stopTimes) {
    const char* tripId = st["tripId"];
    if (!tripId || !trips[tripId].is<JsonObject>()) continue;

    const char* routeId = trips[tripId]["routeId"];
    if (!routeId || !routes[routeId].is<JsonObject>()) continue;

    const char* shortName = routes[routeId]["shortName"];
    if (!shortName || strcmp(shortName, BUS_ROUTE_SHORT_NAME) != 0) continue;

    const char* headsign = st["stopHeadsign"] | trips[tripId]["tripHeadsign"].as<const char*>();
    if (!headsign || strstr(headsign, BUS_DIRECTION_HEADSIGN) == nullptr) continue;

    time_t dep = st["predictedDepartureTime"] | (time_t)st["departureTime"];
    if (dep <= now) continue;

    if (earliest == 0 || dep < earliest) earliest = dep;
  }

  nextBusDepartureEpoch = earliest;
}

void setup() {
  Serial.begin(115200);
  delay(1500);
  Serial.println("Boot: starting");

  pinMode(BUTTON_PIN, INPUT_PULLUP);

    Serial.println("Boot: I2C init");
    Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

    Serial.println("Boot: I2C scan");
    int devicesFound = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0) {
        Serial.print("  found device at 0x");
        Serial.println(addr, HEX);
        devicesFound++;
      }
    }
    if (devicesFound == 0) {
      Serial.println("  no I2C devices found - check SDA/SCL/VCC/GND wiring");
    }

    Serial.println("Boot: display init");
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
      Serial.println(F("SSD1306 init failed - continuing without display"));
    } else {
      Serial.println("Boot: display OK");
    }
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.display();
    showStatus("Booting...");

  connectToWiFi();
  syncNTP();

  bkvAPI();
  lastBkvFetch = millis();
}

unsigned long lastButtonDebugPrint = 0;

void loop() {

  if (millis() - lastButtonDebugPrint > 1000) {
    lastButtonDebugPrint = millis();
    Serial.print("Button pin raw state: ");
    Serial.println(digitalRead(BUTTON_PIN));
  }

  bool currentButtonState = digitalRead(BUTTON_PIN);
  if (lastButtonState == HIGH && currentButtonState == LOW) {
    Serial.println("Button pressed");
    currentView = (currentView == VIEW_CLOCK) ? VIEW_BUS : VIEW_CLOCK;
    delay(50);
  }
  lastButtonState = currentButtonState;

  if (millis() - lastBkvFetch > BKV_FETCH_INTERVAL_MS) {
  lastBkvFetch = millis();
  bkvAPI();
  }


  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    display.clearDisplay();

    if (currentView == VIEW_CLOCK) {
      char dateStr[16];
      strftime(dateStr, sizeof(dateStr), "%a, %b %d", &timeinfo);
      display.setTextSize(1);
      display.setCursor((128 - strlen(dateStr) * 6) / 2, 4);
      display.print(dateStr);
      display.setTextSize(3);
      display.setCursor(1, 29);
      display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
      display.setTextSize(2);
      display.setCursor(91, 36);
      display.printf(":%02d", timeinfo.tm_sec);
    } else { // VIEW_BUS
      // Top: current time, where the date sits on the clock view
      char timeStr[8];
      strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
      display.setTextSize(1);
      display.setCursor((128 - strlen(timeStr) * 6) / 2, 4);
      display.print(timeStr);

      // Big area: MM:SS countdown to the next outbound 260
      display.setTextSize(3);
      if (nextBusDepartureEpoch > 0) {
        time_t now;
        time(&now);
        long secsLeft = (long)(nextBusDepartureEpoch - now);
        if (secsLeft < 0) secsLeft = 0;
        long mm = secsLeft / 60;
        long ss = secsLeft % 60;
        char cd[8];
        snprintf(cd, sizeof(cd), "%02ld:%02ld", mm, ss);
        display.setCursor((128 - strlen(cd) * 18) / 2, 29);
        display.print(cd);
      } else {
        display.setCursor((128 - 2 * 18) / 2, 29);
        display.print("--");
      }
    }

    display.display();
  }
  delay(200);
}
