#ifndef  _myconnection_h_
#define _myconnection_h_ 
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//Wifi
#define STASSID "Tommmm"
#define STAPSK "12345678"

//MQTT Broker
#define mqtt_server "m16.cloudmqtt.com" 
#define mqtt_user "tuanngo"
#define mqtt_pwd "123456789"

#define mqtt_topic_sub1 "ControlDevices"
#define mqtt_topic_sub2 "SetAlarm"

#define mqtt_topic_pub1 "TempHumid"
#define mqtt_topic_pub2 "SetAlarm"
#define mqtt_topic_pub3 "DisplayAlarm"
#define mqtt_topic_pub4 "SystemState"

extern int state_system;
extern int state_connection;
extern bool active_system;
extern int screen;

extern myLCD mylcd;
extern myAlarm myalarm;
extern myBT mybt;

extern int temperature;
extern int humidity;

extern long long time_receive_mess;

extern int type_control;
extern int type_active;

int state_set_alarm = 0;
unsigned long int time_set_alarm=0;

//WiFi
const char* ssid     = STASSID;
const char* password = STAPSK;
//MQTT
const uint16_t mqtt_port = 12986; //Port of CloudMQTT
WiFiClient espClient;
PubSubClient client(espClient);



void sendInfo(){
    String str1="";
    str1+=(String)temperature;
    str1 += "|";
    str1 +=(String)humidity;
    bool check = client.publish(mqtt_topic_pub1,str1.c_str());
    if(check){
        mylcd.print(1,0,"Successful");
        return;
    }
    mylcd.print(1,0,"Failed");
}

void callback(char* topic, byte* payload, unsigned int length) {
    time_receive_mess=millis();
    //Serial.println("Received");
    String topicStr(topic);
    String messStr=String((char*)payload);
    messStr.remove(length);
    if(topicStr=="ControlDevices"){ // active
        if(messStr=="123456789"&&type_control!=1){
            // set display for setAlarm
            client.publish("setAlarm","ALARM");
            type_active=1;//master
        }
        else if(messStr=="123456"&&type_control==0){
            type_active=2;//member
        }
    }
    switch (state_system){
        case 0:
        case 1:
        case 2:
        case 3:
        break;
        case 4:
            if(type_control==4){
                if(topicStr==mqtt_topic_sub1){
                    if(messStr=="send_info"){
                        mylcd.displayMQTT_STATE(1);
                        sendInfo();
                    }
                    else if(messStr=="open_door"){
                        mylcd.displayMQTT_STATE(2);
                        mybt.doTask(0);
                    }
                    else if(messStr=="switch_lamp"){
                        mylcd.displayMQTT_STATE(3);
                        mybt.doTask(1);
                    }
                    else if(messStr=="switch_fan"){
                        mylcd.displayMQTT_STATE(4);
                        mybt.doTask(2);
                    }
                    else if(messStr=="display_alarm")
                        mylcd.displayMQTT_STATE(5);
                }
                else if(topicStr == mqtt_topic_sub2){
                    if (messStr=="added"||messStr=="wrong format"||messStr=="deleted"||messStr=="ALARM")
                        return;
                    state_set_alarm = myalarm.setAlarm(messStr);
                    myalarm.setAlarmTime();
                    if (state_set_alarm == 1)
                        client.publish(mqtt_topic_pub2,"added");
                    else if (state_set_alarm == 0)
                        client.publish(mqtt_topic_pub2,"wrong format");
                    else if (state_set_alarm == 2)
                        client.publish(mqtt_topic_pub2,"deleted");
                    delay(2000);
                    client.publish(mqtt_topic_pub3,"ALARM");    // return the display on mqttdash to "ALARM" (default)
                }
            }
            else if(type_control==2){
                if(topicStr==mqtt_topic_sub1){
                    if(messStr=="send_info_m"){
                        mylcd.displayMQTT_STATE(1);
                        sendInfo();
                    }
                    else if(messStr=="open_door_m"){
                        mylcd.displayMQTT_STATE(2);
                        mybt.doTask(0);
                    }
                    else if(messStr=="switch_lamp_m"){
                        mylcd.displayMQTT_STATE(3);
                        mybt.doTask(1);
                    }
                    else if(messStr=="switch_fan_m"){
                        mylcd.displayMQTT_STATE(4);
                        mybt.doTask(2);
                    }
                    else if(messStr=="display_alarm_m")
                        mylcd.displayMQTT_STATE(5);
                }
                else if(topicStr == mqtt_topic_sub2){
                    if (messStr=="added"||messStr=="wrong format"||messStr=="deleted"||messStr=="ALARM")
                        return;
                    state_set_alarm = myalarm.setAlarm(messStr);
                    myalarm.setAlarmTime();
                    if (state_set_alarm == 1)
                        client.publish(mqtt_topic_pub2,"added");
                    else if (state_set_alarm == 0)
                        client.publish(mqtt_topic_pub2,"wrong format");
                    else if (state_set_alarm == 2)
                        client.publish(mqtt_topic_pub2,"deleted");
                    delay(2000);
                    client.publish(mqtt_topic_pub2,"ALARM");    // return the display on mqttdash to "ALARM" (default)
                }
            }
        case 5:
        case 6:
        break;
    }
}

