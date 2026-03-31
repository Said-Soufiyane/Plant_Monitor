// // // ---- WiFi + Telegram ----
// // const char* WIFI_SSID = "ORBI76";
// // const char* WIFI_PASS = "85600000";

// // const char* TG_BOT_TOKEN = "8229863237:AAHkn_FSejsTF6VPJR0eKkPc0dwlTXor24o";
// // const char* TG_CHAT_ID   = "6953094737";  // your chat_id (or group id like -100xxxx)
// #include "esp_log.h"   // optional: for log level control

// static bool wifiStarted = false;
// static unsigned long lastReconnectTry = 0;

// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>

// #include <WiFi.h>
// #include <WiFiClientSecure.h>
// #include <HTTPClient.h>

// // ----------------- Your original pinout -----------------
// #define I2C_SDA 6
// #define I2C_SCL 7

// #define OLED_ADDR 0x3C
// Adafruit_SH1106G display(128, 64, &Wire, -1);

// // -------- Optional sensors --------
// #define USE_BME280 1
// #define USE_BH1750 1

// #if USE_BME280
//   #include <Adafruit_BME280.h>
//   Adafruit_BME280 bme;
//   bool bme_ok = false;
// #endif

// #if USE_BH1750
//   #include <BH1750.h>
//   BH1750 lightMeter;
//   bool bh_ok = false;
// #endif

// // ----------------- Soil sensor -----------------
// #define SOIL_PIN 3   // soil sensor on pin 3

// // ===================== EASY SETTINGS =====================
// bool SHOW_NUMBERS = false;         // set true if YOU want numbers on screen
// bool ENABLE_TELEGRAM = true;       // set false to disable Telegram alerts

// const char* PLANT_NAME = "Schefflera";

// // Temp offset (example: -5.0 subtracts 5C)
// const float TEMP_OFFSET_C = -5.0;

// // ---- WiFi + Telegram ----
// const char* WIFI_SSID = "ORBI76";
// const char* WIFI_PASS = "85600000";


// const char* TG_BOT_TOKEN = "8229863237:AAHkn_FSejsTF6VPJR0eKkPc0dwlTXor24o";
// const char* TG_CHAT_ID   = "6953094737";  // your chat_id (or group id like -100xxxx)

// // ---- Ranges (broad, not sensitive) ----
// // Light (lux)
// float LIGHT_TOO_LOW  = 150.0;      // below -> "More light"
// float LIGHT_TOO_HIGH = 7000.0;     // above -> "Less sun"

// // Temp (C)
// float TEMP_TOO_COLD = 17.0;        // below -> "Too cold"
// float TEMP_TOO_HOT  = 32.0;        // above -> "Too hot"

// // Soil (%) from calibration below
// int SOIL_TOO_DRY_PCT = 25;         // below -> "Water soon"
// int SOIL_TOO_WET_PCT = 85;         // above -> "Let dry"

// // ---- Soil calibration (RAW ADC) ----
// // 1) sensor in AIR -> read SoilRaw in Serial -> set SOIL_DRY_RAW
// // 2) sensor in WET SOIL / water -> set SOIL_WET_RAW
// int SOIL_DRY_RAW = 2800;
// int SOIL_WET_RAW = 1300;

// // Update rate (slower = less heat + less noise)
// const unsigned long UPDATE_MS = 2000;

// // Alerts (anti-spam)
// const unsigned long HOLD_LIGHT_MS = 10UL * 60UL * 1000UL;   // 10 min
// const unsigned long HOLD_TEMP_MS  = 10UL * 60UL * 1000UL;   // 10 min
// const unsigned long HOLD_SOIL_MS  = 30UL * 60UL * 1000UL;   // 30 min
// const unsigned long COOLDOWN_MS   = 6UL * 60UL * 60UL * 1000UL; // 6 hours
// // =========================================================

// enum PlantLevel : uint8_t { LV_LO = 0, LV_OK = 1, LV_HI = 2 };

// struct AlertState {
//   PlantLevel lastLv = LV_OK;
//   unsigned long badSince = 0;
//   unsigned long lastSent = 0;
// };

// AlertState alLight, alTemp, alSoil;

// float luxSm = NAN;
// float tempSm = NAN;
// int soilRawSm = -1;

// unsigned long lastUpdate = 0;

// // ------------------ OLED helper ------------------
// void printTrunc(int x, int y, uint8_t maxChars, const String &s) {
//   display.setCursor(x, y);
//   if ((int)s.length() <= maxChars) {
//     display.print(s);
//     return;
//   }
//   if (maxChars <= 1) return;
//   display.print(s.substring(0, maxChars - 1));
//   display.print(".");
// }

// // ------------------ Soil raw -> percent ------------------
// int soilPercentFromRaw(int raw, int dryRaw, int wetRaw) {
//   if (dryRaw == wetRaw) return 0;

//   float pct = 0.0f;
//   if (dryRaw > wetRaw) {
//     pct = (float)(dryRaw - raw) * 100.0f / (float)(dryRaw - wetRaw);
//   } else {
//     pct = (float)(raw - dryRaw) * 100.0f / (float)(wetRaw - dryRaw);
//   }

//   if (pct < 0) pct = 0;
//   if (pct > 100) pct = 100;
//   return (int)(pct + 0.5f);
// }

// // ------------------ Evaluators ------------------
// PlantLevel evalLight(float lux) {
//   if (lux < LIGHT_TOO_LOW)  return LV_LO;
//   if (lux > LIGHT_TOO_HIGH) return LV_HI;
//   return LV_OK;
// }
// PlantLevel evalTemp(float c) {
//   if (c < TEMP_TOO_COLD) return LV_LO;
//   if (c > TEMP_TOO_HOT)  return LV_HI;
//   return LV_OK;
// }
// PlantLevel evalSoil(int pct) {
//   if (pct < SOIL_TOO_DRY_PCT) return LV_LO;
//   if (pct > SOIL_TOO_WET_PCT) return LV_HI;
//   return LV_OK;
// }

// const char* statusShort(PlantLevel lv) {
//   if (lv == LV_OK) return "OK";
//   if (lv == LV_LO) return "LOW";
//   return "HIGH";
// }

// const char* adviceLight(PlantLevel lv) {
//   if (lv == LV_LO) return "More light";
//   if (lv == LV_HI) return "Less sun";
//   return "All good";
// }
// const char* adviceSoil(PlantLevel lv) {
//   if (lv == LV_LO) return "Water soon";
//   if (lv == LV_HI) return "Let dry";
//   return "All good";
// }
// const char* adviceTemp(PlantLevel lv) {
//   if (lv == LV_LO) return "Too cold";
//   if (lv == LV_HI) return "Too hot";
//   return "All good";
// }

// // ------------------ Small icons ------------------
// void iconSun(int x, int y) {
//   display.drawCircle(x+5, y+5, 3, SH110X_WHITE);
//   display.drawLine(x+5, y,   x+5, y+2, SH110X_WHITE);
//   display.drawLine(x+5, y+8, x+5, y+10, SH110X_WHITE);
//   display.drawLine(x,   y+5, x+2, y+5, SH110X_WHITE);
//   display.drawLine(x+8, y+5, x+10,y+5, SH110X_WHITE);
// }
// void iconDrop(int x, int y) {
//   display.fillCircle(x+5, y+4, 2, SH110X_WHITE);
//   display.fillTriangle(x+3, y+5, x+7, y+5, x+5, y+10, SH110X_WHITE);
// }
// void iconThermo(int x, int y) {
//   display.drawRect(x+4, y, 3, 8, SH110X_WHITE);
//   display.fillCircle(x+5, y+10, 2, SH110X_WHITE);
// }

