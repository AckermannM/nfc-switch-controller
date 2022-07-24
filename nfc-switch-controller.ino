#include <MFRC522v2.h>
#include <MFRC522DriverSPI.h>
#include <MFRC522DriverPinSimple.h>
#include <MFRC522Debug.h>
#include "tag-data.h"

#define DEBUG

#ifndef TAG_DATA 
#pragma GCC error "Please create tag-data.h with KEY_UID definition"
#endif

MFRC522DriverPinSimple ss_pin(10);

MFRC522DriverSPI driver{ss_pin}; // Create SPI driver.
MFRC522 mfrc522{driver};  // Create MFRC522 instance.

const int RELAY_SIG_PIN = 6;
byte relayState = 0;
bool tagIsNotPresent = true;
bool wrongCardPresent = false;

void setup() {
  pinMode(RELAY_SIG_PIN, OUTPUT);
  mfrc522.PCD_Init();
}

void loop() {
  // Workaround for https://github.com/miguelbalboa/rfid/issues/279
  tagIsNotPresent = !mfrc522.PICC_IsNewCardPresent() && !mfrc522.PICC_IsNewCardPresent();
  
  mfrc522.PICC_ReadCardSerial();
  String uidString = uidToString(&(mfrc522.uid));
  
  if (!tagIsNotPresent) {
    if(relayState == 0) {
      if (uidString.equals(KEY_UID)) {
        toggleRelay();
      } else {
        if (!wrongCardPresent) {
          wrongCardPresent = true;
        }
      }
    }
  } else {
    if (!wrongCardPresent && relayState == 1) {
      toggleRelay();
    }
    wrongCardPresent = false;
  }
}

void toggleRelay() {
  digitalWrite(RELAY_SIG_PIN, !digitalRead(RELAY_SIG_PIN));
  relayState = 1 - relayState;
  delay(200);
}

String uidToString(MFRC522::Uid * uids) {
  String recievedUid;
  for(byte i = 0; i < uids->size; i++) {
    if (i > 0) {
      recievedUid += ":";
    }
    if(uids->uidByte[i] < 0x10) {
      recievedUid += "0" + String(uids->uidByte[i], HEX);
    } else {
      recievedUid += String(uids->uidByte[i], HEX);
    }
  }
  return recievedUid;
}
