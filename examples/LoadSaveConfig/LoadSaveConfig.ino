#include <MycilaESPConnect.h>

AsyncWebServer server(80);
Mycila::ESPConnect espConnect(server);
const char* hostname = "arduino-1";

void setup() {
  Serial.begin(115200);
  while (!Serial)
    continue;

  server.on("/clear", HTTP_GET, [&](AsyncWebServerRequest* request) {
    Serial.println("Clearing configuration...");
    espConnect.clearConfiguration();
    request->send(200);
    ESP.restart();
  });

  // network state listener is required here in async mode
  espConnect.listen([](__unused Mycila::ESPConnect::State previous, Mycila::ESPConnect::State state) {
    JsonDocument doc;
    espConnect.toJson(doc.to<JsonObject>());
    serializeJson(doc, Serial);
    Serial.println();

    switch (state) {
      case Mycila::ESPConnect::State::NETWORK_CONNECTED:
      case Mycila::ESPConnect::State::AP_STARTED:
        // serve your home page here
        server.on("/", HTTP_GET, [&](AsyncWebServerRequest* request) {
                return request->send(200, "text/plain", "Hello World!");
              })
          .setFilter([](__unused AsyncWebServerRequest* request) { return espConnect.getState() != Mycila::ESPConnect::State::PORTAL_STARTED; });
        server.begin();
        break;

      case Mycila::ESPConnect::State::NETWORK_DISCONNECTED:
        server.end();
        break;

      default:
        break;
    }
  });

  espConnect.setAutoRestart(true);
  espConnect.setBlocking(true);

  Serial.println("====> Trying to connect to saved WiFi or will start portal in the background...");

  espConnect.begin(hostname, "Captive Portal SSID");

  Serial.println("====> setup() completed...");

  delay(3000);

  Mycila::ESPConnect::IPConfig ipConfig;
  switch (random(0, 3)) {
    case 0:
      ipConfig.ip.fromString("192.168.125.99");
      ipConfig.gateway.fromString("192.168.125.1");
      ipConfig.subnet.fromString("255.255.255.0");
      ipConfig.dns.fromString("192.168.125.1");
      espConnect.setIPConfig(ipConfig);
      espConnect.saveConfiguration();
      break;

    case 1:
      espConnect.setIPConfig(Mycila::ESPConnect::IPConfig());
      espConnect.saveConfiguration();
      break;

    case 2:
      espConnect.clearConfiguration();
      break;

    default:
      break;
  }

  delay(3000);

  ESP.restart();
}

void loop() {
  delay(500);
}
