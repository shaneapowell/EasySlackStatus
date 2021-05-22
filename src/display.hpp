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

#include "status.h"

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
char STATUS_DISPLAY_NAME[] = "---";

typedef enum 
{
    SCREEN_BOOT,
    SCREEN_MAIN,
    SCREEN_SET_EXPIRE,
    SCREEN_WIFI,
    SCREEN_AP
} SCREEN;


const SlackProfile FAKE_PROFILE_SENDING =
{
    "",
    "Sending...",
    "",
    0,
    false
};


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

    int _userSetExpiryInMinutes = -1;
    

    /*************************************************
     *
     ************************************************/ 
    void renderScreen() 
    {
        
        switch(_currentScreen)
        {
            case SCREEN_BOOT:
                /* Currently, do nothing, Might be nice to restore the hard-coded logo? Or .. pull it from an asset? */
            break;

            case SCREEN_MAIN:   
                renderMainScreen();
            break;

            case SCREEN_SET_EXPIRE:
                renderSetExpireScreen();
            break;

            case SCREEN_WIFI:
                renderWifiScreen();
            break;

            case SCREEN_AP:
                renderAPScreen();
            break;
        }

        _display.flushDisplay();

    }


    
    /***********************************************/
    void renderMainScreen()
    {
        
        _display.fillScreen(BLACK);

        /* Current Status */
        _display.setTextSize(1);
        _display.setTextColor(WHITE); 

        /* Status */
        String status = "ERROR";
        if (!_currentProfile.error)
        {
            status = _currentProfile.statusText;
            char* name = strtok(_currentProfile.displayName, " ");
            if (name != NULL)
            {
                _display.setCursor(0, CURSOR_STATUS_BAR);
                _display.print(name);
            }
        }
        int x = _display.width() - (status.length() * 6);
        _display.setCursor(x, CURSOR_STATUS_BAR);
        _display.print(status);

        /* Time */

        /* Selection List  */
        int index = _mainScreenScrollBy;
        renderStatusLine(CURSOR_LINE1, _slackStatusList[index],   index == _mainScreenHighlightedIndex, _slackStatusList[index].expireInMinutes > 0);
        renderStatusLine(CURSOR_LINE2, _slackStatusList[index+1], index+1 == _mainScreenHighlightedIndex,  _slackStatusList[index].expireInMinutes > 0);
        renderStatusLine(CURSOR_LINE3, _slackStatusList[index+2], index+2 == _mainScreenHighlightedIndex,  _slackStatusList[index].expireInMinutes > 0);
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

    }


    /***********************************************/
    void renderSetExpireScreen()
    {

        _display.fillScreen(BLACK);

        /* Current Status */
        _display.setTextSize(2);
        _display.setTextColor(WHITE); 

        SlackStatus status = getHighlightedSlackStatus();

        _display.setCursor(0, CURSOR_LINE1);
        _display.print(status.title);

        _display.setCursor(0, CURSOR_LINE2);
        _display.print("Expire In");

        char expiryLine[32];
        sprintf(expiryLine, "< %02d min >", _userSetExpiryInMinutes);
        _display.setCursor(_display.width() / 2 - ( strlen(expiryLine) / 2 * 12), CURSOR_LINE3);
        _display.print(expiryLine);



    }

    /***********************************************/
    void renderWifiScreen()
    {
        _display.fillScreen(BLACK);
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
            sprintf(time, "%02d/%02d/%02d %02d:%02d:%02d%s", year(), month(), day(), hourFormat12(), minute(), second(), (isPM() ? "pm" : "am"));
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
        _display.fillScreen(BLACK);
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

        /* Brief Version Display */
        _display.setTextSize(1);
        _display.setTextColor(WHITE);
        _display.setCursor(10, TS1_LINE4);
        _display.print(APP_VERSION_NAME);
        _display.flushDisplay();

        delay(1000);

        renderScreen();

    }

    /***********************************************/
    void loop()
    {
        /* I guess you can cal this a whopping 2fps */
        static long lastRenderMs = 0;
        if (millis() - lastRenderMs > 500) 
        {
            _isDirty = true;
        }

        if (_isDirty) 
        {
            lastRenderMs = millis();
            _isDirty = false;
            renderScreen();
        }

    }

    /***********************************************/
    SlackStatus getHighlightedSlackStatus()
    {
        return _slackStatusList[_mainScreenHighlightedIndex];
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

        /* If jumping to the expire set screen, pre-set the expire minutes */
        if (screen == SCREEN_SET_EXPIRE) 
        {
            _userSetExpiryInMinutes = atoi(getHighlightedSlackStatus().expireInMinutes);

            /* Another long-press goes back to the main screen. Aka.. cancel */
            if (_currentScreen == SCREEN_SET_EXPIRE) {
                screen = SCREEN_MAIN;
            }
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
        else if (_currentScreen == SCREEN_SET_EXPIRE)
        {
            if (increase) {
                _userSetExpiryInMinutes++;
            } else {
                _userSetExpiryInMinutes--;
            }
            _userSetExpiryInMinutes = max(_userSetExpiryInMinutes, 0);
            _userSetExpiryInMinutes = min(_userSetExpiryInMinutes, 999);
        }

        _isDirty = true;
    }

    /***********************************************/
    void onRotaryClick(ArduinoSlack *slack) 
    {
        
        SlackStatus status = getHighlightedSlackStatus();
        int expireInMinute = 0;
        int expireUTC = 0;

        switch(_currentScreen)
        {
            default:
            break;

            case(SCREEN_MAIN):
            break;

            case(SCREEN_SET_EXPIRE):
                expireInMinute = _userSetExpiryInMinutes;
            break;
            
        }
        
        setScreen(SCREEN_MAIN);
        setSlackProfile(FAKE_PROFILE_SENDING);
        _isDirty = true;
        renderScreen();

        if (expireInMinute > 0)
        {
            expireUTC = _ntpClient.getEpochTime();
            expireUTC += (expireInMinute * 60);
        }
        
        SlackProfile profile = slack->setCustomStatus(status.title, status.icon, expireUTC);
        setSlackProfile(profile);

        _isDirty = true;
    }

    /***********************************************/
    void onRotaryDoubleClick()
    {
        switch(_currentScreen)
        {
            default:
            break;

            case SCREEN_MAIN:
                setScreen(SCREEN_SET_EXPIRE);
            break;

            case SCREEN_SET_EXPIRE:
                setScreen(SCREEN_MAIN);
            break;
        }
        
    }

    /***********************************************/
    void onRotaryLongClick()
    {
        switch (_currentScreen)
        {
            default:
            case SCREEN_BOOT:
            case SCREEN_SET_EXPIRE:
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

