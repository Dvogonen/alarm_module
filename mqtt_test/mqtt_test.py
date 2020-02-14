#  This code is intended to be used to manage the alarm logic component in the 
#  DAS - Distributed Alarm System
#
#   Copyright 2020 Kjell Kernen
#  
#  Permission is hereby granted, free of charge, to any person obtaining a copy 
#  of this software and associated documentation files (the "Software"), to deal
#  in the Software without restriction, including without limitation the rights 
#  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
#  copies of the Software, and to permit persons to whom the Software is 
#  furnished to do so, subject to the following conditions:
#  The above copyright notice and this permission notice shall be included in all 
#  copies or substantial portions of the Software.
#   
#  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
#  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
#  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
#  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
#  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
#  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
#  SOFTWARE.

import paho.mqtt.client as mqtt
import sys


def parse_result(result: tuple) -> str:
    if result[0] == 0:
        return "success"
    else:
        return "fail"


def my_subscribe(topic: str) -> bool:
    result = client.subscribe((topic))
    print('Subscribing to topic "' + topic + '" with result:', parse_result(result))
    return result == "success"


# The callback for when the client receives a CONNACK response from the server.
def on_connect(client, userdata, flags, rc):
    print("Connected with result: " + parse_result((rc, 0)))

    # Subscribing in on_connect() means that if we lose the connection and
    # reconnect then subscriptions will be renewed.
    # client.subscribe("$SYS/#")
    my_subscribe("$SYS/broker/version")
    my_subscribe("alarm/#")


class Message:
    def __init__(self):
        self.received = False
        self.topic = ""
        self.payload = ""


message = Message()


# The callback for when a PUBLISH message is received from the server.
def on_message(client, userdata, msg):
    message.received = True
    message.topic = msg.topic
    message.payload = msg.payload.decode('UTF-8')


client = mqtt.Client()
client.on_connect = on_connect
client.on_message = on_message

# MQTT servers
# client.connect("192.168.0.198", 1883, 60)     # Raspberry
# client.connect("broker.hivemq.com", 1883,60)  # HiveMQ cloud server
print("Connecting to MQTT broker at", sys.argv[1])
client.connect(sys.argv[1], 1883, 60)  # HiveMQ cloud server

# Blocking call that processes network traffic, dispatches callbacks and
# handles reconnecting.
# Other loop*() functions are available that give a threaded interface and a
# manual interface.

# Start by turning off any alarms and dearm the system
client.publish("alarm/armed", '0', 0, True)
armed = False
client.publish("alarm/entry_alarm", '0', 0, True)
entry_alarm = False
running = True

while (running):
    client.loop()
    if message.received:
        message.received = False

        print(message.topic + ":", message.payload)

        if message.topic == "alarm/armed":
            armed = (message.payload == "1")

        if message.topic == "alarm/entry_alarm":
            entry_alarm = (message.payload == "1")

        if message.topic == "alarm/button":
            armed = not armed
            if armed:
                client.publish("alarm/armed", '1', 0, True)
            else:
                client.publish("alarm/armed", '0', 0, True)
                if entry_alarm:
                    client.publish("alarm/entry_alarm", '0', 0, True)

        if message.topic == "alarm/pir":
            if armed and not entry_alarm:
                client.publish("alarm/entry_alarm", '1', 0, True)

        if message.topic == "alarm/stop":
            running = False

print("Shutting down")
