#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// ================= WIFI =================
const char* ssid = "Infinix NOTE 30 5G";
const char* password = "12345678";

// ================= MQTT =================
const char* mqtt_server = "e99ba94b4eb8453f86ac243e05b95609.s1.eu.hivemq.cloud";
const int mqtt_port = 8883;
const char* mqtt_user = "Atharv";
const char* mqtt_pass = "Atharv@2005";

// ================= TOPICS =================
const char* topic_control = "home/control";
const char* topic_status  = "home/status";
const char* topic_sensors = "home/sensors";

// ================= PINS =================
#define RELAY_PIN 19
#define LED1_PIN 21
#define LED2_PIN 22
#define LED3_PIN 23
#define LED4_PIN 25
#define PIR_PIN 27
#define LDR_PIN 34

// ================= SETTINGS =================
const bool RELAY_ACTIVE_LOW = true;
const bool LDR_NIGHT_WHEN_LOW = true;

int LDR_THRESHOLD = 2500;

unsigned long NO_MOTION_DELAY = 3000;   // 3 sec
unsigned long PIR_WARMUP_TIME = 30000;  // 30 sec

// ================= VARIABLES =================
bool aiMode = true;

bool relayState = false;
bool led1State = false;
bool led2State = false;
bool led3State = false;
bool led4State = false;

bool waitingForPirRelease = false;

unsigned long bootTime = 0;
unsigned long lastMotionTime = 0;
unsigned long lastSensorPublish = 0;

WiFiClientSecure espClient;
PubSubClient client(espClient);

// ================= OUTPUT =================
void writeRelay(bool state) {
  relayState = state;
  digitalWrite(RELAY_PIN, RELAY_ACTIVE_LOW ? !state : state);
}

void setLed1(bool state) {
  led1State = state;
  digitalWrite(LED1_PIN, state ? HIGH : LOW);
}

void setLed2(bool state) {
  led2State = state;
  digitalWrite(LED2_PIN, state ? HIGH : LOW);
}

void setLed3(bool state) {
  led3State = state;
  digitalWrite(LED3_PIN, state ? HIGH : LOW);
}

void setLed4(bool state) {
  led4State = state;
  digitalWrite(LED4_PIN, state ? HIGH : LOW);
}

void setLightGroup(bool state) {
  writeRelay(state);
  setLed1(state);
  setLed2(state);
}

// ================= STATUS =================
void publishStatus(String device, bool state, String controller, String reason) {
  StaticJsonDocument<256> doc;

  doc["device"] = device;
  doc["state"] = state ? "on" : "off";
  doc["controller"] = controller;
  doc["reason"] = reason;

  char buffer[256];
  serializeJson(doc, buffer);

  client.publish(topic_status, buffer);
}

void publishLightStatus(String controller, String reason) {
  publishStatus("relay", relayState, controller, reason);
  publishStatus("led1", led1State, controller, reason);
  publishStatus("led2", led2State, controller, reason);
}

void publishAllStatus(String controller, String reason) {
  publishStatus("relay", relayState, controller, reason);
  publishStatus("led1", led1State, controller, reason);
  publishStatus("led2", led2State, controller, reason);
  publishStatus("led3", led3State, controller, reason);
  publishStatus("led4", led4State, controller, reason);
}

// ================= SENSOR =================
bool isNight(int ldrValue) {
  return LDR_NIGHT_WHEN_LOW ? (ldrValue < LDR_THRESHOLD) : (ldrValue > LDR_THRESHOLD);
}

bool readPIR() {
  return digitalRead(PIR_PIN) == HIGH;
}

void publishSensors() {
  int ldrValue = analogRead(LDR_PIN);
  bool motion = readPIR();
  bool night = isNight(ldrValue);

  unsigned long remaining = 0;

  if (aiMode && relayState && !motion) {
    unsigned long passed = millis() - lastMotionTime;

    if (passed < NO_MOTION_DELAY) {
      remaining = (NO_MOTION_DELAY - passed) / 1000;
    }
  }

  StaticJsonDocument<256> doc;

  doc["ldr"] = ldrValue;
  doc["motion"] = motion;
  doc["night_condition"] = night;
  doc["mode"] = aiMode ? "AI" : "MANUAL";
  doc["off_timer_sec"] = remaining;

  char buffer[256];
  serializeJson(doc, buffer);

  client.publish(topic_sensors, buffer);

  Serial.print("Sensor JSON: ");
  Serial.println(buffer);
}

