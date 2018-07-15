#include "AQI_config.h"
#include "AQI_display.h"
#include "AQI_font.h"

#include <ESP8266WiFi.h>

// U8g2lib, https://github.com/olikraus/U8g2_Arduino
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /*reset Pin*/ D0);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /*reset Pin*/ BLINKER_OLED_RESET_PIN);

static bool isDisplayDetail = false;
static uint8_t displayLanguage = BLINKER_LANGUAGE_CN;
static uint8_t initProgressBar = 0;
static callbackFunction _diplayFunc;

void attachDisplay(callbackFunction _func)
{
    _diplayFunc = _func;
}

void freshDisplay()
{
    u8g2.firstPage();
    do {
        if (_diplayFunc) {
            _diplayFunc();
        }
        // if (!isReset) {
        //     if (isAQI) {
        //         aqiDisplay(pms.getPmAto(1.0), pms.getPmAto(2.5),
        //                 pms.getPmAto(10), pms.getHumi(),
        //                 pms.getForm(), pms.getTemp(),
        //                 Blinker.hour(), Blinker.minute());
        //     }
        //     else {
        //         timeDisplay(pms.getPmAto(2.5), Blinker.month(), 
        //                 Blinker.mday(), Blinker.wday(), 
        //                 Blinker.hour(), Blinker.minute());
        //     }
        // }
        // else {
        //     resetDisplay();
        // }
    } while ( u8g2.nextPage() );
}

void changeDetail()
{
    isDisplayDetail = !isDisplayDetail;
}

String display_num_len_3(uint16_t num)
{
    num = min(num, (uint16_t)999);

    String reNum = "";

    for(uint8_t t = 3; t > 0; t--) {
        reNum += String((uint16_t)(num / pow(10, t-1)) % 10);
    }

    return reNum;
}

String display_num_len_2(uint8_t num)
{
    String reNum = "";

    for(uint8_t t = 2; t > 0; t--) {
        reNum += String((uint8_t)(num / pow(10, t-1)) % 10);
    }

    return reNum;
}

