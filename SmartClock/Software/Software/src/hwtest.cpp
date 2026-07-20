// Testing code for the ESP32
#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>

// PCB routes the OLED with SDA on D5 and SCL on D4 (swapped vs. XIAO default)
#define I2C_SDA_PIN D5
#define I2C_SCL_PIN D4

Adafruit_SSD1306 display(128, 64, &Wire, -1);
bool haveDisplay = false;

const int dGpio[] = {2, 3, 4, 5, 21, 20, 8, 9, 10};
const char* dName[] = {"D0", "D1", "D2", "D3", "D6", "D7", "D8", "D9", "D10"};
const int nPins = sizeof(dGpio) / sizeof(dGpio[0]);

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n=== XIAO ESP32C3 pin monitor (OLED feedback) ===");

  // Line-state check before I2C claims the pins: with pullups, a healthy idle
  // bus reads HIGH on both lines. LOW = shorted to GND or held by a stuck device.
  pinMode(6, INPUT_PULLUP);
  pinMode(7, INPUT_PULLUP);
  delay(10);
  Serial.printf("SCL (D4/GPIO6) idle: %s\n", digitalRead(6) ? "HIGH (good)" : "LOW  <-- STUCK! short or held");
  Serial.printf("SDA (D5/GPIO7) idle: %s\n", digitalRead(7) ? "HIGH (good)" : "LOW  <-- STUCK! short or held");

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.setTimeOut(50); // fail pings fast when the bus is stuck
  haveDisplay = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  Serial.printf("display: %s\n", haveDisplay ? "OK" : "NOT FOUND");
  if (haveDisplay) {
    display.clearDisplay();
    display.setTextColor(SSD1306_WHITE);
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("Pin monitor ready");
    display.println("Touch pin to GND...");
    display.display();
  }

  for (int i = 0; i < nPins; i++) pinMode(dGpio[i], INPUT_PULLUP);

  Serial.println("WiFi scan...");
  if (haveDisplay) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println("WiFi scan...");
    display.display();
  }
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  Serial.printf("%d networks found\n", n);
  if (haveDisplay) {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.printf("%d networks:\n", n);
  }
  for (int i = 0; i < n && i < 6; i++) {
    Serial.printf("  %s  RSSI %d dBm\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    if (haveDisplay) display.printf("%s %d\n", WiFi.SSID(i).substring(0, 15).c_str(), WiFi.RSSI(i));
  }
  for (int i = 6; i < n; i++) Serial.printf("  %s  RSSI %d dBm\n", WiFi.SSID(i).c_str(), WiFi.RSSI(i));
  if (haveDisplay) {
    display.display();
    delay(8000); // leave scan results readable before pin monitor takes over
  }
  WiFi.scanDelete();
}

String lastEvent = "none yet";
int eventCount = 0;

void loop() {
  // Live I2C wiggle test: ping the OLED twice a second, report every flip.
  static unsigned long lastPing = 0;
  static int busOk = -1;
  static int flips = 0;
  if (millis() - lastPing > 500) {
    lastPing = millis();
    // Idle-line readback in the loop, where USB-CDC is fully up (the boot-time
    // print gets eaten during re-enumeration). Both LOW => OLED lost VCC.
    Serial.printf("  idle lines: SCL(D4)=%d SDA(D5)=%d\n", digitalRead(6), digitalRead(7));
    Wire.beginTransmission(0x3C);
    int ok = (Wire.endTransmission() == 0) ? 1 : 0;
    if (ok != busOk) {
      flips++;
      busOk = ok;
      Serial.printf("### DISPLAY BUS %s (flip #%d, t=%lus)\n", ok ? "OK" : "LOST", flips, millis() / 1000);
      if (ok) {
        haveDisplay = display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
        if (haveDisplay) {
          display.clearDisplay();
          display.setTextColor(SSD1306_WHITE);
          display.setTextSize(2);
          display.setCursor(0, 0);
          display.println("BUS OK");
          display.printf("flip %d", flips);
          display.display();
        }
      } else {
        haveDisplay = false;
      }
    }
  }

  static int last[16];
  static bool first = true;
  int now[16];
  bool changed = false;
  String lows = "";

  for (int i = 0; i < nPins; i++) {
    now[i] = digitalRead(dGpio[i]);
    if (first || now[i] != last[i]) changed = true;
    if (!first && last[i] == 1 && now[i] == 0) {
      lastEvent = dName[i];
      eventCount++;
    }
    if (now[i] == 0) {
      lows += dName[i];
      lows += " ";
    }
    last[i] = now[i];
  }
  first = false;

  static unsigned long lastPrint = 0;
  if (changed || millis() - lastPrint > 2000) {
    lastPrint = millis();
    for (int i = 0; i < nPins; i++) Serial.printf("%s=%d ", dName[i], last[i]);
    Serial.println(changed ? "  <-- CHANGE" : "");
  }

  if (haveDisplay && changed) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setCursor(0, 0);
    display.println(lows.length() > 0 ? "CONTACT NOW:" : "All pins HIGH");
    if (lows.length() > 0) {
      display.setTextSize(3);
      display.setCursor(0, 10);
      display.println(lows);
    }
    display.setTextSize(1);
    display.setCursor(0, 40);
    display.println("Last contact seen:");
    display.setTextSize(2);
    display.setCursor(0, 50);
    display.print(lastEvent);
    display.setTextSize(1);
    display.printf("  (%dx)", eventCount);
    display.display();
  }
  delay(30);
}
