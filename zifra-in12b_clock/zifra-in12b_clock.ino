// #include <ArduinoOTA.h>
#include <ArduinoJson.h> // V5.13.5!!!
#include <ESP8266HTTPClient.h>
#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <EasyButton.h>
#include <FS.h>
#include <NTPClient.h>
#include <TickerScheduler.h>
#include <WebSocketsServer.h>
#include <WiFiClient.h>
#include <WiFiManager.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <ds3231.h>

#define DEBUG 1

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

#include "Tools.h"
#include "Webinterface.h"
#include "i2cscanner.h"
#include "pitches.h"
// notes in the melody:
int melody[] = {NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3,
                NOTE_G3, 0,       NOTE_B3, NOTE_C4};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {4, 8, 8, 4, 4, 4, 4, 4};

struct ts t; // for ds2320 rtc time

char identifier[24];

// Set I2C address
int address = 0x20;

byte a = B00000000;
byte b = B00000000;

long utcOffsetInSeconds = 2 * 3600;

char daysOfTheWeek[7][12] = {"Sunday",   "Monday", "Tuesday", "Wednesday",
                             "Thursday", "Friday", "Saturday"};

uint8_t val1 = 0;
uint8_t val2 = 0;
uint8_t val3 = 0;
uint8_t val4 = 0;
uint8_t val5 = 0;
uint8_t val6 = 0;

int Year, Month, Day, Hour, Minute, Second;

// Define NTP Client to get time
String ntpServer = "pool.ntp.org";
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, ntpServer.c_str(), utcOffsetInSeconds);
// dns
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
char cmDNS[33];
String escapedMac;

// Button
#define BUTTON_PIN 0
EasyButton button(BUTTON_PIN, 40, true, true);

#define UP_BUTTON_PIN 5
EasyButton up_button(UP_BUTTON_PIN, 10, true, true);
unsigned long sleep_shake_time = 0;
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
WebSocketsServer webSocket = WebSocketsServer(81);
ESP8266HTTPUpdateServer httpUpdater;

String OldInfo = ""; // old board info
// Websoket Vars
String websocketConnection[10];

bool configModeActive = true;

// System Vars
bool sleepModeActive = false;
bool shouldSaveConfig = false;

// clock
bool clock_leading_hour_zero = true; // show leading zero by default
bool clock_12h = false;              // 24h by default
bool clock_sleep = false;
String clock_sleep_start = "";
String clock_sleep_finish = "";
unsigned long clock_delay = 0;
uint8_t clock_num = 0;

uint8_t hours;
uint8_t minutes;

#define LEAP_YEAR(Y)                                                           \
  ((Y > 0) && !(Y % 4) && ((Y % 100) || !(Y % 400))) // from time-lib
char const *weekDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};

// int Year, Month, Day, Hour, Minute, Second ;

TickerScheduler ticker(5);

bool DS3231_active = false;
bool wifiActive = true;

// ALARM
bool alarm_sound = true;
// alarm 1
String alarm1Time = "";
String alarm1Message = "";
uint8_t alarm1Hour, alarm1Minute;
int alarm1Weekdays[] = {0, 0, 0, 0, 0, 0, 0};
bool alarm1Fired = false;
bool alarm1Active = false;

// alarm 2
String alarm2Time = "";
String alarm2Message = "";
uint8_t alarm2Minute, alarm2Second;
int alarm2Weekdays[] = {0, 0, 0, 0, 0, 0, 0};
bool alarm2Fired = false;
bool alarm2Active = false;

// alarm 3
String alarm3Time = "";
String alarm3Message = "";
uint8_t alarm3Minute, alarm3Second;
int alarm3Weekdays[] = {0, 0, 0, 0, 0, 0, 0};
bool alarm3Fired = false;
bool alarm3Active = false;

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
int year() {
  unsigned long epochTime = timeClient.getEpochTime();

  // Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int currentYear = ptm->tm_year + 1900;
  return currentYear;
}
int month() {
  unsigned long epochTime = timeClient.getEpochTime();

  // Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int currentMonth = ptm->tm_mon + 1;
  return currentMonth;
}
int day() {
  unsigned long epochTime = timeClient.getEpochTime();

  // Get a time structure
  struct tm *ptm = gmtime((time_t *)&epochTime);

  int monthDay = ptm->tm_mday;
  return monthDay;
}
int hour() { return timeClient.getHours(); }
int minute() { return timeClient.getMinutes(); }
int second() { return timeClient.getSeconds(); }

