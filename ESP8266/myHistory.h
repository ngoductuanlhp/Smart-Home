#ifndef _myhistory_h_
#define _myhistory_h_
#include "Arduino.h"

extern myEEPRom myEEPROM;

class myHistory{
  public:
    int size_arr = 10;
    int arr[10][6];
    int rear, front;
    String history_Str[10][2];
    int count1;
    myHistory() {
      front = 10;
      rear = 10;
      for (int i = 0; i < 10; i++){
        for (int j = 0; j < 6; j++){
          arr[i][j] = 0;
        }
        history_Str[i][0] = "";
        history_Str[i][1] = "";
      }
      count1 = 0;
    }
    void enQueue(String day, String time_formated, int state){
        Serial.println(day);
        Serial.println(time_formated);
        char day_temp [17];
        day.toCharArray(day_temp,17);
        char time_temp[6];
        Serial.println(day_temp);
        time_formated.toCharArray(time_temp,6);
        if ( front == 10 && rear == 10){
            front = 0;
            rear = 0;
        }
        else{ 
            rear = (rear+1) % size_arr;
            if (rear == front) front = (front + 1) % size_arr;
        }
        /* if (day_temp[0] == '1'){
            arr[rear][5] = state + 100;
            saveToEEP();
            return;
        }*/
        arr[rear][0] = (int)((day_temp[8] - 48)*10) + (int)(day_temp[9] - 48) + 100;
        arr[rear][1] = (int)((day_temp[11] - 48)*10) + (int)(day_temp[12] - 48) + 100;
        arr[rear][2] = (int)((day_temp[14] - 48)*10) + (int)(day_temp[15] - 48) + 100;
        arr[rear][3] = (int)((time_temp[0] - 48)*10) + (int)(time_temp[1] - 48) + 100;
        arr[rear][4] = (int)((time_temp[3] - 48)*10) + (int)(time_temp[4] - 48) + 100;
        arr[rear][5] = state + 100;
        //saveToEEP();
        Serial.println(arr[rear][0]);
        Serial.println(arr[rear][2]);
        if (count1 < 10) count1++;
        changeToStr();
    }

    void saveToEEP (){
      for (int i = 0; i < 10; i++){
        int address_temp = 300 + i*10;
        for ( int j = 0; j < 6; j++){
          int  address = address_temp + j;
          myEEPROM.write(address,arr[i][j]);
        }
      }
      myEEPROM.write(400 , front);
      myEEPROM.write(401 , rear);
      myEEPROM.write(402 , count1);
    }
    void changeToStr(){
        int front_temp = front;
        for (int i = 0; i < count1; i++) { 
            history_Str[i][0] = " ";
            history_Str[i][0] += (char) ((arr[front_temp][0]-100)/10 + 48);
            history_Str[i][0] += (char) ((arr[front_temp][0]-100)%10 + 48);
            history_Str[i][0] += "-"; 
            history_Str[i][0] += (char) ((arr[front_temp][1]-100)/10 + 48); 
            history_Str[i][0] += (char) ((arr[front_temp][1]-100)%10 + 48);
            history_Str[i][0] += "-"; 
            history_Str[i][0] += (char) ((arr[front_temp][2]-100)/10 + 48);
            history_Str[i][0] += (char) ((arr[front_temp][2]-100)%10 + 48);
            history_Str[i][0] += " "; 
            history_Str[i][0] += (char) ((arr[front_temp][3]-100)/10 + 48);
            history_Str[i][0] += (char) ((arr[front_temp][3]-100)%10 + 48);
            history_Str[i][0] += ":" ;
            history_Str[i][0] += (char) ((arr[front_temp][4]-100)/10 + 48); 
            history_Str[i][0] += (char) ((arr[front_temp][4]-100)%10 + 48);
            //history_Str[i][0] = "hello";
            String temp_index = "";
            if (i >= 9) {
              temp_index = "10";
            }
            else {
              temp_index = (char) (i + 49);
              temp_index += " ";
            }
            switch ( arr[i][5] -100){
                case 0:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] += ":Master Button";
                break;
                case 1:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] += ": Master MQTT ";
                break;
                case 2:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] += ": Member But 1";
                break;
                case 3:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] = ": Member But 2";
                break;
                case 4:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] = ": Member But 3";
                break;
                case 5:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] = ": Member But 4";
                break;
                case 6:
                    history_Str[front_temp][1] = temp_index;
                    history_Str[front_temp][1] = ": Member MQTT ";
                break;
            }
            Serial.println(history_Str[i][0]);
            front_temp = (front_temp + 1) % size_arr;
        }
    }
    void begin (){
      for (int i = 0; i < 10; i++){
        int address_temp = 300 + i*10;
        for ( int j = 0; j < 6; j++){
          int  address = address_temp + j;
          arr[i][j] = myEEPROM.read(address);
        }
      }
      front = myEEPROM.read(400);
      if (front > 9) front = 10;
      rear = myEEPROM.read(401);
      if (rear > 9) rear = 10;
      count1 = myEEPROM.read(402);
      if (count1 > 9) count1 = 0;
      if (count1 == 0){
        front = 10;
        rear = 10;
      }
      Serial.println(front);
      Serial.println(rear);
      Serial.println(count1);
      changeToStr();
    }
};

#endif