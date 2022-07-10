/**************************************************************************
 Easy Slack Status Updater
 By Shane Powell <shaneapowell@hotmail.com>
 Inspired by "Slack Status Updater ESP8266" by Becky Stern

 **************************************************************************/
#ifndef __LCD_HPP__
#define __LCD_HPP__

#include <Wire.h>
#include "lcd/Adafruit_I2C_SH1106.h"
#include "main.h"

#define PIN_OLED_DATA           D2 
#define PIN_OLED_CLOCK          D1 
#define OLED_I2C_ADDR           0x3C
#define OLED_SCREEN_WIDTH       128 
#define OLED_SCREEN_HEIGHT      64 

/* 60 minutes */
#define SCREEN_OFF_INTERVAL_MS (60 * 60 * 1000)

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
char STATUS_DISPLAY_BLANK[] = "---";
char STATUS_DISPLAY_ERROR[] = "ERROR";
char STATUS_DISPLAY_FETCHING[] = "...";

typedef enum 
{
    SCREEN_BOOT,
    SCREEN_MAIN,
    SCREEN_SET_EXPIRE,
    SCREEN_WIFI,
    SCREEN_AP
} SCREEN;

char FAKE_DISPLAY_NAME[] = "";
char FAKE_STATUS[] = "Sending...";
char FAKE_EMOJI[] = "";
const SlackProfile FAKE_PROFILE_SENDING =
{
    FAKE_DISPLAY_NAME,
    FAKE_STATUS,
    FAKE_EMOJI,
    0,
    false
};


class LCD 
{

private:
    Adafruit_I2C_SH1106 _display;
    ArduinoSlack* _slack;

    String _slackDisplayName = STATUS_DISPLAY_BLANK; 
    String _slackStatusText = STATUS_DISPLAY_BLANK;
    bool _slackError = false;

    bool _isDirty = false;
    SCREEN _currentScreen = SCREEN_BOOT;
    bool _lockInSettingsScreen = true;

    int _currentStatusIndex = 0;

    int _mainScreenHighlightedIndex = 0;  /* Which state is currently "highlghted" */
    int _mainScreenScrollBy = 0;          /* The rotary scroll by */

    int _userSetExpiryInMinutes = -1;
    
    unsigned long _lastProfileFetchMillis = 0;
    unsigned long _lastUserInteractionMillis = 0;

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


        /* Screen Saver */
        if (millis() > _lastUserInteractionMillis + SCREEN_OFF_INTERVAL_MS)
        {
            _display.fillScreen(BLACK);
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
        String status = STATUS_DISPLAY_ERROR;
        if (!_slackError)
        {
            /* Status Output */
            status = _slackStatusText;
            if (status.length() == 0)
            {
                status = STATUS_DISPLAY_BLANK;
            }
            else 
            {
                /* Make sure the status fits within our 10 char limit */
                if (status.length() > SLACK_STATUS_CHARS_MAX)
                {
                    status = status.substring(0, SLACK_STATUS_CHARS_MAX);
                }
            }

            /* Make sure the "name" fits within our 10char limit. */
            String name = _slackDisplayName;
            if (name != NULL && name.length() > 0)
            {
                /* First workd/name only */
                int ndx = name.indexOf(' ');
                if (ndx != -1) 
                {
                    name = name.substring(0, ndx);
                }

                name = name.substring(0, SLACK_STATUS_CHARS_MAX);
                _display.setCursor(0, CURSOR_STATUS_BAR);
                _display.print(name);
            }
        } 
        
        /* Render the status line, it's our current status, or error */
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
        _display.print(F("Expire In"));

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
        _display.print(F("WiFi:"));
        _display.setCursor(0, TS1_LINE1);
        _display.print(WiFi.SSID());

        if (!WiFi.isConnected())
        {
            _display.setCursor(0, TS1_LINE2);
            _display.print(F("Connecting..."));
        }
        else if (WiFi.localIP().isSet())
        {
            _display.setCursor(0, TS1_LINE2);
            _display.print(WiFi.localIP().toString());

            char time[32];
            sprintf(time, "%02d/%02d/%02d %02d:%02d:%02d%s", year(), month(), day(), hourFormat12(), minute(), second(), (isPM() ? "pm" : "am"));
            _display.setCursor(0, TS1_LINE3);
            _display.print(time);

            _display.setCursor(0, TS1_LINE4);
            _display.print(currentTZName);

        } 
        else
        {
            _display.setCursor(0, TS1_LINE2);
            _display.print(F("Obtaining IP..."));
        }

    }

    /***********************************************/
    void renderAPScreen()
    {
        _display.fillScreen(BLACK);
        _display.setTextSize(1);
        _display.setTextColor(WHITE);

        _display.setCursor(0, TS1_LINE0);
        _display.print(F("Configure:"));
        _display.setCursor(0, TS1_LINE1);
        _display.print(F("Connect Phone or"));
        _display.setCursor(0, TS1_LINE2);
        _display.print(F("Laptop to this WiFi"));
        _display.setCursor(18, TS1_LINE3);
        _display.print(WiFi.softAPSSID());
        _display.setCursor(0, TS1_LINE4);
        String url = "http://" + WiFi.softAPIP().toString() + "/";
        _display.print(url);

    }

public:
    

    /***********************************************/
    void setSlack(ArduinoSlack* slack)
    {
        _slack = slack;
    }

    /***********************************************/
    void setup() 
    {

        _display.init();
        _display.setTextWrap(false);

        /* Brief Version Display */
        _display.setTextSize(1);
        _display.setTextColor(WHITE);
        _display.setCursor(0, TS1_LINE4);
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


        /* Every minute, query the current slack status  */
        if (_lastProfileFetchMillis == 0 || millis() - _lastProfileFetchMillis > (1000 * 60))
        {
            if (_currentScreen == SCREEN_MAIN)
            {
                Serial.println(F("Fetching Current Slack Status"));
                _slackStatusText = STATUS_DISPLAY_FETCHING;
                renderScreen();
                SlackProfile profile = _slack->getCurrentStatus();
                setSlackProfile(profile);
                _lastProfileFetchMillis = millis();
            }
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
        _lastProfileFetchMillis = millis();

        _slackError = profile.error;

        if (!_slackError)
        {
            _slackDisplayName = profile.displayName;
            _slackStatusText = profile.statusText;

            Serial.println(F("--------- Profile ---------"));
            Serial.print(F("Display Name: ["));     Serial.print(profile.displayName); Serial.println(F("]"));
            Serial.print(F("Status Text: ["));      Serial.print(profile.statusText);  Serial.println(F("]"));
            Serial.print(F("Status Emoji: ["));     Serial.print(profile.statusEmoji); Serial.println(F("]"));
            Serial.print(F("Status Expiration: ")); Serial.println(profile.statusExpiration);
            Serial.println(F("------------------------"));
        } 
        else 
        {
            Serial.println(F("error getting profile"));
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
        _lastUserInteractionMillis = millis();

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
    void onRotaryClick() 
    {
        _lastUserInteractionMillis = millis();
        
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
        
        SlackProfile profile = _slack->setCustomStatus(status.title, status.icon, expireUTC);
        setSlackProfile(profile);

        _isDirty = true;
    }

    /***********************************************/
    void onRotaryDoubleClick()
    {
        _lastUserInteractionMillis = millis();

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
        _lastUserInteractionMillis = millis();

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

