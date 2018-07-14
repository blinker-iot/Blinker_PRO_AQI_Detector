#ifndef BLINKER_AQI_DISPLAY_H
#define BLINKER_AQI_DISPLAY_H

#include "AQI_font.h"

#include <ESP8266WiFi.h>

// U8g2lib, https://github.com/olikraus/U8g2_Arduino
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /*reset Pin*/ D0);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /*reset Pin*/ BLINKER_OLED_RESET_PIN);

static bool isDetail = true;
static bool LANG_TYPE = false;

String display_num_len_3(uint16_t num) {
    num = min(num, (uint16_t)999);

    String reNum = "";

    for(uint8_t t = 3; t > 0; t--) {
        reNum += String((uint16_t)(num / pow(10, t-1)) % 10);
    }

    return reNum;
}

String display_num_len_2(uint8_t num) {
    String reNum = "";

    for(uint8_t t = 2; t > 0; t--) {
        reNum += String((uint8_t)(num / pow(10, t-1)) % 10);
    }

    return reNum;
}

uint8_t getSignals() {
    
    if (WiFi.status() == WL_CONNECTED) {
        int32_t wRSSI = WiFi.RSSI();

        // IOT_DEBUG_PRINT2(F("getSignals: "), wRSSI);

        if (wRSSI < -90) {
            return 0;
        }
        else if (wRSSI >= -90 && wRSSI < -80) {
            return 1;
        }
        else if (wRSSI >= -80 && wRSSI < -70) {
            return 2;
        }
        else if (wRSSI >= -70) {
            return 3;
        }
    }
    else {
        return 0;
    }
}

void drawAQI(uint16_t PM10, uint16_t PM25, uint16_t PM100, double HUMI, 
            double HCHO, double TEMP, int8_t HOUR, int8_t MIN) {
    String pm10data = display_num_len_3(PM10);
    String pm25data = display_num_len_3(PM25);
    String pm100data = display_num_len_3(PM100);
    String tempdata = String((int8_t)TEMP);
    String humidata = String((int16_t)HUMI);
    // String fadata = display_num_len_3(random(0,120));
    String hour = display_num_len_2(HOUR);
    String mins = display_num_len_2(MIN);

    if (isDetail) {
        if (LANG_TYPE) {
            u8g2.setFont(u8g2_font_helvR10_tf);
            u8g2.setCursor(0, 13);
            u8g2.print("PM1.0:" + pm10data);
            u8g2.setCursor(0, 27);
            u8g2.print("PM2.5:" + pm25data);
            u8g2.setCursor(0, 41);
            u8g2.print("PM10:" + pm100data);

            u8g2.setCursor(80, 13);
            u8g2.print("FA:" + String(HCHO));
            u8g2.setCursor(80, 27);
            u8g2.print("RH:" + humidata + "%");
            u8g2.setCursor(80, 41);
            u8g2.print("TP:" + tempdata + "°");//°C
        }
        else {
            u8g2.setFont(u8g2_font_helvR10_tf);
            u8g2.setCursor(0, 13);
            u8g2.print("PM1.0:" + pm10data);
            u8g2.setCursor(0, 27);
            u8g2.print("PM2.5:" + pm25data);
            u8g2.setCursor(0, 41);
            u8g2.print("PM10:" + pm100data);

            u8g2.setCursor(98, 13);
            u8g2.print(":" + String(HCHO));
            u8g2.setCursor(98, 27);
            u8g2.print(":" + humidata + "%");
            u8g2.setCursor(98, 41);
            u8g2.print(":" + tempdata + "°");//°C

            u8g2.setFont(u8g2_font_wqy14_cn);//31x34
            u8g2.setCursor(72, 12);
            u8g2.print("甲醛");
            u8g2.setCursor(72, 26);
            u8g2.print("湿度");
            u8g2.setCursor(72, 40);
            u8g2.print("温度");
        }
    }
    else {
        u8g2.setFont(u8g2_font_fzht48_tf);//31x34
        u8g2.setCursor((128 - 3*fzht48_w)/2, 41);
        u8g2.print(pm25data);
    }
    u8g2.drawLine(0, 46, 128, 46);

    u8g2.setFont(u8g2_font_helvR10_tf);//14x15 u8g2_font_helvR10_tf
    u8g2.setCursor(0, 63);
    u8g2.print(hour + ":" + mins);
    
    // if (getTimerTimingState() == "true")
    //     u8g2.drawXBMP(95, 51, 11, 12, alarmClock);
    u8g2.drawXBMP(110, 51, 17, 12, signalSymbols[getSignals()]);//signalSymbols[(uint8_t)random(0,4)]
    u8g2.sendBuffer();
}

void initPage()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvR24_tr);
    u8g2.setCursor(18, 34);
    u8g2.print("blinker");
    u8g2.drawLine(0, 46, 128, 46);
    u8g2.setFont(u8g2_font_helvR10_te);
    u8g2.setCursor(0, 63);
    u8g2.print("blinker AQI detector");
    u8g2.sendBuffer();
}

void u8g2Init()
{
    u8g2.begin();
    u8g2.setFlipMode(0);
    u8g2.enableUTF8Print();    

    initPage();
}

#endif
