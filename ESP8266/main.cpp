
//--------------------------------------------------------------------
//                            INCLUDE LIBRARIES
//--------------------------------------------------------------------
#include "Arduino.h"
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Ticker.h>
#include "myEEPRom.h"
#include "myDHT.h"
#include "myLCD.h"
#include "myAlarm.h"
#include "myBT.h"
#include "myRFID.h"
#include "myConnection.h"
#include "myHistory.h"

//--------------------------------------------------------------------
//                            DEFINE KEYWORDS
//--------------------------------------------------------------------

#define ledPin D0 
#define buttonPin A0

//--------------------------------------------------------------------
//                            SETUP
//--------------------------------------------------------------------
//EEPROM
myEEPRom myEEPROM;
//LCD
myLCD mylcd;

//myConnection 
myConnection myconnection;

//RFID
myRFID myrfid;

//Interact Arduino 
myBT mybt;

//Alarm
myAlarm myalarm;



//DHT 
myDHT mydht;

//myHistory 
myHistory myhistory;

//NTP (time)
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP,"3.asia.pool.ntp.org", 25200, 60000);
unsigned long int time_check_alarm = 0;
unsigned long int time_checkRealTime= 0;
unsigned long int time_refreshTime  = 0;
unsigned long int time_send_alarm = 0;
int index_alarm = 1;

//Control system
int state_system=0;
bool active_system=false;

//RFID
int state_RFID=-1;
unsigned long int time_check_RFID=0;

//Button 
bool trig=false;
bool change=false;
bool spec=false;
int state_button=0;
unsigned long int time_check_button=0;
unsigned long int time_press_button=0;
unsigned long int time_hold_move_button=0;

//Connection 
unsigned long int time_check_connection=0;
int state_connection=0;
long long time_receive_mess=-1;

//Screen 
int screen=0;

//Sensor value
int temperature=0;
int humidity=0;
unsigned long int time_check_sensors=0;

//LED 
int state_LED=0;
unsigned long int time_check_LED=0;

//States system
enum system_state{
  LOGIN_STATE1,
  LOGIN_STATE2,
  PRE_STATE,
  BUTTON_STATE,
  MQTT_STATE,
  USERMANAGE_STATE,
  SLEEP_STATE,
  HISTORY_STATE
};

//login using Pin Number
int posDot          =0;
int cursorPinNumber =0;

//Control users
enum control_types{
  NO_CONTROL,
  MASTER_BUTTON,
  MASTER_MQTT,
  MEMBER_BUTTON,
  MEMBER_MQTT
};
int type_control=NO_CONTROL;
int type_active=0;

unsigned long int time_checkUser=0;

//tasks 
int tasktoDo=0;

//sleep mode 
//count string display history on LCD
int count_history = 0;
int member_index = 0;
//--------------------------------------------------------------------
//                            IMPLEMENT FUNCTIONS
//--------------------------------------------------------------------

void check_Alarm(){
  
}

void readingButton(){
  int val=analogRead(buttonPin);
  //Serial.println(val);
  switch(state_button){
    case 0:
      if(val>350&&val<500){
         change=true;
         time_press_button=millis();
         time_hold_move_button=millis();
         state_button=1;
      }
      else if(val>550&&val<750){
         time_press_button=millis();
         trig=true;
         state_button=1;
      }
      else if(val>=750&&val<1000){
        time_press_button=millis();
        state_button=2;
      }
    break;
    case 1:
      if(val<100||((val>350&&val<500)&&(millis()-time_hold_move_button>300))){
        state_button = 0;
      }
    break;
    case 2:
      if(millis()-time_press_button>5000&val>=750&&val<1000){
        state_button=0;
        spec=true;
      }
      else if (val<=750 || val>1000){
        state_button = 0;
      }
    break;
  }
}

void checkRFID(){
  state_RFID=myrfid.checkRFID(member_index);
}

void checkConnection(){
  myconnection.connectMQTT();
  if(millis()-time_checkRealTime>10000){
      timeClient.begin();
      timeClient.update();
      time_checkRealTime=millis();
    }
  String formattedDate = timeClient.getFormattedDate();
  int splitT = formattedDate.indexOf("T");
  String date = formattedDate.substring(0, splitT);
  mylcd.setRealTime (date, timeClient.getDay(), timeClient.getFormattedTime());
}

