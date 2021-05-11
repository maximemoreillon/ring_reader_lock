#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include "credentials.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "credentials.h"; // Wifi + MQTT credentials
#include "iot_config_home.h"; // MQTT broker URL + connection config

#define HOSTNAME "ringreader"

#define LED_PIN D4
#define RX_PIN D1
#define TX_PIN D2

#define MQTT_LOCK_COMMAND_TOPIC "lock/command"
#define MQTT_RETAIN true


WiFiClient wifi_client;
PubSubClient MQTT_client(wifi_client); 
SoftwareSerial swSer;

boolean wifi_connected = false;
long wifi_disconnected_time = 0;

// RFID related stuff
byte current_byte = 0;
int response_byte_index = 0;
boolean response_started = false;
int response_length = 0;
long last_read = 0;
byte ring_content[] = {0,0,0,0,0,0,0,0};
byte correct_combination[] = {0xD0, 0xDB, 0x05, 0x05, 0x05, 0x05, 0x05, 0x05};

void setup() {
  // put your setup code here, to run once:

  Serial.begin(115200);
  Serial.println("");
  Serial.println("");
  Serial.println("Start");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN,HIGH);
  
  swSer.begin(9600, SWSERIAL_8N1, RX_PIN, TX_PIN, false, 256);

  wifi_setup();
  MQTT_setup();

}





void loop() {

  wifi_connection_manager();
  MQTT_connection_manager();
  MQTT_client.loop();

  response_passthrough();
  
  long now = millis();
  if(now - last_read > 40){
    last_read = now;
    read_word(current_byte + 0x03);
  }
  

}
