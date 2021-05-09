void flash_led(){
  digitalWrite(LED_PIN,LOW);
  delay(50);
  digitalWrite(LED_PIN,HIGH);
  delay(50);
  digitalWrite(LED_PIN,LOW);
  delay(50);
  digitalWrite(LED_PIN,HIGH);
  delay(50);
  digitalWrite(LED_PIN,LOW);
  delay(700);
  digitalWrite(LED_PIN,HIGH);
}
