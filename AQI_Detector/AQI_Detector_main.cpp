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

#include "AQI_Detector_main.h"
#include "AQI_display.h"
#include "AQI_sensor.h"

static bool inited = false;
static bool isAQI = true;
static bool isReset = false;

/* 
 * Add your command parse code in this function
 * 
 * When get a command and device not parsed this command, device will call this function
 */
bool dataParse(const JsonObject & data)
{
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
void heartbeat()
{
    Blinker.print("aqibase", getAQIbase());
    Blinker.print("langauage", getLanguage());
    Blinker.print("timezone", Blinker.getTimezone());

    String values = "{\"pm1.0\":" + STRING_format(pm1_0Get()) + ",\"pm2.5\":" + STRING_format(pm2_5Get()) + \
                    ",\"pm10\":" + STRING_format(pm10_0Get()) + ",\"hcho\":" + STRING_format(hchoGet()) + \
                    ",\"temp\":" + STRING_format(tempGet()) + ",\"humi\":" + STRING_format(humiGet()) + \
                    ",\"AQICN\":" + STRING_format(aqiLevelGet(BLINKER_AQI_BASE_CN)) + \
                    ",\"AQIUS\":" + STRING_format(aqiLevelGet(BLINKER_AQI_BASE_US)) + "}";

    Blinker.printObject("detector", values);

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
        freshDisplay();
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
        freshDisplay();
    }

    BLINKER_LOG1("Button double clicked!");
}

void longPressStart()
{
    isReset = true;
    freshDisplay();

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
    attachColor(aqiLevelGet);
}

void AQI_run()
{
    Blinker.run();

    aqiFresh();
}

void changeMain()
{
    isAQI = !isAQI;
}

void display()
{
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

void aqiFresh()
{
    if (!checkInit()) {
        if (initDisplay()) {
            Blinker.delay(5);
        }
        else if (isReset) {
            freshDisplay();
        }
        else {
            pmsFresh();
            freshDisplay();
        }
    }
    else {
        pmsFresh();
        freshDisplay();
    }
}
