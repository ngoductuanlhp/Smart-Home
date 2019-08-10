#ifndef _myalarm_h_
#define _myalarm_h_
#include "Arduino.h"

extern myEEPRom myEEPROM;
extern myLCD mylcd;

class myAlarm {
  public:
    int Time_Alarm[4];
    String Time_Alarm_display[4];
    int count;
    myAlarm()
    {
      count = 0;
      Time_Alarm[0] = -1;
      Time_Alarm[1] = -1;
      Time_Alarm[2] = -1;
      Time_Alarm[3] = -1;
      changeStr();
    }
// check the real Time with the Time in the TimeArr (Time set in mqttdash)  
    // read ROM Alarm
    void begin()
    {
      count = myEEPROM.read(218);
      for (int i = 0; i < 4; i++){
        int hour_temp = myEEPROM.read(200 + i*4);
        int minute_temp = myEEPROM.read(201 + i*4);
        if ( hour_temp >= 100 || minute_temp >= 100)
        {
          Time_Alarm[i] = (hour_temp - 100) * 60 + (minute_temp - 100); 
        }
        else Time_Alarm[i] = -1;
      }
      changeStr();
      setAlarmTime();
    } 
    void setAlarmTime(){
        // slot 1 and slot 2  
        mylcd._data_MQTT_STATE[5][0] = "";
        mylcd._data_MQTT_STATE[5][0] += "1."+Time_Alarm_display[0]+"  2."+Time_Alarm_display[1];
        mylcd._data_BUTTON_STATE[18][0] = "";
        mylcd._data_BUTTON_STATE[18][0] += "1."+Time_Alarm_display[0]+"  2."+Time_Alarm_display[1];
        // slot 3 and slot 4 
        mylcd._data_MQTT_STATE[5][1] = "";
        mylcd._data_MQTT_STATE[5][1] += "3."+Time_Alarm_display[2]+"  4."+Time_Alarm_display[3];
        mylcd._data_BUTTON_STATE[18][1] = "";
        mylcd._data_BUTTON_STATE[18][1] += "3."+Time_Alarm_display[2]+"  4."+Time_Alarm_display[3];
    }
    bool check_Time(int time_toCheck)
    {
      for (int i = 0; i < count; i++)
      {
        if ( Time_Alarm[i] == time_toCheck) return true;  
      }
      return false;
    }

    bool check_alarm ( String real_time )
    {
      char cstr_real[6];
      int real_time_int;
      real_time.toCharArray(cstr_real,6);
      real_time_int = ((int) ( cstr_real[0] - 48) *10 +  (int) (cstr_real[1] -48))*60 + (int)(cstr_real[3]-48)*10 + (int)(cstr_real[4]-48);
      //Serial.println (time_real); 
      return (check_Time(real_time_int));
    }
    
// change the int (Time_Array) to String ( Time_Alarm_display )(for displaying on LCD)
    void changeStr()
    {
      for (int i = 0; i < 4 ; i++)
      {
        Time_Alarm_display[i] = "";
        if (Time_Alarm[i] != -1)
        {
           int tmp;
           tmp = (char) ((Time_Alarm[i]/60)/10);
           Time_Alarm_display[i] += tmp;
           tmp = (char) ((Time_Alarm[i] - ((Time_Alarm[i] /60)/10)*10*60)/60);
           Time_Alarm_display[i] += tmp; 
           Time_Alarm_display[i] += ":";
           tmp = (char) ((Time_Alarm[i] %60)/10);
           Time_Alarm_display[i] += tmp;
           tmp = (char) (Time_Alarm[i] %60 - ((Time_Alarm[i] %60)/10)*10);
           Time_Alarm_display[i] += tmp;
        }
        else Time_Alarm_display[i] = "empty";
      }
      return;
    }

// add new time to TimeArr
    void add_Time(int Time)
    { 
      if (check_Time(Time)) return;
      if (count == 4) 
      {
        Time_Alarm[3] = Time_Alarm[2];
        Time_Alarm[2] = Time_Alarm[1];
        Time_Alarm[1] = Time_Alarm[0];
        Time_Alarm[0] = Time;
        saveToROM();
        return; 
      }
      else
      {
        for (int i = count; i > 0; i--)
        {
          Time_Alarm[i] = Time_Alarm[i-1];
        }
        Time_Alarm[0] = Time;
        count ++;
        saveToROM();
      }
      return;
    }

//delete time in TimeArr
    bool delete_Time(int Time)
    {
      if (!(check_Time(Time))) return false;
      for(int i = 0; i < count ; i++)
      {
        if (Time_Alarm[i] == Time) 
        {
          for( int j = i; j < count - 1; j++)
          {
            Time_Alarm[j] = Time_Alarm[j+1]; 
          }
          Time_Alarm[count - 1] = -1;
          count --;
          saveToROM();
          return true;
        }
      }
      return false;
    }

//check the format of newTime (set on mqtt dash) 
// delete return 2;
// add return 1;
// wrong format return 0;
    int setAlarm ( String str)
    {    
      int time_set;
      char cstr [9];
      str.toCharArray(cstr,9);
      if ((('0' <= cstr[0])&&(cstr[0]<= '2'))&&(('0' <= cstr[1])&&(cstr[1]<= '9'))&&(('0' <= cstr[3])&&(cstr[3]<= '9'))&&(('0' <= cstr[4])&&(cstr[4]<= '9'))&&(cstr[2] ==  ':') )
      {
        if ( (((int)(cstr[0] - 48)*10+ (int)(cstr[1]-48)) <= 23) && (((int)(cstr[3] - 48)*10+ (int)(cstr[1]-48)) <= 59) )
        {
          time_set = ((int) ( cstr[0] - 48) *10 +  (int) (cstr[1] -48))*60 + (int)(cstr[3]-48)*10 + (int)(cstr[4]-48);
          if (cstr[5] == 'A' && cstr[6] == 'D' && cstr[7] == 'D') 
          {
            this->add_Time(time_set);
            changeStr();
            return 1;
          }
          else if (cstr[5] == 'D' && cstr[6] == 'E' && cstr[7] == 'L') 
          { 
            this->delete_Time(time_set);
            changeStr(); 
            return 2;
          }
        }
      }
      return 0;
    }
// Save alarm to EEPROM
    void saveToROM()
    {
      myEEPROM.write(218 , count); 
      for (int i = 0; i < 4; i++){
        if ( Time_Alarm[i] != -1){
          myEEPROM.write(200 + i*4, Time_Alarm[i]/60 + 100);
          myEEPROM.write(201 + i*4, Time_Alarm[i]%60 + 100);
        }
        else{
          myEEPROM.write(200 + i*4, 0);
          myEEPROM.write(201 + i*4, 0);
        }
      }
    }
};
#endif