/**************************************************************************
 Slack Status Updater
 By Shane Powell <shaneapowell@hotmail.com>
 Based on Becky Stern Instructable "Slack Status Updater ESP8266"

 ======
 
 This code is intended to use with the following hardware: 
 Wemos D1
 128x64 pixel oled display
 Rotary encoder with push button

 Notes:
  - OLED Menu
    - Main screen is simply the "status" screen.  It has a list of available status lines, that can be
        rotary scrolled up and down.  Once on the one you want, single "click" to send. The line also has the default 
        timeout minutes.   eg.. "
            - Lunch      (30m)
            - Doctor      (1h)
            - Vacation    
    - Main screen has at the top, your current status at the left.  And the Connection status icon at the right.
    - The Connection status icon is 1 of 3 things.  No Wifi; Wifi but no Slack; Wifi and Slack;
    - Add a dedicated "settings" tactile button. This allows into the config menu.
    - Selecting a Status
        - single click sets that status, and the default icon and timeout.
        - Long Press goes to modify before send status.   Allowing selection of different icon and timeout before sending.

    MENU Items
    - Status
        - Clear
        - Available
        - Coffee        (15m)
        - Lunch         (45m)
        - Meeting       (1h)
        - Unavailable   (30m)
        - Doctor        (1.5h)
        - Walking Dog   (30m)
        - Vacation
    - Config
        - WiFi Setup
        - On/Off web-configurator
        - Current Wifi IP address
        - URL to web-configurator
    

    Wifi Setup

    Web Configurator
      - Enter your Slack Token
      - Update Fingerprint
      - Modify list of status entries

    Status Entries
        - Status Title
        - Icon
        - Default Timeout
 
  **************************************************************************/

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
#include "lcd/Adafruit_I2C_SH1106.h"

//#include "state.h"
#include "display.hpp"
#include "creds.h"


#define PIN_SETTINGS_BUTTON     D0
#define PIN_ROTARY_CLK          D6
#define PIN_ROTARY_DT           D7
#define PIN_ROTARY_BUTTON       D5
#define ROTARY_STEPS_PER_CLICK   4   



// -- Initial name of the Thing. Used e.g. as SSID of the own Access Point.
const char _iotThingName[] = "EasySlackStatus";
// -- Initial password to connect to the Thing, when it creates an own Access Point.
const char _iotWifiInitialApPassword[] = "12345678";

DNSServer _dnsServer;
WebServer _server(80);
IotWebConf _iotWebConf(_iotThingName, &_dnsServer, &_server, _iotWifiInitialApPassword);


WiFiEventHandler _wifiGotIPHandler;
WiFiEventHandler _wifiConnectedHandler;
WiFiEventHandler _wifiDisconnectedHandler;

WiFiClientSecure _wifiClient;
ArduinoSlack mSlack(_wifiClient, SLACK_ACCESS_TOKEN);

ESPRotary _rotary = ESPRotary(PIN_ROTARY_CLK, PIN_ROTARY_DT, ROTARY_STEPS_PER_CLICK);
Button2 _rotaryButton = Button2(PIN_ROTARY_BUTTON);
Button2 _settingsButton = Button2(PIN_SETTINGS_BUTTON);

LCD Display;


/***********************************************/ 
bool onSoftAPSetupRequest(const char* apName, const char* password)
{
    Serial.println(">>> AP Connection");
    Display.onWifiDiconnected();
    return WiFi.softAP(apName, password);
}

/***********************************************/ 
void onWifiConnectionRequest(const char* ssid, const char* password)
{
    Serial.println(">>> WiFi Connection");
    WiFi.softAPdisconnect();
    WiFi.begin(ssid, password);
    Display.onWifiConnecting();
}

/***********************************************/ 
void onWifiConnected(const WiFiEventStationModeConnected& event){
	Serial.print(F("Wifi Connected to AP: ")); Serial.println(event.ssid);
    Serial.println(_iotWebConf.getState());
	Display.onWifiConnected();
}

