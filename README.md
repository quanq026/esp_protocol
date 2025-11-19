# ESP32 Protocol

## Mục lục

- [Bài tập 1:...]()

- [Bài tập 2: Web Server (HTTP)](https://github.com/quanq026/esp_protocol/blob/main/README.md#b%C3%A0i-t%E1%BA%ADp-2-web-server-http)

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
```

### Hướng dẫn upload và chạy code

1. Mở Arduino IDE.
2. Chọn board ESP32 (Tools > Board > ESP32 Arduino > ESP32 Dev Module).
3. Kết nối ESP32 qua USB.
4. Paste code vào, chỉnh `ssid` và `password` cho phù hợp với router của bạn.
5. Upload (Ctrl+U).
6. Mở Serial Monitor (baud 115200) để theo dõi log kết nối.

## Giải thích mã nguồn

### Cách sử dụng
1. Upload code lên ESP32-S3.
2. Mở Serial Monitor → copy địa chỉ IP (ví dụ: 192.168.1.150).
3. Truy cập bằng trình duyệt: http://192.168.1.150
4. Giao diện nút BẬT / TẮT → LED sẽ được hiển thị.
### API
| Phương thức | URL              | Mô tả              | Ví dụ                           |
|-------------|------------------|--------------------|---------------------------------|
| GET         | /api/sensor      | Lấy trạng thái LED | {"led":1}                       |
| POST        | /api/led?state=1 | Bật LED            | curl -X POST IP/api/led?state=1 |
| POST        | /api/led?state=0 | Tắt LED            | curl -X POST IP/api/led?state=0 |
