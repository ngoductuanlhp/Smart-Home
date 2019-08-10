 

//********************************************************************
//                 THIS IS THE SLAVE - ARDUINO UNO R3
//********************************************************************

//--------------------------------------------------------------------
//                         INCLUDE LIBRARIES
//--------------------------------------------------------------------
#include "Arduino.h"
#include <SoftwareSerial.h>
//--------------------------------------------------------------------
//                          DEFINE KEYWORDS
//--------------------------------------------------------------------
//PINs to read sensors:

//PINs to control devices
#define fanPIN1 9
#define fanPIN2 8
#define doorPIN1 10
#define doorPIN2 12
#define buzzerPIN 13
#define infraredPIN 2
#define lightPIN A0
#define TX_Bluetooth 7
#define RX_Bluetooth 4
#define buttonPIN1 A5
#define buttonPIN2 A4
#define ledPIN1 A1
#define ledPIN2 A2
#define lampPIN1 6
#define lampPIN2 5
#define lampDoorPIN A3
//Bluetooth
SoftwareSerial BTSerial(RX_Bluetooth, TX_Bluetooth);
//--------------------------------------------------------------------
//                            SETUP
//--------------------------------------------------------------------

//--------------------------------------------------------------------
//                            GLOBAL VARIABLES
//--------------------------------------------------------------------

long long int time_open_door = -1;
long long int time_close_door = -1;
long long int time_alarm = -1;
unsigned long int time_buzzer = 0;
unsigned long int time_check_button = 0;
unsigned long int time_check_lamp = 0;
unsigned long int time_not_alarm =0;
bool state_fan = false;
bool state_lamp = false;
bool state_buzzer = false;
bool state_fire = false;
int data = 0;
int state_butA = 0;
int state_butB = 0;
int state_door = 0;
bool state_system = false;
bool button1 = false;
bool button2 = false;
bool button3 = false;
bool button4 = false;
bool rec=false;
bool newMess=false;

String mess="";

//--------------------------------------------------------------------
//                             IMPLEMENT FUNCTIONS
//--------------------------------------------------------------------


void openDoor(){
  //analogWrite(doorPIN1, 70);
  digitalWrite(doorPIN1, HIGH);
  digitalWrite(doorPIN2, LOW);
  digitalWrite(lampDoorPIN, HIGH);
}
void stopDoor(){
  digitalWrite(doorPIN1, LOW);
  digitalWrite(doorPIN2, LOW);
}
void closeDoor(){
  digitalWrite(doorPIN1, LOW);
  //analogWrite(doorPIN2, 70);
  digitalWrite(doorPIN2, HIGH);
  digitalWrite(lampDoorPIN , LOW);
}

void controlDoor(bool trig = false){
  switch(state_door){
    case 0:
      if (trig){
        openDoor();
        time_open_door = millis();
        state_door = 1; 
      }
    break;
    case 1:
      if (millis() - time_open_door > 300){
        stopDoor();
        time_open_door = millis();
        state_door = 2;
      }
    break;
    case 2:
      if (digitalRead(infraredPIN) == 0 || trig){
        time_open_door = millis();
      }
      if (millis() - time_open_door > 5000){
        closeDoor();
        time_open_door = millis();
        state_door = 3; 
      }
    break;
    case 3:
      if (millis() - time_open_door > 300){
        stopDoor();
        BTSerial.println("<door_off>");
        time_open_door = millis();
        state_door = 0;
      }
    break;
  }
}

void turnLamp( int temp ){
  if (temp == 0){
    digitalWrite(lampPIN1, LOW);
    digitalWrite(lampPIN2, LOW);
    state_lamp = false;
  }
  else {
    analogWrite (lampPIN1, temp);
    analogWrite (lampPIN2, temp);
    state_lamp = true;
  }
}

void controlLamp(){
  if (state_lamp == true) turnLamp(0);
  else {
    int light_value = map(analogRead(lightPIN),0,900,5,255);
    turnLamp(light_value);
  }
}

void maintainLamp(){
  if (state_lamp && millis() - time_check_lamp > 100){
    int light_value = map(analogRead(lightPIN),0,900,5,255);
    turnLamp(light_value);
    time_check_lamp = millis();
  }
}

void controlFan(int temp = 1){
  if (temp == 0){
    digitalWrite(fanPIN1, LOW);
    digitalWrite(fanPIN2, LOW);
    state_fan = false;
  }
  else if (temp == 1){
    if(state_fan == false){
      digitalWrite(fanPIN1, HIGH);
      digitalWrite(fanPIN2, LOW);
      state_fan = true;
    }
    else{
      digitalWrite(fanPIN1, LOW);
      digitalWrite(fanPIN2, LOW);
      state_fan = false;
    }
  }
}

void controlBuzzer(int temp_temp){
  if (temp_temp == 0){
    digitalWrite(buzzerPIN, HIGH);
    state_buzzer = false;
    time_alarm == -1;
    return;
  }
  else {
    if (state_buzzer == false){
      digitalWrite(buzzerPIN, LOW);
      state_buzzer = true;
    }
    else if(state_buzzer == true) {
      digitalWrite(buzzerPIN, HIGH);
      state_buzzer = false;
    }
  }
}
void Alarm(int trig = 2)
{
  if (trig  == 1 && millis() -  time_not_alarm > 60500 ) {
    time_alarm = millis(); 
    time_not_alarm = millis();
  }
  else if (trig == 0) time_alarm = -1;
  else {
    if (time_alarm != -1 && millis() - time_alarm > 20000){
      controlBuzzer(0);
      time_alarm == -1;
    }
    if (time_alarm != -1 && millis() - time_buzzer > 600){
      controlBuzzer(1);
      time_buzzer = millis();
    }
  }
}

