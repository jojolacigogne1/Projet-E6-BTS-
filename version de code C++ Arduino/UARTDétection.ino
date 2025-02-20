#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <PN532.h>

// Déclaration du port ou est brancher le capteur sur l'arduino 
SoftwareSerial SWSerial(6, 7); // RX = 6, TX = 7

// Déclaration du capteur RFID en UART
PN532_SWHSU pn532swhsu(SWSerial);
PN532 nfc(pn532swhsu);

// Variable pour stocker le dernier tag détecté
String lastTag = "";
bool tagDetected = false; // Empêche la détection multiple du même tag tant qu'il reste présent

void setup() {
  Serial.begin(115200); //Reglage du Baudrate
  Serial.println("Initialisation du capteur RFID...");


  SWSerial.begin(115200);
  nfc.begin();

  // Vérification si le capteur est bien détecté
  if (!nfc.getFirmwareVersion()) {
    Serial.println("Erreur : Capteur RFID (UART) non détecté !");
    while (1); // Bloque l'exécution si le capteur est absent
  }

  //  Activation de la lecture passive
  nfc.SAMConfig();
  Serial.println("Capteur RFID (UART) prêt.");
}

void loop() {
  readTag(); 
  delay(100); 
}
// Vérifie si un tag est présent et l'affiche une seule fois jusqu'à ce qu'il soit retiré.

void readTag() {
  uint8_t uid[7]; 
  uint8_t uidLength; 


  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength)) {
    if (!tagDetected) { // Évite la relecture du même tag tant qu'il est en place
      String tagId = "";
      
      //Formatage de l'UID du tag [En majuscule et sans espace]
      for (uint8_t i = 0; i < uidLength; i++) {
        char hexChar[3];
        sprintf(hexChar, "%02X", uid[i]); 
      }

      // Affichage de l'UID du tag détecté
      Serial.print("Tag détecté : ");
      Serial.println(tagId);

      tagDetected = true; 
      lastTag = tagId; 
    }
  } else {
    tagDetected = false; // Réinitialise l'état lorsque le tag est retiré
  }
}