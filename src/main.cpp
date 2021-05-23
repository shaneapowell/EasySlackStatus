/**************************************************************************
 Slack Status Updater
 By Shane Powell <shaneapowell@hotmail.com>
 Based on Becky Stern Instructable "Slack Status Updater ESP8266"

 ======
 
 This code is intended to use with the following hardware: 
 Wemos D1
 128x64 pixel SH1106 oled display
 Rotary encoder with push button

 **************************************************************************/

#include <Time.h>
#include <Timezone.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <Button2.h>
#include <ESPRotary.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoSlack.h>
#include <ArduinoSlackCert.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <IotWebConf.h>
#include <IotWebConfUsing.h> 
#include <IotWebConfParameter.h>
#include <ESP8266HTTPUpdateServer.h>
#include <string.h>

#include "main.h"
#include "tz.h"
#include "display.hpp"

#define PIN_ROTARY_DT           D7
#define PIN_ROTARY_CLK          D6
#define PIN_ROTARY_BUTTON       D5
#define ROTARY_STEPS_PER_CLICK   4   

/* Must be a string no more than 4 chars.  Only alter when the config changes  */
#define IOT_WEB_CONFIG_VERSION_CODE  "0001"

const char* _iotThingName               = "EasySlackStatus";
const char* _iotWifiInitialApPassword   = "12345678";
const char* _slackTokenLabel            = "Slack-Token";
const char* _tzLabel                    = "TimeZone";
const char* _statusLabel                = "Status";
const char* _iconLabel                  = "Icon (must include wrapping \':\')";
const char* _expireLabel                = "Default Expire in Minutes";

WiFiUDP _ntpUDP;
NTPClient _ntpClient(_ntpUDP, "us.pool.ntp.org"); /* Used extern in display.cpp  */

DNSServer _dnsServer;
WebServer _server(80);
IotWebConf _iotWebConf(_iotThingName, &_dnsServer, &_server, _iotWifiInitialApPassword, IOT_WEB_CONFIG_VERSION_CODE);
ESP8266HTTPUpdateServer _httpUpdater;

// WiFiEventHandler _wifiGotIPHandler;
WiFiEventHandler _wifiConnectedHandler;
WiFiEventHandler _wifiDisconnectedHandler;

char _slackAccessToken[MAX_SLACK_TOKEN_LENGTH] = "xoxp-xxxx";
WiFiClientSecure _wifiClient;

ESPRotary _rotary = ESPRotary(PIN_ROTARY_CLK, PIN_ROTARY_DT, ROTARY_STEPS_PER_CLICK);
Button2 _rotaryButton = Button2(PIN_ROTARY_BUTTON);

LCD Display;


/***********************************************/ 
void onIotWebStateChanged(byte oldState, byte newState)
{
    switch (newState) 
    {
        case IOTWEBCONF_STATE_BOOT:
            Display.setScreen(SCREEN_BOOT);
        break;

        case IOTWEBCONF_STATE_NOT_CONFIGURED:
        case IOTWEBCONF_STATE_AP_MODE:
            Display.lockSettings(true);
            Display.setScreen(SCREEN_AP);
        break;
        
        case IOTWEBCONF_STATE_CONNECTING:
            Display.lockSettings(true);
            Display.setScreen(SCREEN_WIFI);
        break;

        case IOTWEBCONF_STATE_ONLINE:
            Display.lockSettings(false);
            Display.setScreen(SCREEN_MAIN);
            _ntpClient.begin();
        break;

        default:
            /* Not possible, do nothing */
        break;

    }

}


/***********************************************/ 
bool onIotWebSoftAPSetupRequest(const char* apName, const char* password)
{
    Serial.println(F(">>> AP Connection"));
//    Display.onWifiDiconnected();
    return WiFi.softAP(apName, password);
}

