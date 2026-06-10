#pragma once
#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>
#ifndef _Zifra_Config_
#define _Zifra_Config_

struct AlarmProperties {
  String time{};
  int weekdays[7] = {0, 0, 0, 0, 0, 0, 0};
  bool fired{false};
  unsigned long fireTime{0};
  bool active{false};
};

struct Clock {
  bool iso{true};
  bool leadingHourZero{true};
  bool sleep{false};
  String sleepStart{};
  String sleepFinish{};
};

class ZifraConfig {
  public:
    static constexpr uint8_t ALARM_COUNT = 3;
    AlarmProperties alarms[ALARM_COUNT]{};
    Clock clock{};
    int utcOffsetInSeconds{2 * 3600};
    String ntpServer{"pool.ntp.org"};
    bool wifiActive{true};
    bool DS3231_active{false};

    void setCb(std::function<void()> cb)
    {
      m_cb = cb;
    }
    void saveConfigCallback() {
      m_shouldSaveConfig = true;
    }
    void saveConfig(bool force = false) {
      // save the custom parameters to FS
      if (m_shouldSaveConfig || force) {
        DynamicJsonDocument json(4096);

        json["utcOffsetInSeconds"] = utcOffsetInSeconds;
        json["ntpServer"] = ntpServer;
        json["wifiActive"] = wifiActive;

        json["clock_12h"] = !clock.iso;
        json["clock_leading_hour_zero"] = clock.leadingHourZero;

        json["clock_sleep"] = clock.sleep;
        json["clock_sleep_start"] = clock.sleepStart;
        json["clock_sleep_finish"] = clock.sleepFinish;

        for (uint8_t i = 0; i < ALARM_COUNT; i++) {
          const String prefix = alarmPrefix(i);
          json[prefix + "Weekdays"] = join(alarms[i].weekdays, ",", 7);
          json[prefix + "Active"] = alarms[i].active;
          json[prefix + "Time"] = alarms[i].time;
        }

        File configFile = SPIFFS.open("/config.json", "w");
        serializeJson(json, configFile);
        configFile.close();
        D_println(F("Config saved"));
        // end save
      }
    }

    void loadConfig() {
      if (SPIFFS.exists("/config.json")) {
        // file exists, reading and loading
        File configFile = SPIFFS.open("/config.json", "r");

        if (configFile) {
          D_println(F("opened config file"));

          DynamicJsonDocument json(4096);
          const auto error = deserializeJson(json, configFile);

          if (!error) {
            JsonObject documentRoot = json.as<JsonObject>();
            setConfigParameters(documentRoot);

            D_println(F("Config loaded"));
          } else {
            switch (error.code()) {
              case DeserializationError::InvalidInput:
                D_println(F("Invalid input!"));
                break;
              case DeserializationError::NoMemory:
                D_println(F("Not enough memory"));
                break;
              default:
                D_println(F("Deserialization failed"));
                break;
            }
          }
        }
      } else {
        D_println(F("No configfile found, create a new file"));
        saveConfigCallback();
        saveConfig();
      }
    }

    void setConfig(JsonObject &json) {
      setConfigParameters(json);
      saveConfigCallback();
      saveConfig();

      delay(1000);
      ESP.restart();
    }

    String getConfig() {
      File configFile = SPIFFS.open("/config.json", "r");
      if (!configFile) {
        return "{}";
      }

      const size_t size = configFile.size();
      std::unique_ptr<char[]> buf(new char[size]);
      configFile.readBytes(buf.get(), size);

      DynamicJsonDocument root(4096);
      deserializeJson(root, buf.get());
      String json;
      serializeJson(root, json);
      return json;
    }

  private:

    bool m_shouldSaveConfig = false;
    std::function<void()> m_cb{nullptr};

    static String alarmPrefix(uint8_t index) {
      return "alarm" + String(index + 1);
    }
    void callback()
    {
      if (m_cb) {
        m_cb();
      }
    }
    void setConfigParameters(const JsonObject &json) {
      D_println("Config:");
      if (DEBUG) {
        for (JsonPair kv : json) {
          D_print(kv.key().c_str());
          D_print(" = ");
          D_println(kv.value().as<String>());
        }
        D_println();
      }
      if (json.containsKey("utcOffsetInSeconds")) {
        callback();
      }
      setData(json, "wifiActive", wifiActive);
      setData(json, "utcOffsetInSeconds", utcOffsetInSeconds);
      setData(json, "ntpServer", ntpServer);
      if (json.containsKey("clock_12h")) {
        clock.iso = !json["clock_12h"].as<bool>();
      }
      setData(json, "clock_leading_hour_zero", clock.leadingHourZero);
      setData(json, "clock_sleep", clock.sleep);
      setData(json, "clock_sleep_start", clock.sleepStart);
      setData(json, "clock_sleep_finish", clock.sleepFinish);

      for (uint8_t i = 0; i < ALARM_COUNT; i++) {
        const String prefix = alarmPrefix(i);
        if (json.containsKey(prefix + "Weekdays")) {
          createWeekdaysElements(json[prefix + "Weekdays"], alarms[i].weekdays);
        }
        setData(json, prefix + "Active", alarms[i].active);
        setData(json, prefix + "Time", alarms[i].time);
      }
    }

    template<typename T>
    void setData(const JsonObject &json, const String& key, T & var)
    {
      if (json.containsKey(key)) {
        var = json[key].as<T>();
      }
    }

    void createWeekdaysElements(const char *str, int *arr) {
      sscanf(str, "%d,%d,%d,%d,%d,%d,%d", &arr[0], &arr[1], &arr[2], &arr[3],
             &arr[4], &arr[5], &arr[6]);
    }

}; // class Config

#endif
