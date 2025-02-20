
#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>

// Déclaration pour le capteur A (I2C)
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfcA = NfcAdapter(pn532_i2c);

// Déclaration pour le capteur B (UART)
SoftwareSerial SWSerialB(6, 7); // RX = 6, TX = 7
PN532_SWHSU pn532swhsuB(SWSerialB);
PN532 nfcB(pn532swhsuB);

// Variables pour la détection des tags
String lastTagA = "", lastTagB = "";
bool tagDetectedA = false, tagDetectedB = false;

void setup() {
  Serial.begin(115200);
  Serial.println("Initialisation des capteurs RFID...");

  // Initialisation du capteur A (I2C)
  nfcA.begin();
  Serial.println("Capteur A (I2C) prêt.");

  // Initialisation du capteur B (UART)
  SWSerialB.begin(115200);
  nfcB.begin();
  if (!nfcB.getFirmwareVersion()) {
    Serial.println("Capteur B (UART) non détecté !");
  } else {
    nfcB.SAMConfig();
    Serial.println("Capteur B (UART) prêt.");
  }
}

void loop() {
  readTagI2C();
  readTagUART();
  delay(0); // Petit délai pour éviter chevauchement
}

void readTagI2C() {
  if (nfcA.tagPresent()) {
    if (!tagDetectedA) { // Permet une seule détection tant que le tag est présent
      NfcTag tag = nfcA.read();
      String tagId = formatUID(tag.getUidString());
      Serial.print("Tag détecté sur Capteur A (I2C) : ");
      Serial.println(tagId);
      tagDetectedA = true; // Empêche la relecture tant que le tag reste présent
      lastTagA = tagId;
    }
  } else {
    tagDetectedA = false; // Réinitialise lorsque le tag est retiré
  }
}

void readTagUART() {
  uint8_t uid[7];
  uint8_t uidLength;

  if (nfcB.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    if (!tagDetectedB) { // Permet une seule détection tant que le tag est présent
      String tagId = "";
      for (uint8_t i = 0; i < uidLength; i++) {
        char hexChar[3];
        sprintf(hexChar, "%02X", uid[i]); // Conversion en hexadécimal majuscule
        tagId += hexChar;
      }
      Serial.print("Tag détecté sur Capteur B (UART) : ");
      Serial.println(tagId);
      tagDetectedB = true; // Empêche la relecture tant que le tag reste présent
      lastTagB = tagId;
    }
  } else {
    tagDetectedB = false; // Réinitialise lorsque le tag est retiré
  }
}

// Fonction pour formater l'UID (supprime les espaces et convertit en majuscules)
String formatUID(String rawUID) {
  String formattedUID = "";
  for (uint8_t i = 0; i < rawUID.length(); i++) {
    char c = rawUID.charAt(i);
    if (c != ' ') { // Ignore les espaces
      formattedUID += String(c);
    }
  }
  formattedUID.toUpperCase(); // Conversion en majuscules
  return formattedUID;
}