// // ------------------ WiFi + Telegram ------------------
// bool ensureWiFi() {
//   if (!wifiStarted) return false;
//   if (WiFi.status() == WL_CONNECTED) return true;

//   // Don't spam reconnect attempts
//   unsigned long now = millis();
//   if (now - lastReconnectTry < 30000UL) return false; // try every 30s
//   lastReconnectTry = now;

//   WiFi.reconnect();  // does NOT set new config like begin()
//   return false;
// }

//   WiFi.mode(WIFI_STA);
//   WiFi.setAutoReconnect(true);
//   WiFi.persistent(false);
//   WiFi.setSleep(true);

//   WiFi.disconnect(true, true);
//   delay(50);
//   WiFi.begin(WIFI_SSID, WIFI_PASS);

//   unsigned long start = millis();
//   while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
//     delay(200);
//   }
//   return WiFi.status() == WL_CONNECTED;
// }

// String urlEncode(const String &s) {
//   String out;
//   out.reserve(s.length() * 2);
//   const char *hex = "0123456789ABCDEF";
//   for (size_t i = 0; i < s.length(); i++) {
//     char c = s[i];
//     if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')
//         || c == '-' || c == '_' || c == '.' || c == '~') out += c;
//     else if (c == ' ') out += "%20";
//     else {
//       out += '%';
//       out += hex[(c >> 4) & 0xF];
//       out += hex[c & 0xF];
//     }
//   }
//   return out;
// }

// bool sendTelegram(const String &msg) {
//   if (!ENABLE_TELEGRAM) return false;
//   if (String(TG_BOT_TOKEN).length() < 20) return false;
//   if (String(TG_CHAT_ID).length() < 3) return false;
//   if (!ensureWiFi()) return false;

//   WiFiClientSecure client;
//   client.setInsecure();

//   String url = String("https://api.telegram.org/bot") + TG_BOT_TOKEN
//              + "/sendMessage?chat_id=" + TG_CHAT_ID
//              + "&disable_notification=true"
//              + "&text=" + urlEncode(msg);

//   HTTPClient https;
//   if (!https.begin(client, url)) return false;

//   int code = https.GET();
//   https.end();

//   Serial.print("TG HTTP code = ");
//   Serial.println(code);

//   return (code > 0 && code < 300);
// }

// // Send only if bad state persists (hold) and cooldown passed
// void updateAlert(const char* name, AlertState &st, PlantLevel curLv, unsigned long holdMs, const String &detail) {
//   unsigned long now = millis();

//   if (curLv == LV_OK) {
//     st.badSince = 0;
//     st.lastLv = curLv;
//     return;
//   }

//   if (st.lastLv != curLv) {
//     st.badSince = now;
//     st.lastLv = curLv;
//     return;
//   }

//   if (st.badSince == 0) st.badSince = now;

//   if ((now - st.badSince) >= holdMs && (now - st.lastSent) >= COOLDOWN_MS) {
//     String text = String("Plant Buddy (") + PLANT_NAME + "): " + name + " needs attention.\n" + detail;
//     if (sendTelegram(text)) st.lastSent = now;
//   }
// }

// // ------------------ OLED UI ------------------
// void drawHeader(bool allGood) {
//   display.fillRect(0, 0, 128, 10, SH110X_WHITE);
//   display.setTextSize(1);
//   display.setTextColor(SH110X_BLACK);

//   display.setCursor(2, 1);
//   display.print(PLANT_NAME);

//   display.setCursor(86, 1);
//   display.print(allGood ? "HAPPY" : "CHECK");

//   display.setTextColor(SH110X_WHITE);
// }

// void drawRow(int y, const char* label, const char* stat, const char* advice, int iconType, const String &val) {
//   display.drawRoundRect(0, y, 128, 18, 4, SH110X_WHITE);

//   int ix = 3, iy = y + 3;
//   if (iconType == 0) iconSun(ix, iy);
//   if (iconType == 1) iconDrop(ix, iy);
//   if (iconType == 2) iconThermo(ix, iy);

//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);

//   display.setCursor(16, y + 2);
//   display.print(label);

//   display.setCursor(104, y + 2);
//   display.print(stat);

//   String adv = String(advice);
//   if (SHOW_NUMBERS && val.length()) {
//     adv += " ";
//     adv += val;
//   }

//   printTrunc(16, y + 10, 20, adv);
// }

// // ------------------ Setup / Loop ------------------
// void setup() {
// 	WiFi.mode(WIFI_STA);
// WiFi.persistent(false);
// WiFi.setAutoReconnect(true);
// WiFi.setSleep(true);

// WiFi.begin(WIFI_SSID, WIFI_PASS);
// wifiStarted = true;
//   Serial.begin(115200);

//   Wire.begin(I2C_SDA, I2C_SCL);

//   display.begin(OLED_ADDR, true);
//   display.clearDisplay();
//   display.display();

//   analogReadResolution(12);
//   analogSetAttenuation(ADC_11db);

// #if USE_BME280
//   bme_ok = bme.begin(0x76) || bme.begin(0x77);
//   if (bme_ok) {
//     // Forced mode reduces continuous self-heating
//     bme.setSampling(
//       Adafruit_BME280::MODE_FORCED,
//       Adafruit_BME280::SAMPLING_X1,  // temp
//       Adafruit_BME280::SAMPLING_X1,  // pressure
//       Adafruit_BME280::SAMPLING_X1,  // humidity
//       Adafruit_BME280::FILTER_OFF
//     );
//   }
// #endif

// #if USE_BH1750
//   bh_ok = lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE);
// #endif

//   // Splash
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);
//   display.setCursor(18, 10);
//   display.print("Plant Buddy Ready");
//   display.setCursor(18, 26);
//   display.print("Light / Water / Temp");
//   display.setCursor(18, 42);
//   display.print("Simple & Easy");
//   display.display();
//   delay(1200);
// }

// void loop() {
//   unsigned long now = millis();
//   if (now - lastUpdate < UPDATE_MS) return;
//   lastUpdate = now;

//   float lux = NAN, tempC = NAN;

// #if USE_BH1750
//   if (bh_ok) lux = lightMeter.readLightLevel();
// #endif

// #if USE_BME280
//   if (bme_ok) {
//     bme.takeForcedMeasurement();
//     tempC = bme.readTemperature() + TEMP_OFFSET_C;
//   }
// #endif

//   int soilRaw = analogRead(SOIL_PIN);

//   // Smooth
//   if (isnan(luxSm)) luxSm = lux;
//   else if (!isnan(lux)) luxSm = 0.8f * luxSm + 0.2f * lux;

//   if (isnan(tempSm)) tempSm = tempC;
//   else if (!isnan(tempC)) tempSm = 0.8f * tempSm + 0.2f * tempC;

//   if (soilRawSm < 0) soilRawSm = soilRaw;
//   else soilRawSm = (int)(0.85f * soilRawSm + 0.15f * soilRaw);

//   int soilPct = soilPercentFromRaw(soilRawSm, SOIL_DRY_RAW, SOIL_WET_RAW);

//   PlantLevel lvLight = isnan(luxSm)  ? LV_OK : evalLight(luxSm);
//   PlantLevel lvTemp  = isnan(tempSm) ? LV_OK : evalTemp(tempSm);
//   PlantLevel lvSoil  = evalSoil(soilPct);

//   bool allGood = (lvLight == LV_OK && lvTemp == LV_OK && lvSoil == LV_OK);

//   // Alerts (Telegram) — only if bad persists
//   updateAlert("LIGHT", alLight, lvLight, HOLD_LIGHT_MS, String(adviceLight(lvLight)));
//   updateAlert("TEMP",  alTemp,  lvTemp,  HOLD_TEMP_MS,  String(adviceTemp(lvTemp)));
//   updateAlert("WATER", alSoil,  lvSoil,  HOLD_SOIL_MS,  String(adviceSoil(lvSoil)));

