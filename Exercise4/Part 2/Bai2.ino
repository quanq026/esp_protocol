#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledState = false; // 0 = tắt, 1 = bật

const char* WIFI_SSID = "W_I_F_I";
const char* WIFI_PASS = "P_A_S_S";

WebServer server(80);

//api
void handleSensor() {
  String json = "{\"led\":" + String(ledState ? 1 : 0) + "}";
  server.send(200, "application/json", json);
}

void handleLed() {
  if (!server.hasArg("state")) {
    server.send(400, "application/json", "{\"error\":\"missing state\"}");
    return;
  }

  int state = server.arg("state").toInt();
  ledState = (state == 1);

  if (ledState)
    led.setPixelColor(0, led.Color(0, 255, 40));
  else
    led.clear();
  led.show();

  //JSON
  String json = "{\"led\":" + String(ledState ? 1 : 0) + "}";
  server.send(200, "application/json", json);
}

//web
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32-S3 LED Control</title>
</head>
<body style="text-align:center; font-family:sans-serif;">
  <h2>ESP32-S3 LED</h2>
  <p>Status: <span id="state">?</span></p>
  <button onclick="toggleLed()">ON/OFF</button>

  <script>
    //update status
    async function updateLed() {
      const res = await fetch('/api/sensor');
      const data = await res.json();
      state.innerText = data.led ? 'ON' : 'OFF';
    }

    // POST
    async function toggleLed() {
      const newState = state.innerText == 'ON' ? 0 : 1;
      await fetch('/api/led?state=' + newState, { method: 'POST' });
      updateLed();
    }

    setInterval(updateLed, 2000);
    updateLed();
  </script>
</body>
</html>
)rawliteral";

void handleRoot() {
  server.send(200, "text/html", HTML_PAGE);
}

void ensureWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASS);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 8000)
      delay(500);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESP32-S3 LED Web Server");

  led.begin();
  led.setBrightness(30);
  led.show();

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Kết nối Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("IP ESP32: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/api/sensor", HTTP_GET, handleSensor);
  server.on("/api/led", HTTP_POST, handleLed);
  server.begin();
  Serial.println("Web server đã sẵn sàng!");
}

void loop() {
  ensureWiFi();
  server.handleClient();
}
