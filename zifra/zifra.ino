// #include <ArduinoOTA.h>
#include <ArduinoJson.h> // V5.13.5!!!
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EasyButton.h>
#include <FS.h>
#include <TickerScheduler.h>
#include <WebSocketsServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <Wire.h>

#define DEBUG 0
#if DEBUG
#define D_SerialBegin(...) Serial.begin(__VA_ARGS__)
#define D_print(...) Serial.print(__VA_ARGS__)
#define D_write(...) Serial.write(__VA_ARGS__)
#define D_println(...) Serial.println(__VA_ARGS__)
#else
#define D_SerialBegin(...)
#define D_print(...)
#define D_write(...)
#define D_println(...)
#endif

#include "common_functions.h"
#include "Webinterface.h"
#include "i2cscanner.h"
#include "pitches.h"
#include "zifra.h"
// notes in the melody:
int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3,
                NOTE_G3, 0,       NOTE_B3, NOTE_C4
               };

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};

char identifier[24];

// Set I2C address
int address = 0x20;

byte a = B00000000;
byte b = B00000000;

// Define NTP Client to get time
WiFiUDP ntpUDP;
Zifra zifra(ntpUDP);

// dns
const uint8_t DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
char cmDNS[33];
String escapedMac;

// Button
#define BUTTON_PIN 0
EasyButton button(BUTTON_PIN, 40, true, true);

#define UP_BUTTON_PIN 5
EasyButton up_button(UP_BUTTON_PIN, 10, true, true);

//// HTTP Config

#define COMPILE_HOUR (((__TIME__[0] - '0') * 10) + (__TIME__[1] - '0'))
#define COMPILE_MINUTE (((__TIME__[3] - '0') * 10) + (__TIME__[4] - '0'))
#define COMPILE_SECOND (((__TIME__[6] - '0') * 10) + (__TIME__[7] - '0'))
#define COMPILE_YEAR                                                           \
  ((((__DATE__[7] - '0') * 10 + (__DATE__[8] - '0')) * 10 +                    \
    (__DATE__[9] - '0')) *                                                     \
   10 +                                                                    \
   (__DATE__[10] - '0'))
#define COMPILE_SHORTYEAR (((__DATE__[9] - '0')) * 10 + (__DATE__[10] - '0'))
#define COMPILE_MONTH                                                          \
  ((__DATE__[2] == 'n'   ? (__DATE__[1] == 'a' ? 0 : 5)                        \
    : __DATE__[2] == 'b' ? 1                                                   \
    : __DATE__[2] == 'r' ? (__DATE__[0] == 'M' ? 2 : 3)                        \
    : __DATE__[2] == 'y' ? 4                                                   \
    : __DATE__[2] == 'l' ? 6                                                   \
    : __DATE__[2] == 'g' ? 7                                                   \
    : __DATE__[2] == 'p' ? 8                                                   \
    : __DATE__[2] == 't' ? 9                                                   \
    : __DATE__[2] == 'v' ? 10                                                  \
    : 11) +                                               \
   1)
#define COMPILE_DAY                                                            \
  ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

const String version = String(COMPILE_SHORTYEAR) + IntFormat(COMPILE_MONTH) +
                       IntFormat(COMPILE_DAY) + IntFormat(COMPILE_HOUR) +
                       IntFormat(COMPILE_MINUTE);

WiFiClient espClient;
WiFiManager wifiManager;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
ESP8266HTTPUpdateServer httpUpdater;

String OldInfo = ""; // old board info
// Websoket Vars
String websocketConnection[10];

TickerScheduler ticker(5);

// BUTTON
//  Attach callback.

void doRestart() {
  D_println(F("5 Clicks"));
  ESP.restart();
}
void doFactoryReset() {
  D_println(F("15 Seconds Long Click"));
  Handle_factoryreset();
}

/////////////////////////////////////////////////////////////////////
void WifiSetup() {
  wifiManager.resetSettings();
  ESP.restart();
  delay(300);
}

void HandleGetMainPage() {
  server.sendHeader("Connection", "close");
  server.send(200, "text/html", mainPage);
}