//   // Draw OLED
//   display.clearDisplay();
//   drawHeader(allGood);

//   String lightVal = isnan(luxSm) ? "" : (String((int)luxSm) + "lx");
//   String tempVal  = isnan(tempSm) ? "" : (String(tempSm, 1) + "C");
//   String soilVal  = String(soilPct) + "%";

//   drawRow(10, "LIGHT", statusShort(lvLight), adviceLight(lvLight), 0, lightVal);
//   drawRow(28, "WATER", statusShort(lvSoil),  adviceSoil(lvSoil),  1, soilVal);
//   drawRow(46, "TEMP",  statusShort(lvTemp),  adviceTemp(lvTemp),  2, tempVal);

//   display.display();

//   // Serial debug
//   Serial.print("Lux=");
//   Serial.print(isnan(luxSm) ? -1 : luxSm);
//   Serial.print("  TempC=");
//   Serial.print(isnan(tempSm) ? -1 : tempSm);
//   Serial.print("  SoilRaw=");
//   Serial.print(soilRawSm);
//   Serial.print("  Soil%=");
//   Serial.println(soilPct);
// }



////////////////////////////////////////////////////////////////
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>

// #include <WiFi.h>
// #include <esp_wifi.h>
// #include <WiFiClientSecure.h>
// #include <HTTPClient.h>

// // ----------------- Your original pinout -----------------
// #define I2C_SDA 6
// #define I2C_SCL 7

// #define OLED_ADDR 0x3C
// Adafruit_SH1106G display(128, 64, &Wire, -1);

// // -------- Optional sensors --------
// #define USE_BME280 1
// #define USE_BH1750 1

// #if USE_BME280
//   #include <Adafruit_BME280.h>
//   Adafruit_BME280 bme;
//   bool bme_ok = false;
// #endif

// #if USE_BH1750
//   #include <BH1750.h>
//   BH1750 lightMeter;
//   bool bh_ok = false;
// #endif

// // ----------------- Soil sensor -----------------
// #define SOIL_PIN 3   // ESP32-C3 GPIO3

// // ===================== EASY SETTINGS =====================
// bool SHOW_NUMBERS      = false;   // keep false for gift
// bool ENABLE_TELEGRAM   = true;
// bool SEND_STARTUP_TEST = true;    // set true once to test, then set false

// const char* PLANT_NAME = "Schefflera";
// const float TEMP_OFFSET_C = -5.0f;    // -5 means subtract 5C from reading

// // ---- WiFi + Telegram ----
// const char* WIFI_SSID = "NETGEAR-Guest";
// const char* WIFI_PASS = "85600000";

// const char* TG_BOT_TOKEN = "8229863237:AAHkn_FSejsTF6VPJR0eKkPc0dwlTXor24o";
// const char* TG_CHAT_ID   = "6953094734";  // your private chat_id

// // ---- Ranges (broad) ----
// float LIGHT_TOO_LOW  = 120.0;
// float LIGHT_TOO_HIGH = 9000.0;

// float TEMP_TOO_COLD = 16.0;
// float TEMP_TOO_HOT  = 31.0;

// int SOIL_TOO_DRY_PCT = 25;
// int SOIL_TOO_WET_PCT = 85;

// // ---- Soil calibration (RAW ADC) ----
// int SOIL_DRY_RAW = 2800;
// int SOIL_WET_RAW = 1300;

// // Timing (slower to reduce heat/noise)
// const unsigned long SAMPLE_MS = 1000;

// // Alerts (anti-spam)
// const unsigned long HOLD_LIGHT_MS = 15UL * 60UL * 1000UL;
// const unsigned long HOLD_TEMP_MS  = 15UL * 60UL * 1000UL;
// const unsigned long HOLD_SOIL_MS  = 45UL * 60UL * 1000UL;
// const unsigned long COOLDOWN_MS   = 6UL  * 60UL * 60UL * 1000UL;
// // =========================================================

// enum PlantLevel : uint8_t { LV_LO = 0, LV_OK = 1, LV_HI = 2 };

// struct AlertState {
//   PlantLevel lastLv = LV_OK;
//   unsigned long badSince = 0;
//   unsigned long lastSent = 0;
// };

// AlertState alLight, alTemp, alSoil;

// float luxSm = NAN;
// float tempSm = NAN;
// int soilRawSm = -1;

// unsigned long lastSample = 0;

// // WiFi control (avoid re-config spam)
// static bool wifiStarted = false;
// static unsigned long lastReconnectTry = 0;

// // ------------------ UI helpers ------------------
// void printTrunc(int x, int y, uint8_t maxChars, const String &s) {
//   display.setCursor(x, y);
//   if ((int)s.length() <= maxChars) { display.print(s); return; }
//   if (maxChars <= 1) return;
//   display.print(s.substring(0, maxChars - 1));
//   display.print(".");
// }

// // icons
// void iconSun(int x, int y) {
//   display.drawCircle(x+5, y+5, 3, SH110X_WHITE);
//   display.drawLine(x+5, y,   x+5, y+2, SH110X_WHITE);
//   display.drawLine(x+5, y+8, x+5, y+10, SH110X_WHITE);
//   display.drawLine(x,   y+5, x+2, y+5, SH110X_WHITE);
//   display.drawLine(x+8, y+5, x+10,y+5, SH110X_WHITE);
// }
// void iconDrop(int x, int y) {
//   display.fillCircle(x+5, y+4, 2, SH110X_WHITE);
//   display.fillTriangle(x+3, y+5, x+7, y+5, x+5, y+10, SH110X_WHITE);
// }
// void iconThermo(int x, int y) {
//   display.drawRect(x+4, y, 3, 8, SH110X_WHITE);
//   display.fillCircle(x+5, y+10, 2, SH110X_WHITE);
// }

// void drawHeader(bool allGood) {
//   display.fillRect(0, 0, 128, 10, SH110X_WHITE);
//   display.setTextSize(1);
//   display.setTextColor(SH110X_BLACK);
//   display.setCursor(2, 1);
//   display.print(PLANT_NAME);
//   display.setCursor(86, 1);
//   display.print(allGood ? "HAPPY" : "CHECK");
//   display.setTextColor(SH110X_WHITE);
// }

// void drawRow(int y, const char* label, const char* stat, const char* advice, int iconType, const String &val) {
//   display.drawRoundRect(0, y, 128, 18, 4, SH110X_WHITE);

//   int ix = 3, iy = y + 3;
//   if (iconType == 0) iconSun(ix, iy);
//   if (iconType == 1) iconDrop(ix, iy);
//   if (iconType == 2) iconThermo(ix, iy);

//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);

//   display.setCursor(16, y + 2);
//   display.print(label);

//   display.setCursor(104, y + 2);
//   display.print(stat);

//   String adv = String(advice);
//   if (SHOW_NUMBERS && val.length()) {
//     adv += " ";
//     adv += val;
//   }
//   printTrunc(16, y + 10, 20, adv);
// }

// // ------------------ Soil raw -> percent ------------------
// int soilPercentFromRaw(int raw, int dryRaw, int wetRaw) {
//   if (dryRaw == wetRaw) return 0;
//   float pct;
//   if (dryRaw > wetRaw) pct = (float)(dryRaw - raw) * 100.0f / (float)(dryRaw - wetRaw);
//   else                pct = (float)(raw - dryRaw) * 100.0f / (float)(wetRaw - dryRaw);
//   if (pct < 0) pct = 0;
//   if (pct > 100) pct = 100;
//   return (int)(pct + 0.5f);
// }

