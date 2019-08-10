#ifndef _mybt_h_
#define _mybt_h_
#include "Arduino.h"

extern myLCD mylcd;
class myBT{
private:
public:
  myBT(){}
  String processMess(){
    int i=0;
    String mess="";
    bool recMess=false;
      while(Serial.available()){
        char c=Serial.read();
        if(recMess){
          if(c=='>'){
            mess+='\0';
            recMess=false;
          }
          else{
            mess+=c;
          }
        }
        else{
          if(c=='<'){
            recMess=true;
          }
        }
      }
      //delay(10);
    return mess;
  }
  bool checkTask(int n){
    String checkMess=(String)'R'+(String)n;
    String mess="";
    mess=processMess();
    Serial.println(mess);
    if(checkMess==mess){
      String temp="<D"+(String)n+">\r\n";
      Serial.print(temp);
      return true;
    }
    return false;
  }

  void doTask(int n){
    switch(n){
      case 0:
        Serial.println("<0>");//open door
      break;
      case 1:
        Serial.println("<1>");//switch lamp
      break;
      case 2:
        Serial.println("<2>");//switch fan
      break;
      case 3:
        Serial.println("<3>");//alarm
      break;
      case 4:
        Serial.println("<4>");//fire
      break;
      case 5:
        Serial.println("<5>");//change state;
      break;
      case 6:
        Serial.println("<6>");//change state;
      break;
    }
    delay(500);
    bool check=checkTask(n);
    if(n<3){
      if(check)
        mylcd.print(1,0,"Success");
      else
        mylcd.print(1,0,"Failed");
    }
  }
  
  void openDoor(){
    /*Wire.beginTransmission(8);
    Wire.write("1");
    Wire.endTransmission();*/
    Serial.print("1");
  }
  void switchLamp()
  {
    /*Wire.beginTransmission(8);
    Wire.write("3");
    Wire.endTransmission();*/
    Serial.print("2\r\n");
  }

  void switchFan()
  {
    /*Wire.beginTransmission(8);
    Wire.write("2");
    Wire.endTransmission();*/
    Serial.print("3\r\n");
  }

  
  void alertFire() /*String temp*/
  {
    /*Wire.beginTransmission(8);
    Wire.print(temp);
    Wire.endTransmission();*/
    Serial.print("4\r\n");
  }
  
  void Alarm_arduino()
  {
    /*Wire.beginTransmission(8);
    Wire.write("5");
    Wire.endTransmission();*/
    Serial.print("5\r\n");
  }
};
#endif