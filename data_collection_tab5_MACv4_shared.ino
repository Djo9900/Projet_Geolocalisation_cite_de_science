#include <M5Unified.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <SD.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
 
// ==================== CONFIGURATION ====================
#define WIFI_SSID ""  // add your ID wifi
#define WIFI_PASSWORD "" // add your password wifi
#define MQTT_BROKER ""  // Add your MQTT_Broker
#define MQTT_PORT 8883
#define MQTT_USERNAME ""  // MQTT_USERNAME
#define MQTT_PASSWORD "*" // MQTT_PASSWORD
#define MQTT_CLIENT_ID "" // Add  your MQTT_CLIENT_ID
#define TOPIC_DATA "LaVilette/science/wifi/data"
 
#define MAX_APS 10 
#define SAMPLES_PER_PRESS 50 
#define SAMPLE_DELAY_MS 100 
#define NUM_LOCATIONS 10
 
struct TouchButton { int x, y, w, h; String label; uint16_t color; };
TouchButton locationButtons[NUM_LOCATIONS];
TouchButton sendMqttButton, resetDataButton;
 
String tracked_macs[MAX_APS];
int selected_location = -1;
WiFiClientSecure wifiClient;
PubSubClient mqttClient(wifiClient);
 
// ==================== INTERFACE GRAPHIQUE ====================
void initButtons() {
  int bw = 160, bh = 50, startX = 140;
  for (int i = 0; i < NUM_LOCATIONS; i++) {
    locationButtons[i] = { startX + (i % 2) * 170, 50 + (i / 2) * 60, bw, bh, "Zone " + String(i + 1), TFT_DARKGREY };
  }
  sendMqttButton = { 140, 360, 200, 60, "ENVOYER", TFT_BLUE };
  resetDataButton = { 350, 360, 110, 60, "RESET", TFT_RED };
}
 
void drawUI() {
  M5.Display.clear(TFT_BLACK);
  M5.Display.setTextSize(1.5);
  M5.Display.setTextColor(TFT_WHITE, TFT_BLACK);
  M5.Display.drawCenterString("COLLECTEUR PRECISION IIOT", 240, 10);
  M5.Display.setTextSize(1.0);
  for (int i = 0; i < NUM_LOCATIONS; i++) {
    String filename = "/" + locationButtons[i].label + ".csv";
    int yPos = 55 + (i * 38);
    bool exists = SD.exists(filename);
    M5.Display.setTextColor(exists ? TFT_GREEN : TFT_DARKGREY, TFT_BLACK);
    M5.Display.drawString("Z" + String(i + 1) + (exists ? ": OK" : ": VIDE"), 20, yPos);
  }
  for (int i = 0; i < NUM_LOCATIONS; i++) {
    uint16_t col = (i == selected_location) ? TFT_GREEN : locationButtons[i].color;
    M5.Display.fillRoundRect(locationButtons[i].x, locationButtons[i].y, locationButtons[i].w, locationButtons[i].h, 12, col);
    M5.Display.setTextColor(TFT_WHITE);
    M5.Display.drawCenterString(locationButtons[i].label, locationButtons[i].x + 80, locationButtons[i].y + 18);
  }
  M5.Display.fillRoundRect(sendMqttButton.x, sendMqttButton.y, sendMqttButton.w, sendMqttButton.h, 12, sendMqttButton.color);
  M5.Display.drawCenterString(sendMqttButton.label, sendMqttButton.x + 100, sendMqttButton.y + 22);
  M5.Display.fillRoundRect(resetDataButton.x, resetDataButton.y, resetDataButton.w, resetDataButton.h, 12, resetDataButton.color);
  M5.Display.drawCenterString(resetDataButton.label, resetDataButton.x + 55, resetDataButton.y + 22);
}
 
// ==================== COLLECTE PRÉCISE (50 SCANS) ====================
void collectSample() {
  if (selected_location == -1) return;
  M5.Display.fillRect(140, 320, 320, 35, TFT_BLACK); 
  M5.Display.setTextColor(TFT_YELLOW);
  M5.Display.drawCenterString("SCAN EN COURS (50x)...", 300, 320);
 
  String filename = "/" + locationButtons[selected_location].label + ".csv";
  File f = SD.open(filename, FILE_APPEND);
  if (f) {
    for (int s = 0; s < SAMPLES_PER_PRESS; s++) {
      int barWidth = map(s, 0, SAMPLES_PER_PRESS - 1, 0, 200);
      M5.Display.fillRect(200, 345, 200, 5, TFT_DARKGREY);
      M5.Display.fillRect(200, 345, barWidth, 5, TFT_GREEN);
 
      int n = WiFi.scanNetworks();
      f.print(locationButtons[selected_location].label);
      for (int j = 0; j < MAX_APS; j++) {
        float rssi = -100.0;
        for (int i = 0; i < n; i++) {
          if (WiFi.BSSIDstr(i) == tracked_macs[j]) { rssi = WiFi.RSSI(i); break; }
        }
        f.print(","); f.print(rssi);
      }
      f.println();
      delay(SAMPLE_DELAY_MS);
    }
    f.close();
  }
  drawUI();
}
 
