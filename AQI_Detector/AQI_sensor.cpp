#include "AQI_sensor.h"

// BLINKER_PMSX003ST, https://github.com/i3water/Blinker_PMSX003ST
#include <BLINKER_PMSX003ST.h>

#if defined(ESP32)
    HardwareSerial pmsSerial(2);// Pins 16(RX),17(TX)
#else
    #include <SoftwareSerial.h>
    SoftwareSerial pmsSerial(BLINKER_PMS_RX_PIN, BLINKER_PMS_TX_PIN);
#endif

BLINKER_PMSX003ST pms;

uint32_t PMS_TIME_FRESH = 0;
uint16_t PMS_TIME_LIMIT = 30000UL;

uint16_t pm1_0Get()
{
    pms.getPmAto(1.0);
}

uint16_t pm2_5Get()
{
    pms.getPmAto(2.5);
}

uint16_t pm10_0Get()
{
    pms.getPmAto(10);
}

double humiGet()
{
    pms.getHumi();
} 

double hchoGet()
{
    pms.getForm();
}

double tempGet()
{
    pms.getTemp();
}

uint8_t aqiLevelGet(uint8_t _base)
{
    return pms.getAQILevel(_base);
}

void setTimeLimit(uint16_t _time)
{
    PMS_TIME_LIMIT = _time;
}

bool pmsFresh()
{
    if (millis() - PMS_TIME_FRESH >= PMS_TIME_LIMIT) {
#ifndef ESP32
        if (!pmsSerial.isListening()) {
            pmsSerial.listen();
            // Blinker.delay(10);
        }
#endif

        pms.request();
        if(!pms.read()){
            return false;
        }

        PMS_TIME_FRESH = millis();
        return true;
    }
    else {
        return false;
    }
}

void pmsInit()
{
    pmsSerial.begin(9600);
    pms.begin(pmsSerial);
    pms.setMode(PASSIVE);

    setTimeLimit(30000UL);
}
