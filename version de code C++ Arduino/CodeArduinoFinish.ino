#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <SoftwareSerial.h>
#include <PN532_SWHSU.h>
#include <Ethernet.h>
#include <PubSubClient.h>

// Déclaration pour le capteur A (I2C)
PN532_I2C pn532_i2c(Wire);
PN532 nfcA(pn532_i2c);

// Déclaration pour le capteur B (UART)
SoftwareSerial SWSerialB(6, 7); // RX = 6, TX = 7
PN532_SWHSU pn532swhsuB(SWSerialB);
PN532 nfcB(pn532swhsuB);


byte mac[] = { 0xA8, 0x61, 0x0A, 0xAE, 0x0F, 0x1B};
// Configuration Ethernet LDE
//IPAddress ip(192,168,121,45);
//IPAddress mqtt_server(192,168,121,44);

// Conf Arduino UIMM & Maison 
IPAddress ip(192,168,253,45);
IPAddress mqtt_server(192,168,253,44);
EthernetClient ethClient;
PubSubClient client(ethClient);

void setup() {
  Serial.begin(115200);
  Serial.println("Initialisation des capteurs RFID...");

  Ethernet.begin(mac, ip);
  Serial.print("Adresse IP Arduino : ");
  Serial.println(Ethernet.localIP());

  client.setServer(mqtt_server, 1883);
  connectMQTT();

  // Initialisation du capteur A (I2C)
  nfcA.begin();
  if (!nfcA.getFirmwareVersion()) {
    Serial.println("Capteur A (I2C) non détecté !");
  } else {
    nfcA.SAMConfig();
    Serial.println("Capteur A (I2C) prêt.");
  }

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
  if (!client.connected()) {
    connectMQTT();
  }
  client.loop();

  // Lire les tags en boucle sans restriction
  readTag(nfcA, "A");
  readTag(nfcB, "B");
}

void connectMQTT() {
  Serial.print("Connexion au broker MQTT...");
  while (!client.connected()) {
    if (client.connect("ArduinoTestClient")) {
      Serial.println("Connecté au broker MQTT !");
    } else {
      Serial.print(".");
      delay(1000);
    }
  }
}

void readTag(PN532 &nfc, String antenna) {
  uint8_t uid[7];
  uint8_t uidLength;

  if (nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, uid, &uidLength, 200)) { // Timeout augmenté à 200ms
    String tagId = formatUID(uid, uidLength);

    // Envoi immédiat du tag via MQTT
    String message = "Tag RFID détecté par l'antenne " + antenna + " : " + tagId;
    Serial.println(message);
    sendMQTTMessage(message);

    // Forcer la réinitialisation du lecteur pour garantir une nouvelle détection
    nfc.begin();
    nfc.SAMConfig();

    // Pause pour éviter une boucle infinie avec le même tag
    delay(200);
  }
}

void sendMQTTMessage(String message) {
  Serial.print("Envoi MQTT : ");
  Serial.println(message);
  client.publish("rfid/detection", message.c_str());
}

String formatUID(uint8_t *uid, uint8_t uidLength) {
  String tagId = "";
  for (uint8_t i = 0; i < uidLength; i++) {
    char hexChar[3];
    sprintf(hexChar, "%02X", uid[i]);
    tagId += hexChar;
  }
  return tagId;
}