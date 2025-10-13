#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <WebSocketsClient.h>

WiFiMulti WiFiMulti;
WebSocketsClient webSocket;

#define USE_SERIAL Serial
#define LED_PIN 2  // Change this to your actual LED pin (e.g. D2, GPIO2)

// PWM setup
const int PWM_FREQ = 5000;   // frequency in Hz
const int PWM_CHANNEL = 0;   // PWM channel
const int PWM_RESOLUTION = 8; // 8-bit resolution (0–255)

int currentBrightness = 0;

// Function to set LED brightness using PWM
void setLEDBrightness(int brightness) {
    brightness = constrain(brightness, 0, 255);
    ledcWrite(PWM_CHANNEL, brightness);
    currentBrightness = brightness;
    USE_SERIAL.printf("[LED] Brightness set to: %d\n", brightness);
}

// WebSocket events
void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            USE_SERIAL.println("[WSc] Disconnected!");
            break;

        case WStype_CONNECTED:
            USE_SERIAL.printf("[WSc] Connected to: %s\n", payload);
            // Send hello message
            webSocket.sendTXT("{\"status\":\"esp_connected\"}");
            break;

        case WStype_TEXT:
            USE_SERIAL.printf("[WSc] Received text: %s\n", payload);
            // Parse JSON manually (lightweight approach)
            if (strstr((char*)payload, "brightness") != NULL) {
                // Extract number (simple parsing)
                int value = atoi(strchr((char*)payload, ':') + 1);
                setLEDBrightness(value);
            }
            break;

        case WStype_BIN:
            USE_SERIAL.printf("[WSc] Binary message received: %u bytes\n", length);
            break;

        default:
            break;
    }
}

void setup() {
    USE_SERIAL.begin(115200);
    USE_SERIAL.println();
    USE_SERIAL.println("[SETUP] Booting...");

    // Setup LED PWM
    ledcSetup(PWM_CHANNEL, PWM_FREQ, PWM_RESOLUTION);
    ledcAttachPin(LED_PIN, PWM_CHANNEL);
    setLEDBrightness(0);

    // Connect Wi-Fi
    WiFiMulti.addAP("Loki", "12345678  "); // replace with your credentials
    USE_SERIAL.print("[WiFi] Connecting");
    while(WiFiMulti.run() != WL_CONNECTED) {
        delay(250);
        USE_SERIAL.print(".");
    }
    USE_SERIAL.printf("\n[WiFi] Connected! IP: %s\n", WiFi.localIP().toString().c_str());

    // Connect WebSocket
    webSocket.begin("192.168.1.3", 3000, "/");  // Match your Node.js IP + port
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}

void loop() {
    webSocket.loop();
}