// // ------------------ Evaluators ------------------
// PlantLevel evalLight(float lux) {
//   if (lux < LIGHT_TOO_LOW)  return LV_LO;
//   if (lux > LIGHT_TOO_HIGH) return LV_HI;
//   return LV_OK;
// }
// PlantLevel evalTemp(float c) {
//   if (c < TEMP_TOO_COLD) return LV_LO;
//   if (c > TEMP_TOO_HOT)  return LV_HI;
//   return LV_OK;
// }
// PlantLevel evalSoil(int pct) {
//   if (pct < SOIL_TOO_DRY_PCT) return LV_LO;
//   if (pct > SOIL_TOO_WET_PCT) return LV_HI;
//   return LV_OK;
// }

// const char* statusShort(PlantLevel lv) {
//   if (lv == LV_OK) return "OK";
//   if (lv == LV_LO) return "LOW";
//   return "HIGH";
// }

// const char* adviceLight(PlantLevel lv) {
//   if (lv == LV_LO) return "More light";
//   if (lv == LV_HI) return "Less sun";
//   return "All good";
// }
// const char* adviceSoil(PlantLevel lv) {
//   if (lv == LV_LO) return "Water soon";
//   if (lv == LV_HI) return "Let dry";
//   return "All good";
// }
// const char* adviceTemp(PlantLevel lv) {
//   if (lv == LV_LO) return "Too cold";
//   if (lv == LV_HI) return "Too hot";
//   return "All good";
// }

// // ------------------ WiFi + Telegram ------------------
// void startWiFiOnce() {
	
//   if (wifiStarted) return;

//   WiFi.persistent(false);
//   WiFi.mode(WIFI_STA);
//   WiFi.setAutoReconnect(true);
//   WiFi.setSleep(true);

//   Serial.println("WiFi: begin()");
//   WiFi.begin(WIFI_SSID, WIFI_PASS);
	

//   wifiStarted = true;
// }

// bool ensureWiFi(unsigned long timeoutMs = 0) {
//   startWiFiOnce();

//   if (WiFi.status() == WL_CONNECTED) return true;

//   // try reconnect occasionally (won't reconfigure)
//   unsigned long now = millis();
//   wl_status_t st = WiFi.status();
//   if ((st == WL_DISCONNECTED || st == WL_CONNECTION_LOST || st == WL_CONNECT_FAILED || st == WL_NO_SSID_AVAIL) &&
//       (now - lastReconnectTry > 30000UL)) {
//     lastReconnectTry = now;
//     Serial.println("WiFi: reconnect()");
//     WiFi.reconnect();
//   }

//   if (timeoutMs == 0) return (WiFi.status() == WL_CONNECTED);

//   unsigned long start = millis();
//   while (WiFi.status() != WL_CONNECTED && (millis() - start) < timeoutMs) {
//     delay(200);
//   }

//   if (WiFi.status() == WL_CONNECTED) {
//     Serial.print("WiFi: connected IP=");
//     Serial.println(WiFi.localIP());
//     return true;
//   }

//   Serial.println("WiFi: not connected (timeout)");
//   return false;
// }

// String urlEncode(const String &s) {
//   String out;
//   out.reserve(s.length() * 2);
//   const char *hex = "0123456789ABCDEF";
//   for (size_t i = 0; i < s.length(); i++) {
//     char c = s[i];
//     if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9')
//         || c == '-' || c == '_' || c == '.' || c == '~') out += c;
//     else if (c == ' ') out += "%20";
//     else {
//       out += '%';
//       out += hex[(c >> 4) & 0xF];
//       out += hex[c & 0xF];
//     }
//   }
//   return out;
// }

// bool sendTelegram(const String &msg) {
//   if (!ENABLE_TELEGRAM) return false;

//   // IMPORTANT: wait a bit so startup test actually sends
//   if (!ensureWiFi(15000)) {
//     Serial.println("TG: WiFi not connected");
//     return false;
//   }

//   WiFiClientSecure client;
//   client.setInsecure();

//   String url = String("https://api.telegram.org/bot") + TG_BOT_TOKEN + "/sendMessage";

//   HTTPClient https;
//   if (!https.begin(client, url)) {
//     Serial.println("TG: https.begin failed");
//     return false;
//   }

//   https.addHeader("Content-Type", "application/x-www-form-urlencoded");

//   String body = "chat_id=" + String(TG_CHAT_ID) +
//                 "&disable_notification=true" +
//                 "&text=" + urlEncode(msg);

//   int code = https.POST(body);
//   String resp = https.getString();
//   https.end();

//   Serial.print("TG HTTP code = ");
//   Serial.println(code);
//   Serial.print("TG response = ");
//   Serial.println(resp);

//   return (code > 0 && code < 300);
// }

// void updateAlert(const char* name, AlertState &st, PlantLevel curLv, unsigned long holdMs, const String &detail) {
//   unsigned long now = millis();

//   if (curLv == LV_OK) {
//     st.badSince = 0;
//     st.lastLv = curLv;
//     return;
//   }

//   if (st.lastLv != curLv) {
//     st.badSince = now;
//     st.lastLv = curLv;
//     return;
//   }

//   if (st.badSince == 0) st.badSince = now;

//   if ((now - st.badSince) >= holdMs && (now - st.lastSent) >= COOLDOWN_MS) {
//     String text = String("Plant Buddy (") + PLANT_NAME + "): " + name + " needs attention.\n" + detail;
//     if (sendTelegram(text)) st.lastSent = now;
//   }
// }

// // ------------------ Setup / Loop ------------------
// void setup() {
//   Serial.begin(115200);

//   Wire.begin(I2C_SDA, I2C_SCL);

//   display.begin(OLED_ADDR, true);
//   display.clearDisplay();
//   display.display();

//   analogReadResolution(12);
//   analogSetAttenuation(ADC_11db);

// #if USE_BME280
//   bme_ok = bme.begin(0x76) || bme.begin(0x77);
//   Serial.print("BME ok? ");
//   Serial.println(bme_ok ? "YES" : "NO");
//   if (bme_ok) {
//     bme.setSampling(Adafruit_BME280::MODE_FORCED,
//                     Adafruit_BME280::SAMPLING_X1,
//                     Adafruit_BME280::SAMPLING_NONE,
//                     Adafruit_BME280::SAMPLING_NONE,
//                     Adafruit_BME280::FILTER_OFF);
//   }
// #endif

// #if USE_BH1750
//   bh_ok = lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
//   Serial.print("BH1750 ok? ");
//   Serial.println(bh_ok ? "YES" : "NO");
// #endif

//   // Splash
//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);
//   display.setCursor(18, 12); display.print("Plant Buddy Ready");
//   display.setCursor(18, 28); display.print("Light / Water / Temp");
//   display.setCursor(18, 44); display.print("Simple & Easy");
//   display.display();
//   delay(900);

//   if (ENABLE_TELEGRAM) {
//     // Start WiFi now, and if we're doing test, wait for connect
//     startWiFiOnce();
//     if (SEND_STARTUP_TEST) {
//       sendTelegram(String("Plant Buddy: online ✅ (") + PLANT_NAME + ")");
//       // after you see it once, set SEND_STARTUP_TEST=false
//     }
//   }
// }

// void loop() {
//   unsigned long now = millis();
//   if (now - lastSample < SAMPLE_MS) return;
//   lastSample = now;

// //   float lux = NAN, tempC = NAN;

// // #if USE_BH1750
// //   if (bh_ok) {
// //     lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
// //     delay(180);
// //     lux = lightMeter.readLightLevel();
// //   }
// // #endif

// // #if USE_BME280
// //   if (bme_ok) {
// //     bme.takeForcedMeasurement();
// //     tempC = bme.readTemperature() + TEMP_OFFSET_C;
// //   }
// // #endif

