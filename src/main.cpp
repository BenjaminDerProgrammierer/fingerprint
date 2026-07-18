#include <Adafruit_Fingerprint.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include <Arduino.h>
#include <SPI.h>
#include <Servo.h>
#include <Wire.h>

#define LED_RED_PIN 2
#define LED_YELLOW_PIN 3
#define LED_GREEN_PIN 4
#define SERVO_PIN 5

#define OLED_ADDRESS 0x3c
#define OLED_SCREEN_WIDTH 128 // OLED display width, in pixels
#define OLED_SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1         // QT-PY / XIAO
Adafruit_SH1106G display(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire,
                         OLED_RESET);
Adafruit_Fingerprint finger(&Serial1);
Servo servo;

bool vaultState = false; // false = locked, true = unlocked

void initializeOledDisplay();
void initializeFingerprintSensor();
void printUnexpectedStatus(uint8_t status);
uint8_t detectFingerprint();
void showVaultState(int state);
void waitForFingerprintImage();
void setServoPosition(bool state);

/**
 * @brief Arduino setup function, runs once at startup
 *
 */
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println();
  initializeOledDisplay();

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  servo.attach(SERVO_PIN);
  showVaultState(0);       // start with vault locked
  setServoPosition(false); // start with servo in locked position
  initializeFingerprintSensor();
}

/**
 * @brief Arduino loop function, runs repeatedly after setup
 *
 */
void loop() {
  // put your main code here, to run repeatedly:
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Fingerprint Scanner");
  display.println("---------------------");
  display.println("Place your finger on the sensor");
  display.println("Scanning...");
  display.display();
  if (detectFingerprint() == FINGERPRINT_OK) {
    vaultState = !vaultState;

    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Fingerprint Scanner");
    display.println("---------------------");
    display.print("Found ID #");
    display.println(finger.fingerID);
    display.print("Confidence: ");
    display.println(finger.confidence);
    display.println(vaultState ? "Vault is unlocked" : "Vault is locked");
    display.display();

    showVaultState(2);
    setServoPosition(vaultState);
    delay(2000);
    if (vaultState) {
      showVaultState(2);
      delay(2000);
      showVaultState(1);
      Serial.println(F("Vault unlocked!"));
    } else {
      showVaultState(0);
      Serial.println(F("Vault locked!"));
    }
    showVaultState(vaultState ? 1 : 0);
  } else {
    display.clearDisplay();
    display.setCursor(0, 0);
    display.println("Fingerprint Scanner");
    display.println("---------------------");
    display.println("No match found.");
    display.display();
    delay(2000);
  }
}

/**
 * @brief Initializes the OLED display
 */
void initializeOledDisplay() {
  delay(250);
  if (!display.begin(OLED_ADDRESS, true)) {
    Serial.println(F("Failed to initialize OLED display"));
    while (true) {
      delay(1); // Halt program execution
    }
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("Fingerprint Scanner");
  display.println("---------------------");
  display.println("Initializing...");
  display.display();
}

/**
 * @brief Initializes the fingerprint sensor
 *
 */
void initializeFingerprintSensor() {
  finger.begin(57600);
  if (!finger.verifyPassword()) {
    Serial.println(F("Did not find fingerprint sensor"));
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SH110X_WHITE);
    display.setCursor(0, 0);
    display.println("Fingerprint Scanner");
    display.println("---------------------");
    display.println("FATAL ERROR\nFingerprint sensor not found");
    display.display();
    while (true) {
      delay(1); // Halt program execution
    }
  }

  Serial.println(F("Found fingerprint sensor!"));
  finger.getParameters();

  Serial.print(F("Status: 0x"));
  Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(finger.capacity);
  Serial.print(F("Security level: "));
  Serial.println(finger.security_level);
  Serial.print(F("Device address: "));
  Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet length: "));
  Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(finger.baud_rate);

  finger.getTemplateCount();
  if (finger.templateCount == 0) {
    Serial.println(F("Sensor does not contain any fingerprint templates."));
  } else {
    Serial.print(F("Sensor contains "));
    Serial.print(finger.templateCount);
    Serial.println(F(" templates."));
  }
}

