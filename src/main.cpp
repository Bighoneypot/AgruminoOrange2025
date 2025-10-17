/*
  Gabriele Foddis – gabriele.foddis@lifely.cc
  REVISION 10.3.4 / 2025-10
  Battery-only version with timed Captive Portal,
  advanced button control and Wi-Fi fallback
*/

#include <Arduino.h>
#include <AgruminoOrange.h>
#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManagerWithAPI.h>
#include <ArduinoJson.h>
#include <OrangeSupportss.h>

AgruminoOrange agrumino;
DynamicJsonDocument doc(1024);
WiFiClient client;


constexpr uint16_t CP_TIMEOUT_SECONDS = 60;     // Captive Portal active time
constexpr uint32_t CP_SLEEP_SECONDS   = 3600;   // 30 min deep-sleep after timeout


void delaySec(int sec) { delay(sec * 1000); }

String readJsonStringFromResponse(String response, String jsonKey) {
  int keyIndex = response.indexOf("\"" + jsonKey + "\":");
  if (keyIndex >= 0) {
    response = response.substring(keyIndex + jsonKey.length() + 4);
    int closingQuoteIndex = response.indexOf("\"");
    return response.substring(0, closingQuoteIndex);
  }
  return "";
}

String getRegisterDeviceBodyJsonString(String deviceKey) {
  String input = "{}";
  deserializeJson(doc, input);
  JsonObject jsonPost = doc.as<JsonObject>();
  jsonPost["key"] = deviceKey;
  String jsonPostString;
  serializeJson(doc, jsonPostString);
  return jsonPostString;
}

String doPostApiCall(const char *host, String api, String bodyJsonString) {
  while (!client.connect(host, 80)) {
    Serial.println("Connection failed");
    delay(1000);
  }
  Serial.println("Connected to Lifely host");

  client.println(String("POST ") + api + " HTTP/1.1");
  client.println(String("Host: ") + host + ":80");
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(bodyJsonString.length()));
  client.println();
  client.println(bodyJsonString);

  delay(10);
  int timeout = 300;
  while (!client.available()) {
    delay(100);
    yield();
    if (--timeout <= 0) {
      Serial.println("Error: client.available() timeout reached");
      return "";
    }
  }

  String response;
  while (client.available()) response += (char)client.read();
  response.trim(); response.replace("\n", ""); response.replace("\r", "");
  Serial.println("API response:"); Serial.println(response);
  return response;
}

String getSendDataBodyJsonString(String deviceKey, String deviceToken, float temp,
                                 int soil, unsigned int lux, float battVolt,
                                 unsigned int battLevel, boolean usb, boolean charge) {
  String input = "{}";
  deserializeJson(doc, input);
  JsonObject jsonPost = doc.as<JsonObject>();
  jsonPost["device_key"] = deviceKey;
  jsonPost["device_token"] = deviceToken;
  jsonPost["temperature"] = String(temp);
  jsonPost["humidity"] = String(soil);
  jsonPost["battery"] = String(battLevel);
  jsonPost["light"] = String(lux);
  String jsonPostString;
  serializeJson(doc, jsonPostString);
  return jsonPostString;
}

String registerDevice(String deviceKey) {
  Serial.println("Registering device key: " + deviceKey);
  String body = getRegisterDeviceBodyJsonString(deviceKey);
  String response = doPostApiCall(WEB_SERVER_HOST, WEB_SERVER_API_REGISTER_DEVICE, body);
  String token = readJsonStringFromResponse(response, "token");
  Serial.println("Received token: " + token);
  return token;
}

const String getChipId() { return String(ESP.getChipId()); }
String getDeviceKey() { return DEVICE_KEY.length() ? DEVICE_KEY : deviceType + getChipId(); }
String getDeviceToken(String deviceKey) {
  return DEVICE_TOKEN.length() ? DEVICE_TOKEN : registerDevice(deviceKey);
}

