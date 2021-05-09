void send_on(){
  // UNUSED
  
  byte len = 0x02;
  byte cmd = 0x80;
  byte para = 0x01;
  byte bcc = len ^ cmd ^ para;
  
  char payload[] = { 0x02,len,cmd, para, bcc, 0x03 };

  send_command(payload, sizeof(payload));
}

void read_word(byte address){

  byte cmd = 0x11;

  char payload[] = { 0x02, 0, cmd, address, 0, 0x03 };

  // LEN
  payload[1] = sizeof(payload) -4;

  // BCC
  payload[sizeof(payload)-2] = payload[1] ^ payload[2] ^ payload[3];

  send_command(payload, sizeof(payload));
}


void send_command(char payload[], int payload_length){
  
  swSer.enableTx(true);
  for(int i=0; i<payload_length; i++){
    swSer.write(payload[i]);
  }
  swSer.enableTx(false);
}

void response_passthrough(){
  
  if (swSer.available()) {

    byte res = (byte) swSer.read();

    if(!response_started && res == 0x02){

      // Reset index
      response_byte_index = 0;
      
      // Reset response length
      response_length = 0;

      // Acknowledge start
      response_started = true;
      
      //Serial.println("Response start");
    }

    if(response_started){

      // First byte sets the response length
      if(response_byte_index == 1){
        
        response_length = res;
        //Serial.print("Length: ");
        //Serial.println(response_length);
      }

      // Status
      if(response_byte_index == 2){
        //Serial.print("Status: ");
        //Serial.println(res);
      }

      if(response_length > 1 && response_byte_index > response_length +2 -1 && response_byte_index < response_length +4 -1){

        /*
        Serial.print("Word: ");
        Serial.print(current_byte,HEX);
        Serial.print(", data: ");
        Serial.println(res, HEX);
        */
        
        ring_content[current_byte] = res;

        current_byte ++;
        if(current_byte > sizeof(ring_content) ) {
          boolean correct = true;
          for(int i=0; i<sizeof(ring_content); i++){
            
            Serial.print(ring_content[i],HEX);
            Serial.print(" ");

            if(ring_content[i] != correct_combination[i]) correct = false;

            // Reset byte
            ring_content[i] = 0;
          }
          Serial.println("");
          current_byte=0x00;

          if(correct){
            
            flash_led();
            
            MQTT_client.publish(MQTT_LOCK_COMMAND_TOPIC, "TOGGLE", MQTT_RETAIN);
            
          }
        }
        
      }

      // finding final byte
      // Adding STX LEN BCC and ETX
      if(response_length > 0 && response_byte_index == response_length +4-1 ){

        if(res == 0x03){
          
          //Serial.println("END");
          
        }
        else {
          //Serial.println("Should finish but not finished");
        }

        swSer.flush();
        response_started = false;
        
      }

      response_byte_index ++;
    }
    
    
  }
}
