#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <DHT.h>

// ------------------- LED --------------------
#define LED_PIN 48
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledState = false;

// ------------------- DHT11 --------------------
#define DHTPIN 4          // Chân cảm biến DHT11
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
unsigned long lastDHT = 0;

// ------------------- WiFi --------------------
const char* WIFI_SSID = "VJU Student";
const char* WIFI_PASS = "studentVJU@2022";

WebServer server(80);
WebSocketsServer ws(81);

// ================= WebSocket Event =================
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {

  if (type == WStype_CONNECTED) {
    Serial.printf("Client [%u] connected\n", num);
    ws.sendTXT(num, "{\"led\":" + String(ledState ? 1 : 0) + "}");
  }

  else if (type == WStype_TEXT) {
    Serial.printf("Received: %s\n", payload);

    StaticJsonDocument<100> doc;
    deserializeJson(doc, payload);

    if (doc.containsKey("led")) {
      ledState = (doc["led"] == 1);

      if (ledState) led.setPixelColor(0, led.Color(0, 255, 40));
      else led.clear();
      led.show();

      String response = "{\"led\":" + String(ledState ? 1 : 0) + ",\"status\":\"ok\"}";
      ws.broadcastTXT(response);
    }
  }
}

// ================= HTML =================
const char HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta charset="utf-8">
  <title>ESP32 LED WebSocket</title>
  <style>
    body { font-family: Arial; text-align: center; padding: 50px; }
    #status { font-size: 32px; margin: 20px; padding: 20px; border-radius: 10px; }
    .on { background: #4CAF50; color: white; }
    .off { background: #f44336; color: white; }
    button { font-size: 18px; padding: 15px 30px; background: #2196F3;
             color: white; border: none; border-radius: 5px; cursor: pointer; }
    #log { background: #1e1e1e; color: #0f0; padding: 15px; margin-top: 30px;
           height: 250px; overflow-y: auto; text-align: left; font-family: monospace; }
  </style>
</head>
<body>
  <h2>ESP32-S3 LED + DHT11 Realtime</h2>

  <div id="status" class="off">LED: OFF</div>
  <button onclick="toggle()">Toggle LED</button>

  <h3 style="margin-top:30px;">Logs</h3>
  <div id="log"></div>

  <script>
    var ws = new WebSocket('ws://' + location.hostname + ':81');
    var state = 0;

    ws.onopen = () => addLog('Connected');
    ws.onclose = () => addLog('Disconnected');

    ws.onmessage = (e) => {
      addLog('Recv: ' + e.data);
      var data = JSON.parse(e.data);

      if (data.led !== undefined) {
        state = data.led;
        updateUI();
      }

      // Khi nhận temp/hum
      if (data.temp !== undefined && data.hum !== undefined) {
        addLog("Temp: " + data.temp + "°C | Hum: " + data.hum + "%");
      }
    };

    function toggle() {
      var msg = '{"led":' + (state ? 0 : 1) + '}';
      ws.send(msg);
      addLog('Send: ' + msg);
    }

    function updateUI() {
      var el = document.getElementById('status');
      el.textContent = 'LED: ' + (state ? 'ON' : 'OFF');
      el.className = state ? 'on' : 'off';
    }

    function addLog(msg) {
      var el = document.getElementById('log');
      var time = new Date().toLocaleTimeString();
      el.innerHTML += time + ' ' + msg + '<br>';
      el.scrollTop = el.scrollHeight;
    }
  </script>
</body>
</html>
)rawliteral";

// ================= Setup =================
void setup() {
  Serial.begin(115200);

  led.begin();
  led.setBrightness(30);
  led.show();

  dht.begin();  // DHT11

  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Connecting WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nIP: " + WiFi.localIP().toString());

  ws.begin();
  ws.onEvent(webSocketEvent);

  server.on("/", []() { server.send(200, "text/html", HTML); });
  server.begin();

  Serial.println("Ready!");
}

// ================= Loop =================
void loop() {
  ws.loop();
  server.handleClient();

  // ---- Đọc nhiệt độ / độ ẩm mỗi 2 giây ----
  if (millis() - lastDHT >= 2000) {
    lastDHT = millis();

    float t = dht.readTemperature();
    float h = dht.readHumidity();

    if (!isnan(t) && !isnan(h)) {
      String json = "{\"temp\":" + String(t) + ",\"hum\":" + String(h) + "}";
      ws.broadcastTXT(json);
      Serial.println("DHT -> " + json);
    }
  }
}
