#ifndef BLINKER_AQI_H
#define BLINKER_AQI_H

/* 
 * BLINKER_PRO is use for professional device
 * 
 * Please make sure you have permission to modify professional device!
 * Please read usermanual first! Thanks!
 * https://doc.blinker.app/
 * 
 * Written by i3water for blinker.
 * Learn more:https://blinker.app/
 */

#define BLINKER_PRINT Serial
#define BLINKER_PRO
#define BLINKER_DEBUG_ALL

#define BLINKER_BUTTON
#define BLINKER_BUTTON_PIN D7
#define BLINKER_PMS_RX_PIN D8
#define BLINKER_PMS_TX_PIN D9
#define BLINKER_OLED_RESET_PIN D3

#include <Blinker.h>

#include "AQI_display.h"
#include "AQI_sensor.h"

static bool inited = false;

// #define BUTTON_1 "ButtonKey"

/* 
 * Add your command parse code in this function
 * 
 * When get a command and device not parsed this command, device will call this function
 */
bool dataParse(const JsonObject & data) {
    String getData;

    data.printTo(getData);
    
    BLINKER_LOG2("Get user command: ", getData);
    return true;
}

/* 
 * Add your heartbeat message detail in this function
 * 
 * When get heartbeat command {"get": "state"}, device will call this function
 * For example, you can print message back
 * 
 * Every 30s will get a heartbeat command from app 
 */
void heartbeat() {
    Blinker.print("hello", "blinker");

    BLINKER_LOG1("heartbeat!");
}

#if defined(BLINKER_BUTTON)
/* 
 * Blinker provide a button parse function for user if you defined BLINKER_BUTTON
 * 
 * Blinker button can detect singal click/ double click/ long press
 * 
 * Blinker.tick() will run by default, use interrupt will be better
 */
void buttonTick() {
    Blinker.tick();
}

/* 
 * Add your code in this function
 * 
 * When button clicked, device will call this function
 */
void singalClick() {
    BLINKER_LOG1("Button clicked!");
}

/* 
 * Add your code in this function
 * 
 * When button double clicked, device will call this function
 */
void doubleClick() {
    BLINKER_LOG1("Button double clicked!");
}
#endif

// void drawAQI() {
//     String pm10data = parse3num(PM10);
//     String pm25data = parse3num(PM25);
//     String pm100data = parse3num(PM100);
//     String tempdata = String((int8_t)TEMP);
//     String humidata = String(HUMI);
//     // String fadata = parse3num(random(0,120));
//     String hour = parse2num(timeDisplay.tm_hour);
//     String mins = parse2num(timeDisplay.tm_min);

//     if (isDetail) {
//         if (LANG_TYPE) {
//             u8g2.setFont(u8g2_font_helvR10_tf);
//             u8g2.setCursor(0, 13);
//             u8g2.print("PM1.0:" + pm10data);
//             u8g2.setCursor(0, 27);
//             u8g2.print("PM2.5:" + pm25data);
//             u8g2.setCursor(0, 41);
//             u8g2.print("PM10:" + pm100data);

//             u8g2.setCursor(80, 13);
//             u8g2.print("FA:" + String(HCHO));
//             u8g2.setCursor(80, 27);
//             u8g2.print("RH:" + humidata + "%");
//             u8g2.setCursor(80, 41);
//             u8g2.print("TP:" + tempdata + "°");//°C
//         }
//         else {
//             u8g2.setFont(u8g2_font_helvR10_tf);
//             u8g2.setCursor(0, 13);
//             u8g2.print("PM1.0:" + pm10data);
//             u8g2.setCursor(0, 27);
//             u8g2.print("PM2.5:" + pm25data);
//             u8g2.setCursor(0, 41);
//             u8g2.print("PM10:" + pm100data);

//             u8g2.setCursor(98, 13);
//             u8g2.print(":" + String(HCHO));
//             u8g2.setCursor(98, 27);
//             u8g2.print(":" + humidata + "%");
//             u8g2.setCursor(98, 41);
//             u8g2.print(":" + tempdata + "°");//°C

//             u8g2.setFont(u8g2_font_wqy14_cn);//31x34
//             u8g2.setCursor(72, 12);
//             u8g2.print("甲醛");
//             u8g2.setCursor(72, 26);
//             u8g2.print("湿度");
//             u8g2.setCursor(72, 40);
//             u8g2.print("温度");
//         }
//     }
//     else {
//         u8g2.setFont(u8g2_font_fzht48_tf);//31x34
//         u8g2.setCursor((128 - 3*fzht48_w)/2, 41);
//         u8g2.print(pm25data);
//     }
//     u8g2.drawLine(0, 46, 128, 46);

//     u8g2.setFont(u8g2_font_helvR10_tf);//14x15 u8g2_font_helvR10_tf
//     u8g2.setCursor(0, 63);
//     u8g2.print(hour + ":" + mins);
    
//     if (getTimerTimingState() == "true")
//         u8g2.drawXBMP(95, 51, 11, 12, alarmClock);
//     u8g2.drawXBMP(110, 51, 17, 12, signalSymbols[getSignals()]);//signalSymbols[(uint8_t)random(0,4)]
//     u8g2.sendBuffer();
// }

void display()
{
    u8g2.firstPage();
    do {
        // drawAQI();
        drawAQI(pms.getPmAto(1.0), pms.getPmAto(2.5), pms.getPmAto(10), pms.getHumi(),
                pms.getForm(), pms.getTemp(), Blinker.hour(), Blinker.minute());
    } while ( u8g2.nextPage() );
}

bool checkInit()
{
    if (!inited) {
        if (Blinker.inited()) {
            setTimeLimit(1000UL);
            inited = true;
            return true;
        }
        return false;
    }
    else {
        return true;
    }
}

void AQI_init() {
    Serial.begin(115200);

    u8g2Init();
    pmsInit();
    
    Blinker.begin(BLINKER_AIR_DETECTOR);

    Blinker.attachParse(dataParse);
    Blinker.attachHeartbeat(heartbeat);

#if defined(BLINKER_BUTTON)
    Blinker.attachClick(singalClick);
    Blinker.attachDoubleClick(doubleClick);    
    attachInterrupt(BLINKER_BUTTON_PIN, buttonTick, CHANGE);
#endif
}

void AQI_run()
{
    Blinker.run();

    // pmsFresh();
    if (!checkInit()) {
        if (pmsFresh()) {
            display();
        }
    }
    else {
        pmsFresh();
        display();
    }
}

#endif
