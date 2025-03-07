#include <ESP8266WiFi.h>

// ---------------------- USER SETTINGS ----------------------
const char* ssid     = "ESP_HOTSPOT";     // AP SSID
const char* password = "12345678";        // AP Password

IPAddress serverIP(192, 168, 4, 1);       // Server's AP IP
const uint16_t serverPort = 80;           // Server's TCP port

WiFiClient client;

void setup() {
  Serial.begin(115200);
  delay(100);

  // ------------------ WATCHDOG SETUP ------------------
  ESP.wdtDisable();
  ESP.wdtEnable(0); // default ~3s software WDT

  Serial.println("\n[CLIENT] Connecting to AP...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  // Wait for Wi-Fi to connect
  while (WiFi.status() != WL_CONNECTED) {
    ESP.wdtFeed(); // keep feeding in case connecting takes a while
    delay(500);
    Serial.print(".");
  }
  Serial.println("\n[CLIENT] Wi-Fi connected.");
  Serial.print("[CLIENT] IP address: ");
  Serial.println(WiFi.localIP());

  connectToServer();
}

void loop() {
  // Feed the WDT each loop to prevent resets
  ESP.wdtFeed();

  if (!client.connected()) {
    Serial.println("[CLIENT] Disconnected from server, trying reconnect...");
    connectToServer();
    delay(2000);
    return; 
  }

  // Read from Serial -> send to server
  if (Serial.available()) {
    String fromSerial = Serial.readStringUntil('\n');
    fromSerial.trim();
    if (fromSerial.length() > 0) {
      sendToServer(fromSerial.c_str());
    }
  }

  // Read from server -> print to Serial
  if (client.available()) {
    String fromServer = client.readStringUntil('\n');
    fromServer.trim();
    if (fromServer.length() > 0) {
      Serial.print("[CLIENT] Received from server: ");
      Serial.println(fromServer);
    }
  }

  // Let the system do background tasks & feed hardware WDT
  delay(10);
}

void connectToServer() {
  Serial.print("[CLIENT] Connecting to server at ");
  Serial.print(serverIP);
  Serial.print(":");
  Serial.println(serverPort);

  if (client.connect(serverIP, serverPort)) {
    Serial.println("[CLIENT] Connected to server!");
  } else {
    Serial.println("[CLIENT] Failed to connect!");
  }
}

void sendToServer(const char* msg) {
  if (client.connected()) {
    client.println(msg);
    Serial.print("[CLIENT] Sent to server: ");
    Serial.println(msg);
  } else {
    Serial.println("[CLIENT] Cannot send, not connected to server.");
  }
}
