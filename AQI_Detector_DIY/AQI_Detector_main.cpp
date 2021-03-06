/* 
 * BLINKER_PRO_ESP is use for professional device
 * 
 * Please make sure you have permission to modify professional device!
 * Please read usermanual first! Thanks!
 * https://doc.blinker.app/
 * 
 * Written by i3water for blinker.
 * Learn more:https://blinker.app/
 */

#define BLINKER_PRINT Serial
#define BLINKER_MQTT
#define BLINKER_DEBUG_ALL

// #define BLINKER_BUTTON
// #define BLINKER_BUTTON_PULLDOWN
// #define BLINKER_BUTTON_LONGPRESS_POWERDOWN
// #define BLINKER_BUTTON_PIN 12

#include <Blinker.h>

char auth[] = "2f9a2bad";//
char ssid[] = "mostfun";//"Your WiFi network SSID or name";
char pswd[] = "18038083873";//"Your WiFi network WPA password or WEP key";

#include <modules/OneButton/OneButton.h>

#define BLINKER_BUTTON_PIN 12

OneButton button(BLINKER_BUTTON_PIN, false);

#include "AQI_Detector_main.h"
#include "AQI_display.h"
#include "AQI_sensor.h"

static bool inited = false;
static bool initDisplayed = false;
static bool isAQI = true;
static bool isLongPress = false;
static uint8_t batRead;
static uint8_t batBase;
static uint32_t batFresh = 0;
static uint32_t press_start_time;
static uint16_t pressed_time;

// /* 
//  * Add your command parse code in this function
//  * 
//  * When get a command and device not parsed this command, 
//  * device will call this function with a JsonObject data.
//  */
// bool dataParse(const JsonObject & data)
// {
//     String getData;
//     data.printTo(getData);
//     BLINKER_LOG2("Get user command: ", getData);

//     bool isParsed = false;

//     if (data.containsKey(BLINKER_CMD_SET)) {
//         String setData = data[BLINKER_CMD_SET];

//         DynamicJsonBuffer jsonBuffer;
//         JsonObject& setJson = jsonBuffer.parseObject(setData);

//         if (setJson.containsKey("langauage")) {
//             if (setJson["langauage"] == "cn") {
//                 setLanguage(BLINKER_LANGUAGE_CN);
//             }
//             else {
//                 setLanguage(BLINKER_LANGUAGE_EN);
//             }

//             isParsed = true;
//         }
//         if (setJson.containsKey("aqibase")) {
//             if (setJson["aqibase"] == "us") {
//                 setAQIbase(BLINKER_AQI_BASE_US);
//             }
//             else {
//                 setAQIbase(BLINKER_AQI_BASE_CN);
//             }

//             isParsed = true;
//         }
//         if (setJson.containsKey("contrast")) {
//             uint8_t _contrast = setJson["contrast"];

//             setContrast(_contrast);

//             isParsed = true;
//         }
//     }
//     else if (data.containsKey(BLINKER_CMD_GET)) {
//         String getData = data[BLINKER_CMD_GET];

//         DynamicJsonBuffer jsonBuffer;
//         JsonObject& getJson = jsonBuffer.parseObject(getData);
//     }

//     if (isParsed) {
//         return true;
//     }
//     else {
//         return false;
//     }
// }

/* 
 * Add your heartbeat message detail in this function
 * 
 * When get heartbeat command {"get": "state"}, device will call this function
 * For example, you can print message back
 * 
 * Every 30s will get a heartbeat command from app 
 */

#define Number_1 "pm1.0"
#define Number_2 "pm2.5"
#define Number_3 "pm10"
#define Number_4 "hcho"
#define Number_5 "temp"
#define Number_6 "humi"
#define Number_7 "AQICN"
#define Number_8 "AQIUS"

BlinkerNumber pm1_0(Number_1);
BlinkerNumber pm2_5(Number_2);
BlinkerNumber pm10(Number_3);
BlinkerNumber HCHO(Number_4);
BlinkerNumber TEMP(Number_5);
BlinkerNumber HUMI(Number_6);
BlinkerNumber AQICN(Number_7);
BlinkerNumber AQIUS(Number_8);

String aqi_color[6] = {"#FF0000", "#FFFF00", "#FFA500", "#FF0000", "#800080", "#800000"};

