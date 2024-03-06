""" MQTTPythonLoger.py - Mosquitto <=> MariaDB"""
import paho.mqtt.client as mqtt
import mariadb
import sys
import time

MQTT_HOST = '127.0.0.1'
MQTT_PORT = 1883
MQTT_CLIENT_ID = 'LogerPython'

STOREDATA_SENSOR={}

# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    for sensor in STOREDATA_SENSOR:
        client.subscribe(sensor)

# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    insert_to_database( msg.topic, str(msg.payload.decode("utf-8")) )  
    
def connect_database():
    attempts = 0
    while attempts < 3:
        try:
            conn = mariadb.connect(
                user="PythonDataLoger",
                password="password1",
                host="127.0.0.1",
                port=3306,
                database="meshNetwork"
                )
            return conn
        except mariadb.Error as e:
            attempts += 1
            print(f"Error connecting to MariaDB Platform: {e}")
            time.sleep(1)
            if attempts >= 2:
                sys.exit(1)


def get_sensor_to_monitoring():
    conn = connect_database()
    cur = conn.cursor()
    cur.execute("SELECT SensorId, TopicName, StoreData FROM Sensors")
    for SensorId, TopicName, StoreData in cur:
      if StoreData == True:
            STOREDATA_SENSOR[TopicName] = SensorId
    conn.close()   
                
def insert_to_database(topic, message):
    conn = connect_database()
    cur = conn.cursor()
    now = time.strftime('%Y-%m-%d %H:%M:%S')
    sensor_id =  STOREDATA_SENSOR[topic]
    msg = float(message)
    sql = "INSERT INTO Measurements (SensorId, Value, DataTime) VALUES (%s, %s, %s)"
    val = (sensor_id, msg, now)
    cur.execute(sql, val)
    conn.commit()
    conn.close()

def main():                
    get_sensor_to_monitoring()
    client = mqtt.Client(client_id=MQTT_CLIENT_ID)
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_HOST, MQTT_PORT)
    client.loop_forever()
    
if __name__ == "__main__":
    main()
