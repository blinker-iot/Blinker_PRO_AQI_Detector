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
#include "Ticker.h"

static bool inited = false;
static bool initDisplayed = false;
static bool isAQI = true;
static uint8_t isLongPress = BLINKER_NONE_LONG_PRESS;
static bool tickerTrigged = false;
static uint8_t batRead;
static uint8_t batBase;
static uint32_t batFresh = 0;
static bool isRegisterBlink = false;
Ticker pushTicker;
Ticker wdtTicker;

void aqiStorage()
{
    BLINKER_LOG1("push ticker trigged");

    Blinker.dataStorage("pm1.0", pm1_0Get());
    Blinker.dataStorage("pm2.5", pm2_5Get());
    Blinker.dataStorage("pm10", pm10_0Get());
    Blinker.dataStorage("hcho", hchoGet());
    Blinker.dataStorage("temp", tempGet());
    Blinker.dataStorage("humi", humiGet());

    if (WiFi.status() == WL_CONNECTED && checkInit()) {
        tickerTrigged = true;

        // Blinker.dataUpdate();
        // BLINKER_LOG1("Blinker.dataUpdate()");
    }

    pushTicker.once(BLINKER_AQI_FRESH_TIME, aqiStorage);
}

uint8_t getSignals()
{
    
    if (WiFi.status() == WL_CONNECTED) {
        int32_t wRSSI = WiFi.RSSI();

        // IOT_DEBUG_PRINT2(F("getSignals: "), wRSSI);

        setTimeLimit(BLINKER_PMS_LIMIT_FRESH);

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
        setTimeLimit(BLINKER_PMS_LIMIT_INIT);

        return 0;
    }
}

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

    Blinker.print("pm1.0", pm1_0Get());
    Blinker.print("pm2.5", pm2_5Get());
    Blinker.print("pm10", pm10_0Get());
    Blinker.print("hcho", hchoGet());
    Blinker.print("temp", tempGet());
    Blinker.print("humi", humiGet());
    Blinker.print("AQICN", aqiGet(BLINKER_AQI_BASE_CN));
    Blinker.print("AQIUS", aqiGet(BLINKER_AQI_BASE_US));

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
    isLongPress = BLINKER_LONG_PRESS_START;
    freshDisplay();

    BLINKER_LOG1("Button long press start!");
}

/* 
 * Add your code in this function
 * 
 * When long press stop, device will call this function
 */
void attachLongPressStop()
{
    // isLongPress = false;
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
    // freshDisplay();
    isLongPress = BLINKER_LONG_PRESS_POWER_DOWN;
    clearPage();

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
    // freshDisplay();
    isLongPress = BLINKER_LONG_PRESS_RESET;
    clearPage();

    BLINKER_LOG1("Button long press reset!");
}
#endif

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
    if ((millis() - batFresh) > BLINKER_BAT_CHECK_TIME || BLINKER_BAT_CHECK_TIME == 0)
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
        // int32_t wRSSI = WiFi.RSSI();

        // BLINKER_LOG4("getSignals: ", wRSSI, " , WiFi.status: ", WiFi.status() == WL_CONNECTED);
        // BLINKER_LOG_FreeHeap();

        // // BLINKER_LOG2("aqibase", getAQIbase());
        // // BLINKER_LOG2("langauage", getLanguage());
        // // BLINKER_LOG2("timezone", Blinker.getTimezone());

        // BLINKER_LOG2("pm1.0: ", pm1_0Get());
        // BLINKER_LOG2("pm2.5: ", pm2_5Get());
        // BLINKER_LOG2("pm10: ", pm10_0Get());
        // BLINKER_LOG2("hcho: ", hchoGet());
        // BLINKER_LOG2("temp: ", tempGet());
        // BLINKER_LOG2("humi: ", humiGet());
        // BLINKER_LOG2("AQICN: ", aqiGet(BLINKER_AQI_BASE_CN));
        // BLINKER_LOG2("AQIUS: ", aqiGet(BLINKER_AQI_BASE_US));

        batFresh = millis();

        if (batRead < BLINKER_BAT_POWER_LOW * 10) {
            // digitalWrite(BLINKER_POWER_3V3_PIN, LOW);
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

    pushTicker.once(BLINKER_AQI_FRESH_TIME, aqiStorage);
    // batRead = 40;
}

