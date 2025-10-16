/*
  Created by giuseppe.broccia@lifely.cc on October 2017.
  Maintainer: gabriele.foddis@lifely.cc

  Latest changes and updates August 2022 by:
  
  Gabriele Foddis gabriele.foddis@lifely.cc
  for a better experience use only Visual Studio Code
  

  @see Agrumino.h for the documentation of the lib
*/
#include <Arduino.h>
#include <Agrumino.h>         // Our super cool lib ;)
#include <ESP8266WiFi.h>        
#include <DNSServer.h>          
#include <ESP8266WebServer.h>   
#include <WiFiManagerWithAPI.h> // Custom version of https://github.com/tzapu/WiFiManager
#include <ArduinoJson.h>        
#include <supportss.h>          // custom from Gabriele Foddis for support and secrets (ss) and dynamic data (Important!! Edit before build)


Agrumino agrumino;

// Used for sending Json POST requests
DynamicJsonDocument doc(1024);
// Used to create TCP connections and make Http calls
WiFiClient client;


void delaySec(int sec) {
  delay (sec * 1000);
}

String readJsonStringFromResponse(String response, String jsonKey) {
  int keyIndex = response.indexOf("\"" + jsonKey + "\":");
  if (keyIndex >= 0) {
    // 4 is the lenght of "":"
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


String doPostApiCall(const char* host, String api, String bodyJsonString) {

  // Use WiFiClient class to create TCP connections, we try until the connection is estabilished
  while (!client.connect(host, 80)) {
    Serial.println("connection failed\n");
    delay(1000);
  }
  Serial.println("connected to Lifely host ...amazing :)\n");

  // Print the HTTP POST API data for debug
  //Serial.println("Requesting POST: " + String(host) + api);
  //Serial.println("Requesting POST: " + bodyJsonString);

  // This will send the request to the server
  client.println("POST " + api + " HTTP/1.1");
  client.println("Host: " + String(host) + ":80");
  client.println("Content-Type: application/json");
  client.println("Content-Length: " + String(bodyJsonString.length()));
  client.println();
  client.println(bodyJsonString);

  delay(10);

  int timeout = 300; // 100 ms per loop so 30 sec.
  while (!client.available()) {
    // Waiting for server response
    delay(100);
    Serial.print(".");
    timeout--;
    if (timeout <= 0) {
      Serial.print("Err. client.available() timeout reached!");
      return "";
    }
  }

  String response = "";

  while (client.available() > 0) {
    char c = client.read();
    response = response + c;
  }

  // Remove bad chars from response
  response.trim();
  response.replace("/n", "");
  response.replace("/r", "");

  Serial.println("\nAPI Update successful! Response: \n");
  Serial.println(response);

  return response;
}

String getSendDataBodyJsonString(String deviceKey, String deviceToken, float temp, int soil, unsigned int lux, float battVolt, unsigned int battLevel, boolean usb, boolean charge) {
  String input = "{}";
  deserializeJson(doc, input);
  JsonObject jsonPost = doc.as<JsonObject>();

  jsonPost["device_key"] = deviceKey;
  jsonPost["device_token"] = deviceToken;
  jsonPost["temperature"] = String(temp);
  jsonPost["humidity"] = String(soil);
  jsonPost["battery"] = String(battLevel);
  jsonPost["light"]  = String(lux);
  // Other data is not supported yet on Backend

  String jsonPostString;
  serializeJson(doc, jsonPostString);


  return jsonPostString;
}

// If the Agrumino S1 button is pressed for 5 secs then reset the wifi saved settings.
// If "checkBattery" is true the method return true only if the USB is connected.

boolean checkIfResetWiFiSettings(boolean checkBattery) {
  int delayMs = 100;
  int remainingsLoops = (5 * 1000) / delayMs;

  Serial.println("\n");

  Serial.print("\nCheck if reset WiFi settings: ");
  while (remainingsLoops > 0
         && agrumino.isButtonPressed()
         && (agrumino.isAttachedToUSB() || !checkBattery) // The Agrumino must be attached to USB
        ) {
    // Blink the led every sec as confirmation
    if (remainingsLoops % 10 == 0) {
      agrumino.turnLedOn();
    }
    delay(delayMs);
    agrumino.turnLedOff();
    remainingsLoops--;
    Serial.print(".");
  }
  agrumino.turnLedOff();
  boolean success = (remainingsLoops == 0);
  Serial.println(success ? " YES!" : " NO");
  Serial.println();
  return success;


}

String registerDevice(String deviceKey) {
  Serial.println("Registering device key: " + deviceKey);
  String registerDeviceBody = getRegisterDeviceBodyJsonString(deviceKey);
  String response = doPostApiCall(WEB_SERVER_HOST, WEB_SERVER_API_REGISTER_DEVICE , registerDeviceBody);
  String token = readJsonStringFromResponse(response, "token");
  Serial.println("\nReceived token is: " + token + "\n");
  return token;
}


const String getChipId() {
  return String(ESP.getChipId());
}


String getDeviceKey() {
  if (DEVICE_KEY.length() > 0) {
    return DEVICE_KEY;
  } else {
    return deviceType + getChipId();
  }
}


String getDeviceToken(String deviceKey) {
  if (DEVICE_TOKEN.length() > 0) {
    return DEVICE_TOKEN;
  } else {
    return registerDevice(deviceKey);
  }
}

void reboot() {
  // ESP.reset() doesn't work on first reboot
  agrumino.deepSleepSec(1);
}


void blinkLed(int duration, int blinks) {
  for (int i = 0; i < blinks; i++) {
    agrumino.turnLedOn();
    delay(duration);
    agrumino.turnLedOff();
    if (i < blinks) {
      delay(duration); // Avoid delay in the latest loop ;)
    }
  }
}


void setup() {

  Serial.begin(115200);
  delay(200);
  agrumino.setup();
  printSupport();
  delay(1000);

  agrumino.turnBoardOn();
  delay(100);

  // WiFiManager Logic taken from
  // https://github.com/kentaylor/WiFiManager/blob/master/examples/ConfigOnSwitch/ConfigOnSwitch.ino

  // With batteryCheck true will return true only if the Agrumino is attached to USB with a valid power
  //boolean resetWifi = checkIfResetWiFiSettings(true);
  boolean resetWifi = checkIfResetWiFiSettings(true); 
  boolean hasWifiCredentials = WiFi.SSID().length() > 0;

  if (resetWifi || !hasWifiCredentials) {
    // Show Configuration portal

    // Blink and keep ON led as a feedback :)
    blinkLed(100, 5);
    agrumino.turnLedOn();

    boolean isBatteryPowered = agrumino.isAttachedToUSB();
    if (!isBatteryPowered) {
      // We allow the config portal only when USB powered
      // Put to sleep if no settings and no USB power
      agrumino.deepSleepSec(3600);
      return;
    }

    WiFiManagerWithAPI wifiManager;

    if (resetWifi) {
      Serial.println("into reseWifi");
      // clear wifiCredentials and reboot to avoid AP issues
      wifiManager.resetSettings();
      reboot();
      return;
    }
    // Sets timeout in seconds until configuration portal gets turned off.
    // If not specified device will remain in configuration mode until
    // switched off via webserver or device is restarted.
    // wifiManager.setConfigPortalTimeout(600);

    // Starts an access point and goes into a blocking loop awaiting configuration
    String ssidAP = getDeviceKey();
    boolean gotConnection = wifiManager.startConfigPortal(ssidAP.c_str());

    Serial.print("\nGot connection from Config Portal: ");
    Serial.println(gotConnection);

    agrumino.turnLedOff();

    reboot();
    return;

  } else {
    // Try to connect to saved network
    // Force to station mode because if device was switched off while in access point mode it will start up next time in access point mode.
    agrumino.turnLedOn();
    WiFi.mode(WIFI_STA);
    WiFi.waitForConnectResult();
  }

  boolean connected = (WiFi.status() == WL_CONNECTED);
  Serial.println("Wifi status is: " + String(WiFi.status()));

  if (connected) {
    // If you get here you have connected to the WiFi :D
    Serial.print("\nConnected to WiFi as ");
    Serial.print(WiFi.localIP());
    Serial.println(" ...so magic :)\n");
  } else {
    // No WiFi connection. Skip a cycle and retry later
    Serial.print("\nNot connected!\n");
    // ESP.reset() doesn't work on first reboot
    agrumino.deepSleepSec(sleepTimeSec);
  }
}

void sendData(String deviceKey, String deviceToken, float temp, int soil, unsigned int lux, float battVolt, unsigned int battLevel, boolean usb, boolean charge) {


  String sendDataBody = getSendDataBodyJsonString(deviceKey, deviceToken, temp,  soil,  lux,  battVolt, battLevel, usb, charge);
  doPostApiCall(WEB_SERVER_HOST, WEB_SERVER_API_SEND_DATA , sendDataBody);
}

void loop() {
  Serial.println("#########################\n");

  delay(500);
  agrumino.turnBoardOn();
  agrumino.turnLedOn();

  float temperature =         agrumino.readTempC();
  unsigned int soilMoisture = agrumino.readSoil();
  float illuminance =         agrumino.readLux();
  float batteryVoltage =      agrumino.readBatteryVoltage();
  unsigned int batteryLevel = agrumino.readBatteryLevel();
  boolean isAttachedToUSB =   agrumino.isAttachedToUSB();
  boolean isBatteryCharging = agrumino.isBatteryCharging();

  Serial.println("temperature:       " + String(temperature) + "°C");
  Serial.println("soilMoisture:      " + String(soilMoisture) + "%");
  Serial.println("illuminance :      " + String(illuminance) + " lux");
  Serial.println("batteryVoltage :   " + String(batteryVoltage) + " V");
  Serial.println("batteryLevel :     " + String(batteryLevel) + "%");
  Serial.println("isAttachedToUSB:   " + String(isAttachedToUSB));
  Serial.println("isBatteryCharging: " + String(isBatteryCharging));
  Serial.println();


  String deviceKey = getDeviceKey();

  // Register device on Lifely API
  // TODO: this should happen only the first time and the credential should be saved
  String deviceToken = getDeviceToken(deviceKey);

  // Send data to Lifely web service
  sendData(deviceKey, deviceToken, temperature, soilMoisture, illuminance, batteryVoltage, batteryLevel, isAttachedToUSB, isBatteryCharging);

  // Blink when the business is done for giving an Ack to the user
  blinkLed(200, 1);

  // Board off before delay/sleep to save battery :)
  agrumino.turnBoardOff();

  // delaySec(SLEEP_TIME_SEC); // The ESP8266 stays powered, executes the loop repeatedly
  agrumino.deepSleepSec(sleepTimeSec); // ESP8266 enter in deepSleep and after the selected time starts back from setup() and then loop()

}