#pragma region //////////////////////////// HTTP API
///////////////////////////////
void HandleNotFound() {
  if (server.method() == HTTP_OPTIONS) {
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(204);
  }

  server.sendHeader("Location", "/update", true);
  server.send(302, "text/plain", "");
}
void HandleGetConfig() {
  server.sendHeader("Connection", "close");
  server.send(200, "application/json", zifra.conf.getConfig());
}
void Handle_wifisetup() {
  WifiSetup();
}
void Handle_factoryreset() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Log("Handle_factoryreset", "Failed to open config file for reset");
  }
  configFile.println("");
  configFile.close();
  WifiSetup();
  ESP.restart();
}
void SendConfig() {
  if (webSocket.connectedClients() > 0) {
    for (int i = 0;
         i < sizeof websocketConnection / sizeof websocketConnection[0]; i++) {
      if (websocketConnection[i] == "/settings" ||
          websocketConnection[i] == "/settime" ||
          websocketConnection[i] == "/setalarm" ||
          websocketConnection[i] == "/setsystem"

         ) {
        String config = zifra.conf.getConfig();
        webSocket.sendTXT(i, config);
      }
    }
  }
}
/////////////////////////////////////////////////////////////////////
void Log(String function, String message) {

  const String timeStamp = IntFormat(zifra.time.getYear()) + "-" + IntFormat(zifra.time.getMonth()) + "-" +
                           IntFormat(zifra.time.getMonthDay()) + "T" + IntFormat(zifra.time.getHoursIso()) + ":" +
                           IntFormat(zifra.time.getMinutes()) + ":" + IntFormat(zifra.time.getSeconds());

  D_println("[" + timeStamp + "] " + function + ": " + message);

  // Prüfen ob über Websocket versendet werden muss
  if (webSocket.connectedClients() > 0) {
    for (int i = 0;
         i < sizeof websocketConnection / sizeof websocketConnection[0]; i++) {
      if (websocketConnection[i] == "/main") {
        webSocket.sendTXT(i, "{\"log\":{\"timeStamp\":\"" + timeStamp +
                          "\",\"function\":\"" + function +
                          "\",\"message\":\"" + message + "\"}}");
      }
    }
  }
}

String GetInfo() {
  DynamicJsonDocument root(1024);

  root["gumboardVersion"] = version;
  root["sketchSize"] = ESP.getSketchSize();
  root["freeSketchSpace"] = ESP.getFreeSketchSpace();
  root["wifiRSSI"] = String(WiFi.RSSI());
  root["wifiQuality"] = GetRSSIasQuality(WiFi.RSSI());
  root["wifiSSID"] = WiFi.SSID();
  root["ipAddress"] = WiFi.localIP().toString();
  root["freeHeap"] = ESP.getFreeHeap();
  root["chipID"] = ESP.getChipId();
  root["cpuFreqMHz"] = ESP.getCpuFreqMHz();
  root["clock_sleep"] = zifra.conf.clock.sleep;

  String week_day;
  const uint8_t weekDay = zifra.time.getWeekDay();
  switch (weekDay) {
    case 0:
      week_day = "Sunday";
      break;
    case 1:
      week_day = "Monday";
      break;
    case 2:
      week_day = "Tuesday";
      break;
    case 3:
      week_day = "Wednesday";
      break;
    case 4:
      week_day = "Thursday";
      break;
    case 5:
      week_day = "Friday";
      break;
    case 6:
      week_day = "Saturday";
      break;

  }

  root["weekday"] = week_day + "(" + String(weekDay) + ")";
  String json;
  serializeJson(root, json);

  return json;
}

void SendInfo(bool force) {
  String Info;

  // Prüfen ob die ermittlung der MatrixInfo überhaupt erforderlich ist
  if ((webSocket.connectedClients() > 0)) {
    Info = GetInfo();
  }

  // Prüfen ob über Websocket versendet werden muss
  if (webSocket.connectedClients() > 0 && OldInfo != Info) {
    for (int i = 0;
         i < sizeof websocketConnection / sizeof websocketConnection[0]; i++) {
      if (websocketConnection[i] == "/main" ||
          websocketConnection[i] == "/api/info") {
        webSocket.sendTXT(i, Info);
      }
    }
  }
  OldInfo = Info;
}
#pragma region //////////////////////////// Websocket
///////////////////////////////

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload,
                    size_t length) {

  switch (type) {
    case WStype_DISCONNECTED: {
        Log("WebSocketEvent", "[" + String(num) + "] Disconnected!");
        websocketConnection[num] = "";
        break;
      }
    case WStype_CONNECTED: {
        // Merken für was die Connection hergstellt wurde
        websocketConnection[num] = String((char *)payload);

        // IP der Connection abfragen
        IPAddress ip = webSocket.remoteIP(num);

        // Logausgabe
        Log("WebSocketEvent", "[" + String(num) + "] Connected from " +
            ip.toString() +
            " url: " + websocketConnection[num]);

        // send message to client
        SendInfo(true);
        SendConfig();
        break;
      }
    case WStype_TEXT: {
        if (((char *)payload)[0] == '{') {
          DynamicJsonDocument json(512);

          deserializeJson(json, payload);

          // Logausgabe
          Log("WebSocketEvent",
              "Incomming Json length: " + String(measureJson(json)));

          if (websocketConnection[num] == "/setConfig") {
            JsonObject object = json.as<JsonObject>();
            zifra.conf.setConfig(object);
          }
        }
        break;
      }
    case WStype_BIN:
      // Serial.printf("[%u] get binary length: %u\n", num, length);
      // hexdump(payload, length);

      // send message to client
      // webSocket.sendBIN(num, payload, length);
      break;
  }
}
#pragma endregion