/***********************************************/ 
void onIotWebWifiConnectionRequest(const char* ssid, const char* password)
{
    Serial.println(F(">>> WiFi Connection"));
    WiFi.softAPdisconnect();
    WiFi.begin(ssid, password);
//    Display.onWifiConnecting();
}

/***********************************************/ 
void onWifiConnected(const WiFiEventStationModeConnected& event){
	Serial.print(F("Wifi Connected to AP: ")); Serial.println(event.ssid);
	Display.setScreen(SCREEN_WIFI);
}

/***********************************************/  
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event){
	_ntpClient.end();
}

/***********************************************
 * on Rotate
 ***********************************************/
void onRotate(ESPRotary& r) {
    Display.onRotaryInput(r.getDirection() == RE_RIGHT);
}

/***********************************************
 * single click
 ***********************************************/
void onRotaryClick(Button2& btn) 
{
    Display.onRotaryClick();
}

/***********************************************
 * Double click
 ***********************************************/
void onRotaryDoubleClick(Button2& btn) 
{
    Display.onRotaryDoubleClick();
}

/***********************************************
 * long click
 ***********************************************/
void onRotaryLongClick(Button2& btn) 
{
    Display.onRotaryLongClick();
}

 /***********************************************
 * long click
 ***********************************************/
void handleWebRoot()
{
    // -- Let IotWebConf test and handle captive portal requests.
    if (_iotWebConf.handleCaptivePortal())
    {
        // -- Captive portal request were already served.
        return;
    }
    String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
    s += "<title>IotWebConf 01 Minimal</title></head><body>";
    s += "Go to <a href='config'>Settings Page</a> to change settings.";
    s += "</body></html>\n";

    _server.send(200, "text/html", s);
}

/********************************************
 * Get the NTP fetched time, converted to our timezone 
 ********************************************/
time_t getNtpTime()
{
    Timezone tz = TIMEZONES[0];

    /* Find the matching TZ */
    for (int index = 0; index < TZ_COUNT; index++)
    {
        if (strcmp(currentTZName, TZ_NAMES[index]) == 0)
        {
            tz = TIMEZONES[index];
            break;
        }
    }

    time_t utcNow = _ntpClient.getEpochTime();
	time_t now = tz.toLocal(utcNow);
    return now;
}

/***********************************************/

