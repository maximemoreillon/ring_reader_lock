/*
BCC=LEN XOR Status XOR DATA
*/

#define STX 0x02
#define ETX 0x03

#define PARA_RF_BIT 5

void send_packet(char payload[], int payload_length){
  // Function to send a command to the reader

  // Flush Serial in case there was a response halfway through
  swSer.flush();
 
  swSer.enableTx(true);
  for(int i=0; i<payload_length; i++){
    swSer.write(payload[i]);
  }
  swSer.enableTx(false);
}

byte format_para(byte address){
  byte settings = 0;
  byte address_truncated = address & 0x0f;
  return (settings << 4) | address_truncated;
}

void build_packet_and_send( byte cmd, char para[], byte para_len ){

  // BCC = LEN XOR CMD XOR DATA (set below)
  
  byte len = sizeof(cmd) + para_len;
  byte packet_size = len + 4; // adding STX, ETX, BCC and LEN
  
  char packet[packet_size];
  
  packet[0] = STX;
  packet[1] = len;
  packet[2] = cmd;

  packet[sizeof(packet)-2] = packet[1] ^ packet[2];


  for(int i=0; i<para_len; i++){
    packet[i+3] = para[i];
    //Serial.print(packet[i+3], HEX);
    packet[sizeof(packet)-2] = packet[sizeof(packet)-2] ^ packet[i+3];
  }

  //packet[sizeof(packet)-2] = packet[1] ^ packet[2] ^ packet[3]; // BCC
  //packet[sizeof(packet)-2] = 0; // BCC
  packet[sizeof(packet)-1] = ETX;

  send_packet(packet, sizeof(packet));
}

void read_word(byte address){
  // Read a word at a specific address
  // 0x02+0x02+0x11+PARA(1byte) +BCC+0x03
  // PARA contains the address to read in the last 4 bits
  
  byte cmd = 0x11; // 0x11 is the command for reading
  char para[1];
  
  para[0] = format_para(address);

  build_packet_and_send(cmd, para, sizeof(para) );

}

Response get_reader_response_sync() {
  // This is a synchronous function, i.e. it waits for the response to the sent command
  
  // Todo: Share code with above
  // Todo: Find way to also return status code
  // Todo: Use callback for the actual command

  bool finished = false;
  
  boolean response_started = false;
  int response_byte_index = 0;
  
  int response_length = 0;

  Response reader_response;
    
  while(!finished){    
  
    // if no byte available, do nothing
    if (swSer.available()){
      // The current byte is stored as 'res'
      byte received_byte = (byte) swSer.read();

      // A full response starts with 0x02
      // Warning: DATA may also contain a 0x02
      if(!response_started && received_byte == 0x02){
    
        // Acknowledge start
        response_started = true;
    
        // Reset index
        response_byte_index = 0;
        
        // Reset response length
        reader_response.length = 0;
        //reader_response.data = 0;
        reader_response.status = 0;
      }
    
      if(response_started){

        // STX(0x02)+LEN(1byte)+Status(1byte)+DATA(nbytes)+BCC(1byte)+ETX(0x03)
    
        // Second byte (index 1) sets the response length
        if(response_byte_index == 1){
          response_length = received_byte;
          reader_response.length = received_byte;
        }
    
        // 3rd byte sets the status
        if(response_byte_index == 2){
          
          /*
          0x00 --- success
          0x01 --- failure (Happens when no tag is presented)
          0x02 --- data error
          0x03 --- command error
          0x04 --- parameter errror
          */
    
          reader_response.status = received_byte;
          //Serial.println(received_byte);
          
        }
    
        // Dealing with the data
        // Some responses don't have data so need to check with length byte
        // This implies that the response length is greater than 1 (Why?)
        // Data will be stored in bytes starting from index 3
//        if(response_length > 1 && response_byte_index > response_length +2 -1 && response_byte_index < response_length +4 -1){
//          reader_response.data[0] = received_byte;
//        }

        // Dealing with the data
        // Len = length of status (1) + length of data
        // Check if there is data to read
        if(response_length > 1){
          byte data_start_index = 3;
          byte data_length = response_length -1; // because status is included
          byte data_end_index = data_start_index + data_length;
          byte data_index = response_byte_index - 3; // because STX, LEN and STATUS before
          
          if(response_byte_index >= data_start_index && response_byte_index < data_end_index){
            reader_response.data[data_index] = received_byte;
            
          }
        }
    
        // finding final byte
        // Adding STX LEN BCC and ETX
        
        if(response_length && response_byte_index == response_length +4-1 ){
    
          // Final byte is 0x03
          if(received_byte == 0x03){
            //Serial.println("END");
          }
    
          // if reaching response_length but byte is not 0x03, then we have a problem
          else {
            // Serial.println("Should finish but not finished");
          }
      
          // Mark response as not started anymore
          response_started = false;
    
          // clear the serial buffer
          // Maybe not ideal or necessary
          swSer.flush();
  
          finished = true;

          
          
        }
    
        response_byte_index ++;
      }
    }
  }
  
  return reader_response;
}

Response read_word_sync( byte address ){
  read_word(address);
  delay(5);
  return get_reader_response_sync();
}

void print_response(Response reader_response){
  
  if(reader_response.status == 1) {
    //Serial.print("\t Status: ");
    //Serial.print(reader_response.status);
    return;
  }
  else {
    Serial.print("Status: ");
    Serial.print(reader_response.status);
    
    Serial.print("\t Length: ");
    Serial.print(reader_response.length);

    Serial.print("\t Data: ");
    for(int i=0; i<reader_response.length-1; i++){
      byte reader_data = reader_response.data[i];
      Serial.print(reader_data < 16 ? "0" : "");
      Serial.print(reader_data,HEX);
      Serial.print(" ");
    }
  
    //Serial.print("\t data: ");
    //Serial.print(reader_response.data[0],HEX);
    Serial.println("");
    
  }
}
