#ifndef BLINKER_AQI_H
#define BLINKER_AQI_H

#include "arduino.h"

void getBAT();

void AQI_init();

void AQI_run();

void changeMain();

void display();

bool checkInit();

void aqiFresh();

#endif
