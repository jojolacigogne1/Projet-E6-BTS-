import paho.mqtt.client as mqtt
import mysql.connector
import sys
import time

# ------------------------------
# Connexion MariaDB
# ------------------------------
db_config = {
    "host": "localhost",
    "user": "root",
    "password": "***********", #Crypter
    "database": "rfid_evenements"
}

def connect_db():
    try:
        conn = mysql.connector.connect(**db_config)
        return conn
    except mysql.connector.Error as e:
        print(f"[ERREUR DB] : {e}")
        sys.exit(1)

# ------------------------------
# Suivi des derniers passages
# ------------------------------
last_seen = {}  
timeout = 10    

# ------------------------------
# Enregistrement dans la bonne table
# ------------------------------
def enregistrer_evenement(tag_id, type_event, antenne_1, antenne_2):
    conn = connect_db()
    cursor = conn.cursor()

    if type_event == "entree":
        cursor.execute(
            "INSERT INTO entrees (tag_id, antenne_arrivee, antenne_depart) VALUES (%s, %s, %s)",
            (tag_id, antenne_2, antenne_1)
        )
    elif type_event == "sortie":
        cursor.execute(
            "INSERT INTO sorties (tag_id, antenne_depart, antenne_arrivee) VALUES (%s, %s, %s)",
            (tag_id, antenne_1, antenne_2)
        )

    conn.commit()
    cursor.close()
    conn.close()
    print(f"[ ^|^e {type_event.upper()}] {tag_id} de {antenne_1} vers {antenne_2}")

# ------------------------------
# Traitement des messages
# ------------------------------
def on_message(client, userdata, msg):
    payload = msg.payload.decode("utf-8")
    print(f"[MQTT] Message re  u : {payload}")

    try:
        if "Tag RFID d  tect   par l'antenne" in payload:
            antenne = "A" if "antenne A" in payload else "B"
            tag_id = payload.split(":")[-1].strip()
            now = time.time()

            if tag_id in last_seen:
                prev_antenne, prev_time = last_seen[tag_id]

                if antenne != prev_antenne and (now - prev_time) <= timeout:
                
                    if prev_antenne == "A" and antenne == "B":
                        enregistrer_evenement(tag_id, "sortie", "A", "B")
                    elif prev_antenne == "B" and antenne == "A":
                        enregistrer_evenement(tag_id, "entree", "B", "A")
                    else:
                        print(f"[ ^z   ^o] Mouvement non reconnu")

      
            last_seen[tag_id] = (antenne, now)

    except Exception as e:
        print(f"[ERREUR Traitement] {e}")

# ------------------------------
# Configuration du client MQTT
# ------------------------------
mqtt_client = mqtt.Client()
mqtt_client.on_message = on_message

mqtt_client.connect("localhost", 1883, 60)
mqtt_client.subscribe("rfid/detection")

print("[ ^=^t^d] En   coute sur rfid/detection...")
mqtt_client.loop_forever()