#include <WiFi.h>
#include <HTTPClient.h>
#include <Arduino_JSON.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "DHT.h"
#include "esp_sleep.h"
#include <time.h>

const char* ssid = "wgk-60478";
const char* password = "xxx";
const char* serverURL = "https://lukasschlegel.ch/klimamonitor/load.php";

#define DHTPIN 3
#define DHTTYPE DHT11
#define LIGHTPIN_ANALOG 5
#define SCHWELLE_DUNKEL 3200
DHT dht(DHTPIN, DHTTYPE);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define OLED_SDA 6
#define OLED_SCL 7
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define SLEEP_DURATION_SEC 3600

RTC_DATA_ATTR bool ersterStart = true;

#define BUTTON_GPIO 2

const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 3600;
const int daylightOffset_sec = 3600;

int centerText(const char* text) {
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  return (SCREEN_WIDTH - w) / 2;
}

String getUhrzeit() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  struct tm timeinfo;
  if (getLocalTime(&timeinfo)) {
    char buffer[6];
    strftime(buffer, sizeof(buffer), "%H:%M", &timeinfo);
    return String(buffer);
  }
  return "??:??";
}

void geheInDeepSleep(bool vomButton) {
  if (vomButton) {
    delay(10000);
    display.clearDisplay();
    display.display();
    esp_sleep_enable_ext1_wakeup((1ULL << BUTTON_GPIO), ESP_EXT1_WAKEUP_ALL_LOW);
    Serial.println("Gehe in Deep Sleep (nur externer Wakeup, Timer läuft weiter).");
  } else {
    esp_sleep_enable_timer_wakeup((uint64_t)SLEEP_DURATION_SEC * 1000000ULL);
    esp_sleep_enable_ext1_wakeup((1ULL << BUTTON_GPIO), ESP_EXT1_WAKEUP_ALL_LOW);
    Serial.printf("Gehe in Deep Sleep für %d Sekunden...\n", SLEEP_DURATION_SEC);
  }
  esp_deep_sleep_start();
}

void setup() {
  Serial.begin(115200);
  delay(1000);

  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  bool vomButton = (wakeup_reason == ESP_SLEEP_WAKEUP_EXT1);
  bool vomTimer = (wakeup_reason == ESP_SLEEP_WAKEUP_TIMER);
  bool stromStart = (wakeup_reason == ESP_SLEEP_WAKEUP_UNDEFINED);

  Serial.printf("Bootgrund: %s\n", 
    vomButton ? "Button" : (vomTimer ? "Timer" : "Stromstart"));

  dht.begin();
  delay(2000);

  pinMode(BUTTON_GPIO, INPUT_PULLUP);

  float temperatur = dht.readTemperature();
  float feuchtigkeit = dht.readHumidity();

  int versuche = 0;
  while ((isnan(temperatur) || isnan(feuchtigkeit)) && versuche < 5) {
    delay(1000);
    temperatur = dht.readTemperature();
    feuchtigkeit = dht.readHumidity();
    versuche++;
  }

  if (isnan(temperatur) || isnan(feuchtigkeit)) {
    Serial.println("Warnung: Sensorwerte ungültig, setze auf 0.");
    temperatur = 0;
    feuchtigkeit = 0;
  }

  Serial.printf("Rohwerte – Temperatur: %.2f, Feuchtigkeit: %.2f\n", temperatur, feuchtigkeit);

  int lichtwert = analogRead(LIGHTPIN_ANALOG);
  int licht = lichtwert > SCHWELLE_DUNKEL ? 0 : 1;
  String hellDunkel = licht == 1 ? "Hell" : "Dunkel";
  Serial.printf("Lichtwert: %d → %s\n", lichtwert, hellDunkel.c_str());

  if (vomButton) {
    Serial.println("Button wurde gedrückt. Daten werden 10 Sekunden angezeigt.");

    Wire.begin(OLED_SDA, OLED_SCL);
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    WiFi.begin(ssid, password);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) {
      delay(500);
      retries++;
    }

    String uhrzeit = getUhrzeit();
    char zeile0[20];
    snprintf(zeile0, sizeof(zeile0), "Messung: %s", uhrzeit.c_str());
    display.setCursor(centerText(zeile0), 0);
    display.println(zeile0);

    char zeile1[32];
    sprintf(zeile1, "Temperatur: %.1f C", temperatur);
    display.setCursor(centerText(zeile1), 16);
    display.println(zeile1);

    char zeile2[32];
    sprintf(zeile2, "Feuchtigkeit: %.1f %%", feuchtigkeit);
    display.setCursor(centerText(zeile2), 26);
    display.println(zeile2);

    char zeile3[32];
    sprintf(zeile3, "Licht: %s", hellDunkel.c_str());
    display.setCursor(centerText(zeile3), 36);
    display.println(zeile3);

    char zeile4[50] = "Info: www.lukasschlegel.ch/klimamonitor";
    display.setTextSize(1);
    display.setCursor(centerText(zeile4), 48);
    display.println(zeile4);

    display.display();
    geheInDeepSleep(true);
  }

  if (stromStart || vomTimer) {
    Serial.println("WLAN wird verbunden...");
    WiFi.begin(ssid, password);
    int retries = 0;
    while (WiFi.status() != WL_CONNECTED && retries < 30) {
      delay(200);
      retries++;
    }

    if (WiFi.status() == WL_CONNECTED) {
      Serial.println("WLAN verbunden!");

      JSONVar data;
      data["temperatur"] = temperatur;
      data["feuchtigkeit"] = feuchtigkeit;
      data["licht"] = licht;
      data["licht_status"] = hellDunkel;

      String jsonString = JSON.stringify(data);
      Serial.println("JSON-Daten werden gesendet:");
      Serial.println(jsonString);

      HTTPClient http;
      http.begin(serverURL);
      http.addHeader("Content-Type", "application/json");
      int code = http.POST(jsonString);

      if (code > 0) {
        Serial.printf("Daten gesendet. Antwort [%d]: %s\n", code, http.getString().c_str());
      } else {
        Serial.printf("Daten senden fehlgeschlagen. Fehlercode: %d\n", code);
      }

      http.end();
    } else {
      Serial.println("WLAN-Verbindung fehlgeschlagen.");
    }

    geheInDeepSleep(false);
  }
}

void loop() {}
