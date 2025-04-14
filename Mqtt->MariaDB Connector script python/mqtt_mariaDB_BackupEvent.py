import paho.mqtt.client as mqtt
import mysql.connector
import sys

# ------------------------------
# Parametres de connexion MariaDB
# ------------------------------
db_config = {
    "host": "localhost",
    "user": "root",
    "password": "**********", #Crypter
    "database": "rfid_evenements"
}

# ------------------------------
# Connexion MariaDB
# ------------------------------
def connect_db():
    try:
        conn = mysql.connector.connect(**db_config)
        return conn
    except mysql.connector.Error as e:
        print(f"Erreur de connexion    MariaDB : {e}")
        sys.exit(1)

# ------------------------------
# reception d'un message MQTT
# ------------------------------
def on_message(client, userdata, msg):
    try:
        payload = msg.payload.decode("utf-8")
        print(f"Message re  u : {payload}")

        conn = connect_db()
        cursor = conn.cursor()

        #  Insertion dans les champs 'topic' et 'contenu'
        cursor.execute(
            "INSERT INTO evenements (topic, contenu) VALUES (%s, %s)",
            (msg.topic, payload)
        )

        conn.commit()
        cursor.close()
        conn.close()

        print(" ^iv  nement enregistr   dans la base.")
    except Exception as e:
        print(f"Erreur lors de l'insertion : {e}")

# ------------------------------
# Configuration du client MQTT
# ------------------------------
mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message

mqtt_client.connect("localhost", 1883, 60)
mqtt_client.subscribe("rfid/detection")

# ------------------------------
# Boucle d'écoute continue
# ------------------------------
mqtt_client.loop_forever()




