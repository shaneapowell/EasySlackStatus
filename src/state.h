
/************************************************
 * Sinleton App State variables
 ***********************************************/

#pragma once

typedef enum 
{
    SCREEN_MAIN,
    SCREEN_SETTINGS,
    SCREEN_WIFI
} SCREEN;

typedef struct
{
    String status;
    String icon;
    int timeout;
} Status;

const int STATUS_COUNT = 7;
Status ALL_STATUS[STATUS_COUNT] = 
{
    {"Available", ":thumbsup:", 0},
    {"Coffee", ":coffee:", 15},
    {"Lunch", ":hamburger:", 45},
    {"Meeting", ":calendar:", 60},
    {"Offline", ":x:", 0},
    {"Doctor", ":hospital", 90},
    {"Day off", ":palm_tree:", (24*60)}
};

typedef struct
{
    SCREEN currentScreen = SCREEN_MAIN;
    Status currentStatus = ALL_STATUS[0];
} State;