void createDateElements(const char *str) {
  sscanf(str, "%d-%d-%dT%d:%d", &Year, &Month, &Day, &Hour, &Minute);
}
void createWeekdaysElements(const char *str, int *arr) {
  sscanf(str, "%d,%d,%d,%d,%d,%d,%d", &arr[0], &arr[1], &arr[2], &arr[3],
         &arr[4], &arr[5], &arr[6]);
}

void SaveConfigCallback() { shouldSaveConfig = true; }

void SaveConfig() {
  // save the custom parameters to FS
  if (shouldSaveConfig) {
    DynamicJsonDocument json(1024);

    json["utcOffsetInSeconds"] = utcOffsetInSeconds;

    json["sleepModeActive"] = sleepModeActive;

    json["ntpServer"] = ntpServer;

    json["clock_12h"] = clock_12h;

    json["clock_leading_hour_zero"] = clock_leading_hour_zero;

    json["clock_sleep"] = clock_sleep;
    json["clock_sleep_start"] = clock_sleep_start;
    json["clock_sleep_finish"] = clock_sleep_finish;

    json["alarm1Weekdays"] = join(alarm1Weekdays, ",", 7);
    json["alarm2Weekdays"] = join(alarm2Weekdays, ",", 7);
    json["alarm3Weekdays"] = join(alarm3Weekdays, ",", 7);

    json["alarm1Active"] = alarm1Active;
    json["alarm2Active"] = alarm2Active;
    json["alarm3Active"] = alarm3Active;

    json["alarm1Time"] = alarm1Time;
    json["alarm2Time"] = alarm2Time;
    json["alarm3Time"] = alarm3Time;

    json["alarm1Message"] = alarm1Message;
    json["alarm2Message"] = alarm2Message;
    json["alarm3Message"] = alarm3Message;

    json["wifiActive"] = wifiActive;

    File configFile = SPIFFS.open("/config.json", "w");
    serializeJson(json, configFile);
    configFile.close();
    D_println("SaveConfig: Saved");
    // end save
  }
}

void setConfigParameters(JsonObject &json) {

  D_println("Config:");
  for (JsonPair kv : json) {
    D_print(kv.key().c_str());
    D_print(" = ");
    D_println(kv.value().as<String>());
  }
  D_println();
  if (json.containsKey("sleepModeActive")) {
    sleepModeActive = json["sleepModeActive"];
  }
  if (json.containsKey("utcOffsetInSeconds")) {
    utcOffsetInSeconds = json["utcOffsetInSeconds"];
  }
  if (json.containsKey("ntpServer")) {
    ntpServer = json["ntpServer"].as<String>();
  }

  if (json.containsKey("clock_12h")) {
    clock_12h = json["clock_12h"];
  }

  if (json.containsKey("clock_leading_hour_zero")) {
    clock_leading_hour_zero = json["clock_leading_hour_zero"];
  }
  if (json.containsKey("clock_sleep")) {
    clock_sleep = json["clock_sleep"];
  }
  if (json.containsKey("clock_sleep_start")) {
    clock_sleep_start = json["clock_sleep_start"].as<String>();
  }
  if (json.containsKey("clock_sleep_finish")) {
    clock_sleep_finish = json["clock_sleep_finish"].as<String>();
  }

  // alarm
  if (json.containsKey("alarm1Weekdays")) {
    createWeekdaysElements(json["alarm1Weekdays"], alarm1Weekdays);
  }
  if (json.containsKey("alarm1Active")) {
    alarm1Active = json["alarm1Active"];
  }
  if (json.containsKey("alarm1Time")) {
    alarm1Time = json["alarm1Time"].as<String>();
  }
  if (json.containsKey("alarm1Message")) {
    alarm1Message = json["alarm1Message"].as<String>();
  }

  if (json.containsKey("alarm2Weekdays")) {
    createWeekdaysElements(json["alarm2Weekdays"], alarm2Weekdays);
  }
  if (json.containsKey("alarm2Active")) {
    alarm2Active = json["alarm2Active"];
  }
  if (json.containsKey("alarm2Time")) {
    alarm2Time = json["alarm2Time"].as<String>();
  }
  if (json.containsKey("alarm2Message")) {
    alarm2Message = json["alarm2Message"].as<String>();
  }

  if (json.containsKey("alarm3Weekdays")) {
    createWeekdaysElements(json["alarm3Weekdays"], alarm3Weekdays);
  }
  if (json.containsKey("alarm3Active")) {
    alarm3Active = json["alarm3Active"];
  }
  if (json.containsKey("alarm3Time")) {
    alarm3Time = json["alarm3Time"].as<String>();
  }
  if (json.containsKey("alarm3Message")) {
    alarm3Message = json["alarm3Message"].as<String>();
  }

  if (json.containsKey("wifiActive")) {
    wifiActive = json["wifiActive"];
  }
}

