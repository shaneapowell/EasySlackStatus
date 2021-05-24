#ifndef __TZ_H__
#define __TZ_H__

#define TZ_COUNT     7
#define TZ_NAME_MAX  22

/* Dynamically set by the iotWebConfig interface */
char currentTZName[TZ_NAME_MAX] = {0};

/* 1:1 mapped tz names to timezones */
const char TZ_NAMES[TZ_COUNT][TZ_NAME_MAX] 
{
    "UTC",
    "America/LA",
    "America/Phoenix",
    "America/Denver",
    "America/Chicago",
    "America/NewYork",
    "Argentina/BuenosAires"
};

/* All defined timezones. Must be 1:1 mapped to above names array */
const Timezone TIMEZONES[TZ_COUNT] = 
{ 
    Timezone(
        TimeChangeRule{"UTC", First, Sun, Nov, 2,  0},  //UTC - 8 hours
        TimeChangeRule{"UTC", First, Sun, Nov, 2,  0}  //UTC - 8 hours
    ),

    Timezone(
        TimeChangeRule{"PDT", Second, Sun, Mar, 2, -(7*60)}, //UTC - 7 hours
        TimeChangeRule{"PST", First, Sun, Nov, 2,  -(8*60)}  //UTC - 8 hours
    ),

    Timezone(
        TimeChangeRule{"MST", First, Sun, Nov, 2,  -(7*60)}  //UTC - 7 hours
    ),

    Timezone(
        TimeChangeRule{"MDT", Second, Sun, Mar, 2, -(6*60)}, //UTC - 6 hours
        TimeChangeRule{"MST", First, Sun, Nov, 2,  -(7*60)}  //UTC - 7 hours
    ),

    Timezone(
        TimeChangeRule{"CDT", Second, Sun, Mar, 2, -(5*60)}, //UTC - 5 hours
        TimeChangeRule{"CST", First, Sun, Nov, 2,  -(6*60)}  //UTC - 6 hours
    ),

    Timezone(
        TimeChangeRule{"EDT", Second, Sun, Mar, 2, -(4*60)}, //UTC - 4 hours
        TimeChangeRule{"EST", First, Sun, Nov, 2,  -(5*60)}  //UTC - 5 hours
    ),

    Timezone(
        TimeChangeRule{"AST", First, Sun, Nov, 2,  -(3*60)}  //UTC - 3 hours
    )

};

#endif