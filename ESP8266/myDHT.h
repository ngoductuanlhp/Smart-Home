#ifndef _mydht_h_
#define _mydht_h_
#include "Arduino.h"
#include "DHT.h"

#define dhtTYPE DHT11
#define dhtPIN D4


int checkValid(int x){
    if(x==NAN)
        return 0;
    return x;
}
class myDHT{
private:
    //DHT
    DHT dht;
    int idx=0;
    int temperature=0;
    int humidity=0;
    int sumTemp=0;
    int sumHumi=0;
    int arrTemp[20];
    int arrHumi[20];
public:
    myDHT(){

    }
    void begin(){
        dht.setup(dhtPIN);
        delay(500);
        memset(arrTemp,0,sizeof(arrTemp));
        memset(arrHumi,0,sizeof(arrHumi));
        while(idx<20){
            arrTemp[idx]=checkValid(dht.getTemperature());
            arrHumi[idx]=checkValid(dht.getHumidity());
            sumTemp+=arrTemp[idx];
            sumHumi+=arrHumi[idx];
            idx++;
            delay(50);
        }
        temperature=sumTemp/20;
        humidity=sumHumi/20;
    }
    void readSensors(int& temp,int& humi){
        idx++;
        idx=idx%20;
        sumTemp-=arrTemp[idx];
        sumHumi-=arrHumi[idx];
        arrTemp[idx]=checkValid(dht.getTemperature());
        arrHumi[idx]=checkValid(dht.getHumidity());
        sumTemp+=arrTemp[idx];
        sumHumi+=arrHumi[idx];
        temperature=sumTemp/20;
        humidity=sumHumi/20;
        temp=temperature;
        humi=humidity;
    }
};
#endif