void LoadConfig() {
  if (SPIFFS.exists("/config.json")) {
    // file exists, reading and loading
    File configFile = SPIFFS.open("/config.json", "r");

    if (configFile) {
      D_println("opened config file");

      DynamicJsonDocument json(1024);
      auto error = deserializeJson(json, configFile);

      if (!error) {
        JsonObject documentRoot = json.as<JsonObject>();
        setConfigParameters(documentRoot);
        D_println("LoadConfig: Loaded");
      }
    }
  } else {
    D_println("LoadConfig: No Configfile, init new file");
    SaveConfigCallback();
    SaveConfig();
  }
}

void SetConfig(JsonObject &json) {

  if (json.containsKey("utcOffsetInSeconds")) {
    utcOffsetInSeconds = json["utcOffsetInSeconds"];
    timeClient.setTimeOffset(utcOffsetInSeconds);
    if (DS3231_active) {
      t.hour = timeClient.getHours();
      t.min = timeClient.getMinutes();
      t.sec = timeClient.getSeconds();
      t.mday = day();
      t.mon = month();
      t.year = year();
      DS3231_set(t);
    }
  }
  setConfigParameters(json);
  SaveConfigCallback();
  SaveConfig();

  delay(1000);
  ESP.restart();
}

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
  server.send(200, "application/json", GetConfig());
}
void Handle_wifisetup() { WifiSetup(); }
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
        String config = GetConfig();
        webSocket.sendTXT(i, config);
      }
    }
  }
}
String GetConfig() {
  File configFile = SPIFFS.open("/config.json", "r");

  if (configFile) {
    Log(F("GetConfig"), F("Opened config file"));
    size_t size = configFile.size();
    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    configFile.readBytes(buf.get(), size);
    DynamicJsonDocument root(1024);
    if (DeserializationError::Ok == deserializeJson(root, buf.get())) {
    }

    String json;
    serializeJson(root, json);

    return json;
  }
  return "";
}

