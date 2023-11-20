import paho.mqtt.client as mqtt
import requests
import json
import socket
import uuid 

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("Connected to MQTT Broker")

        client.subscribe("dht11")
    else:
        print(f"Connection failed with result code {rc}")

def on_message(client, userdata, msg):
    print(f"Received message on topic {msg.topic}: {msg.payload.decode()}")

    # Parse the received JSON payload
    payload_dict = json.loads(msg.payload.decode())
    
    # Extract temperature and humidity from the payload
    temperature = payload_dict["temperature"]
    humidity = payload_dict["humidity"]

    # Get the IP Address of the device
    IP_Address = get_ip_address()

    # Generate a unique id using UUID
    unique_id = str(uuid.uuid4())

    # Create a JSON payload for the HTTP POST request
    http_payload = {
        "id": unique_id,  # ใช้ unique_id แทน id ที่ซ้ำกัน
        "humidity": humidity,
        "temperature": temperature,
        "IP Address": IP_Address
    }


    try:
        response = requests.post("http://192.168.43.167:3000/Sensor", json=http_payload)
        if response.status_code == 200:
            print("Data sent to JSON Server successfully")
        else:
            print(f"Failed to send data to JSON Server. Status code: {response.status_code}")
            print(f"Response content: {response.content}")
    except Exception as e:
        print(f"Error sending HTTP POST request: {e}")

def get_ip_address():
    # Get the IP Address of the device
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
    s.connect(("8.8.8.8", 80))
    ip_address = s.getsockname()[0]
    s.close()
    return ip_address

# Create an MQTT client
client = mqtt.Client()

# Set up the callbacks
client.on_connect = on_connect
client.on_message = on_message

# Connect to the MQTT broker
client.connect("192.168.43.167", port=1883, keepalive=60)

# Loop to keep the script running
client.loop_forever()
