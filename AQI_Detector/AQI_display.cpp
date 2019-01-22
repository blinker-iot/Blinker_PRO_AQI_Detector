#include "AQI_display.h"
#include "AQI_font.h"

// #include <ESP8266WiFi.h>
#include <Ticker.h>

// U8g2lib, https://github.com/olikraus/U8g2_Arduino
// Release version 2.24.3
#include <U8g2lib.h>

#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

// U8G2_SSD1306_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, /*reset Pin*/ D0);
// U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /*reset Pin*/ BLINKER_OLED_RESET_PIN);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0,/* reset=*/ BLINKER_OLED_RESET_PIN, BLINKER_IIC_SCK_PIN, BLINKER_IIC_SDA_PIN);

#include <Adafruit_NeoPixel.h>

#define NUMPIXELS 1

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, BLINKER_WS2812_PIN, NEO_GRB + NEO_KHZ800);

static bool isDisplayDetail = true;
static bool isNormalDisplay = true;
static double batGet;
static uint8_t displayLanguage = BLINKER_LANGUAGE_CN;
static uint8_t initProgressBar = 0;
static uint8_t targetContrast = 255;
static uint8_t AQI_BASE = BLINKER_AQI_BASE_CN;
static uint8_t wlanLevel = 0;
static callbackFunction _diplayFunc;
static callbackFunction_arg_u8 _colorFunc;
static uint32_t freshTime = 0;

Ticker ledTicker;

static bool isAuth = false;
static bool isBlink = false;
static bool isConnected = false;

void setContrast(uint8_t _contrast)
{
    targetContrast = _contrast;
}

uint8_t getContrast()
{
    return targetContrast;
}

void setAQIbase(uint8_t _base)
{
    if (_base > BLINKER_AQI_BASE_CN) {
        AQI_BASE = BLINKER_AQI_BASE_US;
    }
    else {
        AQI_BASE = _base;
    }
}

String getAQIbase()
{
    switch(AQI_BASE) {
        case BLINKER_AQI_BASE_US :
            return "us";
        case BLINKER_AQI_BASE_CN :
            return "cn";
        default :
            return "us";
    } 
}

void setLanguage(uint8_t _lang)
{
    if (_lang > BLINKER_LANGUAGE_EN) {
        displayLanguage = BLINKER_LANGUAGE_CN;
    }
    else {
        displayLanguage = _lang;
    }
}

String getLanguage()
{
    switch(displayLanguage) {
        case BLINKER_LANGUAGE_CN :
            return "cn";
        case BLINKER_LANGUAGE_EN :
            return "en";
        default :
            return "cn";
    }
}

void attachDisplay(callbackFunction _func)
{
    _diplayFunc = _func;
}

void attachColor(callbackFunction_arg_u8 _func)
{
    _colorFunc = _func;
}

