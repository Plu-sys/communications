import paho.mqtt.client as mqtt
from paho.mqtt.client import MQTTProtocolVersion, CallbackAPIVersion
def messageHandler(client, userdata, message):
    print(f'Recieved message on {message.topic}: {message.payload}')

def onconnect(client, userdata, flags, reason_code, properties):
    print("Client connection status: ", reason_code);

client_id = "reciever"

client = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=client_id, protocol=MQTTProtocolVersion.MQTTv5, transport="tcp")

client.username_pw_set("robot_77_2","umbagDavgo")
client.connect("mqtt.ics.ele.tue.nl", 1883, 60) # Address of the tue server
client.on_connect = onconnect
client.on_message = messageHandler

client.subscribe("/pynqbridge/77/#",2)

try:
    client.loop_forever()   
except:
    print("Comms ended")