#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>

MFRC522DriverPinSimple ss_pin(10);

MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
MFRC522 mfrc522{driver};  // Create MFRC522 instance.

const int MOSFET_SIG_PIN = 6;
bool isMosfetOn = false;
bool tagIsNotPresent = true;
bool wrongCardPresent = false;

// Key for activation
const String KEY_UUID = "<redacted>";

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(MOSFET_SIG_PIN, OUTPUT);
  Serial.begin(115200);
  // while (!Serial);
  mfrc522.PCD_Init();
  Serial.println(F("Ready!"));
  switchMosfetOn();
}

void loop() {
  // Weird workaround for https://github.com/miguelbalboa/rfid/issues/279
  tagIsNotPresent = !mfrc522.PICC_IsNewCardPresent() && !mfrc522.PICC_IsNewCardPresent();
  if (!tagIsNotPresent) {
    if(isMosfetOn) {
      if(!wrongCardPresent) {
        Serial.print(F("Tag found, checking key..."));
      }
      if (getTextRecord().equals(KEY_UUID)) {
        Serial.print(F("PASS\n"));
        Serial.println(F("Switching MOSFET off!"));
        switchMosfetOff();
      } else {
        if (!wrongCardPresent) {
          Serial.print(F("FAIL\n"));
          Serial.println(F("Provide correct tag!"));
          wrongCardPresent = true;
        }
      }
    }
  } else {
    if (wrongCardPresent) {
      Serial.println(F("Tag removed."));
    } else {
      if(!isMosfetOn) {
        Serial.println(F("Tag removed, switching MOSFET on!"));
        switchMosfetOn();
      }
    }
    wrongCardPresent = false;
  }
}

void switchMosfetOn() {
  digitalWrite(MOSFET_SIG_PIN, HIGH);
  digitalWrite(LED_BUILTIN, HIGH);
  isMosfetOn = true;
  delay(200);
}

void switchMosfetOff() {
  digitalWrite(MOSFET_SIG_PIN, LOW);
  digitalWrite(LED_BUILTIN, LOW);
  isMosfetOn = false;
  delay(200);
}

String getTextRecord() {
  byte byteCount;
  byte buffer[18];
  byte i;
  String outputString = "";
  bool startFound = false;
  bool endFound = false;
  //  Ultralight C has 135 pages
  for(byte page = 0; page < 135; page += 4) { // Read returns data for 4 pages at a time.
    // Read pages
    byteCount = sizeof(buffer);
    mfrc522.MIFARE_Read(page, buffer, &byteCount);
    // Dump data
    for(byte offset = 0; offset < 4; offset++) {
      i = page+offset;
      for(byte index = 0; index < 4; index++) {
        i = 4*offset+index;
        // NTAG215 text record content starts with 0x6E
        if(buffer[i] == 0x6E) {
          startFound = true;
          continue;
        }
        // NTAG215 text record content ends with 0xFE
        if(buffer[i] == 0xFE) {
          endFound = true;
          continue;
        }
        if(startFound && !endFound) {
          outputString += (char)buffer[i];
        }
      }
    }
  }
  outputString.trim();
  return outputString;
}