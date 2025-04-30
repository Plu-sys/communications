import paho.mqtt.client as mqtt
from paho.mqtt.client import MQTTProtocolVersion, CallbackAPIVersion
import time

def messageHandler(client, userdata, message):
    print(f'Recieved message on {message.topic}: {message.payload}')

def onconnect(client, userdata, flags, reason_code, properties):
    print("Client connection status: ", reason_code);

client_id = "reciever77"
client_id2 = "reciever78"
client1 = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=client_id, protocol=MQTTProtocolVersion.MQTTv5, transport="tcp")
client2 = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=client_id2, protocol=MQTTProtocolVersion.MQTTv5, transport="tcp")

client1.username_pw_set("robot_77_2","umbagDavgo")
client2.username_pw_set("robot_78_2","tomCipveu")

client1.connect("mqtt.ics.ele.tue.nl", 1883, 60) # Address of the tue server
client2.connect("mqtt.ics.ele.tue.nl", 1883, 60) # Address of the tue server

client1.on_connect = onconnect
client1.on_message = messageHandler

client2.on_connect = onconnect
client2.on_message = messageHandler

client1.subscribe("/pynqbridge/77/#",2)
client2.subscribe("/pynqbridge/78/#",2)

client1.loop_start()
client2.loop_start()

try:
    while True:
        time.sleep(0.1)  # Keep the main thread alive
except KeyboardInterrupt:
    print("Comms ended")
finally:
    client1.loop_stop()
    client1.disconnect()
    client2.loop_stop()
    client2.disconnect()
    print("Clients disconnected")
    