// //   int soilRaw = analogRead(SOIL_PIN);

// //   // Smooth
// //   if (isnan(luxSm)) luxSm = lux;
// //   else if (!isnan(lux)) luxSm = 0.85f * luxSm + 0.15f * lux;

// //   if (isnan(tempSm)) tempSm = tempC;
// //   else if (!isnan(tempC)) tempSm = 0.85f * tempSm + 0.15f * tempC;

// //   if (soilRawSm < 0) soilRawSm = soilRaw;
// //   else soilRawSm = (int)(0.85f * soilRawSm + 0.15f * soilRaw);

// //   int soilPct = soilPercentFromRaw(soilRawSm, SOIL_DRY_RAW, SOIL_WET_RAW);

// //   // Evaluate
// //   PlantLevel lvLight = isnan(luxSm)  ? LV_OK : evalLight(luxSm);
// //   PlantLevel lvTemp  = isnan(tempSm) ? LV_OK : evalTemp(tempSm);
// //   PlantLevel lvSoil  = evalSoil(soilPct);

// //   bool allGood = (lvLight == LV_OK && lvTemp == LV_OK && lvSoil == LV_OK);

// //   // Alerts (Telegram)
// //   updateAlert("LIGHT", alLight, lvLight, HOLD_LIGHT_MS, adviceLight(lvLight));
// //   updateAlert("TEMP",  alTemp,  lvTemp,  HOLD_TEMP_MS,  adviceTemp(lvTemp));
// //   updateAlert("WATER", alSoil,  lvSoil,  HOLD_SOIL_MS,  adviceSoil(lvSoil));

// //   // OLED
// //   display.clearDisplay();
// //   drawHeader(allGood);

// //   String lightVal = isnan(luxSm) ? "" : (String((int)luxSm) + "lx");
// //   String tempVal  = isnan(tempSm) ? "" : (String(tempSm, 1) + "C");
// //   String soilVal  = String(soilPct) + "%";

// //   drawRow(10, "LIGHT", statusShort(lvLight), adviceLight(lvLight), 0, lightVal);
// //   drawRow(28, "WATER", statusShort(lvSoil),  adviceSoil(lvSoil),  1, soilVal);
// //   drawRow(46, "TEMP",  statusShort(lvTemp),  adviceTemp(lvTemp),  2, tempVal);

// //   display.display();

// //   // Serial debug
// //   Serial.print("Lux=");      Serial.print(isnan(luxSm) ? -1 : luxSm);
// //   Serial.print("  TempC=");  Serial.print(isnan(tempSm) ? -1 : tempSm);
// //   Serial.print("  SoilRaw=");Serial.print(soilRawSm);
// //   Serial.print("  Soil%=");  Serial.println(soilPct);
// // // }
// #include <Wire.h>
// #include <Adafruit_GFX.h>
// #include <Adafruit_SH110X.h>

// // ----------------- Pinout -----------------
// #define I2C_SDA 6
// #define I2C_SCL 7
// #define SOIL_PIN 3   // ESP32-C3 GPIO3

// #define OLED_ADDR 0x3C
// Adafruit_SH1106G display(128, 64, &Wire, -1);

// // Optional sensors (still displayed, but NO flashing)
// #define USE_BME280 1
// #define USE_BH1750 1

// #if USE_BME280
//   #include <Adafruit_BME280.h>
//   Adafruit_BME280 bme;
//   bool bme_ok = false;
// #endif

// #if USE_BH1750
//   #include <BH1750.h>
//   BH1750 lightMeter;
//   bool bh_ok = false;
// #endif

// // ===================== EASY SETTINGS =====================
// const char* PLANT_NAME = "Schefflera";
// bool SHOW_NUMBERS = false;            // keep false for gift
// const float TEMP_OFFSET_C = -5.0f;    // optional

// // Sampling (slow + stable)
// const unsigned long SAMPLE_MS = 1000; // read every 5 seconds

// // ---- Soil calibration (RAW ADC) ----
// int SOIL_DRY_RAW = 2800;
// int SOIL_WET_RAW = 1300;

// // ---- Soil thresholds (% after calibration) ----
// int SOIL_TOO_DRY_PCT = 25;        // below this = "Water soon"
// int SOIL_OKAY_PCT    = 30;        // must recover above this to clear (hysteresis)

// // ---- Flash behavior (SOIL ONLY) ----
// bool FLASH_ENABLED = true;
// // unsigned long SOIL_FLASH_AFTER_MS = 2UL * 60UL * 60UL * 1000UL; // 2 hours dry -> flash
// unsigned long SOIL_FLASH_AFTER_MS = 5UL * 1000UL;  // 5 seconds (TEST)
// unsigned long SOIL_CLEAR_MS       = 1UL * 1000UL;        // 2 min ok -> stop flashing
// unsigned long FLASH_PERIOD_MS     = 350;                        // flash speed

// // ===================== RANGES (for display only) =====================
// // Light (lux)
// float LIGHT_TOO_LOW  = 120.0;
// float LIGHT_TOO_HIGH = 9000.0;

// // Temp (C)
// float TEMP_TOO_COLD = 16.0;
// float TEMP_TOO_HOT  = 31.0;
// // =========================================================

// // ----- Smoothed readings -----
// float luxSm = NAN;
// float tempSm = NAN;
// int soilRawSm = -1;

// unsigned long lastSample = 0;

// // ----- Flash state -----
// bool flashActive = false;
// bool invertOn = false;
// unsigned long drySince = 0;
// unsigned long okSince = 0;
// unsigned long lastFlashToggle = 0;

// // ------------------ Helpers ------------------
// int soilPercentFromRaw(int raw, int dryRaw, int wetRaw) {
//   if (dryRaw == wetRaw) return 0;

//   float pct;
//   if (dryRaw > wetRaw) pct = (float)(dryRaw - raw) * 100.0f / (float)(dryRaw - wetRaw);
//   else                 pct = (float)(raw - dryRaw) * 100.0f / (float)(wetRaw - dryRaw);

//   if (pct < 0) pct = 0;
//   if (pct > 100) pct = 100;
//   return (int)(pct + 0.5f);
// }

// const char* lightAdvice(float lux) {
//   if (isnan(lux)) return "All good";
//   if (lux < LIGHT_TOO_LOW)  return "More light";
//   if (lux > LIGHT_TOO_HIGH) return "Less sun";
//   return "All good";
// }
// const char* tempAdvice(float c) {
//   if (isnan(c)) return "All good";
//   if (c < TEMP_TOO_COLD) return "Too cold";
//   if (c > TEMP_TOO_HOT)  return "Too hot";
//   return "All good";
// }
// const char* soilAdvice(int pct) {
//   if (pct < SOIL_TOO_DRY_PCT) return "Water soon";
//   return "All good";
// }

// void printTrunc(int x, int y, uint8_t maxChars, const String &s) {
//   display.setCursor(x, y);
//   if ((int)s.length() <= maxChars) { display.print(s); return; }
//   if (maxChars <= 1) return;
//   display.print(s.substring(0, maxChars - 1));
//   display.print(".");
// }

// // icons
// void iconSun(int x, int y) {
//   display.drawCircle(x+5, y+5, 3, SH110X_WHITE);
//   display.drawLine(x+5, y,   x+5, y+2, SH110X_WHITE);
//   display.drawLine(x+5, y+8, x+5, y+10, SH110X_WHITE);
//   display.drawLine(x,   y+5, x+2, y+5, SH110X_WHITE);
//   display.drawLine(x+8, y+5, x+10,y+5, SH110X_WHITE);
// }
// void iconDrop(int x, int y) {
//   display.fillCircle(x+5, y+4, 2, SH110X_WHITE);
//   display.fillTriangle(x+3, y+5, x+7, y+5, x+5, y+10, SH110X_WHITE);
// }
// void iconThermo(int x, int y) {
//   display.drawRect(x+4, y, 3, 8, SH110X_WHITE);
//   display.fillCircle(x+5, y+10, 2, SH110X_WHITE);
// }

