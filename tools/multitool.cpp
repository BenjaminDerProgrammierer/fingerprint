#include <Adafruit_Fingerprint.h>
#include <Arduino.h>

#define FIRST_TEMPLATE_ID 1
#define LAST_TEMPLATE_ID 127

Adafruit_Fingerprint sensor(&Serial1);

// --- Function declarations ---

void printMenu();
char readCommand();
void waitForConsoleInput();
void discardPendingInput();
uint8_t readTemplateId();
bool confirm(String prompt);
void eraseDatabase();
void enrollFingerprintFromConsole();
void deleteFingerprintFromConsole();
void printUnexpectedStatus(uint8_t status);

void waitForFingerprintImage();
void waitForFingerRemoval();
uint8_t convertFingerprintImage(uint8_t slot);
uint8_t detectFingerprint();
uint8_t enrollFingerprint(uint8_t templateId);
uint8_t deleteFingerprint(uint8_t templateId);

void initializeFingerprintSensor();

// --- Setup - ran once ---

void setup() {
  Serial.begin(9600);
  Serial.println();

  initializeFingerprintSensor();
}

// --- Loop - ran repeatedly after setup ---

void loop() {
  printMenu();

  switch (readCommand()) {
  case 'F':
    if (confirm(F("This will delete every fingerprint template."))) {
      eraseDatabase();
    }
    break;

  case 'E':
    enrollFingerprintFromConsole();
    break;

  case 'D':
    deleteFingerprintFromConsole();
    break;

  case 'T':
    detectFingerprint();
    break;

  default:
    Serial.println(F("Unknown command"));
    break;
  }
}

// --- Function definitions ---

void printMenu() {
  Serial.println();
  Serial.println(F("Fingerprint Menu"));
  Serial.println(F("F - Flush all fingerprint templates"));
  Serial.println(F("E - Enroll a new fingerprint template"));
  Serial.println(F("D - Delete a fingerprint template"));
  Serial.println(F("T - Test fingerprint detection"));
}

char readCommand() {
  waitForConsoleInput();

  char command = Serial.read();
  discardPendingInput();

  if (command >= 'a' && command <= 'z') {
    command -= 'a' - 'A';
  }

  return command;
}

void waitForConsoleInput() {
  while (!Serial.available()) {
    delay(1);
  }
}

void discardPendingInput() {
  while (Serial.available()) {
    Serial.read();
  }
}

uint8_t readTemplateId() {
  Serial.print(F("Enter a template ID ("));
  Serial.print(FIRST_TEMPLATE_ID);
  Serial.print(F("-"));
  Serial.print(LAST_TEMPLATE_ID);
  Serial.println(F("):"));

  while (true) {
    waitForConsoleInput();
    const long templateId = Serial.parseInt();
    discardPendingInput();
    if (templateId >= FIRST_TEMPLATE_ID && templateId <= LAST_TEMPLATE_ID) {
      return static_cast<uint8_t>(templateId);
    }

    Serial.println(F("Invalid ID. Try again:"));
  }
}

bool confirm(String prompt) {
  Serial.println(prompt);
  Serial.println(F("Press Y to continue or any other key to cancel."));

  return readCommand() == 'Y';
}

void eraseDatabase() {
  const uint8_t status = sensor.emptyDatabase();
  if (status == FINGERPRINT_OK) {
    Serial.println(F("Fingerprint database is now empty."));
    return;
  }

  Serial.print(F("Could not erase database. "));
  printUnexpectedStatus(status);
}

void enrollFingerprintFromConsole() {
  Serial.println(F("Ready to enroll a fingerprint!"));
  const uint8_t templateId = readTemplateId();
  Serial.print(F("Enrolling ID #"));
  Serial.println(templateId);
  enrollFingerprint(templateId);
}

void deleteFingerprintFromConsole() {
  const uint8_t templateId = readTemplateId();
  Serial.print(F("Deleting ID #"));
  Serial.println(templateId);
  deleteFingerprint(templateId);
}

void printUnexpectedStatus(uint8_t status) {
  if (status == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println(F("Communication error"));
    return;
  }

  Serial.print(F("Sensor error: 0x"));
  Serial.println(status, HEX);
}