// ================= AI AUTOMATION =================
void aiAutomation() {
  if (!aiMode) return;

  if (millis() - bootTime < PIR_WARMUP_TIME) return;

  int ldrValue = analogRead(LDR_PIN);
  bool motion = readPIR();
  bool night = isNight(ldrValue);

  Serial.print("PIR=");
  Serial.print(motion);
  Serial.print(" | Night=");
  Serial.print(night);
  Serial.print(" | Relay=");
  Serial.print(relayState);
  Serial.print(" | WaitingRelease=");
  Serial.println(waitingForPirRelease);

  // Day = OFF
  if (!night) {
    if (relayState || led1State || led2State) {
      setLightGroup(false);
      publishLightStatus("AI", "Day detected, light OFF");
    }

    waitingForPirRelease = false;
    return;
  }

  // After OFF, wait until PIR becomes LOW before allowing ON again
  if (waitingForPirRelease) {
    if (!motion) {
      waitingForPirRelease = false;
      Serial.println("PIR released. Ready for next motion.");
    } else {
      Serial.println("Ignoring PIR HIGH until LOW.");
      return;
    }
  }

  // Night + motion = ON
  if (night && motion) {
    lastMotionTime = millis();

    if (!relayState || !led1State || !led2State) {
      setLightGroup(true);
      publishLightStatus("AI", "Night + motion detected, light ON");
    }

    return;
  }

  // Night + no motion = OFF after 3 sec
  if (night && !motion && relayState) {
    if (millis() - lastMotionTime >= NO_MOTION_DELAY) {
      setLightGroup(false);
      publishLightStatus("AI", "No motion for 3 sec, light OFF");

      // prevents blinking ON-OFF-ON loop
      waitingForPirRelease = true;
    }
  }
}

