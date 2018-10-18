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
uint32_t PMS_TIME_LIMIT = BLINKER_PMS_LIMIT_INIT;
bool isSleep = false;

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

uint8_t aqiGet(uint8_t _base)
{
    return pms.getAQI(_base);
}

void setTimeLimit(uint16_t _time)
{
    PMS_TIME_LIMIT = _time;
}

void sleep()
{
    pms.sleep();

    isSleep = true;
}

void wakeUp()
{
    pms.wakeUp();

    PMS_TIME_FRESH += BLINKER_PMS_SLEEP_TIME * 1000 + 30000;

    isSleep = false;
}

bool pmsFresh()
{
    if (((millis() > PMS_TIME_FRESH && (millis() - PMS_TIME_FRESH) >= PMS_TIME_LIMIT) 
        || PMS_TIME_FRESH == 0) && !isSleep) {
#ifndef ESP32
        if (!pmsSerial.isListening()) {
            pmsSerial.listen();
            // Blinker.delay(10);
        }
#endif
        Serial.print("[");
        Serial.print(millis());
        Serial.print("] ");
        Serial.print("Read PMS , PMS_TIME_FRESH: ");
        Serial.print(PMS_TIME_FRESH);
        Serial.print(", PMS_TIME_LIMIT: ");
        Serial.print(PMS_TIME_LIMIT);

        // pms.wakeUp();
        // delay(10);
        pms.request();
        // while (!pms.read()){
        //     // return false;
        //     PMS_TIME_FRESH = millis();
        // }

        // PMS_TIME_FRESH += PMS_TIME_LIMIT;

        if (!pms.read()){
            if (PMS_TIME_FRESH != 0) {
                PMS_TIME_FRESH += 5000;
            }
            // else {
            //     PMS_TIME_FRESH = millis();
            // }
            // pms.sleep();

            Serial.println(", not get PMS data");
            return false;
        }
        PMS_TIME_FRESH += PMS_TIME_LIMIT;
        // pms.sleep();

        Serial.println(", get PMS data");
        return true;

        // return pms.read();
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
    pms.wakeUp();

    setTimeLimit(BLINKER_PMS_LIMIT_INIT);
}
