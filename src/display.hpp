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
#ifndef __LCD_HPP__
#define __LCD_HPP__

#include <Wire.h>
#include "lcd/Adafruit_I2C_SH1106.h"

#include "state.h"

#define PIN_OLED_DATA           D2 
#define PIN_OLED_CLOCK          D1 
#define OLED_I2C_ADDR           0x3C
#define OLED_SCREEN_WIDTH       128 
#define OLED_SCREEN_HEIGHT      64 

/* Text Size 1 */
#define CURSOR_STATUS_BAR 0

/* Text Size 2 */
#define CURSOR_LINE1 10
#define CURSOR_LINE2 28
#define CURSOR_LINE3 46  // 64 - 18



class LCD 
{

private:
    //Adafruit_SSD1306 mDisplay(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1);
    Adafruit_I2C_SH1106 _display;
    //State *_state;
    bool _isDirty = false;
    SlackStatus _currentStatus = ALL_SLACK_STATUS[0];
    String _ipAddr = "0.0.0.0";

    /*************************************************
     *
     ************************************************/ 
    void renderScreen() 
    {
        
    //display.resetDisplay();

    //  display.setChar(2, 2, ('0' + statusId));
    //  display.display();

        _display.fillScreen(BLACK);

        _display.setTextSize(1);
        _display.setTextColor(WHITE); 
        _display.setCursor(_display.width() / 2, CURSOR_STATUS_BAR);
        _display.print(_ipAddr);

        renderMainScreen();

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
    void renderMainScreen()
    {
        _display.setTextSize(2);
        _display.setCursor(0, CURSOR_LINE1);
        _display.print(ALL_SLACK_STATUS[0].status);
        _display.setCursor(0, CURSOR_LINE2);
        _display.print(ALL_SLACK_STATUS[1].status);
        _display.setCursor(0, CURSOR_LINE3);
        _display.print(ALL_SLACK_STATUS[2].status);
    }

public:
    

    /***********************************************/
    void setup() 
    {

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    // if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    //   Serial.println(F("SSD1306 allocation failed"));
    //   for (;;); // Don't proceed, loop forever
    // }

        _display.init();
        _display.setTextWrap(false);
        
        delay(500);

        renderScreen();

    }

    /***********************************************/
    void loop()
    {
        if (_isDirty) 
        {
            Serial.println("LCD isDirty");
            _isDirty = false;
            renderScreen();
        }
    }

    /***********************************************/
    void onWifiConnected() 
    {
        _ipAddr = "0.0.0.0";
        _isDirty = true;
    }

    /***********************************************/
    void onWifiGotIp(const String ipAddr)
    {
        _ipAddr = ipAddr;
        _isDirty = true;
    }

    /***********************************************/
    void onWifiDiconnected()
    {
        _ipAddr = "0.0.0.0";
        _isDirty = true;
    }

    /***********************************************/
    void onRotaryInput(boolean increase) 
    {
        _isDirty = true;
    }

    /***********************************************/
    void onRotaryClick() 
    {
        _isDirty = true;
    }

    /***********************************************/
    void onRotaryLongClick()
    {
        _isDirty = true;
    }

    /***********************************************/
    void onSettingsClick()
    {
        _isDirty = true;
    }


};

extern LCD Display;

#endif