/////////////////////////////////////////////////////////////////////
// BUTTON
//  Attach callback.
void singleClick() {
  zifra.alarm.mute();
  tone(4, NOTE_C4, 1000 / 16);
  // stop the tone playing:
  delay(200);
  noTone(4);
  D_println(F("singleClick"));
  Log(F("singleClick"), F("singleClick!"));
}
void upClick() {
  zifra.vol.sleepShakeTime = millis();
  zifra.alarm.mute();
  tone(4, NOTE_C4, 1000 / 16);
  // stop the tone playing:
  delay(200);
  noTone(4);
  D_println(F("upClick"));
  Log(F("upClick"), F("upClick!"));
}
void toggleWifi() {
  zifra.conf.wifiActive = !zifra.conf.wifiActive;
  zifra.conf.saveConfigCallback();
  zifra.conf.saveConfig();
  delay(1000);
  ESP.restart();
}
// CLOCK
//  Function for writing two Bytes to the I2C expander device
void pf575_write(uint16_t data) {
  Wire.beginTransmission(address);
  Wire.write(lowByte(data));
  Wire.write(highByte(data));
  Wire.endTransmission();
}

uint16_t pcf8575_read() {
  Wire.beginTransmission(address);
  Wire.endTransmission();
  Wire.requestFrom(address, 2);
  if (Wire.available()) {
    uint8_t lo = Wire.read();
    uint8_t hi = Wire.read();
    return (word(hi, lo)); // joing bytes
  }
  return 0;
}

// turn all ports off
void turn_all_off() {
  // turn all off
  a = B00000000;
  b = B00000000;
  pf575_write(word(a, b));
}

void turn_on() {
  // turn on
  pf575_write(word(a, b));
}

void show_number(int num, bool dot = false) {
  if (dot == true) {
    bitSet(a, 2); // dot pin
  }
  if (num < 8) {
    bitSet(b, num); // 0-7 Pins
  } else {
    num = num - 8;
    bitSet(a, num); // 8-9 + dot Pins
  }
  turn_on();
  D_print(num);
}

void read_time() {
  zifra.time.update();
  zifra.vol.clockNum = 0;
}

void time2nixie()
{
  const uint8_t val1 = zifra.time.getNum(0);
  const uint8_t val2 = zifra.time.getNum(1);
  const uint8_t val3 = zifra.time.getNum(2);
  const uint8_t val4 = zifra.time.getNum(3);

  unsigned long l_millis = millis();
  if (val1 != 0 || zifra.conf.clock.leadingHourZero) {
    if (zifra.vol.clockNum == 0) {
      show_number(val1, false);
      zifra.vol.clockNum++;
      zifra.vol.clockDelay = l_millis;
    }
    // 100 delay
    if (zifra.vol.clockNum == 1 and l_millis - zifra.vol.clockDelay > 800) {
      turn_all_off();
      zifra.vol.clockNum++;
    }
  } else if (zifra.vol.clockNum < 2) {
    zifra.vol.clockNum = 2;
    zifra.vol.clockDelay = l_millis - 1000;
  }

  if (zifra.vol.clockNum == 2 && l_millis - zifra.vol.clockDelay > 1000) {
    show_number(val2, false);
    zifra.vol.clockNum++;
  }
  else if (zifra.vol.clockNum == 3 and l_millis - zifra.vol.clockDelay > 1800) {
    turn_all_off();
    zifra.vol.clockNum++;
  }
  else if (zifra.vol.clockNum == 4 and l_millis - zifra.vol.clockDelay > 2500 ) {
    show_number(val3, true);
    zifra.vol.clockNum++;
  }
  else if (zifra.vol.clockNum == 5 and l_millis - zifra.vol.clockDelay > 3300) {
    turn_all_off();
    zifra.vol.clockNum++;
  }
  else if (zifra.vol.clockNum == 6 && l_millis - zifra.vol.clockDelay > 3500) {
    show_number(val4, true);
    zifra.vol.clockNum++;
  }
  else if (zifra.vol.clockNum == 7 and l_millis - zifra.vol.clockDelay > 4300) {
    turn_all_off();
    zifra.vol.clockNum++;
  }
}

