#include <WiFi.h>
#include <WebSocketsClient.h>
#include <math.h>

WebSocketsClient webSocket;

#define USE_SERIAL Serial
#define LED_PIN 13  // GPIO13 (change to your LED pin)

const char *ssid = "Algohary";
const char *pass = "hamdyahmedhanarana2022";

#define SERVER  "192.168.1.7"  // your server IP
#define PORT    3000
#define URL     "/"

int currentBrightness = 0;

// Function to set LED brightness
void setLEDBrightness(int brightness) {
  brightness = constrain(brightness, 0, 255);
  analogWrite(LED_PIN, brightness);
  currentBrightness = brightness;
  USE_SERIAL.printf("[LED] Brightness set to: %d\n", brightness);
}

// WebSocket events
void webSocketEvent(WStype_t type, uint8_t *payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      USE_SERIAL.println("[WSc] Disconnected!");
      break;

    case WStype_CONNECTED:
      USE_SERIAL.printf("[WSc] Connected to: %s\n", payload);
      webSocket.sendTXT("{\"status\":\"esp_connected\"}");
      break;

    case WStype_TEXT: {
      USE_SERIAL.printf("[WSc] Received text: %s\n", payload);

      // Look for "brightness" in the JSON
      char *ptr = strstr((char *)payload, "\"brightness\":");
      if (ptr) {
        // Move the pointer after the colon
        ptr = strchr(ptr, ':');
        if (ptr) {
          int value = atoi(ptr + 1);
          setLEDBrightness(value);
        }
      }
      break;
    }

    default:
      break;
  }
}

void setup() {
  USE_SERIAL.begin(115200);
  USE_SERIAL.println("\n[SETUP] Booting...");

  pinMode(LED_PIN, OUTPUT);
  setLEDBrightness(0);

  // Wi-Fi connection
  WiFi.begin(ssid, pass);
  USE_SERIAL.print("[WiFi] Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    USE_SERIAL.print(".");
  }
  USE_SERIAL.printf("\n[WiFi] Connected! SSID: %s | IP: %s\n",
                    WiFi.SSID().c_str(),
                    WiFi.localIP().toString().c_str());

  // WebSocket connection
  webSocket.begin(SERVER, PORT, URL);
  webSocket.onEvent(webSocketEvent);
  webSocket.setReconnectInterval(1000);
}

void loop() {
  webSocket.loop();
}