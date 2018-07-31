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
#define BLINKER_BUTTON_PULLDOWN
#define BLINKER_BUTTON_LONGPRESS_POWERDOWN
#define BLINKER_BUTTON_PIN 12

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
 * When get a command and device not parsed this command, 
 * device will call this function with a JsonObject data.
 */
bool dataParse(const JsonObject & data)
{
    String getData;
    data.printTo(getData);
    BLINKER_LOG2("Get user command: ", getData);

    bool isParsed = false;

    if (data.containsKey(BLINKER_CMD_SET)) {
        String setData = data[BLINKER_CMD_SET];

        DynamicJsonBuffer jsonBuffer;
        JsonObject& setJson = jsonBuffer.parseObject(setData);

        if (setJson.containsKey("langauage")) {
            if (setJson["langauage"] == "cn") {
                setLanguage(BLINKER_LANGUAGE_CN);
            }
            else {
                setLanguage(BLINKER_LANGUAGE_EN);
            }

            isParsed = true;
        }
        if (setJson.containsKey("aqibase")) {
            if (setJson["aqibase"] == "us") {
                setAQIbase(BLINKER_AQI_BASE_US);
            }
            else {
                setAQIbase(BLINKER_AQI_BASE_CN);
            }

            isParsed = true;
        }
        if (setJson.containsKey("contrast")) {
            uint8_t _contrast = setJson["contrast"];

            setContrast(_contrast);

            isParsed = true;
        }
    }
    else if (data.containsKey(BLINKER_CMD_GET)) {
        String getData = data[BLINKER_CMD_GET];

        DynamicJsonBuffer jsonBuffer;
        JsonObject& getJson = jsonBuffer.parseObject(getData);
    }

    if (isParsed) {
        return true;
    }
    else {
        return false;
    }
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

    String values = "{\"pm1.0\":" + STRING_format(pm1_0Get()) + \
                    ",\"pm2.5\":" + STRING_format(pm2_5Get()) + \
                    ",\"pm10\":" + STRING_format(pm10_0Get()) + \
                    ",\"hcho\":" + STRING_format(hchoGet()) + \
                    ",\"temp\":" + STRING_format(tempGet()) + \
                    ",\"humi\":" + STRING_format(humiGet()) + \
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

/* 
 * Add your code in this function
 * 
 * When long press start, device will call this function
 */
// void longPressStart()
// {
//     // isReset = true;
//     freshDisplay();

//     BLINKER_LOG1("Button long press start!");
// }

void longPressPowerdown()
{
    freshDisplay();

    BLINKER_LOG1("Button long press powerdown!");
}

void longPressReset()
{
    isReset = true;
    freshDisplay();

    BLINKER_LOG1("Button long press reset!");
}
#endif

void getBAT()
{
    int sensorValue = analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    float voltage = sensorValue * (5.926 / 1023.0 / 8.0);
    BLINKER_LOG2("bat: ", voltage);

}

void AQI_init()
{
    Serial.begin(115200);

    pinMode(BLINKER_POWER_3V3_PIN, OUTPUT);
    digitalWrite(BLINKER_POWER_3V3_PIN, HIGH); //自身3.3V控制
    pinMode(BLINKER_POWER_5V_PIN, OUTPUT);
    digitalWrite(BLINKER_POWER_5V_PIN, HIGH); //5V升压控制

    u8g2Init();
    pmsInit();
    
    Blinker.begin(BLINKER_AIR_DETECTOR);

    Blinker.attachParse(dataParse);
    Blinker.attachHeartbeat(heartbeat);

#if defined(BLINKER_BUTTON)
    Blinker.attachClick(singalClick);
    Blinker.attachDoubleClick(doubleClick);
    // Blinker.attachLongPressStart(longPressStart);
    Blinker.attachLongPressPowerdown(longPressPowerdown);
    Blinker.attachLongPressReset(longPressReset);

    attachInterrupt(BLINKER_BUTTON_PIN, buttonTick, CHANGE);
#endif

    attachDisplay(display);
    attachColor(aqiLevelGet);
}

uint32_t fresh = 0;

void AQI_run()
{
    Blinker.run();

    aqiFresh();

    if ((millis() - fresh) > 10000)
    {
        getBAT();
        fresh = millis();
    }
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