void waitForFingerprintImage() {
  while (true) {
    const uint8_t status = sensor.getImage();
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

void waitForFingerRemoval() {
  Serial.println(F("Remove finger"));
  delay(2000);
  while (sensor.getImage() != FINGERPRINT_NOFINGER) {
    delay(1);
  }
}

uint8_t convertFingerprintImage(uint8_t slot) {
  const uint8_t status = sensor.image2Tz(slot);
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

uint8_t detectFingerprint() {
  Serial.println(F("Waiting for a valid finger..."));
  waitForFingerprintImage();

  uint8_t status = convertFingerprintImage(1);
  if (status != FINGERPRINT_OK) {
    return status;
  }

  status = sensor.fingerSearch();
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
  Serial.print(sensor.fingerID);
  Serial.print(F(" with confidence of "));
  Serial.println(sensor.confidence);

  return status;
}

uint8_t enrollFingerprint(uint8_t templateId) {
  Serial.print(F("Waiting for a valid finger to enroll as #"));
  Serial.println(templateId);
  waitForFingerprintImage();

  uint8_t status = convertFingerprintImage(1);
  if (status != FINGERPRINT_OK) {
    return status;
  }

  waitForFingerRemoval();

  Serial.println(F("Place the same finger again"));
  waitForFingerprintImage();

  status = convertFingerprintImage(2);
  if (status != FINGERPRINT_OK) {
    return status;
  }

  Serial.print(F("Creating model for #"));
  Serial.println(templateId);
  status = sensor.createModel();
  if (status == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println(F("Fingerprints did not match"));
    return status;
  }
  if (status != FINGERPRINT_OK) {
    printUnexpectedStatus(status);
    return status;
  }

  Serial.println(F("Prints matched!"));
  status = sensor.storeModel(templateId);
  switch (status) {
  case FINGERPRINT_OK:
    Serial.println(F("Stored!"));
    break;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println(F("Communication error"));
    break;
  case FINGERPRINT_BADLOCATION:
    Serial.println(F("Could not store in that location"));
    break;
  case FINGERPRINT_FLASHERR:
    Serial.println(F("Error writing to flash"));
    break;
  default:
    printUnexpectedStatus(status);
    break;
  }
  return status;
}

uint8_t deleteFingerprint(uint8_t templateId) {
  const uint8_t status = sensor.deleteModel(templateId);
  switch (status) {
  case FINGERPRINT_OK:
    Serial.println(F("Deleted!"));
    break;
  case FINGERPRINT_PACKETRECIEVEERR:
    Serial.println(F("Communication error"));
    break;
  case FINGERPRINT_BADLOCATION:
    Serial.println(F("Could not delete in that location"));
    break;
  case FINGERPRINT_FLASHERR:
    Serial.println(F("Error writing to flash"));
    break;
  default:
    printUnexpectedStatus(status);
    break;
  }
  return status;
}

void initializeFingerprintSensor() {
  sensor.begin(57600);
  if (!sensor.verifyPassword()) {
    Serial.println(F("Did not find fingerprint sensor"));
    while (true) {
      delay(1); // Halt program execution
    }
  }

  Serial.println(F("Found fingerprint sensor!"));
  sensor.getParameters();

  Serial.print(F("Status: 0x"));
  Serial.println(sensor.status_reg, HEX);
  Serial.print(F("Sys ID: 0x"));
  Serial.println(sensor.system_id, HEX);
  Serial.print(F("Capacity: "));
  Serial.println(sensor.capacity);
  Serial.print(F("Security level: "));
  Serial.println(sensor.security_level);
  Serial.print(F("Device address: "));
  Serial.println(sensor.device_addr, HEX);
  Serial.print(F("Packet length: "));
  Serial.println(sensor.packet_len);
  Serial.print(F("Baud rate: "));
  Serial.println(sensor.baud_rate);

  sensor.getTemplateCount();
  if (sensor.templateCount == 0) {
    Serial.println(F("Sensor does not contain any fingerprint templates."));
  } else {
    Serial.print(F("Sensor contains "));
    Serial.print(sensor.templateCount);
    Serial.println(F(" templates."));
  }
}