void readSensors(){
  mydht.readSensors(temperature,humidity);
  mylcd.setInfo(temperature,humidity);
}


void resetSystem(){
  mylcd.print(0,0,"Reset System",0,true);
  myEEPROM.clear(0,300);
  mybt.doTask(6);
  myconnection.updateState(false);
  ESP.restart();
}

void turntoSleepMode(){
  mylcd.print(0,0,"Sleep Mode ",2000,true);
  state_system=SLEEP_STATE;
  type_control=NO_CONTROL;
  tasktoDo=0;
  screen=0;
  mybt.doTask(6);
  myconnection.updateState(0);
}

void blinkingLED(){
  state_LED=(++state_LED)%(state_connection+1);
  if(state_LED==0)
    digitalWrite(ledPin,LOW);
  else
    digitalWrite(ledPin,HIGH);  
}

void checkUser(){
  if(state_system==USERMANAGE_STATE||state_system==HISTORY_STATE)
    return;
  switch(type_control){
    case NO_CONTROL:
      if(state_RFID==1){
        state_RFID=-1;
        type_control=MASTER_BUTTON;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"Button mode",2000,false);
      }
      else if(type_active==1){
        type_active=0;
        type_control=MASTER_MQTT;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"MQTT mode",2000,false);
      }
      else if(state_RFID==2){
        state_RFID=-1;
        type_control=MEMBER_BUTTON;
        mylcd.print(0,0,"Member control",0,true);
        mylcd.print(1,0,"Button mode",2000,false);
      }
      else if(type_active==2){
        type_active=0;
        type_control=MEMBER_MQTT;
        mylcd.print(0,0,"Member control",0,true);
        mylcd.print(1,0,"MQTT mode",2000,false);
      }
    break;
    case MASTER_BUTTON:
      if(state_RFID==1&&state_system!=USERMANAGE_STATE){
        state_RFID=-1;
        type_control=NO_CONTROL;
        turntoSleepMode();
      }
    break;
    case MASTER_MQTT:
      if(state_RFID==1){
        state_RFID=-1;
        type_control=MASTER_BUTTON;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"Button mode",2000,false);
      }
      else if(type_active==1||state_connection!=2){
        type_active=0;
        type_control=NO_CONTROL;
        turntoSleepMode();
      }
    break;
    case MEMBER_BUTTON:
      if(type_active==1){
        type_active=0;
        type_control=MASTER_MQTT;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"MQTT mode",2000,false);
      }
      else if(state_RFID==2){
        state_RFID=-1;
        type_control=NO_CONTROL;
        turntoSleepMode();
      }
      else if(state_RFID==1){
        state_RFID=-1;
        type_control=MASTER_BUTTON;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"Button mode",2000,false);
      } 
    break;
    case MEMBER_MQTT:
      if(state_RFID==1){
        state_RFID=-1;
        type_control=MASTER_BUTTON;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"Button mode",2000,false);
      }
      else if(state_RFID==2){
        state_RFID=-1;
        type_control=MEMBER_BUTTON;
        mylcd.print(0,0,"Member control",0,true);
        mylcd.print(1,0,"Button mode",2000,false);
      }
      else if(type_active==1){
        type_active=0;
        type_control=MASTER_MQTT;
        mylcd.print(0,0,"Master control",0,true);
        mylcd.print(1,0,"MQTT mode",2000,false);
      }
      else if(type_active==2||state_connection!=2){
        type_active=0;
        type_control=NO_CONTROL;
        turntoSleepMode();
      }
    break;
  }
  if((type_control==MASTER_MQTT||type_control==MEMBER_MQTT)&&(state_system!=MQTT_STATE)){
    screen=0;
    state_system=MQTT_STATE;
    mylcd.displayMQTT_STATE(0);
    mybt.doTask(5);
    myconnection.updateState(type_control);
    if (type_control == MASTER_MQTT) myhistory.enQueue(mylcd.dateStr, mylcd.hourStr, 1);
    else if (type_control == MEMBER_MQTT) myhistory.enQueue(mylcd.dateStr, mylcd.hourStr, 6);
  }
  else if((type_control==MASTER_BUTTON||type_control==MEMBER_BUTTON)&&(state_system!=BUTTON_STATE)){
    state_system=BUTTON_STATE;
    time_press_button=millis();
    mylcd.displayBUTTON_STATE(screen,false,false,0);
    mybt.doTask(5);
    myconnection.updateState(type_control);
    if (type_control == MASTER_BUTTON) myhistory.enQueue(mylcd.dateStr, mylcd.hourStr, 0);
    else if (type_control == MEMBER_BUTTON) myhistory.enQueue(mylcd.dateStr, mylcd.hourStr, 2 + member_index);
  }
}

