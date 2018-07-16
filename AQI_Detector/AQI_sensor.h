#ifndef BLINKER_AQI_SENSOR_H
#define BLINKER_AQI_SENSOR_H

#include "AQI_config.h"

uint16_t pm1_0Get();

uint16_t pm2_5Get();

uint16_t pm10_0Get();

double humiGet();

double hchoGet();

double tempGet();

uint8_t aqiLevelGet(uint8_t _base);

void setTimeLimit(uint16_t _time);

bool pmsFresh();

void pmsInit();

#endif
