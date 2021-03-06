#ifndef BLINKER_AQI_DISPLAY_H
#define BLINKER_AQI_DISPLAY_H

#include "AQI_config.h"

extern "C" {
    typedef void (*callbackFunction)(void);
    typedef uint8_t (*callbackFunction_arg_u8)(uint8_t arg);
}

enum LED_TYPE{
    NORMAL,
    WLAN_CONNECTING,
    WLAN_CONNECTED,
    DEVICE_CONNECTING,
    DEVICE_CONNECTED
};

void setContrast(uint8_t _contrast);

uint8_t getContrast();

void setAQIbase(uint8_t _base);

String getAQIbase();

void setLanguage(uint8_t _lang);

String getLanguage();

void attachDisplay(callbackFunction _func);

void attachColor(callbackFunction_arg_u8 _func);

void freshDisplay();

void changeDetail();

String display_num_len_3(uint16_t num);

String display_num_len_2(uint8_t num);

void setSignals(uint8_t level);

// uint8_t getSignals();

static String weekDays(uint8_t weekDay);

static String months(uint8_t mons);

void setColorType(uint8_t type);

void colorDisplay();

void batDisplay(double _bat);

void aqiDisplay(uint16_t _pm1_0, uint16_t _pm2_5, uint16_t _pm10_0, double _humi, 
            double _hcho, double _temp, int8_t _hour, int8_t _min);

void timeDisplay(uint16_t _pm2_5, int8_t _mon, int8_t _mday, 
            int8_t _wday, int8_t _hour, int8_t _min);

void initPage();

void clearPage();

void resetDisplay(uint16_t _time);

bool initDisplay();

void u8g2Init();

#endif
