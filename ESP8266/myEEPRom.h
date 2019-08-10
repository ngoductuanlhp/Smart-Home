#ifndef _myeeprom_h_
#define _myeeprom_h_
#include "Arduino.h"
#include <EEPROM.h>


/*
512 bytes in EEPROM:
0->9: active system
10->130: member RFID card
150->169: temperature
170->189: humidity
200->299: alarm
300->...: log-in history
 */
class myEEPRom{
public:
    myEEPRom(){
        EEPROM.begin(512);
        delay(500);
    }
    bool write(int address,byte val){
        if(address<0||address>=512||val>=256||val<0)
            return false;
        EEPROM.write(address,val);
        EEPROM.commit();
        delay(5);
        return true;
    }
    byte read(int address){
        if(address<0||address>=512)
            return 0;
        return EEPROM.read(address);
    }
    void clear(int fromIdx=0,int toIdx=512){
        for(int i=fromIdx;i<toIdx;i++){
            EEPROM.write(i,0);
            EEPROM.commit();
            delay(5);
        }
    }
};
#endif