/***********************************************/  
void onWifiDisconnect(const WiFiEventStationModeDisconnected& event){
	Serial.println(F("Wifi Disconnected."));
    Serial.println(_iotWebConf.getState());
	Display.onWifiDiconnected();
}

/***********************************************/ 
void onWifiGotIP(const WiFiEventStationModeGotIP& event){
	Serial.print(F("Wifi Got IP: ")); Serial.println(event.ip);
    Serial.println(_iotWebConf.getState());
	Display.onWifiGotIp();
}


/***********************************************
 * on Rotate
 ***********************************************/
void onRotate(ESPRotary& r) {
    int position = r.getPosition();
    Serial.println(position);
    Serial.println(r.directionToString(r.getDirection()));
    Display.onRotaryInput(r.getDirection() == RE_RIGHT);
//    showNewStatus(position);    
}

// /***********************************************
//  * on left or right rotation
//  ***********************************************/
// void onRotateDirection(ESPRotary& r) {
//     Serial.println(r.directionToString(r.getDirection()));
//     _lcd.onRotaryInput(r.getDirection() == RE_LEFT);
// }

/***********************************************
 * single click
 ***********************************************/
void onRotaryClick(Button2& btn) 
{
    //profile = mSlack.setCustomStatus(status[position][0], status[position][1]);
    //displayProfile(profile);
    Display.onRotaryClick();
}

/***********************************************
 * single click
 ***********************************************/
void onSettingsClick(Button2& btn) 
{
    Serial.println(F("Settings Click"));
    Display.onSettingsClick();
}

/***********************************************
 * long click
 ***********************************************/
void onRotaryLongClick(Button2& btn) 
{
    Serial.println(F("Rotary Long Click!"));
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



/***********************************************/
void setup() {
    Serial.begin(9600);

    delay(10);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
  //   Serial.println(F("SSD1306 allocation failed"));
  //   for (;;); // Don't proceed, loop forever
  // }

    Serial.println("Setting Up LCD");
    Wire.begin();
    Wire.setClock(400000L);
    delay(100);
    Display.setup();

    _rotary.setChangedHandler(onRotate);
    _rotaryButton.setClickHandler(onRotaryClick);
    _rotaryButton.setLongClickDetectedHandler(onRotaryLongClick); 
    _rotaryButton.setLongClickDetectedRetriggerable(false);
    _rotaryButton.setLongClickTime(1000);

    _settingsButton.setTapHandler(onSettingsClick);

    _wifiConnectedHandler = WiFi.onStationModeConnected(onWifiConnected);
    _wifiGotIPHandler = WiFi.onStationModeGotIP(onWifiGotIP);
    _wifiDisconnectedHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

    // Web Configurator
    _iotWebConf.setApConnectionHandler(onSoftAPSetupRequest);
    _iotWebConf.setWifiConnectionHandler(onWifiConnectionRequest);
    _iotWebConf.skipApStartup();
    _iotWebConf.setStatusPin(LED_BUILTIN);
    _iotWebConf.setConfigPin(PIN_SETTINGS_BUTTON);
    _iotWebConf.init();
    _server.on("/", handleWebRoot);
    _server.on("/config", []{ _iotWebConf.handleConfig(); });
    _server.onNotFound([](){ _iotWebConf.handleNotFound(); });

//    WiFi.mode(WIFI_STA);
//    WiFi.disconnect();
    delay(100);

    // Attempt to connect to Wifi network:
//    WiFi.setAutoConnect(false);
//    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Display.onWifiConnecting();

    if (_wifiClient.setFingerprint(SLACK_ACCESS_FINGERPRINT)) {
        Serial.println(F("finger print set success"));
    } else {
        Serial.println(F("finger print set failure"));
    }


}


/***********************************************/
void loop() 
{
    _iotWebConf.doLoop();
    _rotary.loop();
    _rotaryButton.loop();
    _settingsButton.loop();
    Display.loop();
}