void turnOffAll(){
  // DOOR
  openDoor();
  time_open_door = -1;
  delay(300);
  stopDoor();
  time_close_door = -1;
  // LED & LAMP
  digitalWrite(ledPIN1, LOW);
  digitalWrite(ledPIN2, LOW);
  digitalWrite(lampPIN1, HIGH);
  digitalWrite(lampPIN2, HIGH);
  digitalWrite(lampDoorPIN, HIGH);
  state_lamp = false;
  state_fan = false;
  // ALARM
  time_alarm = -1;
  // FAN
  controlFan(0);
  digitalWrite(buzzerPIN, LOW);
  state_buzzer = true;
}

void Alert(int i = 0){
  if ( i == 1){
    digitalWrite(buzzerPIN, HIGH);
    state_buzzer = false;
    return;
  }
  turnOffAll(); 
  digitalWrite(buzzerPIN, LOW);
  state_buzzer = true;
}

void readingButton(){
  int butA = analogRead(buttonPIN1);
  int butB = analogRead(buttonPIN2);
  switch(state_butA){
    case 0:
      if (200 < butA && butA < 400){
        button2 = true;   
        state_butA = 1;
      }
      else if ( 420 < butA && butA < 750){
        button1 = true;
        state_butA = 1;
      }
    break;
    case 1:
      if (butA < 100){
        state_butA = 0;
      }
    break;
  }
  switch(state_butB){
    case 0:
      if (200 < butB && butB < 400){
        button3 = true;   
        state_butB = 1;
      }
      else if ( 420 < butB && butB < 750){
        button4 = true;
        state_butB = 1;
      }
    break;
    case 1:
      if (butB < 100){
        state_butB = 0;
      }
    break;
  }    
}

void checkButton(){
  readingButton();
  if (button1){
    controlDoor(true);
    BTSerial.println("<door_on>");
    button1 = false;
  }
  else if (button2){
    controlFan();
    if (state_fan == true) BTSerial.println("<fan_on>");
    else if (state_fan == false) BTSerial.println("<fan_off>");
    button2 = false;
  }
  else if (button3){
    controlLamp();
    if (state_lamp == true) BTSerial.println("<lamp_on>");
    else if (state_lamp == false) BTSerial.println("<lamp_off>");
    button3 = false;
  }
  else if (button4){
    Alarm(0);
    Serial.println("asad");
    button4 = false;
  }
}

void checkMess(){
  // Keep reading from HC-06 and send to Arduino Serial Monitor
  recMess();
  if(newMess){
    Serial.println(mess);
    if (mess.length() == 2 && 47 < mess[0] && mess[0] < 55) {
      String temp_mess = "<R";
      temp_mess += mess[0];
      temp_mess += ">";
      Serial.println(temp_mess);
      BTSerial.println(temp_mess);
    }
    else if (mess.length() == 3 && 47 < mess[1] && mess[1] < 55){
      int task = mess[1] - 48;
      if (task == 4){
        if (state_fire == false){
          Alert();
          state_fire = true;
        }
        else {
          Alert(1);
          state_fire = false;
        }
      }
      if (state_fire == false) {
        switch(task){
          case 0: // Open Door
              Serial.println("open Door");
              controlDoor(true);
          break;
          case 1: // Switch Lamp
            Serial.println("switch Lamp");
            controlLamp();
          break;
          case 2: // Switch Fan
            Serial.println("switch Fan");
            controlFan();
          break;
          case 3:
            Serial.println("ALARM");
            Alarm(1);
          break; 
          case 4:
          break;
          case 5:
            state_system = true;
            digitalWrite(ledPIN1, HIGH);
            digitalWrite(ledPIN2, HIGH);
          break;
          case 6:
            state_system = false;
            digitalWrite(ledPIN1, LOW);
            digitalWrite(ledPIN2, LOW);
          break;
        }
      }
    }
    else {
      Serial.println("<F>");
      BTSerial.println("<F>");
    }
    newMess=false;
    mess="";
  }
  // Keep reading from Arduino Serial Monitor and send to HC-06
}

void recMess(){
  while(BTSerial.available()){
    char c=BTSerial.read();
    if(rec){
      if(c!='>')
        mess+=c;
      else{
        newMess=true;
        rec=false;
        mess+='\0';
      }
    }
    else{
      if(c=='<'){
        rec=true;
      }
    }
  }
}
//--------------------------------------------------------------------
//                            MAIN FUNCTIONS
//--------------------------------------------------------------------
void setup() {
  //Init 
  Serial.begin(38400);
  while (!Serial) {}
  BTSerial.begin(38400);
  while (!BTSerial) {}

  //Declare INPUT/OUTPUT
  pinMode(fanPIN1,OUTPUT);
  pinMode(fanPIN2,OUTPUT);
  pinMode(doorPIN1,OUTPUT);
  pinMode(doorPIN2,OUTPUT);
  pinMode(buzzerPIN, OUTPUT);
  pinMode(infraredPIN, INPUT_PULLUP);
  pinMode(lightPIN,INPUT);
  pinMode(buttonPIN1, INPUT);
  pinMode(buttonPIN2, INPUT);
  pinMode(ledPIN1, OUTPUT);
  pinMode(ledPIN2, OUTPUT);
  pinMode(lampPIN1, OUTPUT); 
  pinMode(lampPIN2, OUTPUT);  
  pinMode(lampDoorPIN, OUTPUT);

  //Setup OUTPUT
  turnLamp(0);
  controlFan(0);
  controlBuzzer(0);
  stopDoor();
}

void loop() {
  //Check all tasks step by step
  if (millis() - time_check_button > 50){
    time_check_button = millis();
    checkButton();
  }
  checkMess();
  maintainLamp();
  controlDoor();
  Alarm();
}


//********************************************************************
//                                  END
//********************************************************************
