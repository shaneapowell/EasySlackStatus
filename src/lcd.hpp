/**************************************************************************
 Easy Slack Status Updater
 By Shane Powell <shaneapowell@hotmail.com>
 Inspired by "Slack Status Updater ESP8266" by Becky Stern

 ======
 
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
#pragma once

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <Wire.h>
#include "lcd/Adafruit_I2C_SH1106.h"

#include "state.h"

#define PIN_OLED_DATA           D2 
#define PIN_OLED_CLOCK          D1 
#define OLED_I2C_ADDR           0x3C
#define OLED_SCREEN_WIDTH       128 
#define OLED_SCREEN_HEIGHT      64 

#define LINE0 0
#define LINE1 17
#define LINE2 30
#define LINE3 43
#define LINE4 56


class LCD 
{


private:
    //Adafruit_SSD1306 mDisplay(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1);
    Adafruit_I2C_SH1106 _display;
    State *_state;


public:
    /*************************************************
     *
     ************************************************/ 
    LCD(State *state)
    {
        _state = state;
    }

    /*************************************************
     *
     ************************************************/ 
    void renderScreen() 
    {
    //display.resetDisplay();

    //  display.setChar(2, 2, ('0' + statusId));
    //  display.display();

        _display.fillScreen(BLACK);

        _display.setCursor(0, LINE0);
        _display.print(_state->currentStatus.status);
        _display.drawLine(0, LINE0 + 10, _display.width(), LINE0 + 10, WHITE);

        _display.flushDisplay();
    // display.clearDisplay();
    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.setCursor(15, 26);
    // display.println(status[statusId][0]);
    // display.display();
    }


    // void displayProfile(SlackProfile profile)
    // {
    //     if (!profile.error)
    //     {
    //         Serial.println("--------- Profile ---------");


    //         Serial.print("Display Name: ");
    //         Serial.println(profile.displayName);

    //         Serial.print("Status Text: ");
    //         Serial.println(profile.statusText);

    //         Serial.print("Status Emoji: ");
    //         Serial.println(profile.statusEmoji);

    //         Serial.print("Status Expiration: ");
    //         Serial.println(profile.statusExpiration);

    //         Serial.println("------------------------");
    //     } 
    //     else 
    //     {
    //         Serial.println("error getting profile");
    //     }
    // }


    /***********************************************/
    void setup() 
    {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    //   Serial.println(F("SSD1306 allocation failed"));
    //   for (;;); // Don't proceed, loop forever
    // }

        Wire.begin();
        Wire.setClock(400000L);
        delay(100);

        _display.init();
        _display.setTextSize(1);
        _display.setTextColor(WHITE);
        _display.setTextWrap(false);
        _display.flushDisplay();
        delay(500);

        renderScreen();

    }

    /***********************************************/
    void onWifiConnected(const WiFiEventStationModeConnected& event) {
	    
    }

    /***********************************************/
    void onWifiGotIp(const WiFiEventStationModeGotIP& event)
    {

    }

    /***********************************************/
    void onWifiDiconnected(const WiFiEventStationModeDisconnected& event)
    {

    }

    /***********************************************/
    void onRotaryInput(boolean increase) 
    {

    }

    /***********************************************/
    void onRotaryClick() 
    {

    }

    /***********************************************/
    void onRotaryLongClick()
    {

    }

    /***********************************************/
    void onSettingsClick()
    {

    }

};