void setup() {
    Serial.begin(115200);

    delay(10);

    Serial.println(F("Setting Up LCD"));
    Wire.begin();
    Wire.setClock(400000L);
    delay(100);
    Display.setup();

    _rotary.setChangedHandler(onRotate);
    _rotaryButton.setClickHandler(onRotaryClick);
    _rotaryButton.setLongClickDetectedHandler(onRotaryLongClick); 
    _rotaryButton.setLongClickDetectedRetriggerable(false);
    _rotaryButton.setLongClickTime(1000);
    _rotaryButton.setDoubleClickHandler(onRotaryDoubleClick);

    _wifiConnectedHandler = WiFi.onStationModeConnected(onWifiConnected);
    // _wifiGotIPHandler = WiFi.onStationModeGotIP(onWifiGotIP);
    _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    // Web Configurator
    _iotWebConf.setStateChangedCallback(onIotWebStateChanged);
    _iotWebConf.setApConnectionHandler(onIotWebSoftAPSetupRequest);
    _iotWebConf.setWifiConnectionHandler(onIotWebWifiConnectionRequest);
    _iotWebConf.skipApStartup();
    _iotWebConf.setStatusPin(LED_BUILTIN);
    _iotWebConf.setConfigPin(PIN_ROTARY_BUTTON);

    /* Slack Token */
    _iotWebConf.addSystemParameter( new IotWebConfTextParameter(_slackTokenLabel, 
                                                                _slackTokenLabel,
                                                                _slackAccessToken,
                                                                MAX_SLACK_TOKEN_LENGTH,
                                                                _slackAccessToken));

    /* TimeZone */
    _iotWebConf.addSystemParameter(new IotWebConfSelectParameter(_tzLabel,
                                                                _tzLabel,
                                                                currentTZName,
                                                                TZ_NAME_MAX,
                                                                (char*)TZ_NAMES,
                                                                (char*)TZ_NAMES,
                                                                TZ_COUNT,
                                                                TZ_NAME_MAX,
                                                                TZ_NAMES[0]));


    /* Add each slack status override to the conf. Create on the heap, NOT the stack */
    char  buff[48];
    for (int index = 0; index < SLACK_STATUS_COUNT; index++)
    {
        /* Group Param, needs to be created, and left on the heap */
        sprintf(buff, "Status%02d", index);
        String* paramKey = new String(buff);
        sprintf(buff, "Status %d", index+1);
        String* groupLabel = new String(buff);
        iotwebconf::ParameterGroup* paramGroup = new IotWebConfParameterGroup(paramKey->c_str(), 
                                                                              groupLabel->c_str());

        /* Status Label */
        sprintf(buff, "label%02d", index);
        paramKey = new String(buff);
        paramGroup->addItem(new IotWebConfTextParameter(_statusLabel, 
                                                        paramKey->c_str(), 
                                                        _slackStatusList[index].title, 
                                                        SLACK_STATUS_CHARS_MAX+1,
                                                        _slackStatusList[index].title,
                                                        SLACK_STATUS_BLANK_STATUS));
        /* Status Icon */
        sprintf(buff, "icon%02d", index);
        paramKey = new String(buff);
        paramGroup->addItem(new IotWebConfTextParameter(_iconLabel, 
                                                        paramKey->c_str(), 
                                                        _slackStatusList[index].icon, 
                                                        SLACK_STATUS_ICON_CHARS_MAX+1,
                                                        _slackStatusList[index].icon,
                                                        SLACK_STATUS_BLANK_ICON));
        /* Status Expiry */
        sprintf(buff, "expire%02d", index);
        paramKey = new String(buff);
        paramGroup->addItem(new IotWebConfNumberParameter(_expireLabel, 
                                                          paramKey->c_str(), 
                                                          _slackStatusList[index].expireInMinutes, 
                                                          SLACK_STATUS_EXPIRE_CHARS_MAX+1,
                                                          _slackStatusList[index].expireInMinutes,
                                                          SLACK_STATUS_BLANK_EXPIRE));

        _iotWebConf.addParameterGroup(paramGroup);
    }

    /* Firmware Updater */
    _iotWebConf.setupUpdateServer(
        [](const char* updatePath) { _httpUpdater.setup(&_server, updatePath); },
        [](const char* userName, char* password) { _httpUpdater.updateCredentials(userName, password); });


    _iotWebConf.init();
    _server.on("/", handleWebRoot);
    _server.on("/config", []{ _iotWebConf.handleConfig(); });
    _server.onNotFound([](){ _iotWebConf.handleNotFound(); });

    delay(100);

    Display.setScreen(SCREEN_BOOT);

    if (_wifiClient.setFingerprint(SLACK_ACCESS_FINGERPRINT)) {
        Serial.println(F("finger print set success"));
    } else {
        Serial.println(F("finger print set failure"));
    }

    /* Finally, confiure the slack client and pass it to the display */
    ArduinoSlack* slack = new ArduinoSlack(_wifiClient, _slackAccessToken);
    Display.setSlack(slack);

}


/***********************************************/
void loop() 
{
    _ntpClient.update();
	if (timeStatus() == timeNotSet && _ntpClient.getEpochTime() > 946688461) /* Jan 1 2000 */
	{
		/* DateTime, Sync fast until both ntptime and time are equal */
		Serial.print(F("DATE TIME SYNCED with TZ "));
        Serial.println(currentTZName);
        setSyncProvider(getNtpTime);
	}

    _iotWebConf.doLoop();
    _rotary.loop();
    _rotaryButton.loop();
    Display.loop();

}