// ================= COMMAND =================
void handleOneCommand(String cmd) {
  cmd.trim();
  cmd.toLowerCase();

  Serial.print("Command: ");
  Serial.println(cmd);

  if (cmd == "ai_mode") {
    aiMode = true;
    waitingForPirRelease = false;
    lastMotionTime = millis();
    publishStatus("system", true, "ESP32", "AI mode enabled");
    return;
  }

  if (cmd == "manual_mode") {
    aiMode = false;
    waitingForPirRelease = false;
    publishStatus("system", true, "ESP32", "Manual mode enabled");
    return;
  }

  if (cmd == "status_request") {
    publishAllStatus(aiMode ? "AI" : "Manual", "Status requested");
    publishSensors();
    return;
  }

  // Manual commands switch to manual mode
  if (
    cmd == "relay_on" || cmd == "relay_off" ||
    cmd == "led1_on" || cmd == "led1_off" ||
    cmd == "led2_on" || cmd == "led2_off" ||
    cmd == "led3_on" || cmd == "led3_off" ||
    cmd == "led4_on" || cmd == "led4_off" ||
    cmd == "light_on" || cmd == "light_off" ||
    cmd == "fan_on" || cmd == "fan_off" ||
    cmd == "all_on" || cmd == "all_off"
  ) {
    aiMode = false;
    waitingForPirRelease = false;
  }

  if (cmd == "relay_on") {
    writeRelay(true);
    publishStatus("relay", true, "Manual", "Relay ON");
    return;
  }

  if (cmd == "relay_off") {
    writeRelay(false);
    publishStatus("relay", false, "Manual", "Relay OFF");
    return;
  }

  if (cmd == "led1_on") {
    setLed1(true);
    publishStatus("led1", true, "Manual", "Light 1 ON");
    return;
  }

  if (cmd == "led1_off") {
    setLed1(false);
    publishStatus("led1", false, "Manual", "Light 1 OFF");
    return;
  }

  if (cmd == "led2_on") {
    setLed2(true);
    publishStatus("led2", true, "Manual", "Light 2 ON");
    return;
  }

  if (cmd == "led2_off") {
    setLed2(false);
    publishStatus("led2", false, "Manual", "Light 2 OFF");
    return;
  }

  if (cmd == "led3_on") {
    setLed3(true);
    publishStatus("led3", true, "Manual", "Fan 1 ON");
    return;
  }

  if (cmd == "led3_off") {
    setLed3(false);
    publishStatus("led3", false, "Manual", "Fan 1 OFF");
    return;
  }

  if (cmd == "led4_on") {
    setLed4(true);
    publishStatus("led4", true, "Manual", "Fan 2 ON");
    return;
  }

  if (cmd == "led4_off") {
    setLed4(false);
    publishStatus("led4", false, "Manual", "Fan 2 OFF");
    return;
  }

  if (cmd == "light_on") {
    setLightGroup(true);
    publishLightStatus("Manual", "Light group ON");
    return;
  }

  if (cmd == "light_off") {
    setLightGroup(false);
    publishLightStatus("Manual", "Light group OFF");
    return;
  }

  if (cmd == "fan_on") {
    setLed3(true);
    setLed4(true);
    publishStatus("led3", true, "Manual", "Fan 1 ON");
    publishStatus("led4", true, "Manual", "Fan 2 ON");
    return;
  }

  if (cmd == "fan_off") {
    setLed3(false);
    setLed4(false);
    publishStatus("led3", false, "Manual", "Fan 1 OFF");
    publishStatus("led4", false, "Manual", "Fan 2 OFF");
    return;
  }

  if (cmd == "all_on") {
    writeRelay(true);
    setLed1(true);
    setLed2(true);
    setLed3(true);
    setLed4(true);
    publishAllStatus("Manual", "All devices ON");
    return;
  }

  if (cmd == "all_off") {
    writeRelay(false);
    setLed1(false);
    setLed2(false);
    setLed3(false);
    setLed4(false);
    publishAllStatus("Manual", "All devices OFF");
    return;
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String msg = "";

  for (int i = 0; i < length; i++) {
    msg += (char)payload[i];
  }

  int start = 0;

  while (true) {
    int sep = msg.indexOf(';', start);

    if (sep == -1) {
      handleOneCommand(msg.substring(start));
      break;
    }

    handleOneCommand(msg.substring(start, sep));
    start = sep + 1;
  }
}

// ================= CONNECTION =================
void connectWiFi() {
  Serial.print("Connecting WiFi");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
}

void reconnectMQTT() {
  while (!client.connected()) {
    Serial.print("Connecting MQTT...");

    String clientId = "ESP32-AIoT-" + String(random(0xffff), HEX);

    if (client.connect(clientId.c_str(), mqtt_user, mqtt_pass)) {
      Serial.println("connected");

      client.subscribe(topic_control);

      publishStatus("system", true, "ESP32", "ESP32 connected");
      publishAllStatus(aiMode ? "AI" : "Manual", "Boot status");
      publishSensors();
    } else {
      Serial.print("failed rc=");
      Serial.println(client.state());
      delay(3000);
    }
  }
}

// ================= SETUP =================
void setup() {
  Serial.begin(115200);

  bootTime = millis();
  lastMotionTime = millis();

  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED1_PIN, OUTPUT);
  pinMode(LED2_PIN, OUTPUT);
  pinMode(LED3_PIN, OUTPUT);
  pinMode(LED4_PIN, OUTPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LDR_PIN, INPUT);

  writeRelay(false);
  setLed1(false);
  setLed2(false);
  setLed3(false);
  setLed4(false);

  connectWiFi();

  espClient.setInsecure();

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
}

// ================= LOOP =================
void loop() {
  if (!client.connected()) {
    reconnectMQTT();
  }

  client.loop();

  aiAutomation();

  if (millis() - lastSensorPublish >= 1000) {
    lastSensorPublish = millis();
    publishSensors();
  }
}