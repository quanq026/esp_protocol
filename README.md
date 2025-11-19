# ESP32 Protocol

## Mục lục

- [Bài tập 1:...]()

- [Bài tập 2: Web Server HTTP](https://github.com/quanq026/esp_protocol/blob/main/README.md#b%C3%A0i-t%E1%BA%ADp-2-web-server-http))

- [Bài tập 3: WebSocket (Realtime)](https://github.com/quanq026/ESP_Wifi/blob/main/README.md#b%C3%A0i-t%E1%BA%ADp-3-esp32-dual-mode-ap--sta)

- [Bài tập 4:...]()

## Phần cứng & phần mềm chung
- **Phần cứng:** ESP32 (bất kỳ board nào hỗ trợ Arduino IDE).
- **Phần mềm:**
  - Arduino IDE với ESP32 core (cài qua Board Manager).
  - Serial Monitor (trong Arduino IDE).

---
# Bài tập 2: Web Server (HTTP)
Dự án điều khiển **1 đèn NeoPixel** trên ESP32-S3 qua giao diện **Web đơn giản**.
Hỗ trợ:
- Bật/tắt LED bằng nút trên trang web.
- Tự động reconnect Wi-Fi khi mất kết nối.
- API (`/api/sensor` và `/api/led`).

## Phần cứng yêu cầu
- ESP32 (trong dự án này dùng esp32-s3 để điều khiển led onborad GPIO48, cảm biến DHT11 sẽ không được sử dụng trong dự án này vì thiếu phần cứng)

## Thư viện cần cài
Arduino IDE → Library Manager:
- `Adafruit NeoPixel` by Adafruit
- `WiFi` và `WebServer` (đã có sẵn trong ESP32 core)

## Mã nguồn hoàn chỉnh

```cpp
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN    48        // Chân điều khiển NeoPixel
#define LED_COUNT  1         // Số lượng LED

Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);
bool ledState = false;       // Trạng thái LED: false = tắt, true = bật

const char* WIFI_SSID = "VJU Student";
const char* WIFI_PASS = "studentVJU@2022";

WebServer server(80);

/* ====================== API ====================== */
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
    led.setPixelColor(0, led.Color(0, 255, 40));  // Màu xanh lá đẹp
  else
    led.clear();
  led.show();

  String json = "{\"led\":" + String(ledState ? 1 : 0) + "}";
  server.send(200, "application/json", json);
}

/* ====================== Trang Web ====================== */
const char HTML_PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="vi">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>ESP32-S3 LED Control</title>
  <style>
    body{font-family:Arial,sans-serif;text-align:center;margin-top:50px;background:#f0f0f0;}
    h2{color:#2c3e50;}
    button{padding:15px 40px;font-size:18px;cursor:pointer;background:#3498db;color:white;border:none;border-radius:10px;}
    button:hover{background:#2980b9;}
    #state{font-weight:bold;font-size:24px;color:#2c3e50;}
  </style>
</head>
<body>
  <h2>ESP32-S3 NeoPixel Control</h2>
  <p>Trạng thái LED: <span id="state">?</span></p>
  <button onclick="toggleLed()">BẬT / TẮT</button>

  <script>
    const state = document.getElementById('state');

    async function updateLed() {
      const res = await fetch('/api/sensor');
      const data = await res.json();
      state.innerText = data.led ? 'BẬT' : 'TẮT';
      state.style.color = data.led ? '#27ae60' : '#c0392b';
    }

    async function toggleLed() {
      const newState = state.innerText === 'BẬT' ? 0 : 1;
      await fetch('/api/led?state=' + newState, {method: 'POST'});
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

/* ====================== Tự động reconnect Wi-Fi ====================== */
void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.println("[WiFi] Mất kết nối! Đang thử kết nối lại...");
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n[WiFi] Đã kết nối lại! IP: " + WiFi.localIP().toString());
  }
}

/* ====================== Setup & Loop ====================== */
void setup() {
  Serial.begin(115200);
  Serial.println("\n=== ESP32-S3 LED Web Server ===");

  // Khởi động NeoPixel
  led.begin();
  led.setBrightness(50);   // Độ sáng 0-255
  led.show();

  // Kết nối Wi-Fi lần đầu
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("Đang kết nối Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[WiFi] Đã kết nối!");
  Serial.print("[WiFi] IP Address: http://");
  Serial.println(WiFi.localIP());

  // Định tuyến web
  server.on("/", handleRoot);
  server.on("/api/sensor", HTTP_GET, handleSensor);
  server.on("/api/led", HTTP_POST, handleLed);
  server.begin();
  Serial.println("HTTP server started!");
}

void loop() {
  ensureWiFi();       // Tự động reconnect nếu cần
  server.handleClient();  // Xử lý request từ client
}
