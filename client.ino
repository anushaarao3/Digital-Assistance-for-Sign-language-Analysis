#include <ESP8266WiFi.h>
#include <Wire.h>
#include "SSD1306Wire.h"  // From the ESP8266 OLED library

// ------------------------- Wi-Fi & Server Config -------------------------
const char* ssid     = "ESP_HOTSPOT";
const char* password = "12345678"; // AP password

WiFiServer server(80);   // Listening on port 80
WiFiClient activeClient; // We'll handle one client at a time

// ------------------------- LED Indicator (Optional) ----------------------
#define LED_PIN LED_BUILTIN  // Built-in LED on many ESP8266 boards
// Note: Often active LOW, so digitalWrite(LED_PIN, LOW) = LED ON

// ------------------------- OLED Display Object --------------------------
// Using the same style as your working demo:
//  - 0x3C is the common I2C address for most SSD1306 modules
//  - D3 = SDA, D5 = SCL on NodeMCU / Wemos
SSD1306Wire display(0x3C, D3, D5);

// ------------------------- Setup ----------------------------------------
void setup() {
  Serial.begin(115200);
  delay(100);

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Turn LED off initially (if active LOW)

  // ------------------- Init the OLED (Library Version) -----------------
  display.init();
  display.flipScreenVertically();   // Flip if your display is upside down
  display.setFont(ArialMT_Plain_10);

  // Show a quick boot message
  display.clear();
  display.drawString(0, 0, "Starting Wi-Fi AP...");
  display.display();

  // ------------------- Start Wi-Fi AP + TCP Server ---------------------
  Serial.println("\n[SERVER] Setting up AP...");
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  server.begin();
  Serial.println("[SERVER] TCP Server started");

  // Print the AP IP (usually 192.168.4.1)
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("[SERVER] AP IP Address: ");
  Serial.println(apIP);

  // Update OLED to show AP info
  display.clear();
  display.drawString(0, 0, "AP: " + String(ssid));
  display.drawString(0, 12, "IP: " + apIP.toString());
  display.display();
}

// ------------------------- Main Loop ------------------------------------
void loop() {
  // 1) Check if no client is connected OR the old client disconnected
  if (!activeClient || !activeClient.connected()) {
    // See if a new client is available
    WiFiClient newClient = server.available();
    if (newClient) {
      // If we already had a client, stop it
      if (activeClient && activeClient.connected()) {
        activeClient.stop();
      }
      activeClient = newClient;
      Serial.println("[SERVER] Client connected!");

      // Turn LED on to indicate connection (if active LOW)
      digitalWrite(LED_PIN, LOW);

      // Update OLED
      display.clear();
      display.drawString(0, 0, "Client connected!");
      display.display();
    }
    // If still no connected client, turn LED off
    if (!activeClient || !activeClient.connected()) {
      digitalWrite(LED_PIN, HIGH);
    }
  }

  // 2) If we do have a client, handle data
  if (activeClient && activeClient.connected()) {
    // a) Read incoming data from client
    if (activeClient.available()) {
      String fromClient = activeClient.readStringUntil('\n');
      fromClient.trim();
      if (fromClient.length() > 0) {
        Serial.print("[SERVER] Received from client: ");
        Serial.println(fromClient);

        // Show on the OLED
        display.clear();
        display.drawString(0, 0, "Client says:");
        display.drawString(0, 26, fromClient);
        display.display();
      }
    }

    // b) If user typed something in Serial Monitor, send to client
    if (Serial.available()) {
      String fromSerial = Serial.readStringUntil('\n');
      fromSerial.trim();
      if (fromSerial.length() > 0) {
        activeClient.println(fromSerial);
        Serial.print("[SERVER] Sent to client: ");
        Serial.println(fromSerial);

        // Optionally show this on the OLED as well
        display.clear();
        display.drawString(0, 0, "Sent to client:");
        display.drawString(0, 16, fromSerial);
        display.display();
      }
    }
  }

  delay(10); // small delay yields CPU time to Wi-Fi stack
}
