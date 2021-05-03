
/************************************************
 * Sinleton App State variables
 ***********************************************/

#ifndef __STATE_H__
#define __STATE_H__

typedef enum 
{
    SCREEN_MAIN,
    SCREEN_SETTINGS,
    SCREEN_WIFI
} SCREEN;

typedef struct
{
    String title;
    String icon;
    int timeout;
} SlackStatus;

const int SLACK_STATUS_COUNT = 7;
SlackStatus ALL_SLACK_STATUS[SLACK_STATUS_COUNT] = 
{
    {"Available", ":thumbsup:", 0},
    {"Coffee", ":coffee:", 15},
    {"Lunch", ":hamburger:", 45},
    {"Meeting", ":calendar:", 60},
    {"Offline", ":x:", 0},
    {"Doctor", ":hospital", 90},
    {"Day off", ":palm_tree:", (24*60)}
};

#endif