/////////////////////////////////////////////////////////////////////
void Log(String function, String message) {

  String timeStamp = IntFormat(year()) + "-" + IntFormat(month()) + "-" +
                     IntFormat(day()) + "T" + IntFormat(hour()) + ":" +
                     IntFormat(minute()) + ":" + IntFormat(second());

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
  root["clock_sleep"] = clock_sleep;

  int DoW = dayOfWeek(year(), month(), day());
  String week_day;
  switch (DoW) {
  case 0:
    week_day = "Saturday";
    break;
  case 1:
    week_day = "Sunday";
    break;
  case 2:
    week_day = "Monday";
    break;
  case 3:
    week_day = "Tuesday";
    break;
  case 4:
    week_day = "Wednesday";
    break;
  case 5:
    week_day = "Thursday";
    break;
  case 6:
    week_day = "Friday";
    break;
  }
  
  root["weekday"] = week_day;
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
        SetConfig(object);
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
// alarm
void playAlarm() {
  if (alarm1Fired || alarm2Fired || alarm3Fired) {
    if (alarm_sound)
      tone(4, 780, 180);
    else
      tone(4, 500, 180);
    alarm_sound = !alarm_sound;
  }
}
void handleAlarm() {

  String currentTime =
      IntFormat(hour()) + ":" + IntFormat(minute()) + ":" + IntFormat(second());
  String chekAlarm1Time = alarm1Time + ":00";
  String chekAlarm2Time = alarm2Time + ":00";
  String chekAlarm3Time = alarm3Time + ":00";
  byte NumDayOfWeek = dayOfWeek(year(), month(), day());
  if (chekAlarm1Time == currentTime && alarm1Active == true &&
      alarm1Weekdays[NumDayOfWeek] == 1) {
    alarm1Fired = true;
    D_println(F("ALARM 1!"));
    Log(F("handleAlarm"), (alarm1Message));
  } else if (chekAlarm2Time == currentTime && alarm2Active == true &&
             alarm2Weekdays[NumDayOfWeek] == 1) {
    alarm2Fired = true;
    D_println(F("ALARM 2!"));
    Log(F("handleAlarm"), (alarm2Message));
  } else if (chekAlarm3Time == currentTime && alarm3Active == true &&
             alarm3Weekdays[NumDayOfWeek] == 1) {
    alarm3Fired = true;
    D_println(F("ALARM 3!"));
    Log(F("handleAlarm"), (alarm3Message));
  }

  playAlarm();
}

void handleMuteAlarms() {
  alarm1Fired = false;
  alarm2Fired = false;
  alarm3Fired = false;

  noTone(4);
}
// BUTTON
//  Attach callback.
void singleClick() {
  handleMuteAlarms();
  tone(4, NOTE_C4, 1000 / 16);
  // stop the tone playing:
  delay(200);
  noTone(4);
  D_println(F("singleClick"));
  Log(F("singleClick"), F("singleClick!"));
}
void upClick() {
  sleep_shake_time = millis();
  handleMuteAlarms();
  tone(4, NOTE_C4, 1000 / 16);
  // stop the tone playing:
  delay(200);
  noTone(4);
  D_println(F("upClick"));
  Log(F("upClick"), F("upClick!"));
}
void toggleWifi() {
  wifiActive = !wifiActive;
  SaveConfigCallback();
  SaveConfig();
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

uint8_t hi, lo;
uint16_t pcf8575_read() {
  Wire.beginTransmission(address);
  Wire.endTransmission();

  Wire.requestFrom(address, 2);
  if (Wire.available()) {
    lo = Wire.read();
    hi = Wire.read();
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
  byte c, d;
  c = a;
  d = b;
  // turn on
  for (int i = 0; i < 50; i++) {
    // Set port on
    pf575_write(word(c, d));
  }
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

  if (wifiActive) {
    timeClient.update();
    D_print(daysOfTheWeek[timeClient.getDay()]);
    D_print(", ");
    D_print(timeClient.getHours());
    D_print(":");
    D_print(timeClient.getMinutes());
    D_print(":");
    D_println(timeClient.getSeconds());

    hours = timeClient.getHours();
    minutes = timeClient.getMinutes();
  } else {
    DS3231_get(&t);
    hours = t.hour;
    minutes = t.min;
    D_print("\t Weekday : ");
    D_print(dayOfWeek(t.year, t.mon, t.mday));
    D_print(" Date : ");
    D_print(t.mday);
    D_print("/");
    D_print(t.mon);
    D_print("/");
    D_print(t.year);
    D_print("\t Hour : ");
    D_print(t.hour);
    D_print(":");
    D_print(t.min);
    D_print(".");
    D_println(t.sec);
  }
  if (clock_12h && hours > 12) {
    hours = hours - 12;
  } else if (clock_12h && hours == 0) {
    hours = 12;
  }
  clock_num = 0;
}

void time2nixie()
{
    unsigned long l_millis = millis();
    if ((clock_leading_hour_zero == false && val1 != 0) ||
        clock_leading_hour_zero == true) {
      if (clock_num == 0) {
        show_number(val1, false);
        clock_num++;
        clock_delay = l_millis;
      }
      // 100 delay
      if (clock_num == 1 and l_millis - clock_delay > 800) {
        turn_all_off();
        clock_num++;
      }
    } else {
      clock_num = 2;
      clock_delay = l_millis - 1000;
    }

    if (clock_num == 2 && l_millis - clock_delay > 1000) {
      show_number(val2, false);
      clock_num++;
    }
    else if (clock_num == 3 and l_millis - clock_delay > 1800) {
      turn_all_off();
      clock_num++;
    }
    else if (clock_num == 4 and l_millis - clock_delay > 2500 ) {
      show_number(val3, true);
      clock_num++;
    }
    else if (clock_num == 5 and l_millis - clock_delay > 3300) {
      turn_all_off();
      clock_num++;
    }
    else if (clock_num == 6 && l_millis - clock_delay > 3500) {
      show_number(val4, true);
      clock_num++;
    }
    else if (clock_num == 7 and l_millis - clock_delay > 4300) {
      turn_all_off();
      clock_num++;
    }
}

void show_time() {
  
  val1 = hours / 10;
  val2 = hours % 10;
  val3 = minutes / 10;
  val4 = minutes % 10;
  // val5 = timeClient.getSeconds() / 10; // seconds not used
  // val6 = timeClient.getSeconds() % 10; // seconds not used
  handleAlarm();

  bool doSleep = (millis() - sleep_shake_time) > 1000 * 60 * 3; //  3 minutes shake wakeup
  if (clock_sleep_start != "" && clock_sleep_finish != "" &&
      clock_sleep == true) {

    uint8_t sleep_start_hour = clock_sleep_start.substring(0, 2).toInt();
    uint8_t sleep_start_minute = clock_sleep_start.substring(3, 5).toInt();

    uint8_t sleep_finish_hour = clock_sleep_finish.substring(0, 2).toInt();
    uint8_t sleep_finish_minute = clock_sleep_finish.substring(3, 5).toInt();
    uint16_t current_hours_with_minutes = timeClient.getHours()  * 100 + minutes;
    uint16_t start_hours_with_minutes = sleep_start_hour * 100 + sleep_start_minute;
    uint16_t end_hours_with_minutes = sleep_finish_hour * 100 + sleep_finish_minute;
  
    doSleep = (sleep_start_hour > sleep_finish_hour && 
    (current_hours_with_minutes >= start_hours_with_minutes || current_hours_with_minutes <= end_hours_with_minutes))
    || (current_hours_with_minutes >= start_hours_with_minutes && current_hours_with_minutes <= end_hours_with_minutes);
  }
  if(doSleep)
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

void setupWifi() {

  wifiManager.setDebugOutput(true);
  // Set config save notify callback
  wifiManager.setSaveConfigCallback(SaveConfigCallback);
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
  if (shouldSaveConfig) {
    SaveConfig();
  }

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

  // Set all ports as output
  turn_all_off();

  Serial.begin(115200);
  while (!Serial)
    ;

  // Mounting FileSystem
  D_println(F("Mounting file system..."));
  if (SPIFFS.begin()) {
    D_println(F("Successfully mounted file system."));
    LoadConfig();
  } else {
    D_println(F("Failed to mount FS"));
  }

  i2cscanner::scan();

  if (strContains(i2cscanner::i2c_addresses.c_str(), "0x68") == 1) {
    DS3231_active = true;
    DS3231_init(DS3231_CONTROL_INTCN);
  }
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    // e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(4, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(4);
  }

  timeClient.setTimeOffset(utcOffsetInSeconds);

  if (wifiActive) {
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
    timeClient.begin();
  } else {
    // Turn off WiFi
    WiFi.mode(WIFI_OFF);
  }

  ticker.add(
      0, 100, [&](void *) { show_time(); }, nullptr, true);
  ticker.add(
      1, 5500, [&](void *) { read_time(); }, nullptr, true);

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
  if (wifiActive) {
    MDNS.update();
    server.handleClient();
    yield();
    webSocket.loop();
    yield();
  }

  ticker.update();
  yield();
}