// ==================== ENVOI SÉCURISÉ (PAR PAQUETS) ====================
void syncMQTT() {
  M5.Display.clear(TFT_BLACK);
  M5.Display.drawCenterString("CONNEXION WIFI...", 240, 150);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  int retry = 0;
  while (WiFi.status() != WL_CONNECTED && retry < 20) { delay(500); retry++; }
 
  if (WiFi.status() == WL_CONNECTED) {
    wifiClient.setInsecure();
    mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
    mqttClient.setBufferSize(8000); // Taille de buffer stable
 
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) {
      M5.Display.drawCenterString("TRANSFERT PAR PAQUETS...", 240, 180);
      for (int i = 0; i < NUM_LOCATIONS; i++) {
        String locLabel = locationButtons[i].label;
        String filename = "/" + locLabel + ".csv";
        if (SD.exists(filename)) {
          File f = SD.open(filename);
          while (f.available()) {
            DynamicJsonDocument doc(7000); // JSON plus petit pour la RAM
            doc["location"] = locLabel;
            JsonArray samplesArray = doc.createNestedArray("samples");
 
            // On envoie par 10 pour ne pas saturer la mémoire
            for (int k = 0; k < 10 && f.available(); k++) {
              String line = f.readStringUntil('\n');
              if (line.length() < 10) continue;
              JsonObject sampleObj = samplesArray.createNestedObject();
              JsonArray aps = sampleObj.createNestedArray("aps");
              int lastComma = line.indexOf(',');
              for (int j = 0; j < MAX_APS; j++) {
                int nextComma = line.indexOf(',', lastComma + 1);
                String val = (nextComma == -1) ? line.substring(lastComma + 1) : line.substring(lastComma + 1, nextComma);
                JsonObject ap = aps.createNestedObject();
                ap["mac"] = tracked_macs[j];
                ap["rssi"] = val.toFloat();
                lastComma = nextComma;
              }
            }
            String payload; serializeJson(doc, payload);
            mqttClient.publish(TOPIC_DATA, payload.c_str());
            delay(250); // Pause pour laisser le temps au broker de traiter
          }
          f.close();
        }
      }
      M5.Display.fillScreen(TFT_GREEN); delay(1500);
    }
  }
  WiFi.disconnect();
  drawUI();
}
 
void setup() {
  auto cfg = M5.config();
  M5.begin(cfg);
  M5.Display.setRotation(1);
  if (!SD.begin()) {
    M5.Display.drawCenterString("ERREUR SD", 240, 120);
    while(1);
  }
  WiFi.mode(WIFI_STA);
  int n = WiFi.scanNetworks();
  for(int i=0; i < MAX_APS; i++) tracked_macs[i] = (i < n) ? WiFi.BSSIDstr(i) : "00:00:00:00:00:00";
  initButtons();
  drawUI();
}
 
void loop() {
  M5.update();
  auto t = M5.Touch.getDetail();
  if (t.wasPressed()) {
    for (int i = 0; i < NUM_LOCATIONS; i++) {
      if (t.x > locationButtons[i].x && t.x < locationButtons[i].x + 160 &&
          t.y > locationButtons[i].y && t.y < locationButtons[i].y + 50) {
        selected_location = i; collectSample();
      }
    }
    if (t.x > sendMqttButton.x && t.x < sendMqttButton.x + 200 &&
        t.y > sendMqttButton.y && t.y < sendMqttButton.y + 60) syncMQTT();
    if (t.x > resetDataButton.x && t.x < resetDataButton.x + 110 &&
        t.y > resetDataButton.y && t.y < resetDataButton.y + 60) {
        M5.Display.fillScreen(TFT_RED);
        for(int i=1; i<=10; i++) SD.remove("/Zone " + String(i) + ".csv");
        delay(1000); drawUI();
    }
  }
}