void show_time() {
  zifra.alarm.update();
  bool shakeWakeUp = ((millis() - zifra.vol.sleepShakeTime) <= 1000 * 60 * 3) && zifra.vol.sleepShakeTime > 0;  //  3 minutes shake wakeup
  if (shakeWakeUp == false && zifra.sleep())
  {
    turn_all_off();
  } else {
    time2nixie();
  }
}
// end clock
/*
   OTA UPDATES

  void setupOTA() {
    ArduinoOTA.onStart([]() { D_println("Start"); });
    ArduinoOTA.onEnd([]() { D_println("\nEnd"); });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
        Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
            Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });

    ArduinoOTA.setHostname(identifier);

    // This is less of a security measure and more a accidential flash
  prevention ArduinoOTA.setPassword(identifier); ArduinoOTA.begin();
  }
*/
void saveConfigCallback() {
  zifra.conf.saveConfigCallback();
}
void setupWifi() {

  wifiManager.setDebugOutput(true);
  // Set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.setMinimumSignalQuality();
  // Config menue timeout 180 seconds.

  wifiManager.setConfigPortalTimeout(180);

  WiFi.hostname(identifier);
  // set custom ip for portal
  wifiManager.setAPStaticIPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  if (!wifiManager.autoConnect(identifier)) {
    D_println(F("Setup: Wifi failed to connect and hit timeout"));
    delay(3000);
    // Reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  }

  D_println(F("Wifi connected...yeey :)"));
  zifra.conf.saveConfig();
  D_println("Setup: Local IP");
  D_println("Setup " + WiFi.localIP().toString());
  D_println("Setup " + WiFi.gatewayIP().toString());
  D_println("Setup " + WiFi.subnetMask().toString());

  setupMDSN();
}

void setupMDSN() {
  // generate module IDs
  escapedMac = WiFi.macAddress();
  escapedMac.replace(":", "");
  escapedMac.toLowerCase();
  strcpy_P(cmDNS, PSTR("zifra"));
  // Set up mDNS responder:
  if (strlen(cmDNS) > 0) {
    // "end" must be called before "begin" is called a 2nd time
    // see https://github.com/esp8266/Arduino/issues/7213
    MDNS.end();
    MDNS.begin(cmDNS);
    D_println(cmDNS);
    D_println(F("mDNS started"));
    MDNS.addService("http", "tcp", 80);
    MDNS.addService("zifra", "tcp", 80);
    MDNS.addServiceTxt("zifra", "tcp", "mac", escapedMac.c_str());
  }
}
void setRtcCb()
{
  zifra.time.setRTC();
}

void startUpTone()
{
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    const int noteDuration = 1000 / noteDurations[thisNote];
    tone(4, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    const int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(4);
  }
}
void startUpNumbers()
{
  // Set all ports as output
  turn_all_off();
  // iterate over the notes of the melody:
  for (int digit = 0; digit < 11; digit++) {
    turn_all_off();
    delay(10);
    show_number(digit);
    delay(100);
  }
  // Set all ports as output
  turn_all_off();
}
void setup() {
  snprintf(identifier, sizeof(identifier), "ZIFRA-%X", ESP.getChipId());

  Wire.begin(2, 0);
  startUpNumbers();

  Serial.begin(115200);
  while (!Serial)
    ;

  // Mounting FileSystem
  D_println(F("Mounting file system..."));
  if (SPIFFS.begin()) {
    D_println(F("Successfully mounted file system."));
    zifra.conf.loadConfig();
  } else {
    D_println(F("Failed to mount FS"));
  }

  i2cScanner::scan();

  if (i2cScanner::addressExists("0x68")) {
    zifra.conf.DS3231_active = true;
  }
  
  startUpTone();

  zifra.conf.setCb(setRtcCb);
  zifra.time.setTimeOffset();

  if (zifra.conf.wifiActive) {
    setupWifi();

    httpUpdater.setup(&server);
    server.on(F("/update"), HTTP_GET, HandleNotFound);
    server.on(F("/"), HTTP_GET, HandleGetMainPage);
    server.onNotFound(HandleNotFound);

    server.begin();

    webSocket.begin();
    webSocket.onEvent(webSocketEvent);
    Log(F("Setup"), F("Webserver started"));

    delay(1000);
    zifra.time.begin();
  } else {
    // Turn off WiFi
    WiFi.mode(WIFI_OFF);
  }

  ticker.add(
  0, 100, [&](void *) {
    show_time();
  }, nullptr, true);
  ticker.add(
  1, 5500, [&](void *) {
    read_time();
  }, nullptr, true);

  // Initialize the button.
  button.begin();
  // Attach callback.
  button.onPressed(singleClick);

  // Attach callback to factory reset after 15 seconds press.
  button.onPressedFor(35000, doFactoryReset);

  // Initialize the button.
  up_button.begin();
  // Attach callback.
  up_button.onPressed(upClick);

  // Attach callback to after 10 seconds press.
  up_button.onPressedFor(10000, toggleWifi);
}

void loop() {
  button.read();
  up_button.read();
  yield();
  if (zifra.conf.wifiActive) {
    MDNS.update();
    server.handleClient();
    yield();
    webSocket.loop();
    yield();
  }

  ticker.update();
  yield();
}
