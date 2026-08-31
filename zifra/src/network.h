#pragma once
#ifndef ZIFRA_NETWORK_H
#define ZIFRA_NETWORK_H

// WiFi, captive-portal and mDNS management for the clock.
class Network {
  public:
    Network(WiFiManager &wifiManager, ZifraConfig &conf)
      : m_wifiManager(wifiManager), m_conf(conf) {}

    // Connects with the saved credentials or opens the config portal;
    // restarts the board when nothing connects within the portal timeout.
    void begin(const char *identifier) {
      m_wifiManager.setDebugOutput(true);
      m_wifiManager.setSaveConfigCallback([this]() { m_conf.saveConfigCallback(); });
      m_wifiManager.setMinimumSignalQuality();
      m_wifiManager.setConfigPortalTimeout(180);

      WiFi.hostname(identifier);
      const IPAddress apIP(192, 168, 4, 1);
      m_wifiManager.setAPStaticIPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
      if (!m_wifiManager.autoConnect(identifier)) {
        D_println(F("Network: WiFi failed to connect and hit timeout"));
        delay(3000);
        ESP.reset();
        delay(5000);
      }

      D_println(F("Network: WiFi connected"));
      m_conf.saveConfig();
      D_println("Network: IP " + WiFi.localIP().toString());
      D_println("Network: gateway " + WiFi.gatewayIP().toString());
      D_println("Network: subnet " + WiFi.subnetMask().toString());

      beginMDNS();
    }

    // Forgets the WiFi credentials and restarts into the config portal.
    void reset() {
      m_wifiManager.resetSettings();
      ESP.restart();
      delay(300);
    }

    void loop() { MDNS.update(); }

  private:
    void beginMDNS() {
      String escapedMac = WiFi.macAddress();
      escapedMac.replace(":", "");
      escapedMac.toLowerCase();
      // "end" must be called before "begin" is called a 2nd time
      // see https://github.com/esp8266/Arduino/issues/7213
      MDNS.end();
      MDNS.begin("zifra");
      D_println(F("Network: mDNS started"));
      MDNS.addService("http", "tcp", 80);
      MDNS.addService("zifra", "tcp", 80);
      MDNS.addServiceTxt("zifra", "tcp", "mac", escapedMac.c_str());
    }

    WiFiManager &m_wifiManager;
    ZifraConfig &m_conf;
};

#endif
