#define BLYNK_TEMPLATE_ID "xxx"
#define BLYNK_TEMPLATE_NAME "xxx"
#define BLYNK_AUTH_TOKEN "xxx"
#define BLYNK_PRINT Serial

#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Adafruit_NeoPixel.h>

// --- NeoPixel ---
#define LED_PIN   48
#define LED_COUNT 1
Adafruit_NeoPixel led(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// --- WiFi ---
char ssid[] = "VJU Student";
char pass[] = "studentVJU@2022";

// --- Blynk Virtual Pin ---
#define VPIN_LED V1

// -------------------------------------------------------------------
// Khi Blynk gửi lệnh (0/1) từ VPIN
// -------------------------------------------------------------------
BLYNK_WRITE(VPIN_LED)
{
  int state = param.asInt();

  if (state == 1) {
    led.setPixelColor(0, led.Color(0, 255, 40));  // Màu xanh lá nhẹ
  } else {
    led.clear();
  }
  led.show();
}

// -------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);

  led.begin();
  led.clear();
  led.show();

  WiFi.begin(ssid, pass);
  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

// -------------------------------------------------------------------
void loop()
{
  Blynk.run();
}