void checkUpdate()
{
    if (tickerTrigged) {
        tickerTrigged = false;

        Blinker.dataUpdate();
        BLINKER_LOG1("Blinker.dataUpdate()");
    }
}

void fresh() {
	wdtTicker.once(3, fresh);
	BLINKER_LOG1("Fresh & Feed wdt");

	ESP.wdtFeed();
}

void AQI_init()
{
    Serial.begin(115200);

    hardwareInit();
    u8g2Init();
    pmsInit();
    
    Blinker.begin(BLINKER_AIR_DETECTOR);

    // Blinker.deleteTimer();

    Blinker.attachParse(dataParse);
    Blinker.attachHeartbeat(heartbeat);

#if defined(BLINKER_BUTTON)
    Blinker.attachClick(singalClick);
    Blinker.attachDoubleClick(doubleClick);
    Blinker.attachLongPressStart(longPressStart);
    Blinker.attachLongPressPowerdown(longPressPowerdown);
    Blinker.attachLongPressReset(longPressReset);

    attachInterrupt(BLINKER_BUTTON_PIN, buttonTick, CHANGE);
#endif

    attachDisplay(display);
    attachColor(aqiLevelGet);

    // wdtTicker.once(3, fresh);
}

void AQI_run()
{
    Blinker.run();

    batCheck();

    aqiFresh();

    checkUpdate();
}

void changeMain()
{
    isAQI = !isAQI;
}

void display()
{
    if (isLongPress == BLINKER_NONE_LONG_PRESS) {
        batDisplay(batRead / 10.0);

        if (isAQI) {
            aqiDisplay(pm1_0Get(), pm2_5Get(), pm10_0Get(), humiGet(),
                    hchoGet(), tempGet(), Blinker.hour(), Blinker.minute());
        }
        else {
            timeDisplay(pm2_5Get(), Blinker.month(), Blinker.mday(), 
                    Blinker.wday(), Blinker.hour(), Blinker.minute());
        }

        setSignals(getSignals());
    }
    else if (isLongPress == BLINKER_LONG_PRESS_START) {
        resetDisplay(Blinker.pressedTime());
    }
    else {
        clearPage();
    }
}

uint32_t fresh_time = 0;

bool checkInit()
{
    uint8_t deviceStatus = Blinker.status();
    if (deviceStatus == PRO_WLAN_SMARTCONFIG_DONE) {
        setColorType(WLAN_CONNECTING);
        isRegisterBlink = true;
    }
    else if (deviceStatus >= PRO_DEV_AUTHCHECK_FAIL
        && deviceStatus <= PRO_DEV_REGISTER_FAIL
        && isRegisterBlink) {
        setColorType(WLAN_CONNECTED);
    }
    else if (deviceStatus >= PRO_DEV_REGISTER_FAIL
        && deviceStatus < PRO_DEV_CONNECTED
        && isRegisterBlink) {
        setColorType(DEVICE_CONNECTING);
    }
    else if (deviceStatus == PRO_DEV_CONNECTED
        && isRegisterBlink) {
        setColorType(DEVICE_CONNECTED);
        isRegisterBlink = false;
    }

    if (!inited) {
        if (Blinker.init()) {
            setTimeLimit(BLINKER_PMS_LIMIT_FRESH);
            inited = true;
            wdtTicker.once(3, fresh);
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

            // if (pmsFresh()) {
            //     freshDisplay();
            // }
            pmsFresh();
            freshDisplay();
        }
        colorDisplay();
    }
    else {
        if (getSignals()) {
            pmsFresh();
            freshDisplay();
        }
        else {
            // if (pmsFresh()) {
            //     freshDisplay();
            // }
            pmsFresh();
            freshDisplay();
        }
        colorDisplay();
    }
}