void freshDisplay()
{
    if ((millis() - freshTime) >= 1000 || freshTime == 0) {
        u8g2.setContrast(targetContrast);

        u8g2.firstPage();
        do {
            if (_diplayFunc) {
                _diplayFunc();
            }
        } while ( u8g2.nextPage() );

        freshTime += 1000;
    }

    // colorDisplay();
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

void setSignals(uint8_t level)
{
    wlanLevel = level;
}

// uint8_t getSignals()
// {
    
//     if (WiFi.status() == WL_CONNECTED) {
//         int32_t wRSSI = WiFi.RSSI();

//         // IOT_DEBUG_PRINT2(F("getSignals: "), wRSSI);

//         if (wRSSI < -90) {
//             return 0;
//         }
//         else if (wRSSI >= -90 && wRSSI < -80) {
//             return 1;
//         }
//         else if (wRSSI >= -80 && wRSSI < -70) {
//             return 2;
//         }
//         else if (wRSSI >= -70) {
//             return 3;
//         }
//     }
//     else {
//         return 0;
//     }
// }

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

void disconnectBlink()
{
    if (isAuth) {
        // digitalWrite(PLUGIN_GREEN_LED_PIN, !digitalRead(PLUGIN_GREEN_LED_PIN));
        // digitalWrite(PLUGIN_RED_LED_PIN, HIGH);
        // uint8_t R = 0, G = 0, B = 0;
        uint32_t color = pixels.getPixelColor(0);
        if (color >> 8 & 0xFF) {
            pixels.setPixelColor(0, pixels.Color(0, 0, 0)); 
            pixels.show();
        }
        else {
        // R = color >> 16 & 0xFF;
        // G = color >> 8  & 0xFF;
        // B = color       & 0xFF;
            pixels.setPixelColor(0, pixels.Color(0, 255, 0)); 
            pixels.show();
        }        
    }
    else {
        if (isConnected) {
            uint32_t color = pixels.getPixelColor(0);
            if (color >> 16 & 0xFF) {
                pixels.setPixelColor(0, pixels.Color(0, 0, 0)); 
                pixels.show();
            }
            else {
                pixels.setPixelColor(0, pixels.Color(255, 255, 0)); 
                pixels.show();
            }
        }
        else {
            uint32_t color = pixels.getPixelColor(0);
            if (color >> 16 & 0xFF) {
                pixels.setPixelColor(0, pixels.Color(0, 0, 0)); 
                pixels.show();
            }
            else {
                pixels.setPixelColor(0, pixels.Color(255, 0, 0)); 
                pixels.show();
            }
        }
    }

    ledTicker.once(0.5, disconnectBlink);
}

// void connectBright(bool state)
// {
//     detachBlink();
//     // if (state && digitalRead(PLUGIN_GREEN_LED_PIN)) {
//     if (state) {
//         // digitalWrite(PLUGIN_GREEN_LED_PIN, LOW);
//         // digitalWrite(PLUGIN_RED_LED_PIN, HIGH);

//         pixels.setPixelColor(0, pixels.Color(0, 255, 0)); 
//         pixels.show();
//     }
//     else if (!state && digitalRead(PLUGIN_RED_LED_PIN)) {
//         // digitalWrite(PLUGIN_GREEN_LED_PIN, HIGH);
//         // digitalWrite(PLUGIN_RED_LED_PIN, LOW);
//         pixels.setPixelColor(0, pixels.Color(255, 0, 0)); 
//         pixels.show();
//     }
// }

void attachBlink()
{
    if (!isBlink) {
        ledTicker.once(0.5, disconnectBlink);
        isBlink = true;
    }
}

void detachBlink()
{
    if (isBlink) {
        ledTicker.detach();
        isBlink = false;
    }
}

void setColorType(uint8_t type)
{
    switch (type) {
        case NORMAL :
            isNormalDisplay = true;
            detachBlink();
            break;
        case WLAN_CONNECTING :
            isNormalDisplay = false;
            // detachBlink();
            isAuth = false;
            isConnected = false;
            attachBlink();
            break;
        case WLAN_CONNECTED :
            isNormalDisplay = false;
            // detachBlink();
            isAuth = false;
            isConnected = true;
            attachBlink();
            break;
        case DEVICE_CONNECTING :
            isNormalDisplay = false;
            // detachBlink();
            isAuth = true;
            isConnected = false;
            attachBlink();
            break;
        case DEVICE_CONNECTED :
            isNormalDisplay = true;
            detachBlink();
            break;
        default :
            isNormalDisplay = true;
            detachBlink();
            break;
    }
}

void colorDisplay()
{
    uint8_t R = 0, G = 0, B = 0;
    // switch (AQIBUFFER[AQI_BASE][1]) {
    
    if (isNormalDisplay) {
        if (!_colorFunc) {
            return;
        }
        switch(_colorFunc(AQI_BASE)) {
            case 0: /*IOT_DEBUG_PRINT1("color Green");*/    
                R = 0;                              
                G = map(targetContrast, 0, 255, 0, 64);  
                B = 0;                              
                pixels.setPixelColor(0, pixels.Color(R, G, B)); 
                pixels.show(); 
                break;
            case 1: /*IOT_DEBUG_PRINT1("color Yellow");*/   
                R = map(targetContrast, 0, 255, 0, 64);  
                G = map(targetContrast, 0, 255, 0, 64);  
                B = 0;                              
                pixels.setPixelColor(0, pixels.Color(R, G, B)); 
                pixels.show(); 
                break;
            case 2: /*IOT_DEBUG_PRINT1("color Orange");*/   
                R = map(targetContrast, 0, 255, 0, 64);  
                G = map(targetContrast, 0, 255, 0, 32);  
                B = 0;                              
                pixels.setPixelColor(0, pixels.Color(R, G, B)); 
                pixels.show(); 
                break;
            case 3: /*IOT_DEBUG_PRINT1("color Red");*/      
                R = map(targetContrast, 0, 255, 0, 64);  
                G = 0;                              
                B = 0;                              
                pixels.setPixelColor(0, pixels.Color(R, G, B)); 
                pixels.show(); 
                break;
            case 4: /*IOT_DEBUG_PRINT1("color Purple");*/   
                R = map(targetContrast, 0, 255, 0, 32);  
                G = 0;                              
                B = map(targetContrast, 0, 255, 0, 32);  
                pixels.setPixelColor(0, pixels.Color(R, G, B)); 
                pixels.show(); 
                break;
            case 5: /*IOT_DEBUG_PRINT1("color Maroon");*/   
                R = map(targetContrast, 0, 255, 0, 32);  
                G = map(targetContrast, 0, 255, 0, 4);   
                B = map(targetContrast, 0, 255, 0, 6);   
                pixels.setPixelColor(0, pixels.Color(R, G, B)); 
                pixels.show(); 
                break;
        }
    }
}

void batDisplay(double _bat)
{
    if (_bat > BLINKER_BAT_POWER_HIGH) {
        _bat = BLINKER_BAT_POWER_HIGH;
    }

    batGet = _bat;

    uint8_t drawBat = (_bat - BLINKER_BAT_POWER_LOW) / (BLINKER_BAT_POWER_HIGH - BLINKER_BAT_POWER_LOW) * 14;

    if (drawBat > 14) drawBat = 14;

    u8g2.drawRFrame(108, 52, 18, 10, 2);
    u8g2.drawBox(110, 54, drawBat, 6);
    u8g2.drawLine(127, 55, 127, 58);
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
            u8g2.print("PM10:" + pm100data);

            u8g2.setCursor(78, 13);
            u8g2.print("FA:" + String(_hcho));
            u8g2.setCursor(78, 27);
            u8g2.print("RH:" + humidata + "%");
            u8g2.setCursor(78, 41);
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
    u8g2.print(hour + ":" + mins + " " + millis()/1000/60 + ":" + millis()/1000%60);
    // u8g2.print(hour + ":" + mins);
    
    // if (getTimerTimingState() == "true")
    //     u8g2.drawXBMP(95, 51, 11, 12, alarmClock);
    // u8g2.drawXBMP(110, 51, 17, 12, signalSymbols[getSignals()]);//signalSymbols[(uint8_t)random(0,4)]
    u8g2.drawXBMP(88, 51, 17, 12, signalSymbols[wlanLevel]);//signalSymbols[(uint8_t)random(0,4)]
    // batDisplay();
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
    u8g2.print("PM2.5 " + pm25data + " " + millis()/1000/60 + ":" + millis()/1000%60);

    // u8g2.print("PM2.5 " + pm25data);

    // if (getTimerTimingState() == "true")
    //     u8g2.drawXBMP(95, 51, 11, 12, alarmClock);
    // u8g2.drawXBMP(110, 51, 17, 12, signalSymbols[getSignals()]);
    u8g2.drawXBMP(88, 51, 17, 12, signalSymbols[wlanLevel]);
    // batDisplay();
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
    u8g2.setCursor(1, 63);
    u8g2.print("blinker AQI detector");
    u8g2.sendBuffer();
}

void clearPage()
{
    u8g2.clearBuffer();
    u8g2.sendBuffer();
}

void resetDisplay(uint16_t _time)
{
    detachBlink();
    
    if (displayLanguage) {
        pixels.setPixelColor(0, pixels.Color(0, 64, 0)); 
        pixels.show();
        u8g2.setFont(u8g2_font_helvR10_te);
        u8g2.setCursor(64 - helvR10_w * 5, 20);
        u8g2.print("Power down");
        u8g2.setCursor(26, 45);
        u8g2.print("Realse to reset");
        u8g2.setCursor(4, 45);
        u8g2.print(String(10 - _time / 1000)+"s");

        u8g2.drawLine(0, 54, 128 * _time / 10000, 54);
        // u8g2.clearBuffer();
        // u8g2.setFont(u8g2_font_helvR24_tr);
        // u8g2.setCursor(18, 34);
        // u8g2.print("blinker");
        // u8g2.drawLine(0, 46, 128, 46);
        // u8g2.setFont(u8g2_font_helvR10_te);
        // // u8g2.setCursor(40, 63);
        // u8g2.setCursor(0, 63);
        // u8g2.print("PowerDown / Reset!");
        // u8g2.sendBuffer();
    }
    else {
        pixels.setPixelColor(0, pixels.Color(0, 64, 0)); 
        pixels.show();
        u8g2.setFont(u8g2_font_wqy14_cn);
        u8g2.setCursor(64 - wqy14_w * 3, 20);
        u8g2.print("释放即可关机");
        u8g2.setCursor(64 - wqy14_w * 2, 45);
        u8g2.print("后释放将重置");

        u8g2.setFont(u8g2_font_helvR10_tf);//14x15 u8g2_font_helvR10_tf
        u8g2.setCursor(64 - wqy14_w * 2 - helvR10_w * 3, 45);
        u8g2.print(String(10 - _time / 1000)+"s");

        u8g2.drawLine(0, 54, 128 * _time / 10000, 54);
        // // u8g2.clearBuffer();
        // u8g2.setFont(u8g2_font_helvR24_tr);
        // u8g2.setCursor(18, 34);
        // u8g2.print("blinker");
        // u8g2.drawLine(0, 46, 128, 46);
        // u8g2.setFont(u8g2_font_helvR10_te);
        // // u8g2.setCursor(40, 63);
        // u8g2.setCursor(0, 63);
        // u8g2.print("PowerDown / Reset!");
        // // u8g2.sendBuffer();
    }
}

bool initDisplay()
{
    if (initProgressBar > 128) {
        return false;
    }

    u8g2.drawLine(0, 46, initProgressBar, 46);
    u8g2.sendBuffer();
    initProgressBar += 4;
    
    return true;
}

void u8g2Init()
{
    pixels.begin();

    u8g2.begin();
    u8g2.setFlipMode(0);
    u8g2.enableUTF8Print();

    pixels.setPixelColor(0, pixels.Color(0, 64, 0)); 
    pixels.show();

    initPage();
}