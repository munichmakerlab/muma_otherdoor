#include <ESP8266WiFi.h>
#include <PubSubClient.h>
//#include <MQTT.h>

#include "config.h"

WiFiClient wifiClient;
PubSubClient mqtt_client( wifiClient );

int buttonState;
int lastButtonState = LOW;
int reading;

long lastDebounceTime = 0;
long debounceDelay = 200;

int connectState = 0;

void callback(char* topic, byte* payload, unsigned int length) {

}

void setup() {
  pinMode(14, INPUT);
  lastButtonState = digitalRead(14);
  buttonState = digitalRead(14);
  Serial.begin(9600);
  Serial.println("");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WLAN ");
  while ( WiFi.status() != WL_CONNECTED ) {
    Serial.print(".");
    delay(500);
  }
  Serial.println(" connected");

  mqtt_client.setServer(mqtt_broker, 1883);
  mqtt_client.setCallback(callback);  

}

void checkConnect() {
  uint32_t beginWait = millis();
  Serial.println("MQTT reconnecting.");
  while (millis() - beginWait < 1000) {
    delay(100);
    if ( WiFi.status() == WL_CONNECTED ) {
      connectState = 1;

      mqtt_client.loop();
      mqtt_client.disconnect();
      mqtt_client.loop();
      delay(100);
      mqtt_client.connect("OtherDoorSensor",mqtt_user, mqtt_pass,MQTT_TOPIC,1,1,"?");
      mqtt_client.loop();
      delay(100);

      if ( mqtt_client.connected() ) {
        Serial.println("connected");

        sendState();

        return;
      }
    }
  }
  Serial.print(".");
}

void sendState() {
    
    if (connectState == 1) {
      Serial.println("Sending state.");
        if (buttonState == HIGH) {
            mqtt_client.publish(MQTT_TOPIC, "1", 1);
        } else {
            mqtt_client.publish(MQTT_TOPIC, "0", 1);
        }
    }
}

void loop() {
  reading = digitalRead(14);

  if (reading != lastButtonState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }

  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer
    // than the debounce delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      Serial.print("State changed to ");
      Serial.println(reading);
      buttonState = reading;
      sendState();
    }
  }

  if ( WiFi.status() != WL_CONNECTED || connectState == 0 || mqtt_client.connected() != 1 ) {
    connectState = 0;
    checkConnect();
  } else {
    mqtt_client.loop();
  }
  delay(10);
  lastButtonState = reading;
}