// void drawHeader(bool happy) {
//   display.fillRect(0, 0, 128, 10, SH110X_WHITE);
//   display.setTextSize(1);
//   display.setTextColor(SH110X_BLACK);

//   display.setCursor(2, 1);
//   display.print(PLANT_NAME);

//   display.setCursor(86, 1);
//   display.print(happy ? "HAPPY" : "CHECK");

//   display.setTextColor(SH110X_WHITE);
// }

// void drawRow(int y, const char* label, const char* advice, int iconType, const String &val) {
//   display.drawRoundRect(0, y, 128, 18, 4, SH110X_WHITE);

//   int ix = 3, iy = y + 3;
//   if (iconType == 0) iconSun(ix, iy);
//   if (iconType == 1) iconDrop(ix, iy);
//   if (iconType == 2) iconThermo(ix, iy);

//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);

//   display.setCursor(16, y + 2);
//   display.print(label);

//   String adv = String(advice);
//   if (SHOW_NUMBERS && val.length()) {
//     adv += " ";
//     adv += val;
//   }
//   printTrunc(16, y + 10, 20, adv);
// }

// // ------------------ SOIL-ONLY flash logic ------------------
// void updateSoilFlash(unsigned long now, int soilPct) {
//   if (!FLASH_ENABLED) {
//     flashActive = false;
//     invertOn = false;
//     display.invertDisplay(false);
//     drySince = 0;
//     okSince = 0;
//     return;
//   }

//   bool isDry = (soilPct < SOIL_TOO_DRY_PCT);
//   bool isRecovered = (soilPct >= SOIL_OKAY_PCT); // hysteresis so it doesn’t chatter

//   if (isDry) {
//     okSince = 0;
//     if (drySince == 0) drySince = now;

//     if (!flashActive && (now - drySince) >= SOIL_FLASH_AFTER_MS) {
//       flashActive = true;
//       lastFlashToggle = 0;
//     }
//   } else {
//     drySince = 0;

//     // Only clear when it has actually recovered above OKAY threshold
//     if (isRecovered) {
//       if (okSince == 0) okSince = now;
//       if (flashActive && (now - okSince) >= SOIL_CLEAR_MS) {
//         flashActive = false;
//         invertOn = false;
//         display.invertDisplay(false);
//       }
//     } else {
//       okSince = 0; // in between dry + okay, keep waiting
//     }
//   }

//   if (flashActive) {
//     if (lastFlashToggle == 0 || (now - lastFlashToggle) >= FLASH_PERIOD_MS) {
//       lastFlashToggle = now;
//       invertOn = !invertOn;
//       display.invertDisplay(invertOn);
//     }
//   }
// }

// // ------------------ Setup / Loop ------------------
// void setup() {
//   Serial.begin(115200);

//   Wire.begin(I2C_SDA, I2C_SCL);

//   display.begin(OLED_ADDR, true);
//   display.clearDisplay();
//   display.invertDisplay(false);
//   display.display();

//   analogReadResolution(12);
//   analogSetAttenuation(ADC_11db);

// #if USE_BME280
//   bme_ok = bme.begin(0x76) || bme.begin(0x77);
//   if (bme_ok) {
//     bme.setSampling(Adafruit_BME280::MODE_FORCED,
//                     Adafruit_BME280::SAMPLING_X1,
//                     Adafruit_BME280::SAMPLING_NONE,
//                     Adafruit_BME280::SAMPLING_NONE,
//                     Adafruit_BME280::FILTER_OFF);
//   }
// #endif

// #if USE_BH1750
//   bh_ok = lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
// #endif

//   display.clearDisplay();
//   display.setTextSize(1);
//   display.setTextColor(SH110X_WHITE);
//   display.setCursor(18, 12); display.print("Plant Buddy Ready");
//   display.setCursor(18, 28); display.print("Soil alert only");
//   display.setCursor(18, 44); display.print("Simple & Easy");
//   display.display();
//   delay(900);
// }

// void loop() {
//   unsigned long now = millis();

//   // Sample sensors on schedule
//   if (now - lastSample >= SAMPLE_MS) {
//     lastSample = now;

//     float lux = NAN, tempC = NAN;

// #if USE_BH1750
//     if (bh_ok) {
//       lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
//       delay(180);
//       lux = lightMeter.readLightLevel();
//     }
// #endif

// #if USE_BME280
//     if (bme_ok) {
//       bme.takeForcedMeasurement();
//       tempC = bme.readTemperature() + TEMP_OFFSET_C;
//     }
// #endif

//     int soilRaw = analogRead(SOIL_PIN);

//     // Smooth
//     if (isnan(luxSm)) luxSm = lux;
//     else if (!isnan(lux)) luxSm = 0.85f * luxSm + 0.15f * lux;

//     if (isnan(tempSm)) tempSm = tempC;
//     else if (!isnan(tempC)) tempSm = 0.85f * tempSm + 0.15f * tempC;

//     if (soilRawSm < 0) soilRawSm = soilRaw;
//     else soilRawSm = (int)(0.85f * soilRawSm + 0.15f * soilRaw);

//     int soilPct = soilPercentFromRaw(soilRawSm, SOIL_DRY_RAW, SOIL_WET_RAW);

//     // Update soil-only flashing
//     updateSoilFlash(now, soilPct);

//     // Draw UI
//     bool happy = (soilPct >= SOIL_TOO_DRY_PCT); // headline mood mostly soil-based

//     display.clearDisplay();
//     drawHeader(happy);

//     String lightVal = isnan(luxSm) ? "" : (String((int)luxSm) + "lx");
//     String tempVal  = isnan(tempSm) ? "" : (String(tempSm, 1) + "C");
//     String soilVal  = String(soilPct) + "%";

//     drawRow(10, "LIGHT", lightAdvice(luxSm), 0, lightVal);
//     drawRow(28, "WATER", soilAdvice(soilPct), 1, soilVal);
//     drawRow(46, "TEMP",  tempAdvice(tempSm), 2, tempVal);

//     display.display();

//     // Serial debug
//     Serial.print("Lux="); Serial.print(isnan(luxSm) ? -1 : luxSm);
//     Serial.print("  TempC="); Serial.print(isnan(tempSm) ? -1 : tempSm);
//     Serial.print("  SoilRaw="); Serial.print(soilRawSm);
//     Serial.print("  Soil%="); Serial.print(soilPct);
//     Serial.print("  Flash="); Serial.println(flashActive ? "ON" : "OFF");
//   }

//   // Keep flashing responsive between samples
//   int soilPctNow = (soilRawSm < 0) ? 0 : soilPercentFromRaw(soilRawSm, SOIL_DRY_RAW, SOIL_WET_RAW);
//   updateSoilFlash(millis(), soilPctNow);
//}//////////////////////////////////////


#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

#define USE_BME280 1
#define USE_BH1750 1

#if USE_BME280
  #include <Adafruit_BME280.h>
  Adafruit_BME280 bme;
  bool bme_ok = false;
#endif

#if USE_BH1750
  #include <BH1750.h>
  BH1750 lightMeter;
  bool bh_ok = false;
#endif

// ----------------- Pinout -----------------
#define I2C_SDA 6
#define I2C_SCL 7
#define OLED_ADDR 0x3C
Adafruit_SH1106G display(128, 64, &Wire, -1);

