#ifndef BLINKER_AQI_CONFIG_H
#define BLINKER_AQI_CONFIG_H

#include <Arduino.h>

#define BLINKER_NONE_LONG_PRESS 0
#define BLINKER_LONG_PRESS_START 1
#define BLINKER_LONG_PRESS_POWER_DOWN 2
#define BLINKER_LONG_PRESS_RESET 3

#define BLINKER_AQI_BASE_US 0
#define BLINKER_AQI_BASE_CN 1

#define BLINKER_LANGUAGE_CN 0
#define BLINKER_LANGUAGE_EN 1

#define BLINKER_PMS_RX_PIN 4
#define BLINKER_PMS_TX_PIN 5

#define BLINKER_WS2812_PIN 13

#define BLINKER_POWER_3V3_PIN 14
#define BLINKER_POWER_5V_PIN 15

#define BLINKER_IIC_SCK_PIN 2
#define BLINKER_IIC_SDA_PIN 0

#define BLINKER_BAT_POWER_LOW 3.5
#define BLINKER_BAT_POWER_HIGH 4.0
#define BLINKER_BAT_POWER_USEUP 0.1
#define BLINKER_BAT_CHECK_TIME 1000UL

#define BLINKER_PMS_LIMIT_INIT 30000UL
#define BLINKER_PMS_LIMIT_FRESH 1000UL

#define BLINKER_PMS_SLEEP_TIME 60UL
#define BLINKER_PMS_WAKE_TIME  300UL

#define BLINKER_AQI_FRESH_TIME 60

#define BLINKER_OLED_RESET_PIN U8X8_PIN_NONE

#endif