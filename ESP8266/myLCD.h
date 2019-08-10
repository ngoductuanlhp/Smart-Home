#ifndef _mylcd_h_
#define _mylcd_h_ 
#include "Arduino.h"
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>


#define SDA_PIN D2
#define SCL_PIN D1


class myLCD{
public:
    LiquidCrystal_I2C _lcd;
    int member_remove=0;
    String _data_MQTT_STATE[7][2]={
{" Connected MQTT "," Remote control "},//0
{"Sending info....","                "},//1
{"Openning Door   ","                "},//2
{"Switching Lamp  ","                "},//3
{"Switching Fan   ","                "},//4
{"1.Empty  2.Empty","3.Empty  4.Empty"},//5
{"Setting Alarm   ","                "}//6
};
    String _data_BUTTON_STATE[19][2]={
{">A. Temp & Humid"," B. Control Dev "},//0
{" A. Temp & Humid",">B. Control Dev "},//1
{">C. Time & Alarm"," D. User Manage "},//2
{" C. Time & Alarm",">D. User Manage "},//3
{">E. Sleep Mode  "," F. Reset system"},//4
{" E. Sleep Mode  ",">F. Reset system"},//5
{"Temperature:    ","Humidity:       "},//6
{">1. Open door   "," 2. Switch Lamp "},//7
{" 1. Open door   ",">2. Switch Lamp "},//8
{">3. Switch fan  "," 4. Turn to menu"},//9
{" 3. Switch fan  ",">4. Turn to menu"},//10
{">1. Time        "," 2. Alarm List  "},//11
{" 1. Time        ",">2. Alarm List  "},//12
{">3. Turn to menu","                "},//13
{"Opening door    ","                "},//14
{"Switching lamp  ","                "},//15
{"Switching fan   ","                "},//16
{"00:00:00        ","                "},//17
{"1.Empty  2.Empty","3.Empty  4.Empty"},//18
};
    String data_USERMANAGE_STATE[13][2]={
{">1. Add User    "," 2. Remove User "},//0
{" 1. Add User    ",">2. Remove User "},//1
{">3. User list   "," 4. Back to menu"},//2
{" 3. User list   ",">4. Back to menu"},//3
{"   Insert UID   ","  card to add   "},//4
{"   Insert UID   "," card to remove "},//5
{">001:  empty    "," 002:  empty    "},//6
{" 001:  empty    ",">002:  empty    "},//7
{">003:  empty    "," 004:  empty    "},//8
{" 003:  empty    ",">004:  empty    "},//9
{">Back           ","                "},//10
{" Remove ?  > Yes","             No "},//11
{" Remove ?    Yes","           > No "} //12
};
    String dateStr="";
    String hourStr="";
public:
    myLCD():_lcd(0x27,16,2){
    }
    void begin(){
        Wire.begin(SDA_PIN,SCL_PIN);
        _lcd.init(); 
        _lcd.backlight();
    }
    void clear(){
        _lcd.clear();
    }
    void print(int r, int c, String str, int delayTime=0, bool clear=false){
        if(clear)
            _lcd.clear();
        _lcd.setCursor(c,r);
        _lcd.print(str);
        delay(delayTime);
    }
    void setInfo(int temperature,int humidity){
        String str1;
        String str2;
        if((temperature<0||temperature>60)||(humidity<0)||humidity>100){
            str1="Nan";
            str2="Nan";
        }
        else{
            str1=(String)temperature+"C";
            str2=(String)humidity+"%";
        }
        _data_BUTTON_STATE[6][0]="Temperature: "+str1;
        _data_BUTTON_STATE[6][1]="Humidity: "+str2;
    }
    void checkAddUID(int check,int n){
        _lcd.clear();
        _lcd.setCursor(0,0);
        if(check==3){
            _lcd.print("Add success");
            _lcd.setCursor(0,1);
            _lcd.print("ID: KTMT00"+(String)n);
        }
        else{
            _lcd.print("Add failed");
            _lcd.setCursor(0,1);
            if(check==0)
                _lcd.print("Card existed");
            else if(check==1)
                _lcd.print("Full member");
            else
                _lcd.print("Master card");
        }
        delay(2000);
    }

