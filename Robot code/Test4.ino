#include<Servo.h>
Servo myservo_X;
Servo myservo_Y;
int Servopin_X = 11, Servopin_Y = 5;
int Motor11=9,Motor12=10; /*Right Motor*/
int EnMotor1=3;
int Motor21=A1,Motor22=A2; /*Left Motor*/
int EnMotor2=A0;
int Value=200;
int angle = 0;
String Read_data = "";

void setup(){
  Serial.begin(9600);
  pinMode(Motor11,OUTPUT);
  pinMode(Motor12,OUTPUT);
  pinMode(EnMotor1,OUTPUT);
  pinMode(Motor21,OUTPUT);
  pinMode(Motor22,OUTPUT);
  pinMode(EnMotor2,OUTPUT);
  myservo_X.attach(Servopin_X);
  myservo_Y.attach(Servopin_Y);
}

void GoForward(int t1){
  analogWrite(EnMotor1, Value);
  digitalWrite(Motor11, HIGH);
  digitalWrite(Motor12, LOW);
  analogWrite(EnMotor2, Value);
  digitalWrite(Motor21, HIGH);
  digitalWrite(Motor22, LOW);
  delay(t1);
}

void GoBackward(int t2){
  analogWrite(EnMotor1, Value);
  digitalWrite(Motor11, LOW);
  digitalWrite(Motor12, HIGH);
  analogWrite(EnMotor2, Value);
  digitalWrite(Motor21, LOW);
  digitalWrite(Motor22, HIGH);
  delay(t2);
}

void TrunLeft(int t3){
  analogWrite(EnMotor1, Value);
  digitalWrite(Motor11, HIGH);
  digitalWrite(Motor12, LOW);
  analogWrite(EnMotor2, Value / 3);
  digitalWrite(Motor21, HIGH);
  digitalWrite(Motor22, LOW);
  delay(t3);
}

void TrunRight(int t3){
  analogWrite(EnMotor1, Value / 3);
  digitalWrite(Motor11, HIGH);
  digitalWrite(Motor12, LOW);
  analogWrite(EnMotor2, Value);
  digitalWrite(Motor21, HIGH);
  digitalWrite(Motor22, LOW);
  delay(t3);
}

void Stop(int t3){
  analogWrite(EnMotor1, 0);
  digitalWrite(Motor11, LOW);
  digitalWrite(Motor12, LOW);
  analogWrite(EnMotor2, 0);
  digitalWrite(Motor21, LOW);
  digitalWrite(Motor22, LOW);
  delay(t3);
}

void loop(){
  while(Serial.available()){
    Read_data += char(Serial.read());
    delay(2);
  }
  
  if(Read_data.length()){
    if(Read_data[0] == 'W'){
      GoForward(500);
      Stop(0);
      Read_data = "";
    }
    
    else if(Read_data[0] == 'S'){
      GoBackward(500);
      Stop(0);
      Read_data = "";
    }
    
    else if(Read_data[0] == 'A'){
      TrunLeft(500);
      Stop(0);
      Read_data = "";
    }
    
    else if(Read_data[0] == 'D'){
      TrunRight(500);
      Stop(0);
      Read_data = "";
    }
    
    if(Read_data[0] == 'X'){
        if(Read_data[1] == '-'){
          char dat[3]={};
          int i=0;
          for(i=0;i<=1;i++)
          dat[i] = Read_data[i+2];
          angle = 90 - (atof(dat)/80*150);
          Read_data = "";
        }
        else if(Read_data[1] == '+'){
          char dat[3]={};
          int i=0;
          for(i=0;i<=1;i++)
          dat[i] = Read_data[i+2];
          angle = 90 + (atof(dat)/80*150);
          Read_data = "";
        }
        myservo_X.write(angle);
        delay(20);
    }
    
    if(Read_data[0] == 'Y'){
        if(Read_data[1] == '-'){
          char dat[3]={};
          int i=0;
          for(i=0;i<=1;i++)
          dat[i] = Read_data[i+2];
          angle = 90 - (atof(dat)/30*60);
          Read_data = "";
        }
        else if(Read_data[1] == '+'){
          char dat[3]={};
          int i=0;
          for(i=0;i<=1;i++)
          dat[i] = Read_data[i+2];
          angle = 90 + (atof(dat)/30*65);
          Read_data = "";
        }
        myservo_Y.write(angle);
        delay(20);
    }
  }
}
