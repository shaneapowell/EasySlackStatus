
/************************************************
 * Sinleton App State variables
 ***********************************************/

#ifndef __STATE_H__
#define __STATE_H__

typedef struct
{
    String title;
    String icon;
    int expireInMinutes;
} SlackStatus;

const int SLACK_STATUS_COUNT = 9;
SlackStatus ALL_SLACK_STATUS[SLACK_STATUS_COUNT] = 
{
    {"Available", ":computer:", 0},
    {"Coffee", ":coffee:", 15},
    {"Lunch", ":hamburger:", 60},
    {"Meeting", ":calendar:", 60},
    {"Offline", ":x:", 0},
    {"Walk Dog", ":dog2:", 30},
    {"Doctor", ":stethoscope:", 90},
    {"Day off", ":palm_tree:", 0},
    {"Sleeping", ":sleeping:", 0}
};

#endif