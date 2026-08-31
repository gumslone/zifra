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

#include "src/version.h"
#include "src/common_functions.h"
#include "src/Webinterface.h"
#include "src/i2cscanner.h"
#include "src/zifra.h"
#include "src/network.h"
#include "src/webservice.h"

char identifier[24];

// Define NTP Client to get time
WiFiUDP ntpUDP;
Zifra zifra(ntpUDP);

WiFiManager wifiManager;
Network network(wifiManager, zifra.conf);
WebService webService(zifra);

TickerScheduler ticker(5);

// Buttons
#define BUTTON_PIN 0
EasyButton button(BUTTON_PIN, 40, true, true);

#define UP_BUTTON_PIN 5
EasyButton up_button(UP_BUTTON_PIN, 10, true, true);

/////////////////////////////////////////////////////////////////////
// BUTTON callbacks
void singleClick() {
  zifra.alarm.mute();
  zifra.buzzer.beep();
  webService.log(F("singleClick"), F("singleClick!"));
}

void upClick() {
  zifra.vol.sleepShakeTime = millis();
  zifra.alarm.mute();
  zifra.buzzer.beep();
  webService.log(F("upClick"), F("upClick!"));
}

void toggleWifi() {
  zifra.conf.wifiActive = !zifra.conf.wifiActive;
  zifra.conf.saveConfig(true);
  delay(1000);
  ESP.restart();
}

void doFactoryReset() {
  File configFile = SPIFFS.open("/config.json", "w");
  if (configFile) {
    configFile.println("");
    configFile.close();
  } else {
    webService.log(F("FactoryReset"), F("Failed to open config file for reset"));
  }
  network.reset();
}

/////////////////////////////////////////////////////////////////////
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
    network.begin(identifier);
    webService.begin();

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
  // Live stats for the System page; skipped when nothing changed
  ticker.add(
  2, 10000, [&](void *) {
    if (zifra.conf.wifiActive) {
      webService.sendInfo();
    }
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
    network.loop();
    webService.loop();
    yield();
  }

  ticker.update();
  yield();
}