    void checkRemoveUID(int check,int n){
        _lcd.clear();
        _lcd.setCursor(0,0);
        if(check==1){
            _lcd.print("Remove success");
            _lcd.setCursor(0,1);
            _lcd.print("ID: KTMT00"+(String)n);
        }
        else{
            _lcd.print("Remove failed");
            _lcd.setCursor(0,1);
            if(check==0)
                _lcd.print("Card not exist");
            else if(check==2)
                _lcd.print("Master card");
        }
        delay(2000);
    }
    int displayMQTT_STATE(int screen){
        _lcd.clear();
        _lcd.setCursor(0,0);
        _lcd.print(_data_MQTT_STATE[screen][0]);
        _lcd.setCursor(0,1);
        _lcd.print(_data_MQTT_STATE[screen][1]);
        return 0;
    }
    int displayBUTTON_STATE(int &screen, bool trig, bool change, int forceS=-1){
        if(forceS>-1)
            screen=forceS;
        else{
            switch(screen){
                case 0:
                    screen = (trig) ? 6 : (change) ? 1 : 0;
                break;
                case 1:
                    screen = (trig) ? 7 : (change) ? 2 : 1; 
                break;
                case 2: 
                    screen = (trig) ? 11 : (change) ? 3 : 2; 
                break;
                case 3: 
                    screen = (trig) ? 19 : (change) ? 4 : 3; 
                break;
                case 4: 
                    screen = (trig) ? 20 : (change) ? 5 : 4;
                break;
                case 5: 
                    screen = (trig) ? 21 : (change) ? 0 : 5; 
                break;
                case 6: 
                    screen = (trig) ? 0 : 6; 
                break;
                case 7: 
                    screen = (trig) ? 14 : (change) ? 8 : 7; 
                break;
                case 8: 
                    screen = (trig) ? 15 : (change) ? 9 : 8; 
                break;
                case 9:
                    screen = (trig) ? 16 : (change) ? 10 : 9;
                break;
                case 10:
                    screen = (trig) ? 1 : (change) ? 7: 10;
                break;
                case 11:
                    screen = (trig) ? 17 : (change) ? 12 : 11;
                break;
                case 12:
                    screen = (trig) ? 18 : (change) ? 13 : 12;
                break;
                case 13:
                    screen = (trig) ? 2 : (change) ? 11 : 13;
                break;
                case 14:
                    screen = (trig) ? 7 : screen;
                break;
                case 15:
                    screen = (trig) ? 8 : screen;
                break;
                case 16:
                    screen = (trig) ? 9 : screen;
                break;
                case 17:
                    screen = (trig) ? 11 : screen;
                break;
                case 18:
                    screen = (trig) ? 12 : screen;
                break;
            }
        }
        if((trig||change||(forceS>-1))&&screen<19){
            _lcd.clear();
            _lcd.setCursor(0,0);
            _lcd.print(_data_BUTTON_STATE[screen][0]);
            _lcd.setCursor(0,1);
            _lcd.print(_data_BUTTON_STATE[screen][1]);

        }
        return  (screen == 6) ? 1 :
                (screen == 14) ? 2 :
                (screen == 15) ? 3 : 
                (screen == 16) ? 4 : 
                (screen == 19) ? 5 : 
                (screen == 20) ? 6 :
                (screen == 21) ? 7 :
                0;
    }
    int displayUSERMANAGE_STATE(int& state,bool trig,bool change,bool res){
        if(res)
            state=0;
        else{
            switch(state){
                case 0:
                state = trig ? 4 : change ? 1 : 0;                      break;
                case 1:
                state = trig ? 5 : change ? 2 : 1;                      break;
                case 2:
                state = trig ? 6 : change ? 3 : 2;                      break;
                case 3:
                state = trig ? 13 : change ? 0 : 3;                     break;
                case 4:
                state = trig ? 0 : 4;                                   break;
                case 5:
                state = trig ? 1 : 5;                                   break;
                case 6:
                state = trig ? ((member_remove=1)+10) : change ? 7 : 6;   break;                 
                case 7:
                state = trig ? ((member_remove=2)+9) : change ? 8 : 7;   break;
                case 8:
                state = trig ? ((member_remove=3)+8) : change ? 9 : 8;   break;
                case 9:
                state = trig ? ((member_remove=4)+7) : change ? 10 : 9;  break;
                case 10:
                state = trig ? 2 : (change) ? 6 : 10;                   break; 
                case 11:
                state = trig ? 14 : (change) ? 12 : 11;                 break;
                case 12:
                state = trig ? (member_remove=0)+2 : change ? 11 : 12;  break;
                break;
            }
        }
        if((res||trig||change)&&state<13){
            _lcd.clear();
            _lcd.setCursor(0,0);
            _lcd.print(data_USERMANAGE_STATE[state][0]);
            _lcd.setCursor(0,1);
            _lcd.print(data_USERMANAGE_STATE[state][1]);
        }
        if(state==13)
            return 5;
        if(state==14){
            int temp=member_remove;
            member_remove=0;
            return temp;
        }
        return 0;
    }
    
    void setRealTime( String date, int day, String hour  ){
      String day_display ="";
      switch ( day ){
        case 0:
          day_display = "Sun : ";
        break;
        case 1: 
          day_display = "Mon : ";
        break;
        case 2: 
          day_display = "Tue : ";
        break;
        case 3: 
          day_display = "Wed : ";
        break;
        case 4: 
          day_display = "Thu : ";
        break;
        case 5: 
          day_display = "Fri : ";
        break;
        case 6: 
          day_display = "Sat : ";
        break;
      }      
      hourStr=hour;
      _data_BUTTON_STATE[17][0]=hour;
      dateStr=day_display + date;
      _data_BUTTON_STATE[17][1]=day_display + date;
    }
    void refreshTime(){ 
       _lcd.clear();
       _lcd.setCursor(0,0);
       _lcd.print(hourStr);
       _lcd.setCursor(0,1);
       _lcd.print(dateStr); 
    }
};

#endif