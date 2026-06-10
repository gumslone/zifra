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
#ifndef DEBUG
#define DEBUG 0 // can be overridden with -DDEBUG=1 (see build.sh)
#endif
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
#include "zifra.h"

char identifier[24];

// Define NTP Client to get time
WiFiUDP ntpUDP;
Zifra zifra(ntpUDP);

const IPAddress apIP(192, 168, 4, 1);
char cmDNS[33];
String escapedMac;

// Buttons
#define BUTTON_PIN 0
EasyButton button(BUTTON_PIN, 40, true, true);

#define UP_BUTTON_PIN 5
EasyButton up_button(UP_BUTTON_PIN, 10, true, true);

//// HTTP Config

#define COMPILE_HOUR (((__TIME__[0] - '0') * 10) + (__TIME__[1] - '0'))
#define COMPILE_MINUTE (((__TIME__[3] - '0') * 10) + (__TIME__[4] - '0'))
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
    : 11) +                                                                    \
   1)
#define COMPILE_DAY                                                            \
  ((__DATE__[4] == ' ' ? 0 : __DATE__[4] - '0') * 10 + (__DATE__[5] - '0'))

const String version = String(COMPILE_SHORTYEAR) + IntFormat(COMPILE_MONTH) +
                       IntFormat(COMPILE_DAY) + IntFormat(COMPILE_HOUR) +
                       IntFormat(COMPILE_MINUTE);

WiFiManager wifiManager;
ESP8266WebServer server(80);
WebSocketsServer webSocket(81);
ESP8266HTTPUpdateServer httpUpdater;

String OldInfo = ""; // old board info
// Websocket Vars
constexpr uint8_t MAX_WS_CLIENTS = 10;
String websocketConnection[MAX_WS_CLIENTS];

TickerScheduler ticker(5);

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
void Handle_factoryreset() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    configFile.println("");
    configFile.close();
  } else {
    Log("Handle_factoryreset", "Failed to open config file for reset");
  }
  WifiSetup();
}

// Send a message to all websocket clients connected to one of the given paths
void SendToClients(String message,
                   std::initializer_list<const char *> paths) {
  if (webSocket.connectedClients() == 0) {
    return;
  }
  for (uint8_t i = 0; i < MAX_WS_CLIENTS; i++) {
    for (const char *path : paths) {
      if (websocketConnection[i] == path) {
        webSocket.sendTXT(i, message);
        break;
      }
    }
  }
}

void SendConfig() {
  SendToClients(zifra.conf.getConfig(),
                {"/settings", "/settime", "/setalarm", "/setsystem"});
}
/////////////////////////////////////////////////////////////////////
void Log(String function, String message) {

  const String timeStamp = IntFormat(zifra.time.getYear()) + "-" + IntFormat(zifra.time.getMonth()) + "-" +
                           IntFormat(zifra.time.getMonthDay()) + "T" + IntFormat(zifra.time.getHoursIso()) + ":" +
                           IntFormat(zifra.time.getMinutes()) + ":" + IntFormat(zifra.time.getSeconds());

  D_println("[" + timeStamp + "] " + function + ": " + message);

  SendToClients("{\"log\":{\"timeStamp\":\"" + timeStamp +
                "\",\"function\":\"" + function +
                "\",\"message\":\"" + message + "\"}}",
                {"/main"});
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

  static const char *const weekdayNames[7] = {
    "Sunday", "Monday", "Tuesday", "Wednesday",
    "Thursday", "Friday", "Saturday"
  };
  const uint8_t weekDay = zifra.time.getWeekDay();
  root["weekday"] = String(weekdayNames[weekDay % 7]) + "(" + String(weekDay) + ")";

  String json;
  serializeJson(root, json);

  return json;
}

void SendInfo(bool force) {
  if (webSocket.connectedClients() == 0) {
    return;
  }

  const String Info = GetInfo();
  if (force || OldInfo != Info) {
    SendToClients(Info, {"/main", "/api/info"});
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
        // Remember what the connection was established for
        websocketConnection[num] = String((char *)payload);

        const IPAddress ip = webSocket.remoteIP(num);

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
      break;
  }
}
#pragma endregion

/////////////////////////////////////////////////////////////////////
// BUTTON callbacks
void singleClick() {
  zifra.alarm.mute();
  zifra.buzzer.beep();
  D_println(F("singleClick"));
  Log(F("singleClick"), F("singleClick!"));
}
void upClick() {
  zifra.vol.sleepShakeTime = millis();
  zifra.alarm.mute();
  zifra.buzzer.beep();
  D_println(F("upClick"));
  Log(F("upClick"), F("upClick!"));
}
void toggleWifi() {
  zifra.conf.wifiActive = !zifra.conf.wifiActive;
  zifra.conf.saveConfig(true);
  delay(1000);
  ESP.restart();
}
void doFactoryReset() {
  D_println(F("Long Click"));
  Handle_factoryreset();
}

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

void setup() {
  snprintf(identifier, sizeof(identifier), "ZIFRA-%X", ESP.getChipId());
  Wire.begin(2, 0);

  zifra.startUpNixie();

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

  zifra.buzzer.startUpMelody();
  zifra.setUp();

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
    zifra.showTime();
  }, nullptr, true);
  ticker.add(
  1, 5500, [&](void *) {
    zifra.update();
  }, nullptr, true);

  // Initialize the buttons.
  button.begin();
  button.onPressed(singleClick);
  // Factory reset after a long press.
  button.onPressedFor(35000, doFactoryReset);

  up_button.begin();
  up_button.onPressed(upClick);
  // Toggle WiFi after a 10 seconds press.
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