void heartbeat()
{
    // Blinker.print("aqibase", getAQIbase());
    // Blinker.print("langauage", getLanguage());
    // Blinker.print("timezone", Blinker.getTimezone());

    // Blinker.print("pm1.0", pm1_0Get());
    // Blinker.print("pm2.5", pm2_5Get());
    // Blinker.print("pm10", pm10_0Get());
    // Blinker.print("hcho", hchoGet());
    // Blinker.print("temp", tempGet());
    // Blinker.print("humi", humiGet());
    // Blinker.print("AQICN", aqiGet(BLINKER_AQI_BASE_CN));
    // Blinker.print("AQIUS", aqiGet(BLINKER_AQI_BASE_US));

    pm1_0.unit("μg/m3");
    pm1_0.print(pm1_0Get());

    pm2_5.unit("μg/m3");
    pm2_5.print(pm2_5Get());

    pm10.unit("μg/m3");
    pm10.print(pm10_0Get());

    HCHO.unit("mg/m3");
    HCHO.print(hchoGet());

    TEMP.unit("℃");
    TEMP.print(tempGet());

    HUMI.unit("%");
    HUMI.print(humiGet());

    AQICN.unit("");
    AQICN.color(aqi_color[aqiLevelGet(BLINKER_AQI_BASE_CN)]);
    AQICN.print(aqiGet(BLINKER_AQI_BASE_CN));
    
    AQIUS.unit("");
    AQIUS.color(aqi_color[aqiLevelGet(BLINKER_AQI_BASE_US)]);
    AQIUS.print(aqiGet(BLINKER_AQI_BASE_US));

    // String values = "{\"pm1.0\":" + STRING_format(pm1_0Get()) + \
    //                 ",\"pm2.5\":" + STRING_format(pm2_5Get()) + \
    //                 ",\"pm10\":" + STRING_format(pm10_0Get()) + \
    //                 ",\"hcho\":" + STRING_format(hchoGet()) + \
    //                 ",\"temp\":" + STRING_format(tempGet()) + \
    //                 ",\"humi\":" + STRING_format(humiGet()) + \
    //                 ",\"AQICN\":" + STRING_format(aqiLevelGet(BLINKER_AQI_BASE_CN)) + \
    //                 ",\"AQIUS\":" + STRING_format(aqiLevelGet(BLINKER_AQI_BASE_US)) + "}";

    // Blinker.printObject("detector", values);

    BLINKER_LOG1("heartbeat!");
}

// #if defined(BLINKER_BUTTON)
/* 
 * Blinker provide a button parse function for user if you defined BLINKER_BUTTON
 * 
 * Blinker button can detect singal click/ double click/ long press
 * 
 * Blinker.tick() will run by default, use interrupt will be better
 */
void buttonTick()
{
    button.tick();
}

/* 
 * Add your code in this function
 * 
 * When button clicked, device will call this function
 */
void singalClick()
{
    changeDetail();

    if (initDisplayed) {
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

    if (initDisplayed) {
        freshDisplay();
    }

    BLINKER_LOG1("Button double clicked!");
}

/* 
 * Add your code in this function
 * 
 * When long press start, device will call this function
 */
void longPressStart()
{
    isLongPress = true;
    press_start_time = millis();
    freshDisplay();

    BLINKER_LOG1("Button long press start!");
}

/* 
 * Add your code in this function
 * 
 * When long press stop and trigged POWERDOWN, device will call this function
 */
void longPressPowerdown()
{
    freshDisplay();

    BLINKER_LOG1("Button long press powerdown!");

    digitalWrite(BLINKER_POWER_3V3_PIN, LOW);
}

/* 
 * Add your code in this function
 * 
 * When long press stop and trigged RESET, device will call this function
 */
void longPressReset()
{
    freshDisplay();

    BLINKER_LOG1("Button long press reset!");
}

/* 
 * Add your code in this function
 * 
 * When long press stop, device will call this function
 */
void longPressStop()
{
    isLongPress = false;
    freshDisplay();

    isLongPress = false;

    if ((millis() - press_start_time) >= BLINKER_AQI_LONG_PRESS_RESET) {
        longPressReset();
    }
    else {
        longPressPowerdown();
    }

    BLINKER_LOG1("Button long press stop!");
}
// #endif

double getBAT()
{
    int sensorValue = analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);
    sensorValue += analogRead(A0);

    double voltage = sensorValue * (5.926 / 1023.0 / 8.0);

    // BLINKER_LOG2("bat: ", voltage);

    return voltage;
}

