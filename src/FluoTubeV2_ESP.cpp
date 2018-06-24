/*
  FluoTubeV2_ESP.h - FluoTube library
  Copyright (c) 2017 Adriano Costanzo.  All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  See file LICENSE.txt for further informations on licensing terms.
*/
#include <Arduino.h>
#if FLUOARCH == 'E'

#include "FluoTubeV2.h"
#include "Drivers/EasyTransfer.h"

#if CONFIG_FREERTOS_UNICORE
#define ARDUINO_RUNNING_CORE 0
#else
#define ARDUINO_RUNNING_CORE 1
#endif

#define MAX_IO 32

// mode
#define DR   0x01 // DIGITAL READ
#define DW   0x02 // DIGITAL WRITE
#define AR   0x03 // ANALOG READ
#define AW   0x04 // ANALOG WRITE
#define DR_P 0x05 // DIGITAL READ

const byte interruptPin = MISO_HSPI;
volatile byte interruptCounter;
volatile byte interruptline;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

void IRAM_ATTR handleInterrupt() 
{
    portENTER_CRITICAL_ISR(&mux);
    interruptCounter++;
    interruptline = digitalRead(interruptPin);
    portEXIT_CRITICAL_ISR(&mux);
}

//structure IO
struct IO_t
{
    uint8_t mode;   // mode -> DR / DW / AR / AW
    uint8_t type;   // type -> ???

    uint8_t sem1;   // busy IO semaphore ???
    uint8_t sem2;   // busy IO semaphore ???

    int32_t value;  // decimal value
};

//structure MSG
struct MSG_t
{
    uint32_t mode;      // type  
    uint8_t msg[128];   // payload
};

struct SEND_DATA_STRUCT{

    uint32_t seq;     // sequence number -> progressive number (overflow at 255)
    struct IO_t io[MAX_IO];
    struct MSG_t;

} TXdata;

EasyTransfer StructIO_out;

//functions define
void Isetup();
void loopSend(void *pvParameters);
void SETStructIO_out();


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
    //nothing to do 
}

FluoTubeClass FluoTubeV2;


void Isetup() {

    pinMode(LED_STATUS, OUTPUT); pinMode(LED_LINK, OUTPUT); pinMode(LED_CLOUD, OUTPUT); pinMode(LED_BLE, OUTPUT); pinMode(SPI_EN, OUTPUT); pinMode(SDCD, INPUT); pinMode(RESET_644P, OUTPUT);

    digitalWrite(LED_STATUS, 1); // force on  
    digitalWrite(LED_CLOUD, 0); digitalWrite(LED_LINK, 0); digitalWrite(LED_BLE, 0);
    digitalWrite(SPI_EN, 0); // 0 enable - 1 disable // Warning: programmazione ext ICSP non disponibile

    pinMode(interruptPin, INPUT);
    attachInterrupt(digitalPinToInterrupt(interruptPin), handleInterrupt, CHANGE);

    Serial.begin(115200);
    SerialInternal.begin(57600);
    
    xTaskCreatePinnedToCore(loopSend, "loopSend", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);
    //xTaskCreatePinnedToCore(loopReceive, "loopReceive", 4096, NULL, 1, NULL, ARDUINO_RUNNING_CORE);

    StructIO_out.begin(details(TXdata), &SerialInternal);

    Serial.println("ESP32 started");
}



void loopSend(void *pvParameters) 
{

  while (1) 
  {
    
    if (interruptline == 0) // se arriva il primo int non entra piu'
    {
        SETStructIO_out();
        StructIO_out.sendData();
        delay(10);
    }

    if(interruptCounter > 0)
    {
      portENTER_CRITICAL(&mux);
      interruptCounter = 0;
      portEXIT_CRITICAL(&mux);
    }
    
    delay(1);
  }
}


void SETStructIO_out()
{
    TXdata.seq++; 
}

#endif // ARCH choice
