import paho.mqtt.client as mqtt
import time
from paho.mqtt.client import MQTTProtocolVersion, CallbackAPIVersion

def onconnect(client, userdata, flags, reason_code, properties):
    print("Client has ", reason_code);


client_id = "publisher"
client_id2 = "publisher2"

client = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=client_id, protocol=MQTTProtocolVersion.MQTTv5, transport="tcp")
client2 = mqtt.Client(callback_api_version=CallbackAPIVersion.VERSION2, client_id=client_id2, protocol=MQTTProtocolVersion.MQTTv5, transport="tcp")

client.username_pw_set("robot_77_1","unUfGach)")
client2.username_pw_set("robot_78_1","wagvochhol")

client.connect("mqtt.ics.ele.tue.nl", 1883, 60) # Address of the tue server
client.on_connect = onconnect

client2.connect("mqtt.ics.ele.tue.nl", 1883, 60) # Address of the tue server
client2.on_connect = onconnect


client.publish(topic="/pynqbridge/77/send",payload="Hello",qos = 2,retain= False)
time.sleep(5)
client2.publish(topic="/pynqbridge/78/send",payload="5",qos = 2,retain= False)

client.loop_start()
client2.loop_start()

try:
    while True:
        time.sleep(0.1)  # Keep the main thread alive
except KeyboardInterrupt:
    print("Comms ended")
finally:
    client.loop_stop()
    client.disconnect()
    client2.loop_stop()
    client2.disconnect()
    print("Clients disconnected")


