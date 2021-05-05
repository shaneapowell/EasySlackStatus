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
        - Available
        - Coffee        (15m)
        - Lunch         (45m)
        - Meeting       (1h)
        - Unavailable   (30m)
        - Doctor        (1.5h)
        - Walking Dog   (30m)
        - Vacation
    - Config
        - WiFi Setup/Info
            - Connect
            - Back
        - On/Off web-configurator & URL
            -
        - Firmware
    

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

#define TS1_LINE0 0
#define TS1_LINE1 17
#define TS1_LINE2 30
#define TS1_LINE3 43
#define TS1_LINE4 56

extern NTPClient _ntpClient;
char* STATUS_DISPLAY_NAME = "...";

typedef enum 
{
    SCREEN_BOOT,
    SCREEN_MAIN,
    SCREEN_WIFI,
    SCREEN_AP
} SCREEN;


class LCD 
{

private:
    //Adafruit_SSD1306 mDisplay(OLED_SCREEN_WIDTH, OLED_SCREEN_HEIGHT, &Wire, -1);
    Adafruit_I2C_SH1106 _display;
    SlackProfile _currentProfile;
    //State *_state;

    bool _isDirty = false;
    SCREEN _currentScreen = SCREEN_BOOT;
    bool _lockInSettingsScreen = true;

    int _currentStatusIndex = 0;

    int _mainScreenHighlightedIndex = 0;  /* Which state is currently "highlghted" */
    int _mainScreenScrollBy = 0;          /* The rotary scroll by */
    

    /*************************************************
     *
     ************************************************/ 
    void renderScreen() 
    {
        
    //display.resetDisplay();

    //  display.setChar(2, 2, ('0' + statusId));
    //  display.display();

        switch(_currentScreen)
        {
            case SCREEN_BOOT:
                /* Currently, do nothing, Might be nice to restore the hard-coded logo? Or .. pull it from an asset? */
            break;

            case SCREEN_MAIN:   
                _display.fillScreen(BLACK);
                renderMainScreen();
            break;

            case SCREEN_WIFI:
                _display.fillScreen(BLACK);
                renderWifiScreen();
            break;

            case SCREEN_AP:
                _display.fillScreen(BLACK);
                renderAPScreen();
            break;
        }

        _display.flushDisplay();
    // display.clearDisplay();
    // display.setTextSize(2);
    // display.setTextColor(WHITE);
    // display.setCursor(15, 26);
    // display.println(status[statusId][0]);
    // display.display();
    }


    
    /***********************************************/
    void renderMainScreen()
    {
        /* Current Status */
        _display.setTextSize(1);
        _display.setTextColor(WHITE); 

        /* Status */
        String status = "ERROR";
        if (!_currentProfile.error)
        {
            status = _currentProfile.statusText;
            int x = _display.width() - (status.length() * 6);
            _display.setCursor(x, CURSOR_STATUS_BAR);
            _display.print(status);
        }

        /* Time */

        /* Selection List  */
        int index = _mainScreenScrollBy;
        renderStatusLine(CURSOR_LINE1, ALL_SLACK_STATUS[index],   index == _mainScreenHighlightedIndex, ALL_SLACK_STATUS[index].expireInMinutes > 0);
        renderStatusLine(CURSOR_LINE2, ALL_SLACK_STATUS[index+1], index+1 == _mainScreenHighlightedIndex,  ALL_SLACK_STATUS[index].expireInMinutes > 0);
        renderStatusLine(CURSOR_LINE3, ALL_SLACK_STATUS[index+2], index+2 == _mainScreenHighlightedIndex,  ALL_SLACK_STATUS[index].expireInMinutes > 0);
    }


    /***********************************************/
    void renderStatusLine(int cursorLine, SlackStatus status, bool active, bool hasDefaultExpiry)
    {
        _display.setTextSize(2);
        int textColor = WHITE;

        if (active)
        {
            textColor = BLACK;
            _display.fillRect(0, cursorLine-1, _display.width(), LINE_HEIGHT, WHITE);
        }

        _display.setTextColor(textColor);
        _display.setCursor(1, cursorLine);
        _display.print(status.title);

        /* Put a * at the end to indicate this status has a default expiry */
        if (status.expireInMinutes > 0)
        {
            int w = 10;
            int r = w / 2;
            int x = _display.width() - w - 2;
            int y = cursorLine + 3;
            int cx = x + r;
            int cy = y + r;

            _display.drawCircle(cx, cy, r, textColor);
            _display.drawLine(cx, cy, cx, cy-3, textColor);
            _display.drawLine(cx, cy, cx+3, cy, textColor);
            
            
        }
    }

