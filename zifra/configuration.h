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

struct Clock{
      bool iso{true};
      bool leadingHourZero{true};
      bool sleep{false};
      String sleepStart{};
      String sleepFinish{};
};

class ZifraConfig {
  public:
    AlarmProperties alarm1{};
    AlarmProperties alarm2{};
    AlarmProperties alarm3{};
    Clock clock{};
    int utcOffsetInSeconds{2*3600};      
    String ntpServer{"pool.ntp.org"};
    bool wifiActive{true};
    bool DS3231_active{false};
    
    ZifraConfig()
    {
      //loadConfig();
    }
    void setCb(std::function<void()> cb)
    {
      m_cb = cb;
    }
    void saveConfigCallback() {
      m_shouldSaveConfig = true;
    }
    void saveConfig() {
      // save the custom parameters to FS
      if (m_shouldSaveConfig) {
        DynamicJsonDocument json(4096);

          json["utcOffsetInSeconds"] = utcOffsetInSeconds;      
          json["ntpServer"] = ntpServer;
          json["wifiActive"] = wifiActive;
      
          json["clock_12h"] = !clock.iso;
      
          json["clock_leading_hour_zero"] = clock.leadingHourZero;
      
          json["clock_sleep"] = clock.sleep;
          json["clock_sleep_start"] = clock.sleepStart;
          json["clock_sleep_finish"] = clock.sleepFinish;
      
          json["alarm1Weekdays"] = join(alarm1.weekdays, ",", 7);
          json["alarm2Weekdays"] = join(alarm2.weekdays, ",", 7);
          json["alarm3Weekdays"] = join(alarm3.weekdays, ",", 7);
      
          json["alarm1Active"] = alarm1.active;
          json["alarm2Active"] = alarm2.active;
          json["alarm3Active"] = alarm3.active;
      
          json["alarm1Time"] = alarm1.time;
          json["alarm2Time"] = alarm2.time;
          json["alarm3Time"] = alarm3.time;
      
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
              case DeserializationError::Ok:
                D_println(F("Deserialization succeeded"));
                break;
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

      if (configFile) {
        const size_t size = configFile.size();
        std::unique_ptr<char[]> buf(new char[size]);

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument root(4096);

        if (DeserializationError::Ok == deserializeJson(root, buf.get())) {}
        String json;
        serializeJson(root, json);
        return json;

      }
    }

  private:

    bool m_shouldSaveConfig = false;
    void callback()
    {
       if ( m_cb != NULL) {m_cb();}
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
      setData(json, "clock_12h", clock.iso);
      if (json.containsKey("clock_12h")) {
        clock.iso = !json["clock_12h"].as<bool>();
      }
      setData(json, "clock_leading_hour_zero", clock.leadingHourZero);
      setData(json, "clock_sleep", clock.sleep);
      setData(json, "clock_sleep_start", clock.sleepStart);
      setData(json, "clock_sleep_finish", clock.sleepFinish);
      if (json.containsKey("alarm1Weekdays")) {
        createWeekdaysElements(json["alarm1Weekdays"], alarm1.weekdays);
      }
      if (json.containsKey("alarm2Weekdays")) {
        createWeekdaysElements(json["alarm2Weekdays"], alarm2.weekdays);
      }
      if (json.containsKey("alarm3Weekdays")) {
        createWeekdaysElements(json["alarm3Weekdays"], alarm3.weekdays);
      }
      setData(json, "alarm1Active", alarm1.active);
      setData(json, "alarm1Time", alarm1.time);
      
      setData(json, "alarm2Active", alarm2.active);
      setData(json, "alarm2Time", alarm2.time);
      
      setData(json, "alarm3Active", alarm3.active);
      setData(json, "alarm3Time", alarm3.time);
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

    std::function<void()> m_cb{nullptr}; 
    
}; // class Config

#endif
