#include <RH_ASK.h>
RH_ASK driver;
void recieveRadioMessage(){
 uint8_t buf[9];
  uint8_t buflen = sizeof(buf);
    if (driver.recv(buf, &buflen)) // Non-blocking
    {
      int i;
      // Message with a good checksum received, dump it.
      Serial.print("Message: ");
      Serial.println((char*)buf);         
      digitalWrite(10,HIGH);
      delay(6000);
    }else{
      digitalWrite(10,LOW);
    }
}
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("board started!");
  pinMode(10,OUTPUT);
    // Debugging only
  if (!driver.init())
    Serial.println("init failed");
    
}

void loop() {
  recieveRadioMessage();
}