uint8_t getSignals()
{
    
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

static String weekDays(uint8_t weekDay)
{
    switch (weekDay) {
        case 0: return displayLanguage ? F("Sun") : F("星期日");
        case 1: return displayLanguage ? F("Mon") : F("星期一");
        case 2: return displayLanguage ? F("Tues"): F("星期二");
        case 3: return displayLanguage ? F("Wed") : F("星期三");
        case 4: return displayLanguage ? F("Thur"): F("星期四");
        case 5: return displayLanguage ? F("Fri") : F("星期五");
        case 6: return displayLanguage ? F("Sat") : F("星期六");
    }
}

static String months(uint8_t mons) {
    switch (mons) {
        case 0: return F("Jan");
        case 1: return F("Feb");
        case 2: return F("Mar");
        case 3: return F("Apr");
        case 4: return F("May");
        case 5: return F("Jun");
        case 6: return F("July");
        case 7: return F("Aug");
        case 8: return F("Sept");
        case 9: return F("Oct");
        case 10: return F("Nov");
        case 11: return F("Dec");
    }
}

void aqiDisplay(uint16_t _pm1_0, uint16_t _pm2_5, uint16_t _pm10_0, double _humi, 
            double _hcho, double _temp, int8_t _hour, int8_t _min)
{
    String pm10data = display_num_len_3(_pm1_0);
    String pm25data = display_num_len_3(_pm2_5);
    String pm100data = display_num_len_3(_pm10_0);
    String tempdata = String((int8_t)_temp);
    String humidata = String((int16_t)_humi);
    // String fadata = display_num_len_3(random(0,120));
    String hour = display_num_len_2(abs(_hour));
    String mins = display_num_len_2(abs(_min));

    if (isDisplayDetail) {
        if (displayLanguage) {
            u8g2.setFont(u8g2_font_helvR10_tf);
            u8g2.setCursor(0, 13);
            u8g2.print("PM1.0:" + pm10data);
            u8g2.setCursor(0, 27);
            u8g2.print("PM2.5:" + pm25data);
            u8g2.setCursor(0, 41);
            u8g2.print("_pm1_0:" + pm100data);

            u8g2.setCursor(80, 13);
            u8g2.print("FA:" + String(_hcho));
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
            u8g2.print(":" + String(_hcho));
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

void timeDisplay(uint16_t _pm2_5, int8_t _mon, int8_t _mday, 
            int8_t _wday, int8_t _hour, int8_t _min)
{
    String pm25data = display_num_len_3(_pm2_5);
    String weekDay = weekDays(abs(_wday));
    String hour = display_num_len_2(abs(_hour));
    String mins = display_num_len_2(abs(_min));
    String mon;
    String day;
    
    if (isDisplayDetail) {
        u8g2.setFont(u8g2_font_helvR10_tf);
        u8g2.setCursor((128 - ((hour.length() + mins.length())*helvR10_w + 4))/2, 16);
        u8g2.print(hour + ":" + mins);

        // u8g2.setCursor((128 - (4*helvR10_w))/2, 41);
        // u8g2.print(timeDisplay.tm_year + 1900);

        uint8_t start_w;

        if (displayLanguage) {
            mon = months(abs(_mon));
            day = String(abs(_mday));

            start_w = (128 - ((weekDay.length() + mon.length() + day.length())*helvR10_w + 12))/2;

            // String weekDay = weekDaysEN[(uint8_t)random(0,7)];
            u8g2.setFont(u8g2_font_helvR10_tf);//14x15 u8g2_font_helvR10_tf
            u8g2.setCursor(start_w, 38);
            u8g2.print(weekDay+ ", " + mon + " "+ day);
        }
        else {
            mon = String(abs(_mon) + 1);
            day = String(abs(_mday));

            start_w = (128 - ((mon.length() + day.length())*helvR10_w + 5*wqy14_w + 4))/2;
            
            u8g2.setFont(u8g2_font_helvR10_tf);//14x15 u8g2_font_helvR10_tf
            u8g2.setCursor(start_w, 38);
            u8g2.print(mon);
            u8g2.setCursor(start_w + mon.length()*helvR10_w + wqy14_w, 38);
            u8g2.print(day);
            u8g2.setFont(u8g2_font_wqy14_cn);//6x13 u8g2_font_helvR10_tf//u8g2_font_wqy14_t_chinese3
            u8g2.setCursor(start_w + mon.length()*helvR10_w, 38-1);
            u8g2.print(F("月"));
            u8g2.setCursor(start_w + (mon.length() + day.length())*helvR10_w + wqy14_w, 38-1);
            u8g2.print(F("日"));
            u8g2.setCursor(start_w + (mon.length() + day.length())*helvR10_w + wqy14_w*2 + 4, 38-1);
            u8g2.print(weekDays(abs(_wday)));//weekDaysCN[(uint8_t)random(0,7)]
        }

        // displayLanguage = !displayLanguage;
    }
    else {
        u8g2.setFont(u8g2_font_fzht48_tf);//31x34
        u8g2.setCursor(63 - 6 - 2*fzht48_w, 38);
        u8g2.print(hour);
        u8g2.drawXBMP(64 - 3, 12, 6, 24, signal_risk);
        u8g2.setCursor(64 + 6, 38);
        u8g2.print(mins);
    }


    u8g2.drawLine(0, 46, 128, 46);

    u8g2.setFont(u8g2_font_helvR10_tf);//14x15 u8g2_font_helvR10_tf
    u8g2.setCursor(0, 63);
    u8g2.print("PM2.5 " + pm25data);

    // if (getTimerTimingState() == "true")
    //     u8g2.drawXBMP(95, 51, 11, 12, alarmClock);
    u8g2.drawXBMP(110, 51, 17, 12, signalSymbols[getSignals()]);
    u8g2.sendBuffer();
}

void initPage()
{
    u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvR24_tr);
    u8g2.setCursor(18, 34);
    u8g2.print("blinker");
    // u8g2.drawLine(0, 46, 128, 46);
    u8g2.setFont(u8g2_font_helvR10_te);
    u8g2.setCursor(0, 63);
    u8g2.print("blinker AQI detector");
    u8g2.sendBuffer();
}

void resetDisplay()
{
    // u8g2.clearBuffer();
    u8g2.setFont(u8g2_font_helvR24_tr);
    u8g2.setCursor(18, 34);
    u8g2.print("blinker");
    u8g2.drawLine(0, 46, 128, 46);
    u8g2.setFont(u8g2_font_helvR10_te);
    u8g2.setCursor(40, 63);
    u8g2.print("Reset ...");
    // u8g2.sendBuffer();
}

bool initDisplay()
{
    if (initProgressBar > 128) {
        return false;
    }

    u8g2.drawLine(0, 46, initProgressBar, 46);
    u8g2.sendBuffer();
    initProgressBar++;
    
    return true;
}

void u8g2Init()
{
    u8g2.begin();
    u8g2.setFlipMode(0);
    u8g2.enableUTF8Print();    

    initPage();
}