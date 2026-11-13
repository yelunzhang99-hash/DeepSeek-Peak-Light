// C3-Mini DeepSeek Peak Light
// Shows on a 0.66" OLED and the onboard RGB LED whether DeepSeek API prices are at peak.
// Red = peak, green = off-peak (half price), blue = connecting.
//
// First start: join the WiFi network "PeakTimer" and enter your home WiFi on the page that opens.
// Hold BOOT for 3 seconds to forget the saved WiFi.
//
// Peak: Monday to Friday, 09:00-12:00 and 14:00-18:00 Beijing time.
// Not handled: Chinese public holidays, which are off-peak all day.
//
// Arduino IDE: board "ESP32C3 Dev Module", USB CDC On Boot "Enabled". Libraries: Adafruit SSD1306,
// Adafruit GFX, Adafruit NeoPixel, WiFiManager (tzapu).

#include <WiFi.h>
#include <Wire.h>
#include <time.h>
#include <Adafruit_GFX.h>
#include <Adafruit_NeoPixel.h>
#include <Adafruit_SSD1306.h>
#ifndef WOKWI_SIM
#include <WiFiManager.h>
#endif

const int SDA_PIN = 8;
const int SCL_PIN = 10;
const int RGB_PIN = 7;
const int BOOT_BUTTON = 9;
const unsigned long FORGET_HOLD_MS = 3000;
const time_t BEIJING_OFFSET = 8 * 3600;
const time_t VALID_TIME = 1704067200;  // 2024-01-01: anything earlier means no time yet

const char* const DAYS[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

Adafruit_SSD1306 display(64, 48, &Wire, -1);
Adafruit_NeoPixel rgb(1, RGB_PIN, NEO_GRB + NEO_KHZ800);
time_t shownMinute = 0;
unsigned long bootPressedAt = 0;

void setLight(uint8_t red, uint8_t green, uint8_t blue) {
  rgb.setPixelColor(0, rgb.Color(red, green, blue));
  rgb.show();
}

void showMessage(const char* message) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print(message);
  display.display();
  Serial.println(message);
}

// Takes Beijing time as a plain epoch value, so gmtime gives Beijing's fields.
bool isPeak(time_t beijing) {
  struct tm t;
  gmtime_r(&beijing, &t);
  if (t.tm_wday == 0 || t.tm_wday == 6) return false;
  return (t.tm_hour >= 9 && t.tm_hour < 12) || (t.tm_hour >= 14 && t.tm_hour < 18);
}

// Every change happens on a full hour, so stepping hour by hour finds the next one.
time_t nextChange(time_t beijing) {
  time_t next = beijing - beijing % 3600 + 3600;
  while (isPeak(next) == isPeak(beijing)) next += 3600;
  return next;
}

void connectWifi() {
  showMessage("Connecting\nto WiFi...");
#ifdef WOKWI_SIM
  WiFi.begin("Wokwi-GUEST", "", 6);  // the simulator's open network
  while (WiFi.status() != WL_CONNECTED) delay(100);
#else
  WiFiManager manager;
  manager.setDebugOutput(false);
  manager.setAPCallback([](WiFiManager*) {
    showMessage("Join WiFi\nPeakTimer\nOpen site:\n192.168.\n4.1");
  });
  if (!manager.autoConnect("PeakTimer")) ESP.restart();
#endif
}

void syncClock() {
  showMessage("Getting\nthe time...");
  configTime(0, 0, "pool.ntp.org");
  while (time(nullptr) < VALID_TIME) delay(100);
}

void forgetWifiIfHeld() {
  if (digitalRead(BOOT_BUTTON) == HIGH) {
    bootPressedAt = 0;
    return;
  }
  if (bootPressedAt == 0) bootPressedAt = millis();
  if (millis() - bootPressedAt < FORGET_HOLD_MS) return;

  showMessage("WiFi\nforgotten\nRestarting");
  WiFi.disconnect(true, true);
  delay(1000);
  ESP.restart();
}

void showStatus(time_t now) {
  time_t beijing = now + BEIJING_OFFSET;
  bool peak = isPeak(beijing);
  time_t next = nextChange(beijing);
  setLight(peak ? 40 : 0, peak ? 0 : 40, 0);

  struct tm t, n;
  gmtime_r(&beijing, &t);
  gmtime_r(&next, &n);

  char timeText[6];
  char nextText[10];
  snprintf(timeText, sizeof(timeText), "%02d:%02d", t.tm_hour, t.tm_min);
  snprintf(nextText, sizeof(nextText), "%s %02d:00", DAYS[n.tm_wday], n.tm_hour);

  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("DS Beijing");
  display.setTextSize(2);
  display.setCursor(0, 10);
  display.print(timeText);
  display.setTextSize(1);
  display.setCursor(0, 29);
  display.print(peak ? "PEAK" : "OFF-PEAK");
  display.setCursor(0, 40);
  display.print(nextText);
  display.display();

  Serial.print(DAYS[t.tm_wday]);
  Serial.print(' ');
  Serial.print(timeText);
  Serial.print(peak ? " PEAK, next " : " OFF-PEAK, next ");
  Serial.println(nextText);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  pinMode(BOOT_BUTTON, INPUT_PULLUP);
  rgb.begin();
  setLight(0, 0, 40);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("OLED buffer allocation failed");
    while (true) delay(1000);
  }
  // The library leaves 64x48 panels on COM pins 0x02, which garbles them.
  display.ssd1306_command(SSD1306_SETCOMPINS);
  display.ssd1306_command(0x12);
  display.setTextColor(SSD1306_WHITE);

  connectWifi();
  syncClock();
}

void loop() {
  forgetWifiIfHeld();

  time_t now = time(nullptr);
  if (now / 60 != shownMinute) {
    shownMinute = now / 60;
    showStatus(now);
  }

  delay(50);
}
