import ssl
import sys
import time
import paho.mqtt.client as mqtt

# --- Configuration ---
BROKER_HOST = "1ea6ac40907c404eb1eb3edfd51eedce.s1.eu.hivemq.cloud"
BROKER_PORT = 8883
MQTT_USER = "rikilamp"
MQTT_PASS = "ChakriLovesNikki2903"

TOPIC_SUBSCRIBE = "riki/v1/+/light"

def on_connect(client, userdata, flags, rc):
    if rc == 0:
        print("[CONNECTED] Successfully connected to HiveMQ Broker.")
        client.subscribe(TOPIC_SUBSCRIBE)
        print(f"[SUBSCRIBED] Listening for updates on: {TOPIC_SUBSCRIBE}")
    else:
        print(f"[ERROR] Connection failed with response code: {rc}")

def on_message(client, userdata, msg):
    # Skip if message is already marked as retained
    if msg.retain:
        return

    try:
        topic = msg.topic
        payload = msg.payload
        print(f"[INTERCEPT] Received live update on '{topic}': {payload.decode('utf-8')}")

        # Republish the exact same message to the same topic with retain=True
        result = client.publish(topic, payload, qos=1, retain=True)
        if result.rc == mqtt.MQTT_ERR_SUCCESS:
            print(f"[RETAINED] Successfully converted to retained message on '{topic}'")
        else:
            print(f"[ERROR] Failed to publish retained message, code: {result.rc}")
    except Exception as e:
        print(f"[EXCEPTION] Failed to process message: {e}")

def main():
    print("Starting RiKi Lamp State Retainer Daemon...")

    # Paho MQTT Client setup
    client = mqtt.Client(client_id="riki-retainer-daemon", clean_session=True)
    client.username_pw_set(MQTT_USER, MQTT_PASS)

    # SSL configuration for HiveMQ Cloud (TLS port 8883)
    context = ssl.create_default_context()
    client.tls_set_context(context)

    client.on_connect = on_connect
    client.on_message = on_message

    # Automatic reconnect logic
    while True:
        try:
            print(f"Connecting to broker {BROKER_HOST}:{BROKER_PORT}...")
            client.connect(BROKER_HOST, BROKER_PORT, keepalive=60)
            break
        except Exception as e:
            print(f"Connection failed: {e}. Retrying in 5 seconds...")
            time.sleep(5)

    try:
        # Start infinite loop
        client.loop_forever()
    except KeyboardInterrupt:
        print("\nStopping daemon...")
        client.disconnect()
        print("Daemon stopped.")

if __name__ == "__main__":
    main()