void batCheck()
{
    if ((millis() - batFresh) > BLINKER_BAT_CHECK_TIME)
    {
        batRead = getBAT() * 10;

        if (batBase - batRead > BLINKER_BAT_POWER_USEUP * 10) {
            batBase = batRead;
            BLINKER_ERR_LOG1("BLINKER_BAT_POWER_USEUP");
            BLINKER_LOG6("batBase: ", batBase / 10.0, " v", ", batRead: ", batRead / 10.0, " v");
        }
        else {
            if (batRead > batBase) batBase = batRead;
        }
        
        BLINKER_LOG3("bat: ", batRead / 10.0, " v");
        // BLINKER_LOG_FreeHeap();

        // BLINKER_LOG2("aqibase", getAQIbase());
        // BLINKER_LOG2("langauage", getLanguage());
        // BLINKER_LOG2("timezone", Blinker.getTimezone());

        BLINKER_LOG2("pm1.0: ", pm1_0Get());
        BLINKER_LOG2("pm2.5: ", pm2_5Get());
        BLINKER_LOG2("pm10: ", pm10_0Get());
        BLINKER_LOG2("hcho: ", hchoGet());
        BLINKER_LOG2("temp: ", tempGet());
        BLINKER_LOG2("humi: ", humiGet());
        BLINKER_LOG2("AQICN: ", aqiGet(BLINKER_AQI_BASE_CN));
        BLINKER_LOG2("AQIUS: ", aqiGet(BLINKER_AQI_BASE_US));

        batFresh = millis();

        if (batRead < BLINKER_BAT_POWER_LOW * 10) {
            digitalWrite(BLINKER_POWER_3V3_PIN, LOW);
        }
    }
}

void hardwareInit()
{
    pinMode(BLINKER_POWER_3V3_PIN, OUTPUT);
    digitalWrite(BLINKER_POWER_3V3_PIN, HIGH);
    pinMode(BLINKER_POWER_5V_PIN, OUTPUT);
    digitalWrite(BLINKER_POWER_5V_PIN, HIGH);

    batRead = getBAT() * 10;
    // batRead = 40;
}

void AQI_init()
{
    Serial.begin(115200);

    hardwareInit();
    u8g2Init();
    pmsInit();
    
    Blinker.begin(auth, ssid, pswd);

    // Blinker.attachParse(dataParse);
    Blinker.attachHeartbeat(heartbeat);

// #if defined(BLINKER_BUTTON)
    button.attachClick(singalClick);
    button.attachDoubleClick(doubleClick);
    button.attachLongPressStart(longPressStart);
    button.attachLongPressStop(longPressStop);
    // Blinker.attachLongPressPowerdown(longPressPowerdown);
    // Blinker.attachLongPressReset(longPressReset);

    attachInterrupt(BLINKER_BUTTON_PIN, buttonTick, CHANGE);
// #endif

    attachDisplay(display);
    attachColor(aqiLevelGet);
}

void AQI_run()
{
    Blinker.run();

    buttonTick();

    batCheck();

    aqiFresh();
}

void changeMain()
{
    isAQI = !isAQI;
}

void display()
{
    if (!isLongPress) {
        batDisplay(batRead / 10.0);

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
        resetDisplay(millis() - press_start_time);
    }
}

bool checkInit()
{
    if (!inited) {
        if (Blinker.connected()) {
            setTimeLimit(BLINKER_PMS_LIMIT_FRESH);
            inited = true;
        }
    }
    
    return inited;
}

void aqiFresh()
{
    if (isLongPress) {
        freshDisplay();
    }
    else if (!checkInit()) {
        if (initDisplay()) {
            Blinker.delay(1);
        }
        else {
            initDisplayed = true;

            if (pmsFresh()) {
                freshDisplay();
            }
        }
    }
    else {
        pmsFresh();
        
        freshDisplay();
    }
}
