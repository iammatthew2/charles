#include <Arduino.h>
#include <ESP32Servo.h>
#include <WiFi.h>
#include <esp_now.h>

#include <cstring>

// Optional sender MAC filter. Keep all zeros to accept packets from any sender.
static uint8_t DARYL_MAC[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static constexpr uint8_t SERVO_PIN = 1;  // GPIO1 / D1
static constexpr int SERVO_MIN_DEG = 0;
static constexpr int SERVO_MAX_DEG = 180;
static constexpr int SERVO_DEFAULT_DEG = 90;
static constexpr int SERVO_DEG_PER_ENCODER_STEP = 4;
static constexpr uint32_t LINK_TIMEOUT_MS = 1500;

struct __attribute__((packed)) RemotePacket {
  uint32_t seq;
  uint32_t uptimeMs;
  int32_t encoderPosition;
  int16_t encoderDelta;
  uint8_t buttonsMask;
  uint8_t encoderPressed;
};

Servo gServo;
volatile bool gPacketReady = false;
volatile RemotePacket gLatestPacket = {};
volatile uint8_t gSourceMac[6] = {0};

int gServoAngle = SERVO_DEFAULT_DEG;
uint32_t gLastRxMs = 0;
bool gHasLastEncoderPosition = false;
int32_t gLastEncoderPosition = 0;

static bool isZeroMac(const uint8_t* mac) {
  for (size_t i = 0; i < 6; ++i) {
    if (mac[i] != 0) {
      return false;
    }
  }
  return true;
}

static bool macEquals(const uint8_t* a, const uint8_t* b) {
  return memcmp(a, b, 6) == 0;
}

static void printMac(const uint8_t* mac) {
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02X:%02X:%02X:%02X:%02X:%02X", mac[0],
           mac[1], mac[2], mac[3], mac[4], mac[5]);
  Serial.print(macStr);
}

static void setServoAngle(int angle) {
  gServoAngle = constrain(angle, SERVO_MIN_DEG, SERVO_MAX_DEG);
  gServo.write(gServoAngle);
}

static void applyButtonsToServo(uint8_t buttonsMask) {
  if (buttonsMask & (1u << 0)) {
    setServoAngle(0);
  } else if (buttonsMask & (1u << 1)) {
    setServoAngle(45);
  } else if (buttonsMask & (1u << 2)) {
    setServoAngle(90);
  } else if (buttonsMask & (1u << 3)) {
    setServoAngle(135);
  } else if (buttonsMask & (1u << 4)) {
    setServoAngle(180);
  }

  if (buttonsMask & (1u << 5)) {
    setServoAngle(SERVO_DEFAULT_DEG);
  }
}

static void onDataRecv(const esp_now_recv_info_t* info, const uint8_t* data,
                       int len) {
  if (len != static_cast<int>(sizeof(RemotePacket))) {
    return;
  }

  if (!isZeroMac(DARYL_MAC) && !macEquals(info->src_addr, DARYL_MAC)) {
    return;
  }

  memcpy((void*)&gLatestPacket, data, sizeof(RemotePacket));
  memcpy((void*)gSourceMac, info->src_addr, sizeof(gSourceMac));
  gPacketReady = true;
}

static bool initEspNow() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed");
    return false;
  }

  esp_now_register_recv_cb(onDataRecv);

  Serial.print("Charles MAC: ");
  Serial.println(WiFi.macAddress());
  if (!isZeroMac(DARYL_MAC)) {
    Serial.print("Accepting only Daryl MAC: ");
    printMac(DARYL_MAC);
    Serial.println();
  } else {
    Serial.println("Accepting sender from any MAC (filter disabled)");
  }
  return true;
}

static void initServo() {
  ESP32PWM::allocateTimer(0);
  ESP32PWM::allocateTimer(1);
  ESP32PWM::allocateTimer(2);
  ESP32PWM::allocateTimer(3);

  gServo.setPeriodHertz(50);
  gServo.attach(SERVO_PIN, 1000, 2000);
  setServoAngle(SERVO_DEFAULT_DEG);

  Serial.print("Servo attached to GPIO");
  Serial.println(SERVO_PIN);
}

void setup() {
  Serial.begin(115200);
  delay(800);

  Serial.println();
  Serial.println("Charles ESP-NOW receiver booting...");

  initServo();
  if (!initEspNow()) {
    Serial.println("Setup failed. Rebooting in 3 seconds...");
    delay(3000);
    ESP.restart();
  }
}

void loop() {
  if (gPacketReady) {
    noInterrupts();
    RemotePacket packet = {};
    memcpy(&packet, (const void*)&gLatestPacket, sizeof(packet));
    uint8_t srcMac[6];
    memcpy(srcMac, (const void*)gSourceMac, sizeof(srcMac));
    gPacketReady = false;
    interrupts();

    gLastRxMs = millis();

    int32_t movementSteps = packet.encoderDelta;
    if (gHasLastEncoderPosition) {
      movementSteps = packet.encoderPosition - gLastEncoderPosition;
    }
    gLastEncoderPosition = packet.encoderPosition;
    gHasLastEncoderPosition = true;

    if (movementSteps != 0) {
      setServoAngle(gServoAngle + static_cast<int>(movementSteps) *
                                      SERVO_DEG_PER_ENCODER_STEP);
    }

    applyButtonsToServo(packet.buttonsMask);

    if (packet.encoderPressed) {
      setServoAngle(SERVO_DEFAULT_DEG);
    }

    Serial.print("rx seq=");
    Serial.print(packet.seq);
    Serial.print(" from=");
    printMac(srcMac);
    Serial.print(" buttons=0b");
    Serial.print(packet.buttonsMask, BIN);
    Serial.print(" encDelta=");
    Serial.print(packet.encoderDelta);
    Serial.print(" servo=");
    Serial.println(gServoAngle);
  }

  if (gLastRxMs != 0 && (millis() - gLastRxMs) > LINK_TIMEOUT_MS) {
    gLastRxMs = 0;
    setServoAngle(SERVO_DEFAULT_DEG);
    Serial.println("link timeout -> servo centered");
  }

  delay(2);
}