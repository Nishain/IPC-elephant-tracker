
#include <SoftwareSerial.h>
#include <RH_ASK.h>
#include <SPI.h> // Not actually used but needed to compile
#include <Servo.h>
#include <TimerFreeTone.h>
  
  
  Servo servo;
  Servo servo2; 
  RH_ASK driver;
  
  
  #define TONE_PIN 11
  
  int ultraSoundTrigger = 7;
  int ultraSoundEcho = 6;
  int pausePin = 5;
  int elephantAlarmPin = 8;
  boolean direction = false;
  int i = 0;
  int widthAngle = 0;
  int elapsedTime = 0;
  boolean shouldPause = true;
  boolean shouldCollectNormalData = true;
  float normalData[180];
  int previousState = LOW;
  float minDistance = 50.0;
  unsigned long time;
  int suspectStage = 0;
  int angleChange = 0;
  double lastDistance = 0;
  int faultTolerance = 0;
  void elevate(int angle){
    Serial.println("elevating");
    servo2.write(angle);
    delay(angle * 15);
  }
  void rollBackToMiddle(int angle){
    int newAngle = 0; 
    
    if(direction)
        newAngle = i-angle/2;    
      else
        newAngle = i+angle/2;
     i=newAngle;   
     Serial.println("bouncing to middle");
     servo.write(i);   
     delay(angle*15);
}

float readEchoDistance(){
  digitalWrite(ultraSoundTrigger,LOW);
  delay(2);
  digitalWrite(ultraSoundTrigger,HIGH);
  delay(10);
  digitalWrite(ultraSoundTrigger,LOW);
  return pulseIn(ultraSoundEcho,HIGH);
}

void backAndForth(){
  Serial.println("running");
  direction = !direction;
  int angle = 0;
  if(direction)
    angle = 45;
  else
    angle = 135;
  servo.write(angle);
  delay(15*angle);
}
void setup() {
  Serial.begin(9600);
  if (!driver.init())
         Serial.println("init failed");
  pinMode(ultraSoundTrigger,OUTPUT);
  pinMode(ultraSoundEcho,INPUT);
  //pinMode(pausePin,INPUT);
  pinMode(elephantAlarmPin,OUTPUT);
  
  servo.attach(5);
  //servo2.attach(11);
 
  //digitalWrite(5,HIGH);
  servo.write(90);
  delay(1000);
  servo.write(0);
  delay(1000);
}
void sendRadioMessage(){
     const char *msg = "detected!";
    for(int i=0;i<40;i++){ 
      driver.send((uint8_t *)msg, strlen(msg));
      driver.waitPacketSent();
      Serial.println("radio mesg sent!");
    }
}
void handlePauseState(){
  
//  int pausePinState = digitalRead(pausePin);
//  if (pausePinState == HIGH && previousState ==  LOW){
//      shouldPause = !shouldPause;
//      previousState = shouldPause;
//      if(shouldPause)
//       Serial.println("paused");
//  }
//  previousState = pausePinState;
    shouldPause = (digitalRead(pausePin) == LOW);
}

void blink(){
  int flashDelay = 50;
  digitalWrite(elephantAlarmPin,HIGH);
      delay(flashDelay);
      digitalWrite(elephantAlarmPin,LOW);
      delay(flashDelay);
      digitalWrite(elephantAlarmPin,HIGH);
      delay(flashDelay);
      digitalWrite(elephantAlarmPin,LOW);
}
boolean finishWidthEstimation(double distance){
  double radian = (((double) angleChange) / 180.0) * 3.14159267;
  double width = sin(radian) * (distance);
  if(width < 0)
    width *= -1;
  Serial.print("width detected ");
    Serial.println(width);
  if(width > 10.0){
    suspectStage = 2; 
    rollBackToMiddle(angleChange);
    return true;
  }
  return false;
}
int getRandom(int lower, int upper){
 return (rand() % (upper - lower + 1)) + lower;
}
void beeSound(){
  int adder = 0;
  int f;
  for(int j=0;j<50;j++){
    for(int i=0;i<getRandom(3,6);i++){    
      int adder = getRandom(0,15);
      //tone(11,getRandom(300+adder,330+adder),1);
      f = getRandom(300+adder,330+adder);
      TimerFreeTone(3,f,getRandom(10,15)*getRandom(1,5));
      //delay(getRandom(10,15)*getRandom(1,5));
    }
    TimerFreeTone(3,f,getRandom(10,15)*getRandom(1,5));
  }
}
void loop() {
  time = millis();
 
  float duration = readEchoDistance();
  double distance = duration * 0.034 / 2;
  if(i==0 || i==180){
    if(suspectStage == 1){
      if(finishWidthEstimation(distance))
        return;
    }
    direction = !direction;
  }
  
  if(shouldCollectNormalData && i==180){
    shouldCollectNormalData = false;
    Serial.println("data collection completed!");
  }
  Serial.print("normal data -- ");
  Serial.println(normalData[i==180 ? 179 : i]);
  if(shouldCollectNormalData){
    normalData[i] = distance;
  }
  else if(distance < minDistance && distance < (normalData[i==180 ? 179 : i]-4) && distance!=0 ){
    if(suspectStage == 0){
      Serial.println("suspected stage one......");
      angleChange = 0;
      suspectStage = 1;
      blink();
    }  
    else if(suspectStage == 1){
      faultTolerance = 0;
      lastDistance = distance;
    }
    else if(suspectStage == 2){
      faultTolerance = 0;
      Serial.println("suspected stage two......");
      elapsedTime += millis() - time;
      if(elapsedTime > 6000){
        digitalWrite(elephantAlarmPin,HIGH);
        sendRadioMessage();
        faultTolerance = 100;
        beeSound();
      }
      return; 
    }
  }
  else{
    bool ignore = false;
    if(suspectStage == 1){
      if(finishWidthEstimation(lastDistance))
        return;
      else if (faultTolerance < 7){
        ignore = true;
        ++faultTolerance;
      }
    }else if(suspectStage == 2){
      if (faultTolerance < 4){
        ++faultTolerance;
        return;
      }
    }
    if(!ignore){
      angleChange = 0;
      suspectStage = 0;
      elapsedTime = 0; 
      digitalWrite(elephantAlarmPin,LOW);
    }
  }
 if(direction)
    i++;
  else
    i--;
  if(suspectStage == 1)
    angleChange++;
  servo.write(i);
  Serial.println(distance);
}


