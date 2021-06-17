#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include "credentials.h"
#include <PubSubClient.h>
#include <ArduinoJson.h>

#include "credentials.h"; // Wifi + MQTT credentials
#include "iot_config_home.h"; // MQTT broker URL + connection config

#define HOSTNAME "ringreader5"

#define LED_PIN D4
#define RX_PIN D1
#define TX_PIN D2

#define CODE_ADDRESS 0x0b

#define MQTT_LOCK_COMMAND_TOPIC "lock/command"
#define MQTT_RETAIN true

#define COOLDOWN_DURATION 3000



WiFiClient wifi_client;
PubSubClient MQTT_client(wifi_client); 
SoftwareSerial swSer;

boolean wifi_connected = false;
long wifi_disconnected_time = 0;

char code[4] = {0x11, 0x12, 0x19, 0x89};

long cooldown_start_time = -COOLDOWN_DURATION;

typedef struct Response {
  byte status;
  byte length;
  char data[4]; // needs a length to be set
} Response;

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




boolean compare_code( char code_1[], char code_2[] ){
  boolean match = true;
  for(int i=0; i<4; i++){
    if(code_1[i] != code_2[i]) {
      match = false;
    }
  }
  return match;
}


void loop() {

  wifi_connection_manager();
  MQTT_connection_manager();
  MQTT_client.loop();

  read_word(CODE_ADDRESS);
  Response reader_read_response = get_reader_response_sync();
  //print_response(reader_read_response);
  boolean match = compare_code(code, reader_read_response.data);

  if(match) {
    if(millis() - cooldown_start_time > COOLDOWN_DURATION){
      cooldown_start_time = millis();
      MQTT_publish_toggle();
      flash_led();
    }
  }

  
  

}
