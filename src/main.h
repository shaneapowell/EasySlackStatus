#ifndef __MAIN_H__
#define __MAIN_H__

#include <TimeLib.h>

#define APP_VESION_MAJOR 1
#define APP_VESION_MINOR 1
#define APP_VESION_PATCH 0


const char* APP_VERSION_NAME = "v1.1.0";

time_t getNtpTime();

#define SLACK_ACCESS_FINGERPRINT "82 AE FD 93 36 30 DA 03 0A 2F 63 53 DE 2E B0 43 8B F4 41 F6"
#define MAX_SLACK_TOKEN_LENGTH 128


/* The most chars that fit across the OLED with the current font size */
const int SLACK_STATUS_CHARS_MAX = 10;
const int SLACK_STATUS_ICON_CHARS_MAX = 32;
const int SLACK_STATUS_EXPIRE_CHARS_MAX = 3;
const int SLACK_STATUS_COUNT = 15;

const char SLACK_STATUS_BLANK_STATUS[]  = "---";
const char SLACK_STATUS_BLANK_ICON[]    = ":O:";
const char SLACK_STATUS_BLANK_EXPIRE[]  = "0";

typedef struct
{
    char title[SLACK_STATUS_CHARS_MAX+1];
    char icon[SLACK_STATUS_ICON_CHARS_MAX+1];
    char expireInMinutes[SLACK_STATUS_EXPIRE_CHARS_MAX+1];
} SlackStatus;

SlackStatus _slackStatusList[SLACK_STATUS_COUNT] = 
{
    {"Available",   ":here:",               "0"},
    {"Coffee",      ":coffee:",             "15"},
    {"Lunch",       ":hamburger:",          "60"},
    {"Meeting",     ":calendar:",           "60"},
    {"OOO",         ":away:",               "0"},
    {"Offline",     ":x:",                  "0"},
    {"Walk Dog",    ":walking-the-dog:",    "30"},
    {"On a Break",  ":hourglass:",          "30"},
    {"Very Busy",   ":heads-down:",         "120"},
    {"Doctor",      ":stethoscope:",        "90"},
    {"PTO",         ":palm_tree:",          "0"},
    {"Sleeping",    ":sleeping:",           "0"},
    {"---",         ":O:",                  "0"},
    {"---",         ":O:",                  "0"},
    {"---",         ":O:",                  "0"}
};


#endif