    /***********************************************/
    void renderWifiScreen()
    {
        _display.setTextSize(1);
        _display.setTextColor(WHITE);


        _display.setCursor(0, TS1_LINE0);
        _display.print("WiFi:");
        _display.setCursor(0, TS1_LINE1);
        _display.print(WiFi.SSID());

        if (!WiFi.isConnected())
        {
            _display.setCursor(0, TS1_LINE2);
            _display.print("Connecting...");
}
        else if (WiFi.localIP().isSet())
        {
            _display.setCursor(0, TS1_LINE2);
            String url = WiFi.localIP().toString();
            _display.print(url);

            _display.setCursor(0, TS1_LINE3);
            url = "http://" + url + "/";
            _display.print(url);

            char time[32];
            sprintf(time, "%02d/%02d/%02d %02d:%02d:02d %s", year(), month(), day(), hourFormat12(), minute(), second(), (isPM() ? "pm" : "am"));
            _display.setCursor(0, TS1_LINE4);
            _display.print(time);

        } 
        else
        {
            _display.setCursor(0, TS1_LINE2);
            _display.print("Obtaining IP...");
        }

    }

    /***********************************************/
    void renderAPScreen()
    {
        _display.setTextSize(1);
        _display.setTextColor(WHITE);

        _display.setCursor(0, TS1_LINE0);
        _display.print("Configure:");
        _display.setCursor(0, TS1_LINE1);
        _display.print("Connect Phone or");
        _display.setCursor(0, TS1_LINE2);
        _display.print("Laptop to this WiFi");
        _display.setCursor(18, TS1_LINE3);
        _display.print(WiFi.softAPSSID());
        _display.setCursor(0, TS1_LINE4);
        String url = "http://" + WiFi.softAPIP().toString() + "/";
        _display.print(url);

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

        _currentProfile.displayName = STATUS_DISPLAY_NAME;
        _currentProfile.statusText = STATUS_DISPLAY_NAME;
        _currentProfile.statusEmoji = NULL;
        _currentProfile.statusExpiration = 0;
        _currentProfile.error = false;
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
    SlackStatus getHighlightedSlackStatus()
    {
        return ALL_SLACK_STATUS[_mainScreenHighlightedIndex];
    }

    /********************************************/
    void setSlackProfile(SlackProfile profile)
    {
        _currentProfile = profile;
        if (!profile.error)
        {
            Serial.println("--------- Profile ---------");


            Serial.print("Display Name: ");
            Serial.println(profile.displayName);

            Serial.print("Status Text: ");
            Serial.println(profile.statusText);

            Serial.print("Status Emoji: ");
            Serial.println(profile.statusEmoji);

            Serial.print("Status Expiration: ");
            Serial.println(profile.statusExpiration);

            Serial.println("------------------------");
        } 
        else 
        {
            Serial.println("error getting profile");
        }

        _isDirty = true;
    }


    /***********************************************/
    void setScreen(SCREEN screen)
    {
        /* Only set the user to the main, if we're not locked in settings.
        Locked in settings is a result of the wifi not yet connected, or in
        the AP configure mode. */
        if (screen == SCREEN_MAIN && _lockInSettingsScreen)
        {
            return;
        }

        _currentScreen = screen;
        _isDirty = true;
    }

    /***********************************************/
    void lockSettings(bool lock)
    {
        _lockInSettingsScreen = lock;
    }


    /***********************************************/
    void onRotaryInput(boolean increase) 
    {
        
        /* MAIN screen   */
        if (_currentScreen == SCREEN_MAIN)
        {
            /* Move Highlight Bar */
            _mainScreenHighlightedIndex += (increase ? 1 : -1);
            _mainScreenHighlightedIndex = max(_mainScreenHighlightedIndex, 0);
            _mainScreenHighlightedIndex = min(_mainScreenHighlightedIndex, SLACK_STATUS_COUNT-1);

            /* Calculate any change in the scroll by */
            if (_mainScreenHighlightedIndex < _mainScreenScrollBy)
            {
                _mainScreenScrollBy = _mainScreenHighlightedIndex;
            } 
            else if (_mainScreenHighlightedIndex >= _mainScreenScrollBy + LINE_COUNT)
            {
                _mainScreenScrollBy = _mainScreenHighlightedIndex - LINE_COUNT + 1;
            }
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
        switch (_currentScreen)
        {
            default:
            case SCREEN_BOOT:
            break;

            case SCREEN_MAIN:
                setScreen(SCREEN_WIFI);
            break;

            case SCREEN_WIFI:
                setScreen(SCREEN_MAIN);
            break;

            case SCREEN_AP:
                setScreen(SCREEN_MAIN);
            break;
        }

        _isDirty = true;
    }


};

extern LCD Display;

#endif