class myConnection{
public:
    myConnection(){       
    }

    void begin(){
        //Init wifi
        mylcd.print(0,0,"Setup WiFi",2000,true);
        mylcd.print(0,0,"Connect to "+(String)ssid,0,true);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
        delay(2000);
        int count1=0;
        while (WiFi.status() != WL_CONNECTED&&count1<16){
            mylcd.print(1,count1,"*");
            count1++;
            delay(500);
        }
        mylcd.print(0,0,"Connect to "+(String)ssid,0,true);
        if(count1==16){
            mylcd.print(1,0,"failed",2000,false);
            state_connection=0;
        }
        else{
            mylcd.print(1,0,"successful",2000,false);
            mylcd.print(0,0,"Setup MQTT",2000,true);
            mylcd.print(0,0,"Connect to "+(String)mqtt_user,0,true);
            int count2=0;
            client.setServer(mqtt_server, mqtt_port);
            client.setCallback(callback);
            while(!client.connected()&&count2<16){
                client.connect("ESP8266Client",mqtt_user, mqtt_pwd);
                mylcd.print(1,count2,"*");
                count2++;
            }
            mylcd.print(0,0,"Connect to "+(String)mqtt_user,0,true);
            if(count2==16){
                mylcd.print(1,0,"failed",2000,false);
                state_connection=1;
            }
            else{
                mylcd.print(1,0,"successful",2000,false);
                client.subscribe(mqtt_topic_sub1);
                client.subscribe(mqtt_topic_sub2);
                state_connection=2;
            }                
        }
    }   

    void connectMQTT(){
        switch(state_connection){
            case 0:
                if(WiFi.status() == WL_CONNECTED)
                    state_connection=1;
            break;
            case 1:
                if(WiFi.status() != WL_CONNECTED){
                    WiFi.begin(ssid, password);
                    state_connection=0;
                }
                else if(client.connect("ESP8266Client",mqtt_user, mqtt_pwd)){
                    client.subscribe(mqtt_topic_sub1);
                    client.subscribe(mqtt_topic_sub2);
                    state_connection=2; 
                }
            break;
            case 2:
                if(client.loop()==false)
                    state_connection=0;
                else{
                    //Serial.println("Loop");
                }
            break;
        }
    }

    void sendAlarm(int i){
      if ( i > 4 ) return;
      myalarm.changeStr();
      String temp_alarm = "";
      temp_alarm += (char)(i + 48);
      temp_alarm += ":" + myalarm.Time_Alarm_display[i-1];
      client.publish(mqtt_topic_pub3,temp_alarm.c_str());
    }
    void updateState(int type){
        switch(type){
            case 0:
                client.publish(mqtt_topic_pub4,"sleep",true);
            break;
            case 1:
                client.publish(mqtt_topic_pub4,"MASTER_BUT",true);
            break;
            case 2:
                client.publish(mqtt_topic_pub4,"MASTER_MQTT",true);
            break;
            case 3:
                client.publish(mqtt_topic_pub4,"MEMBER_BUT",true);
            break;
            case 4:
                client.publish(mqtt_topic_pub4,"MEMBER_MQTT",true);
            break;
        }
    }
};
#endif