void blinkLed(int duration, int blinks) {
  for (int i = 0; i < blinks; i++) {
    agrumino.turnLedOn(); delay(duration);
    agrumino.turnLedOff(); if (i < blinks - 1) delay(duration);
  }
}


void enterConfigPortal(WiFiManagerWithAPI &wifiManager) {
  agrumino.turnLedOn();
  String ssidAP = getDeviceKey();
  Serial.println("Starting configuration portal with SSID: " + ssidAP);
  bool result = wifiManager.startConfigPortal(ssidAP.c_str());
  agrumino.turnLedOff();
  Serial.println(result ? "Configuration saved, rebooting" :
                          "Portal closed without configuration");
  agrumino.deepSleepSec(2);
}

void enterTimedConfigPortal(WiFiManagerWithAPI &wifiManager, uint16_t seconds) {
  agrumino.turnLedOn();
  String ssidAP = getDeviceKey();
  Serial.println("Starting timed configuration portal (" + String(seconds) + "s) with SSID: " + ssidAP);
  wifiManager.setConfigPortalTimeout(seconds);
  bool result = wifiManager.startConfigPortal(ssidAP.c_str());
  agrumino.turnLedOff();

  if (result) {
    Serial.println("Configuration updated via Captive Portal - rebooting");
    agrumino.deepSleepSec(2);
  } else {
    Serial.println("No user configuration within timeout - entering deep sleep for 30 minutes");
    agrumino.turnBoardOff();
    agrumino.deepSleepSec(CP_SLEEP_SECONDS);
  }
}


void setup() {
  Serial.begin(115200);
  delay(800);                  // stabilizza la COM dopo il flash
  agrumino.setup();            // genera FULLNAME
  delay(300);
  Serial.println();
  Serial.println("FULLNAME_TAG:" + FULLNAME);
  Serial.flush();              // forza l'invio del buffer
  delay(2000);                             // forza l’invio immediato sulla seriale

  // -----------------------------
  // Informazioni di supporto e Wi-Fi setup
  // -----------------------------
  printSupport();   // mostra il banner e i dati di supporto (già presente nel tuo codice)

  agrumino.turnLedOff();   // disattiva LED iniziale
  delay(300);

  // -----------------------------
  // WiFi Manager & Portal logic
  // -----------------------------
  WiFiManagerWithAPI wifiManager;
  bool hasWifiCredentials = WiFi.SSID().length() > 0;
  bool configPortalRequest = false;
  bool factoryResetRequest = false;
  unsigned long pressStart = 0;

  // -----------------------------
  // Rilevamento pulsante all'avvio
  // -----------------------------
  bool buttonInitiallyPressed = agrumino.isButtonPressed();
  if (buttonInitiallyPressed) {
    Serial.println("Button detected pressed at boot - detecting hold time");
    pressStart = millis();
    int lastSecond = 0;
    bool factoryBlinkStarted = false;

    while (agrumino.isButtonPressed()) {
      unsigned long held = millis() - pressStart;
      int seconds = held / 1000;

      if (seconds != lastSecond) {
        Serial.print("Hold time: ");
        Serial.print(seconds);
        Serial.println("s");
        lastSecond = seconds;
      }

      if (held > 5000 && held <= 20000 && !factoryBlinkStarted) {
        agrumino.turnLedOn(); delay(300);
        agrumino.turnLedOff(); delay(300);
      }

      if (seconds >= 21 && !factoryBlinkStarted) {
        Serial.println("Factory reset requested - holding LED fast blink until release");
        factoryBlinkStarted = true;
      }

      if (factoryBlinkStarted) {
        agrumino.turnLedOn(); delay(80);
        agrumino.turnLedOff(); delay(80);
      }
      yield();
    }

    unsigned long pressDuration = millis() - pressStart;
    if (pressDuration >= 5000 && pressDuration < 20000) configPortalRequest = true;
    else if (pressDuration >= 20000) factoryResetRequest = true;
  } else {
    Serial.println("No button pressed at boot");
  }

  // -----------------------------
  // Factory Reset (20+ s)
  // -----------------------------
  if (factoryResetRequest) {
    Serial.println("Executing factory reset - clearing WiFi and settings");
    wifiManager.resetSettings();
    agrumino.turnLedOn(); delay(3000); agrumino.turnLedOff();
    enterConfigPortal(wifiManager);
    return;
  }

  // -----------------------------
  // Config Portal Request (5–20 s)
  // -----------------------------
  if (configPortalRequest) {
    Serial.println("Config portal requested - starting AP mode");
    agrumino.turnLedOn();
    enterConfigPortal(wifiManager);
    return;
  }

  // -----------------------------
  // Avvio normale (nessun pulsante premuto)
  // -----------------------------
  if (!hasWifiCredentials) {
    Serial.println("No WiFi credentials found - opening timed Config Portal");
    enterTimedConfigPortal(wifiManager, CP_TIMEOUT_SECONDS);
    return;
  }

  // -----------------------------
  // Tentativi connessione Wi-Fi
  // -----------------------------
  const int maxAttempts = 3;
  bool connected = false;

  for (int attempt = 1; attempt <= maxAttempts && !connected; attempt++) {
    Serial.print("Connecting to saved WiFi (attempt ");
    Serial.print(attempt);
    Serial.println(" of 3)");
    WiFi.mode(WIFI_STA);
    WiFi.begin();

    unsigned long startAttemptTime = millis();
    const unsigned long wifiTimeout = 15000;

    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < wifiTimeout) {
      Serial.print(".");
      delay(500);
      yield();
    }

    if (WiFi.status() == WL_CONNECTED) connected = true;
    else Serial.println("\nConnection failed");
  }

  // -----------------------------
  // Fallback Captive Portal o avvio normale
  // -----------------------------
  if (!connected) {
    Serial.println("WiFi connection failed after 3 attempts - opening timed Config Portal");
    enterTimedConfigPortal(wifiManager, CP_TIMEOUT_SECONDS);
    return;
  }

  Serial.print("Connected to WiFi. IP: ");
  Serial.println(WiFi.localIP());
}



