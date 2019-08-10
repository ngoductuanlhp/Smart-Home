#ifndef  _myrfid_h_
#define _myrfid_h_
#include "Arduino.h"
#include <SPI.h>
#include <MFRC522.h>


//RFID pins
#define SS_PIN D8 //D8
#define RST_PIN D3 //D3

extern myLCD mylcd;

extern int state_system;
extern int screen;

extern myEEPRom myEEPROM;

String _masterUID="1B BC DC 73";
String _memberUID[4]{
  "",
  "",
  "",
  ""
};
int pinNumber=1234;
int pinNumberTemp=0;
int countPinNumber=0;

class myRFID{
private:
  MFRC522 _mfrc522;
//---------------------------------------------METHODS---------------------------------------------
public:
  myRFID():_mfrc522(SS_PIN, RST_PIN){};
  void begin(){
    SPI.begin();       // Init SPI bus
    _mfrc522.PCD_Init(); // Init MFRC522
    for(int i=0;i<4;i++){
      if(myEEPROM.read(10+30*i)==64){
        for(int j=1;j<30;j++){
          int x=myEEPROM.read(10+30*i+j);
          if(x==0)
            break;
          _memberUID[i]+=(char)x;
        }
        switch(i){
          case 0:
            mylcd.data_USERMANAGE_STATE[6][0]=">001:"+_memberUID[i];
            mylcd.data_USERMANAGE_STATE[7][0]=" 001:"+_memberUID[i];
          break;
          case 1:
            mylcd.data_USERMANAGE_STATE[6][1]=" 002:"+_memberUID[i];
            mylcd.data_USERMANAGE_STATE[7][1]=">002:"+_memberUID[i];
          break;
          case 2:
            mylcd.data_USERMANAGE_STATE[8][0]=">003:"+_memberUID[i];
            mylcd.data_USERMANAGE_STATE[9][0]=" 003:"+_memberUID[i];
          break;
          case 3:
            mylcd.data_USERMANAGE_STATE[8][1]=" 004:"+_memberUID[i];
            mylcd.data_USERMANAGE_STATE[9][1]=">004:"+_memberUID[i];
          break;
        }
      }
    }
  }

  String readUID(){
    String content= "";
    for (byte i = 0; i < _mfrc522.uid.size; i++) 
    {
       content.concat(String(_mfrc522.uid.uidByte[i] < 0x10 ? " 0" : " "));
       content.concat(String(_mfrc522.uid.uidByte[i], HEX));
    }
    content.toUpperCase();
    return content.substring(1);
  }

  //check UID to activate
  int checkUID(int & member_index)
  {
    String readUID_temp=readUID();
    if(readUID_temp == _masterUID){
      return 1;//master card
    }
    for(int i=0; i<4; i++){
      if(readUID_temp == _memberUID[i]){
        member_index = i;
        return 2;//member card
      }
    }
    return 0;
  }

  int checkRFID(int & member_index){      
    if(!_mfrc522.PICC_IsNewCardPresent())
      return -1;
    if(!_mfrc522.PICC_ReadCardSerial())
      return -1;
    int checkCard=checkUID(member_index);
    delay(1000);
    return checkCard;
  }

  void activeSystem(){
    myEEPROM.write(0,69);
  }

  void deactiveSystem(){
    myEEPROM.write(0,0);
  }

