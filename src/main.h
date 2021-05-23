#ifndef __MAIN_H__
#define __MAIN_H__

#include <Time.h>

#define APP_VESION_MAJOR 1
#define APP_VESION_MINOR 0
#define APP_VESION_PATCH 0


PROGMEM const char APP_VERSION_CODE[] = { '0'+APP_VESION_MAJOR, '0'+APP_VESION_MINOR, '0'+APP_VESION_PATCH, 0 };
PROGMEM const char APP_VERSION_NAME[] = {'v', '0'+APP_VESION_MAJOR, '.', '0'+APP_VESION_MINOR, '.', '0'+APP_VESION_PATCH, 0};

time_t getNtpTime();

#define SLACK_ACCESS_FINGERPRINT "C3 CC ED 77 87 19 6D E7 76 5E AA A7 3D 67 7E CA 95 D2 46 E2"
#define MAX_SLACK_TOKEN_LENGTH 128


/* The most chars that fit across the OLED with the current font size */
const int SLACK_STATUS_CHARS_MAX = 10;
const int SLACK_STATUS_ICON_CHARS_MAX = 32;
const int SLACK_STATUS_EXPIRE_CHARS_MAX = 3;
const int SLACK_STATUS_COUNT = 15;

const char SLACK_STATUS_BLANK_STATUS[] = "---";
const char SLACK_STATUS_BLANK_ICON[] = ":O:";
const char SLACK_STATUS_BLANK_EXPIRE[] = "0";

typedef struct
{
    char title[SLACK_STATUS_CHARS_MAX+1];
    char icon[SLACK_STATUS_ICON_CHARS_MAX+1];
    char expireInMinutes[SLACK_STATUS_EXPIRE_CHARS_MAX+1];
} SlackStatus;

SlackStatus _slackStatusList[SLACK_STATUS_COUNT] = 
{
    {"Available",   ":white_check_mark:",   "0"},
    {"Coffee",      ":coffee:",             "15"},
    {"Lunch",       ":hamburger:",          "60"},
    {"Meeting",     ":calendar:",           "60"},
    {"OOO",         ":warning:",            "0"},
    {"Offline",     ":x:",                  "0"},
    {"Walk Dog",    ":dog2:",               "30"},
    {"On a Break",  ":hourglass:",          "30"},
    {"Very Busy",   ":lightning:",          "120"},
    {"Doctor",      ":stethoscope:",        "90"},
    {"PTO",         ":palm_tree:",          "0"},
    {"Sleeping",    ":sleeping:",           "0"},
    {"---",         ":O:",                  "0"},
    {"---",         ":O:",                  "0"},
    {"---",         ":O:",                  "0"}
};


#endif