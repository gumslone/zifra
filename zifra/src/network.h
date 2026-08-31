#pragma once
#ifndef ZIFRA_NETWORK_H
#define ZIFRA_NETWORK_H

// Injected after WiFiManager's stock stylesheet, so these rules restyle
// the captive portal in the zifra look (nixie amber on warm dark).
const char PORTAL_STYLE[] PROGMEM =
  "<style>"
  "body{background:#14110c;color:#e8e0d0;font-family:system-ui,-apple-system,sans-serif}"
  ".wrap{background:#1c1813;border:1px solid #3a332a;border-radius:8px;padding:20px;margin-top:24px}"
  "h1{color:#f2be44;letter-spacing:1px;text-shadow:0 0 7px #f2be44,0 0 21px #f2be44,0 0 42px #f45404}"
  "h3{color:#9a8f7c;font-weight:400}"
  "a,a:hover,body.invert a{color:#f2be44}"
  "button,input[type='button'],input[type='submit']{background-color:#f2be44;color:#14110c;font-weight:700;border-radius:6px}"
  "button.D{background-color:#e05d44;color:#fff}"
  "input,select,textarea{background:#211c15;color:#e8e0d0;border:1px solid #4a4036;border-radius:6px}"
  "input:focus{outline:none;border-color:#f2be44}"
  "input[type='file']{border:1px solid #4a4036}"
  ".msg{background:#211c15;color:#e8e0d0;border:1px solid #3a332a;border-left-width:5px;border-radius:6px}"
  ".q[role=img]{-webkit-filter:invert(70%) sepia(65%) saturate(4) hue-rotate(-12deg);filter:invert(70%) sepia(65%) saturate(4) hue-rotate(-12deg)}"
  "dt{color:#f2be44}"
  "</style>";

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
      // The softAP and the join attempt share one radio: without a clean
      // disconnect first, the initial join from the portal often dies as a
      // spurious "Authentication Failure". Disconnect cleanly and retry.
      m_wifiManager.setCleanConnect(true);
      m_wifiManager.setConnectRetries(3);
      m_wifiManager.setConnectTimeout(20);
      // Captive portal in the zifra look
      m_wifiManager.setTitle("ZIFRA");
      m_wifiManager.setCustomHeadElement(PORTAL_STYLE);

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
