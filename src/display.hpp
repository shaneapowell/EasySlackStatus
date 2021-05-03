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
#define STATUS_BAR_HEIGHT 10
#define CURSOR_STATUS_BAR 0

/* Text Size 2 */
#define LINE_HEIGHT  18
#define LINE_COUNT   3
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

    int _currentStatusIndex = 0;

    int _mainScreenHighlightedIndex = 0;  /* Which state is currently "highlghted" */
    int _mainScreenScrollBy = 0;          /* The rotary scroll by */
    
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
        int x = ALL_SLACK_STATUS[_currentStatusIndex].title.length();
        x /= 2;
        x *= (6+2);
        _display.setCursor(x, CURSOR_STATUS_BAR);
        _display.print(ALL_SLACK_STATUS[_currentStatusIndex].title);

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
        int index = _mainScreenScrollBy;
        renderStatusLine(CURSOR_LINE1, ALL_SLACK_STATUS[index],   index == _mainScreenHighlightedIndex);
        renderStatusLine(CURSOR_LINE2, ALL_SLACK_STATUS[index+1], index+1 == _mainScreenHighlightedIndex);
        renderStatusLine(CURSOR_LINE3, ALL_SLACK_STATUS[index+2], index+2 == _mainScreenHighlightedIndex);
    }


    /***********************************************/
    void renderStatusLine(int cursorLine, SlackStatus status, bool active)
    {
        _display.setTextSize(2);

        if (active)
        {
            _display.fillRect(0, cursorLine-1, _display.width(), LINE_HEIGHT, WHITE);
            _display.setTextColor(BLACK);
        }
        else
        {
            _display.setTextColor(WHITE);
        }

        _display.setCursor(1, cursorLine);
        _display.print(status.title);
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
        /* Move Highlight Bar */
        _mainScreenHighlightedIndex += (increase ? 1 : -1);
        _mainScreenHighlightedIndex = max(_mainScreenHighlightedIndex, 0);
        _mainScreenHighlightedIndex = min(_mainScreenHighlightedIndex, SLACK_STATUS_COUNT-1);

        /* Calculate any change in the scroll by */
        if (_mainScreenHighlightedIndex < _mainScreenScrollBy)
        {
            /* Highlight is above the current scroll by bring it back down */
            _mainScreenScrollBy = _mainScreenHighlightedIndex;
        } 
        else if (_mainScreenHighlightedIndex >= _mainScreenScrollBy + LINE_COUNT)
        {
            _mainScreenScrollBy = _mainScreenHighlightedIndex - LINE_COUNT + 1;
        }

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

