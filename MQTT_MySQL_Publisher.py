import paho.mqtt.client as mqtt
import pymysql.cursors
from datetime import datetime
import sys
import json

#User variable for database name
dbName = "data_NodeMCU"

# User variables for MQTT Broker connection
mqttBroker = "broker.mqttdashboard.com"
mqttBrokerPort = 1883
mqttUser = ""
mqttPassword = ""

mysqlHost = ""
mysqlUser = "admin"
mysqlPassword = ""

# This callback function fires when the MQTT Broker conneciton is established.  At this point a connection to MySQL server will be attempted.
def on_connect(client, userdata, flags, rc):
    print("MQTT Client Connected")
    client.subscribe("node/data")
    try:
        db = pymysql.connect(host=mysqlHost, user=mysqlUser, password=mysqlPassword, db=dbName, charset='utf8mb4', cursorclass=pymysql.cursors.DictCursor)
        db.close()
        print("MySQL Client Connected\n")
    except:
        sys.exit("Connection to MySQL failed")

def log_telemetryDistance(db,payload):
    cursor = db.cursor()
    insertRequest = "insert into UltrasoundData(ID_House,ID_Device,DateDistStatus,Distance,devStatus) values (%s, %s, CURRENT_TIMESTAMP, %s, %s)"
    cursor.execute(insertRequest,(payload['casa'],payload['dispositivo'],payload['distancia'],payload['stats']))
    db.commit()
    print('data logged\n')
    db.close()
    
def log_telemetryTemperature(db,payload):
    cursor = db.cursor()
    insertRequest = "insert into TempData(ID_House,ID_Device,DateTempStatus,RoomTemperature,RH,devStatus) values (%s, %s, CURRENT_TIMESTAMP, %s, %s, %s)"
    cursor.execute(insertRequest,(payload['casa'],payload['dispositivo'],payload['temperatura'],payload['humedad'],payload['stats']))
    db.commit()
    print('data logged\n')
    db.close()

def log_telemetryPuerta(db,payload):
    cursor = db.cursor()
    insertRequest = "insert into DoorData(ID_House,ID_Device,DateDoorStatus,devStatus) values (%s, %s, CURRENT_TIMESTAMP, %s)"
    cursor.execute(insertRequest,(payload['casa'],payload['dispositivo'],payload['status']))
    db.commit()
    print('data logged\n')
    db.close()

def log_telemetrySonido(db,payload):
    cursor = db.cursor()
    insertRequest = "insert into SoundData(ID_House,ID_Device,DateSoundStatus,devStatus) values (%s, %s, CURRENT_TIMESTAMP, %s)"
    cursor.execute(insertRequest,(payload['casa'],payload['dispositivo'],payload['status']))
    db.commit()
    print('data logged\n')
    db.close()

# The callback for when a PUBLISH message is received from the MQTT Broker.
def on_message(client, userdata, msg):
    now = datetime.now()
    payload = json.loads((msg.payload).decode("utf-8"))

    if 'distancia' in payload:
        print("Transmission received from ultrasound ",payload['dispositivo']," at",now)
        db = pymysql.connect(host=mysqlHost, user=mysqlUser, password=mysqlPassword, db=dbName,charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor)
        log_telemetryDistance(db,payload)
        
    elif 'temperatura' in payload:
        print("Transmission received from DHT11 ",payload['dispositivo']," at",now)
        db = pymysql.connect(host=mysqlHost, user=mysqlUser, password=mysqlPassword, db=dbName,charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor)
        log_telemetryTemperature(db,payload)

    elif 'sensorType' in payload and payload['sensorType']=='Sonido':
        print("Transmission received from Sound Sensor ",payload['dispositivo']," at",now)
        db = pymysql.connect(host=mysqlHost, user=mysqlUser, password=mysqlPassword, db=dbName,charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor)
        log_telemetrySonido(db,payload)
    
    elif 'sensorType' in payload and payload['sensorType']=='Puerta':
        print("Transmission received from Door Sensor ",payload['dispositivo']," at",now)
        db = pymysql.connect(host=mysqlHost, user=mysqlUser, password=mysqlPassword, db=dbName,charset='utf8mb4',cursorclass=pymysql.cursors.DictCursor)
        log_telemetryPuerta(db,payload)

# Connect the MQTT Client
client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message
try:
    client.connect(mqttBroker, mqttBrokerPort, 60)
except:
    sys.exit("Connection to MQTT Broker failed")
# Stay connected to the MQTT Broker indefinitely
client.loop_forever()

# Retrieved from https://github.com/ncd-io/Mosquitto_Subscriber_MySQL_Publisher/blob/master/Mosquitto_Subscriber_MySQL_Publisher.py