void setup() {
  Serial.begin(38400);
  delay(2000);
  mylcd.begin();
  mylcd.print(0,0,"Welcome",1000,true);
  
  pinMode(buttonPin,INPUT);
  pinMode(ledPin,OUTPUT);

  digitalWrite(ledPin,LOW);
  mydht.begin();
  myrfid.begin();
  myalarm.begin();
  myhistory.begin();
  /*
  //setup Alarm
  time_check_alarm = millis();
  time_updateRealTime = millis();
  */
  if(myrfid.checkActiveSystem()){
    myconnection.begin();
    timeClient.begin();
    timeClient.update();
    time_checkRealTime=millis();
    String formattedDate = timeClient.getFormattedDate();
    int splitT = formattedDate.indexOf("T");
    String date = formattedDate.substring(0, splitT);
    mylcd.setRealTime (date, timeClient.getDay(), timeClient.getFormattedTime());
    state_system=PRE_STATE;
  }
  else{
    mylcd.print(0,0," Insert Master ",0,true);
    mylcd.print(1,0," card to access ",0,false);
  }
}

void loop(){
  if(type_control==MASTER_BUTTON && spec){
    state_system=HISTORY_STATE;
    spec=false;
    count_history = 0;
    mylcd.print(0,0,"History Access",2000,true);
    mylcd.print(0,0,myhistory.history_Str[count_history][1],0,true);
    mylcd.print(1,0,myhistory.history_Str[count_history][0],0,false);
  }
  if(millis()-time_check_button > 100){
    readingButton();
    time_check_button=millis();
  }
  if(millis()-time_check_RFID > 500){
    checkRFID();
    time_check_RFID=millis();
  }
  if(millis()-time_check_LED > 1000){
    blinkingLED();
    time_check_LED=millis();
  }
  if(state_system > 1){
    if(millis()-time_check_connection>500){
      checkConnection();
      time_check_connection=millis();
    }
    if(millis()-time_check_sensors>100){
      readSensors();
      time_check_sensors=millis();
    }
    if (millis() - time_check_alarm > 1500){
      if(myalarm.check_alarm(timeClient.getFormattedTime()))
        mybt.doTask(3);
      time_check_alarm = millis();
    }
    if(millis()-time_checkUser>500){
      checkUser();
      time_checkUser=millis();
    }
    if(millis() - time_send_alarm > 1700)
    {
      myconnection.sendAlarm(index_alarm);
      index_alarm++;
      if (index_alarm == 5) index_alarm = 1;
      time_send_alarm = millis();      
    }
  }
  
  switch(state_system){
    case LOGIN_STATE1:
      if(state_RFID==0){
        mylcd.print(0,0,"Invalid card",2000,true);
        mylcd.print(0,0," Insert Master ",0,true);
        mylcd.print(1,0," card to access ",0,false);
      }
      else if(state_RFID==2){
        mylcd.print(0,0,"Not master card",2000,true);
        mylcd.print(0,0," Insert Master ",0,true);
        mylcd.print(1,0," card to access ",0,false);
      }
      else if(state_RFID==1){
        mylcd.print(0,0,"Successful",0,true);
        mylcd.print(1,0,"Check pin number",2000,false);
        mylcd.print(0,0,"0123456789",0,true);
        mylcd.print(1,0,"|");
        state_system=LOGIN_STATE2;
      }
    break;
    case LOGIN_STATE2:
      if(trig){
        int checkpin=myrfid.addPinNumber(cursorPinNumber);
        mylcd.print(0,15-posDot,"****",500,false);
        posDot++;
        trig=false;
        if(checkpin!=2){
          posDot=0;
          cursorPinNumber=0;
          if(checkpin==0){
            mylcd.print(0,0,"Wrong pin number",2000,true);
            mylcd.print(0,0,"0123456789",0,true);
            mylcd.print(1,0,"|");
          }
          else{
            mylcd.print(0,0,"Successful",0,true);
            myrfid.activeSystem();
            myconnection.begin();
            timeClient.begin();
            timeClient.update();
            time_checkRealTime=millis();
            String formattedDate = timeClient.getFormattedDate();
            int splitT = formattedDate.indexOf("T");
            String date = formattedDate.substring(0, splitT);
            mylcd.setRealTime (date, timeClient.getDay(), timeClient.getFormattedTime());
            state_system=PRE_STATE;
          }
          delay(2000);
        }
      }
      else if(change){
        cursorPinNumber=(++cursorPinNumber)%10;
        mylcd.print(1,0,"                ");
        mylcd.print(1,cursorPinNumber,"|");
        change=false;
      }
    break;
    case PRE_STATE:
      if(millis()-time_refreshTime>1000){
        mylcd.refreshTime();
        time_refreshTime=millis();
      }
    break;
    case BUTTON_STATE:
      if(trig||change){
        time_press_button=millis();
        tasktoDo=mylcd.displayBUTTON_STATE(screen,trig,change);
        switch(tasktoDo){
          case 0:
          break;
          case 1: 
          break;
          case 2: 
            mybt.doTask(0);
          break;
          case 3: 
            mybt.doTask(1);
          break;
          case 4: 
            mybt.doTask(2);
          break;
          case 5: 
            tasktoDo=0;
            if(type_control==1){
              state_system=USERMANAGE_STATE;
              mylcd.displayUSERMANAGE_STATE(screen,false,false,true);
            }
            else{
              mylcd.print(0,0,"Only master RFID",0,true);
              mylcd.print(1,0,"card can access",2000,false);
              mylcd.displayBUTTON_STATE(screen,false,false,3);
            }
          break;
          case 6: 
            type_control=NO_CONTROL;
            turntoSleepMode();
          break;
          case 7: 
            tasktoDo=0;
            if(type_control==1){
              resetSystem();
            }
            else{
              mylcd.print(0,0,"Only master RFID",0,true);
              mylcd.print(1,0,"card can reset",2000,false);
              mylcd.displayBUTTON_STATE(screen,false,false,5);
            }
          break;
        }
        trig=change=false;
      }
      if(millis()-time_press_button>7000&&(screen==6||screen>13))
        mylcd.displayBUTTON_STATE(screen,false,false,0);
      if(millis()-time_press_button>20000&&screen==0)
        turntoSleepMode();
    break;
    case MQTT_STATE:
      if(millis()-time_receive_mess>6000&&time_receive_mess!=-1){
        mylcd.displayMQTT_STATE(0);
        time_receive_mess=-1;
      }
    break;
    case USERMANAGE_STATE: 
      if(screen==4){
        int stt;
        int checkAdd=myrfid.addUIDcard(stt);
        if(checkAdd!=-1){
          mylcd.checkAddUID(checkAdd,stt+1);
          mylcd.displayUSERMANAGE_STATE(screen,false,false,true);
        }
      }
      else if(screen==5){
        int stt;
        int checkRemove=myrfid.removeUIDcard(stt);
        if(checkRemove!=-1){
          mylcd.checkRemoveUID(checkRemove,stt+1);
          mylcd.displayUSERMANAGE_STATE(screen,false,false,true);
        }
      }
      if(trig||change){
        tasktoDo=mylcd.displayUSERMANAGE_STATE(screen,trig,change,false);
        trig=change=false;
      }
      if(tasktoDo==5){
        tasktoDo=0;
        state_system=BUTTON_STATE;
        mylcd.displayBUTTON_STATE(screen,false,false,0);
      }
      else if(tasktoDo>0&&tasktoDo<5){
        int x=myrfid.removeByHand(tasktoDo-1);
        mylcd.checkRemoveUID(x,tasktoDo);
        tasktoDo=0;
        mylcd.displayUSERMANAGE_STATE(screen,false,false,true);
      }
    break;
    case SLEEP_STATE:
      if(millis()-time_refreshTime>1000){
        mylcd.refreshTime();
        time_refreshTime=millis();
      }
    break;
    case HISTORY_STATE:
      if (change){
        count_history = (count_history + 1) % myhistory.count1;
        mylcd.print(0,0,myhistory.history_Str[count_history][1],0,true);
        mylcd.print(1,0,myhistory.history_Str[count_history][0],0,false);
        change = false;
      }
      else if (trig){
        state_system = SLEEP_STATE;
        type_control = NO_CONTROL;
        trig = false;
      }
    break;
  }
}