// Soil sensor (ESP32-C3 GPIO3 ADC)
#define SOIL_PIN 3

// ===================== SETTINGS =====================
bool SHOW_NUMBERS = false;   // gift: keep false
bool DEBUG_SERIAL = true;    // you can turn off later

const char* PLANT_NAME = "Schefflera";
const float TEMP_OFFSET_C = -5.0f;  // subtract 5°C from BME reading

// Broad ranges (not sensitive)
float LIGHT_TOO_LOW  = 120.0f;
float LIGHT_TOO_HIGH = 9000.0f;

float TEMP_TOO_COLD  = 16.0f;
float TEMP_TOO_HOT   = 31.0f;

int SOIL_TOO_DRY_PCT = 25;
int SOIL_TOO_WET_PCT = 85;

// Soil calibration
int SOIL_DRY_RAW = 2800;   // in AIR
int SOIL_WET_RAW = 1300;   // in WET soil / water

// Timing
const unsigned long SAMPLE_MS = 1000;

// Soil warning (flash) behavior
unsigned long SOIL_BAD_HOLD_MS   = 5000;  // TEST: 5s (real: 2UL*60UL*60UL*1000UL)
unsigned long FLASH_BLINK_MS     = 350;
unsigned long WATERED_CONFIRM_MS = 1000;

// Jab behavior
// unsigned long JAB_INTERVAL_MS = 5000; // TEST: 20s (real: 3UL*60UL*60UL*1000UL)
unsigned long JAB_INTERVAL_MS = 30UL * 60UL * 1000UL; // 30 minutes


unsigned long JAB_SHOW_MS     = 3000;
// ====================================================

enum PlantLevel : uint8_t { LV_LO = 0, LV_OK = 1, LV_HI = 2 };

// Smoothed readings
float luxSm = NAN;
float tempSm = NAN;
int   soilRawSm = -1;

unsigned long lastSample = 0;

// Soil trend
int lastSoilPct = -1;
int soilTrend = 0; // -1 dryer, 0 same, +1 wetter

// Warning state
unsigned long soilBadSince = 0;
bool flashActive = false;
unsigned long wateredUntil = 0;

// Jab state
unsigned long stableSince = 0;
unsigned long jabUntil = 0;
uint8_t jabIndex = 0;

// SUPER short jabs (always fit)
const char* jabLines[] = {
  "yay im alive",
  "still alive :)",
  "we vibin",
  "dont let me die",
  "water = love",
  "im doin ok",
  "i survived!!",
  "thanks human"
};
const uint8_t JAB_COUNT = sizeof(jabLines) / sizeof(jabLines[0]);

// ---------- Helpers ----------
int soilPercentFromRaw(int raw, int dryRaw, int wetRaw) {
  if (dryRaw == wetRaw) return 0;
  float pct;
  if (dryRaw > wetRaw) pct = (float)(dryRaw - raw) * 100.0f / (float)(dryRaw - wetRaw);
  else                 pct = (float)(raw - dryRaw) * 100.0f / (float)(wetRaw - dryRaw);
  if (pct < 0) pct = 0;
  if (pct > 100) pct = 100;
  return (int)(pct + 0.5f);
}

PlantLevel evalLight(float lux) {
  if (lux < LIGHT_TOO_LOW)  return LV_LO;
  if (lux > LIGHT_TOO_HIGH) return LV_HI;
  return LV_OK;
}
PlantLevel evalTemp(float c) {
  if (c < TEMP_TOO_COLD) return LV_LO;
  if (c > TEMP_TOO_HOT)  return LV_HI;
  return LV_OK;
}
PlantLevel evalSoil(int pct) {
  if (pct < SOIL_TOO_DRY_PCT) return LV_LO;
  if (pct > SOIL_TOO_WET_PCT) return LV_HI;
  return LV_OK;
}

// small status words that make sense per row
const char* statLight(PlantLevel lv){ return (lv==LV_OK)?"OK":(lv==LV_LO)?"LOW":"HIGH"; }
const char* statTemp(PlantLevel lv) { return (lv==LV_OK)?"OK":(lv==LV_LO)?"COLD":"HOT"; }
const char* statSoil(PlantLevel lv) { return (lv==LV_OK)?"OK":(lv==LV_LO)?"DRY":"WET";  }

// short advice (fits) — OK text is now "i see the light"
const char* adviceLight(PlantLevel lv){
  if(lv==LV_LO) return "more light";
  if(lv==LV_HI) return "less sun";
  return "all good";
}

const char* adviceTemp(PlantLevel lv){
  if(lv==LV_LO) return "too cold";
  if(lv==LV_HI) return "too hot";
  return "all good";
}

const char* adviceSoil(PlantLevel lv){
  if(lv==LV_LO) return "water pls";
  if(lv==LV_HI) return "let dry";
  return "all good"; // <-- keep this ONLY for water OK if you want
}

// only ^ or v, else nothing
char arrowOnly(int t){
  if(t>0) return '^';
  if(t<0) return 'v';
  return 0;
}

void printTrunc(int x, int y, uint8_t maxChars, const String &s) {
  display.setCursor(x, y);
  if ((int)s.length() <= (int)maxChars) { display.print(s); return; }
  if (maxChars <= 1) return;
  display.print(s.substring(0, maxChars - 1));
  display.print(".");
}

// ---------- Icons (same proven ones from your “fits” version) ----------
void iconSun(int x, int y) {
  display.drawCircle(x+5, y+5, 3, SH110X_WHITE);
  display.drawLine(x+5, y,   x+5, y+2, SH110X_WHITE);
  display.drawLine(x+5, y+8, x+5, y+10, SH110X_WHITE);
  display.drawLine(x,   y+5, x+2, y+5, SH110X_WHITE);
  display.drawLine(x+8, y+5, x+10,y+5, SH110X_WHITE);
}
void iconDrop(int x, int y) {
  display.fillCircle(x+5, y+4, 2, SH110X_WHITE);
  display.fillTriangle(x+3, y+5, x+7, y+5, x+5, y+10, SH110X_WHITE);
}
void iconThermo(int x, int y) {
  display.drawRect(x+4, y, 3, 8, SH110X_WHITE);
  display.fillCircle(x+5, y+10, 2, SH110X_WHITE);
}

// tiny plant doodle (fits)
void doodlePlant(int x,int y){
  display.drawRoundRect(x, y+8, 14, 8, 2, SH110X_WHITE);
  display.drawLine(x+2, y+8, x+12, y+8, SH110X_WHITE);
  display.drawLine(x+7, y+8, x+7, y+2, SH110X_WHITE);
  display.drawLine(x+7, y+4, x+3, y+2, SH110X_WHITE);
  display.drawLine(x+7, y+4, x+11,y+2, SH110X_WHITE);
}

// =====================
// HEALTH SCREEN LAYOUT (BORROWED)
// Header = 10px
// Rows at y = 10, 28, 46 (height 18)
// =====================
void drawHeader(bool happy) {
  display.fillRect(0, 0, 128, 10, SH110X_WHITE);
  display.setTextSize(1);
  display.setTextColor(SH110X_BLACK);

  display.setCursor(2, 1);
  display.print(PLANT_NAME);

  // removed "CHECK" like you wanted; only show HAPPY when all good
  if (happy) {
    display.setCursor(86, 1);
    display.print("HAPPY");
  }

  display.setTextColor(SH110X_WHITE);
}

