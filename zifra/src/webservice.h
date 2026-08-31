#pragma once
#ifndef ZIFRA_WEBSERVICE_H
#define ZIFRA_WEBSERVICE_H

// The HTTP server, the OTA updater and the websocket API in one place.
// Serves the redirect page, answers /update, and speaks the UI protocol:
//   /main      -> live info + log stream
//   /settime, /settings, /setalarm, /setsystem -> config on connect
//   /setConfig -> receives config JSON, saves and applies it
class WebService {
  public:
    static constexpr uint8_t MAX_CLIENTS = WEBSOCKETS_SERVER_CLIENT_MAX;

    explicit WebService(Zifra &zifra)
      : m_zifra(zifra), m_server(80), m_webSocket(81) {}

    void begin() {
      m_updater.setup(&m_server);
      m_server.on(F("/update"), HTTP_GET, [this]() { handleNotFound(); });
      m_server.on(F("/"), HTTP_GET, [this]() { handleMainPage(); });
      m_server.onNotFound([this]() { handleNotFound(); });
      m_server.begin();

      m_webSocket.begin();
      // Ping clients and drop the dead ones: lingering half-closed sockets
      // used to exhaust the client slots and stall every broadcast for
      // seconds on writes to peers that were long gone.
      m_webSocket.enableHeartbeat(15000, 3000, 2);
      m_webSocket.onEvent(
      [this](uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
        onEvent(num, type, payload, length);
      });
      log(F("Setup"), F("Webserver started"));
    }

    void loop() {
      m_server.handleClient();
      yield();
      m_webSocket.loop();
    }

    void log(const String &function, const String &message) {
      auto &time = m_zifra.time;
      const String timeStamp =
        IntFormat(time.getYear()) + "-" + IntFormat(time.getMonth()) + "-" +
        IntFormat(time.getMonthDay()) + "T" + IntFormat(time.getHoursIso()) +
        ":" + IntFormat(time.getMinutes()) + ":" + IntFormat(time.getSeconds());

      D_println("[" + timeStamp + "] " + function + ": " + message);

      sendToClients("{\"log\":{\"timeStamp\":\"" + timeStamp +
                    "\",\"function\":\"" + function +
                    "\",\"message\":\"" + message + "\"}}",
                    {"/main"});
    }

    void sendConfig() {
      sendToClients(m_zifra.conf.getConfig(),
                    {"/settings", "/settime", "/setalarm", "/setsystem"});
    }

    // Broadcasts the live system info; unchanged payloads are skipped
    // unless forced, so the periodic ticker stays cheap.
    void sendInfo(bool force = false) {
      if (m_webSocket.connectedClients() == 0) {
        return;
      }
      const String info = getInfo();
      if (force || m_lastInfo != info) {
        sendToClients(info, {"/main", "/api/info"});
      }
      m_lastInfo = info;
    }

  private:
    String getInfo() {
      DynamicJsonDocument root(1024);

      root["gumboardVersion"] = ZIFRA_VERSION;
      root["sketchSize"] = ESP.getSketchSize();
      root["freeSketchSpace"] = ESP.getFreeSketchSpace();
      root["wifiRSSI"] = String(WiFi.RSSI());
      root["wifiQuality"] = GetRSSIasQuality(WiFi.RSSI());
      root["wifiSSID"] = WiFi.SSID();
      root["ipAddress"] = WiFi.localIP().toString();
      root["freeHeap"] = ESP.getFreeHeap();
      root["chipID"] = ESP.getChipId();
      root["cpuFreqMHz"] = ESP.getCpuFreqMHz();
      root["clock_sleep"] = m_zifra.conf.clock.sleep;

      static const char *const weekdayNames[7] = {
        "Sunday", "Monday", "Tuesday", "Wednesday",
        "Thursday", "Friday", "Saturday"
      };
      const uint8_t weekDay = m_zifra.time.getWeekDay();
      root["weekday"] =
        String(weekdayNames[weekDay % 7]) + "(" + String(weekDay) + ")";

      String json;
      serializeJson(root, json);
      return json;
    }

    // Sends a message to every client connected to one of the given paths
    void sendToClients(String message,
                       std::initializer_list<const char *> paths) {
      if (m_webSocket.connectedClients() == 0) {
        return;
      }
      for (uint8_t i = 0; i < MAX_CLIENTS; i++) {
        for (const char *path : paths) {
          if (m_clientPath[i] == path) {
            m_webSocket.sendTXT(i, message);
            break;
          }
        }
      }
    }

    void onEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length) {
      (void)length;
      switch (type) {
        case WStype_DISCONNECTED: {
            log(F("WebSocketEvent"), "[" + String(num) + "] Disconnected!");
            m_clientPath[num] = "";
            break;
          }
        case WStype_CONNECTED: {
            // Remember what the connection was established for
            m_clientPath[num] = String((char *)payload);

            log(F("WebSocketEvent"),
                "[" + String(num) + "] Connected from " +
                m_webSocket.remoteIP(num).toString() +
                " url: " + m_clientPath[num]);

            // Send info and config straight to this client, whatever path
            // it connected on - a page then needs only one socket, and a
            // client that lost the path race still gets its data.
            String info = getInfo();
            m_webSocket.sendTXT(num, info);
            String config = m_zifra.conf.getConfig();
            m_webSocket.sendTXT(num, config);
            break;
          }
        case WStype_TEXT: {
            if (((char *)payload)[0] != '{') {
              break; // keep-alives and other chatter
            }
            DynamicJsonDocument json(512);
            deserializeJson(json, payload);

            log(F("WebSocketEvent"),
                "Incomming Json length: " + String(measureJson(json)));

            if (m_clientPath[num] == "/setConfig") {
              JsonObject object = json.as<JsonObject>();
              m_zifra.conf.setConfig(object);
              log(F("Config"), F("Saved and applied"));
              // push the fresh config to connected settings pages
              sendConfig();
            }
            break;
          }
        default:
          break;
      }
    }

    void handleMainPage() {
      m_server.sendHeader("Connection", "close");
      m_server.send(200, "text/html", mainPage);
    }

    void handleNotFound() {
      if (m_server.method() == HTTP_OPTIONS) {
        m_server.sendHeader("Access-Control-Allow-Origin", "*");
        m_server.send(204);
      }
      m_server.sendHeader("Location", "/update", true);
      m_server.send(302, "text/plain", "");
    }

    Zifra &m_zifra;
    ESP8266WebServer m_server;
    WebSocketsServer m_webSocket;
    ESP8266HTTPUpdateServer m_updater;
    String m_clientPath[MAX_CLIENTS];
    String m_lastInfo;
};

#endif
