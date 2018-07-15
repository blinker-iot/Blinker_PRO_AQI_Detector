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

#include <Blinker.h>

#include "AQI_display.h"
#include "AQI_sensor.h"

static bool inited = false;
static bool isAQI = true;
static bool isReset = false;

// #define BUTTON_1 "ButtonKey"


void changeMain() {
    isAQI = !isAQI;
}

void display()
{
    // u8g2.firstPage();
    // do {
    if (!isReset) {
        if (isAQI) {
            aqiDisplay(pm1_0Get(), pm2_5Get(), pm10_0Get(), humiGet(),
                    hchoGet(), tempGet(), Blinker.hour(), Blinker.minute());
        }
        else {
            timeDisplay(pm2_5Get(), Blinker.month(), Blinker.mday(), 
                    Blinker.wday(), Blinker.hour(), Blinker.minute());
        }
    }
    else {
        resetDisplay();
    }
    // } while ( u8g2.nextPage() );
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
void buttonTick()
{
    Blinker.tick();
}

/* 
 * Add your code in this function
 * 
 * When button clicked, device will call this function
 */
void singalClick()
{
    changeDetail();

    if (inited) {
        display();
    }

    BLINKER_LOG1("Button clicked!");
}

/* 
 * Add your code in this function
 * 
 * When button double clicked, device will call this function
 */
void doubleClick()
{
    changeMain();

    if (inited) {
        display();
    }

    BLINKER_LOG1("Button double clicked!");
}

void longPressStart()
{
    isReset = true;
    display();

    BLINKER_LOG1("Button long press start!");
}
#endif

void AQI_init()
{
    Serial.begin(115200);

    u8g2Init();
    pmsInit();
    
    Blinker.begin(BLINKER_AIR_DETECTOR);

    Blinker.attachParse(dataParse);
    Blinker.attachHeartbeat(heartbeat);

#if defined(BLINKER_BUTTON)
    Blinker.attachClick(singalClick);
    Blinker.attachDoubleClick(doubleClick);
    Blinker.attachLongPressStart(longPressStart);
    attachInterrupt(BLINKER_BUTTON_PIN, buttonTick, CHANGE);
#endif

    attachDisplay(display);
}

void AQI_run()
{
    Blinker.run();

    // pmsFresh();
    if (!checkInit()) {
        // if (pmsFresh()) {
        //     display();
        // }
        // else {
        // }
        if (initDisplay()) {
            Blinker.delay(5);
        }
        else if (isReset) {
            display();
        }
        else {
            pmsFresh();
            display();
        }
    }
    else {
        pmsFresh();
        display();
    }
}

#endif