/**
 * @brief Prints an unexpected status message
 *
 * @param status The status code returned by the fingerprint sensor
 */
void printUnexpectedStatus(uint8_t status) {
  if (status == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return;
  }

  Serial.print(F("Sensor error: 0x"));
  Serial.println(status, HEX);
}

/**
 * @brief Waits for a fingerprint image to be captured
 *
 */
void waitForFingerprintImage() {
  while (true) {
    const uint8_t status = finger.getImage();
    switch (status) {
    case FINGERPRINT_OK:
      Serial.println(F("Image taken"));
      return;
    case FINGERPRINT_NOFINGER:
      Serial.print('.');
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println(F("Communication error"));
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println(F("Imaging error"));
      break;
    default:
      printUnexpectedStatus(status);
      break;
    }
  }
}

/**
 * @brief Converts a fingerprint image to a template
 *
 * @param slot The slot to store the template in
 * @return uint8_t The status code returned by the fingerprint sensor
 */
uint8_t convertFingerprintImage(uint8_t slot) {
  const uint8_t status = finger.image2Tz(slot);
  switch (status) {
  case FINGERPRINT_OK:
    Serial.println(F("Image converted"));
    break;
  case FINGERPRINT_IMAGEMESS:
    Serial.println(F("Image too messy"));
    break;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println(F("Communication error"));
    break;
  case FINGERPRINT_FEATUREFAIL:
  case FINGERPRINT_INVALIDIMAGE:
    Serial.println(F("Could not find fingerprint features"));
    break;
  default:
    printUnexpectedStatus(status);
    break;
  }
  return status;
}

/**
 * @brief Detects a fingerprint and searches for a match
 *
 * @return uint8_t The status code returned by the fingerprint sensor
 */
uint8_t detectFingerprint() {
  Serial.println(F("Waiting for a valid finger..."));
  waitForFingerprintImage();

  uint8_t status = convertFingerprintImage(1);
  if (status != FINGERPRINT_OK) {
    return status;
  }

  status = finger.fingerSearch();
  switch (status) {
  case FINGERPRINT_OK:
    Serial.println(F("Found a print match!"));
    break;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println(F("Communication error"));
    return status;
  case FINGERPRINT_NOTFOUND:
    Serial.println(F("Did not find a match"));
    return status;
  default:
    printUnexpectedStatus(status);
    return status;
  }

  Serial.print(F("Found ID #"));
  Serial.print(finger.fingerID);
  Serial.print(F(" with confidence of "));
  Serial.println(finger.confidence);

  return status;
}

/**
 * @brief Shows the state of the vault
 *
 * @param state The state of the vault (0 = locked, 1 = unlocked, 2 =
 * locking/unlocking)
 */
void showVaultState(int state) {
  bool ledRedState = false;
  bool ledYellowState = false;
  bool ledGreenState = false;
  switch (state) {
  case 0: // locked
    ledRedState = true;
    ledYellowState = false;
    ledGreenState = false;
    break;
  case 1: // unlocked
    ledRedState = false;
    ledYellowState = false;
    ledGreenState = true;
    break;
  case 2: // locking
    ledRedState = false;
    ledYellowState = true;
    ledGreenState = false;
    break;
  default:
    ledRedState = false;
    ledYellowState = false;
    ledGreenState = false;
    break;
  }
  digitalWrite(LED_RED_PIN, ledRedState ? HIGH : LOW);
  digitalWrite(LED_YELLOW_PIN, ledYellowState ? HIGH : LOW);
  digitalWrite(LED_GREEN_PIN, ledGreenState ? HIGH : LOW);
}

/**
 * @brief Sets the position of the servo based on the state of the vault
 *
 * @param state The state of the vault (false = locked, true = unlocked)
 */
void setServoPosition(bool state) { servo.write(state ? 180 : 0); }