// Draw row that STILL includes: label + optional arrow + status + advice/value
void drawHealthRow(int y,
                   const char* label,
                   const char* stat,
                   const char* advice,
                   int iconType,
                   char trendArrow,
                   const String &val) {
  display.drawRoundRect(0, y, 128, 18, 4, SH110X_WHITE);

  int ix = 3, iy = y + 3;
  if (iconType == 0) iconSun(ix, iy);
  if (iconType == 1) iconDrop(ix, iy);
  if (iconType == 2) iconThermo(ix, iy);

  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  // label
  display.setCursor(16, y + 2);
  display.print(label);

  // arrow (only if provided)
  if (trendArrow) {
    display.setCursor(94, y + 2);
    display.print(trendArrow);
  }

  // status
  display.setCursor(104, y + 2);
  display.print(stat);

  // advice line
  String adv = String(advice);
  if (SHOW_NUMBERS && val.length()) {
    adv += " ";
    adv += val;
  }
  printTrunc(16, y + 10, 20, adv);
}

// ---------- Special screens ----------
void drawSoilWarningScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(30, 8);
  display.print("WATER LOW");

  iconDrop(54, 18);

  display.setCursor(26, 40);
  display.print("water me pls");
  display.setCursor(18, 54);
  display.print("i see the light");

  display.display();
}

void drawWateredScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  doodlePlant(8, 8);

  display.setCursor(30, 12);
  display.print("YAY WATER!");
  display.setCursor(30, 28);
  display.print("thank u <3");

  display.setCursor(22, 52);
  display.print("i feel better");

  display.display();
}

void drawJabScreen(){
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);

  display.setCursor(34, 8);
  display.print("STABLE");

  doodlePlant(10, 18);

  String msg = String(jabLines[jabIndex]);
  // keep it short so it always fits
  if ((int)msg.length() > 18) msg = msg.substring(0, 18);

  display.setCursor(30, 34);
  display.print(msg);

  display.setCursor(30, 52);
  

  display.display();
}

// ---------- Setup / Loop ----------
void setup(){
  Serial.begin(115200);
  Wire.begin(I2C_SDA, I2C_SCL);

  display.begin(OLED_ADDR, true);
  display.clearDisplay();
  display.invertDisplay(false);
  display.display();

  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);

#if USE_BME280
  bme_ok = bme.begin(0x76) || bme.begin(0x77);
  if(bme_ok){
    bme.setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1,
                    Adafruit_BME280::SAMPLING_NONE,
                    Adafruit_BME280::SAMPLING_NONE,
                    Adafruit_BME280::FILTER_OFF);
  }
#endif

#if USE_BH1750
  bh_ok = lightMeter.begin(BH1750::ONE_TIME_HIGH_RES_MODE);
#endif

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(34, 18); display.print("Plant Buddy");
  display.setCursor(26, 36); display.print("ready :)");
  display.display();
  delay(700);
}

void loop(){
  unsigned long now = millis();
  if (jabUntil != 0 && now >= jabUntil) jabUntil = 0;
  if(now - lastSample < SAMPLE_MS) return;
  lastSample = now;

  // ---- Read sensors ----
  float lux = NAN, tempC = NAN;

#if USE_BH1750
  if(bh_ok){
    lightMeter.configure(BH1750::ONE_TIME_HIGH_RES_MODE);
    delay(180);
    lux = lightMeter.readLightLevel();
  }
#endif

#if USE_BME280
  if(bme_ok){
    bme.takeForcedMeasurement();
    tempC = bme.readTemperature() + TEMP_OFFSET_C;
  }
#endif

  int soilRaw = analogRead(SOIL_PIN);

  // ---- Smooth ----
  if(isnan(luxSm)) luxSm = lux;
  else if(!isnan(lux)) luxSm = 0.85f*luxSm + 0.15f*lux;

  if(isnan(tempSm)) tempSm = tempC;
  else if(!isnan(tempC)) tempSm = 0.85f*tempSm + 0.15f*tempC;

  if(soilRawSm < 0) soilRawSm = soilRaw;
  else soilRawSm = (int)(0.85f*soilRawSm + 0.15f*soilRaw);

  int soilPct = soilPercentFromRaw(soilRawSm, SOIL_DRY_RAW, SOIL_WET_RAW);

  // ---- Trend (only show arrow when WATER is OK) ----
  if(lastSoilPct >= 0){
    int d = soilPct - lastSoilPct;
    if(d >= 2) soilTrend = +1;
    else if(d <= -2) soilTrend = -1;
    else soilTrend = 0;
  }
  lastSoilPct = soilPct;

  // ---- Evaluate ----
  PlantLevel lvLight = isnan(luxSm)  ? LV_OK : evalLight(luxSm);
  PlantLevel lvTemp  = isnan(tempSm) ? LV_OK : evalTemp(tempSm);
  PlantLevel lvSoil  = evalSoil(soilPct);

  bool allGood = (lvLight==LV_OK && lvTemp==LV_OK && lvSoil==LV_OK);

  // ---- Soil warning flash after hold ----
  bool soilDry = (lvSoil == LV_LO);

  if(soilDry){
    if(soilBadSince == 0) soilBadSince = now;
    if(!flashActive && (now - soilBadSince >= SOIL_BAD_HOLD_MS)){
      flashActive = true;
    }
  } else {
    soilBadSince = 0;
    if(flashActive){
      flashActive = false;
      wateredUntil = now + WATERED_CONFIRM_MS;
      display.invertDisplay(false);
    }
  }

  // ---- Stable -> jab every interval (needs 2/3 OK) ----
  int okCount = (lvLight==LV_OK) + (lvTemp==LV_OK) + (lvSoil==LV_OK);

  if(okCount >= 2 && !flashActive){
    if(stableSince == 0) stableSince = now;

    if(jabUntil == 0 && (now - stableSince >= JAB_INTERVAL_MS)){
      jabUntil = now + JAB_SHOW_MS;
      jabIndex = (jabIndex + 1) % JAB_COUNT;
      stableSince = now; // restart interval
    }
  } else {
    stableSince = 0;
  }

  // ---- Screen priority ----
  if(flashActive){
    drawSoilWarningScreen();
    bool inv = ((now / FLASH_BLINK_MS) % 2) == 1;
    display.invertDisplay(inv);
  }
  else if(wateredUntil > now){
    display.invertDisplay(false);
    drawWateredScreen();
  }
  else if(jabUntil > now){
    display.invertDisplay(false);
    drawJabScreen();
    if(now >= jabUntil) jabUntil = 0;
  }
  else {
    display.invertDisplay(false);
    display.clearDisplay();

    // ----- HEALTH SCREEN (borrowed layout) -----
    drawHeader(allGood);

    String lightVal = isnan(luxSm) ? "" : (String((int)luxSm) + "lx");
    String tempVal  = isnan(tempSm) ? "" : (String(tempSm,1) + "C");
    String soilVal  = String(soilPct) + "%";

    // only show water arrow when WATER is OK
    char wArrow = (lvSoil==LV_OK) ? arrowOnly(soilTrend) : 0;

    drawHealthRow(10, "LIGHT", statLight(lvLight), adviceLight(lvLight), 0, 0,      lightVal);
    drawHealthRow(28, "WATER", statSoil(lvSoil),   adviceSoil(lvSoil),   1, wArrow, soilVal);
    drawHealthRow(46, "TEMP",  statTemp(lvTemp),   adviceTemp(lvTemp),   2, 0,      tempVal);

    display.display();
  }

  if(DEBUG_SERIAL){
    Serial.print("Lux="); Serial.print(isnan(luxSm)?-1:luxSm);
    Serial.print(" TempC="); Serial.print(isnan(tempSm)?-1:tempSm);
    Serial.print(" SoilRaw="); Serial.print(soilRawSm);
    Serial.print(" Soil%="); Serial.print(soilPct);
    Serial.print(" Trend="); Serial.print(soilTrend);
    Serial.print(" Flash="); Serial.println(flashActive?"Y":"N");
  }
}



