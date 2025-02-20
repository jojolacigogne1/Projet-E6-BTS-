#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>

// Déclaration du capteur RFID en I2C
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);

// stockage le dernier tag détecté
String lastTag = "";
bool tagDetected = false; // Évite la détection multiple du même tag

void setup() {
  Serial.begin(115200); // Réglage du Baudrate
  Serial.println("Initialisation du capteur RFID...");

  // Initialisation du capteur RFID en I2C
  nfc.begin();
  Serial.println("Capteur RFID (I2C) prêt.");
}

void loop() {
  readTag(); 
  delay(100); 
}

//Vérifie si un tag est présent et l'affiche une seule fois jusqu'à ce qu'il soit retiré.

void readTag() {
  if (nfc.tagPresent()) {
    if (!tagDetected) { // Évite la relecture du même tag tant qu'il est en place
      NfcTag tag = nfc.read(); 
      String tagId = formatUID(tag.getUidString()); 

      // Affichage de l'UID du tag 
      Serial.print("Tag détecté : ");
      Serial.println(tagId);

      tagDetected = true; 
      lastTag = tagId; 
    }
  } else {
    tagDetected = false; 
  }
}

//Formatage de l'UID du tag [En majuscule et sans espace]

String formatUID(String rawUID) {
  String formattedUID = "";
  for (uint8_t i = 0; i < rawUID.length(); i++) {
    char c = rawUID.charAt(i);
    if (c != ' ') { 
      formattedUID += String(c);
    }
  }
  formattedUID.toUpperCase(); 
  return formattedUID;
}