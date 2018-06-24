/*
  FluoTubeV2_AVR.h - FluoTube library
  Copyright (c) 2017 Adriano Costanzo.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/

#include <Arduino.h>
#if FLUOARCH == 'A' && FLUOMODEL == 'P' // AVR - 644P

#include "FluoTubeV2.h"
#include "Drivers/SoftEasyTransfer.h"

#define MAX_IO 32

// mode
#define DR   0x01 // DIGITAL READ
#define DW   0x02 // DIGITAL WRITE
#define AR   0x03 // ANALOG READ
#define AW   0x04 // ANALOG WRITE

#define DR_P 0x05 // DIGITAL READ PULLUP

SoftwareSerial VirtualSerial(RX_VIRTUALSERIAL, TX_VIRTUALSERIAL);//(RX, TX)

//functions define
void Isetup();
void Iloop();
bool hasExp(unsigned long &prevTime, unsigned long interval);
void ReceiveRoutine();
void CheckSeq();
void ReceiveHandle();


// Class method

FluoTubeClass::FluoTubeClass()
{
    //nothing to do 
}

void FluoTubeClass::begin()
{
    Isetup();
}

void FluoTubeClass::run()
{
    Iloop();
}

FluoTubeClass FluoTubeV2;


//structure IO
struct IO_t
{
    uint8_t mode;   // mode -> DR / DW / AR / AW
    uint8_t type;   // type -> IO / WIFI / AUX

    uint8_t sem1;   // busy IO semaphore
    uint8_t sem2;   // busy IO semaphore

    int32_t value;  // decimal value

};

//structure MSG
struct MSG_t
{
    uint32_t mode;      // type  
    uint8_t msg[128];   // payload
};

struct RECEIVE_DATA_STRUCT{

    uint32_t seq;     // sequence number -> progressive number (overflow at 255)
    struct IO_t io[MAX_IO];
    struct MSG_t;

} RXdata;

SoftEasyTransfer StructIO_in;

void Isetup() 
{
  
    pinMode(MISO, OUTPUT);
    digitalWrite(MISO, LOW);
  
    VirtualSerial.begin(57600);

    Serial.begin(115200);

    StructIO_in.begin(details(RXdata), &VirtualSerial); 

    Serial.println("AVR started");
}

unsigned long prevMillisReceiveRoutine;

void Iloop() 
{
  
    if ( hasExp(prevMillisReceiveRoutine, 5) ) // 10ms interval for ESP32 send time
        ReceiveRoutine();

}

void ReceiveRoutine()
{
    
    
    if ( StructIO_in.receiveData() == 1)
    {
        CheckSeq(); // check pkt lost
        ReceiveHandle();
    } 
}

void ReceiveHandle()
{

}



// AUX

bool hasExp(unsigned long &prevTime, unsigned long interval) 
{
    if (  millis() - prevTime > interval ) 
    {
        prevTime = millis();
        return true;
    }

    else     
        return false;
}

void CheckSeq()
{      

    static long tmp[10] = {0,0,0,0,0,0,0,0,0,0};
    static int ind = 0;
  
    Serial.println(RXdata.seq);
    delay(1);

    tmp[ind] = RXdata.seq;
              
        if(ind == 0)
        {
            if(tmp[0] != (tmp[9] + 1) ) 
            Serial.println("Miss !!! caso 0");
            delay(1);
              
            ind = 1;
            return;
        }
              
        if( (tmp[ind-1] + 1) != tmp[ind] )
        {
            Serial.println("Miss !!! caso normal");
            delay(1);  
        } 
              
        ind++;
              
        if (ind > 9)
            ind = 0;
}

#endif // ARCH choice