void sendData(String deviceKey, String deviceToken, float temp, int soil,
              unsigned int lux, float battVolt, unsigned int battLevel,
              boolean usb, boolean charge) {
  String body = getSendDataBodyJsonString(deviceKey, deviceToken, temp, soil,
                                          lux, battVolt, battLevel, usb, charge);
  doPostApiCall(WEB_SERVER_HOST, WEB_SERVER_API_SEND_DATA, body);
}

void loop() {
  Serial.println("#########################");
  delay(500);
  agrumino.turnBoardOn();
  agrumino.turnLedOn();

  unsigned int soilMoistureRaw = agrumino.readSoilRaw();
  float temperature = agrumino.readTempC();
  unsigned int soilMoisture = agrumino.readSoil();
  float illuminance = agrumino.readLux();
  float batteryVoltage = agrumino.readBatteryVoltage();
  unsigned int batteryLevel = agrumino.readBatteryLevel();
  boolean isBatteryCharging = agrumino.isBatteryCharging();

  Serial.println("temperature:       " + String(temperature) + " °C");
  Serial.println("soilMoisture:      " + String(soilMoisture) + " %");
  Serial.println("soilRaw:           " + String(soilMoistureRaw) + " mV");
  Serial.println("illuminance:       " + String(illuminance) + " lux");
  Serial.println("batteryVoltage:    " + String(batteryVoltage) + " V");
  Serial.println("batteryLevel:      " + String(batteryLevel) + " %");
  Serial.println("isBatteryCharging: " + String(isBatteryCharging));
  Serial.println();

  String deviceKey = getDeviceKey();
  String deviceToken = getDeviceToken(deviceKey);
  sendData(deviceKey, deviceToken, temperature, soilMoisture,
           illuminance, batteryVoltage, batteryLevel, false, isBatteryCharging);

  blinkLed(200, 1);
  agrumino.turnBoardOff();
  agrumino.deepSleepSec(sleepTimeSec);
}