  bool checkActiveSystem(){
    if(myEEPROM.read(0)==69)
      return true;
    return false;
  }
int addUIDcard(int& n){
    if(!_mfrc522.PICC_IsNewCardPresent()){
      return -1;
    }
    if(!_mfrc522.PICC_ReadCardSerial()){
      return -1;
    }
    int idx=-1;
    int exist=0;
    String readUID_temp=readUID();
    if(readUID_temp==_masterUID)
      return 2; // master card
    for(int i=0; i<4; i++){
      if(_memberUID[i]==readUID_temp){
        return 0; //exist
      }
      if(_memberUID[i]==""&&exist==0){
        idx=i;
        exist=1;
      }
    }
    if(idx!=-1){
      n=idx;
      _memberUID[idx]=readUID_temp;
      myEEPROM.write(10+30*idx,64);
      for(int j=0;j<readUID_temp.length();j++){
        myEEPROM.write(10+30*idx+j+1,(int)readUID_temp[j]);
      }
      myEEPROM.write(10+30*idx+readUID_temp.length()+1,0);
      switch(idx){
        case 0:
          mylcd.data_USERMANAGE_STATE[6][0]=">001:"+readUID_temp;
          mylcd.data_USERMANAGE_STATE[7][0]=" 001:"+readUID_temp;
        break;
        case 1:
          mylcd.data_USERMANAGE_STATE[6][1]=" 002:"+readUID_temp;
          mylcd.data_USERMANAGE_STATE[7][1]=">002:"+readUID_temp;
        break;
        case 2:
          mylcd.data_USERMANAGE_STATE[8][0]=">003:"+readUID_temp;
          mylcd.data_USERMANAGE_STATE[9][0]=" 003:"+readUID_temp;
        break;
        case 3:
          mylcd.data_USERMANAGE_STATE[8][1]=" 004:"+readUID_temp;
          mylcd.data_USERMANAGE_STATE[9][1]=">004:"+readUID_temp;
        break;
      }
      return 3; //success
    }
    return 1; //full
  }
  
  int removeUIDcard(int& n){
    if(!_mfrc522.PICC_IsNewCardPresent()){
      return -1;
    }
    if(!_mfrc522.PICC_ReadCardSerial()){
      return -1;
    }
    String readUID_temp=readUID();
    if(readUID_temp==_masterUID)
      return 2; // master
    for(int i=0;i<4;i++){
      if(_memberUID[i]==readUID_temp){
        n=i;
        _memberUID[i]="";
        myEEPROM.write(10+30*i,0);
        switch(i){
          case 0:
            mylcd.data_USERMANAGE_STATE[6][0]=">001:   empty    ";
            mylcd.data_USERMANAGE_STATE[7][0]=" 001:   empty    ";
          break;
          case 1:
            mylcd.data_USERMANAGE_STATE[6][1]=" 002:   empty    ";
            mylcd.data_USERMANAGE_STATE[7][1]=">002:   empty    ";
          break;
          case 2:
            mylcd.data_USERMANAGE_STATE[8][0]=">003:   empty    ";
            mylcd.data_USERMANAGE_STATE[9][0]=" 003:   empty    ";
          break;
          case 3:
            mylcd.data_USERMANAGE_STATE[8][1]=" 004:   empty    ";
            mylcd.data_USERMANAGE_STATE[9][1]=">004:   empty    ";
          break;
        }
        return 1; // exist and delete
      }
    }
    return 0; // not exist
  }

  int removeByHand(int idx){
    if(_memberUID[idx]=="")
      return 0;
    _memberUID[idx]="";
    myEEPROM.write(40*idx,0);
    switch(idx){
      case 0:
        mylcd.data_USERMANAGE_STATE[6][0]=">001:   empty    ";
        mylcd.data_USERMANAGE_STATE[7][0]=" 001:   empty    ";
      break;
      case 1:
        mylcd.data_USERMANAGE_STATE[6][1]=" 002:   empty    ";
        mylcd.data_USERMANAGE_STATE[7][1]=">002:   empty    ";
      break;
      case 2:
        mylcd.data_USERMANAGE_STATE[8][0]=">003:   empty    ";
        mylcd.data_USERMANAGE_STATE[9][0]=" 003:   empty    ";
      break;
      case 3:
        mylcd.data_USERMANAGE_STATE[8][1]=" 004:   empty    ";
        mylcd.data_USERMANAGE_STATE[9][1]=">004:   empty    ";
      break;
    }
    return 1;      
  }
  int addPinNumber(int x){
    pinNumberTemp=pinNumberTemp*10+x;
    countPinNumber++;
    if(countPinNumber==4){
      bool valid=(pinNumberTemp==pinNumber)?1:0;
      countPinNumber=0;
      pinNumberTemp=0;
      return (valid==true)?1:0;
    }
    return 2;
  }
};
#endif