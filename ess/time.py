import time
import lib.utimezone as tz
import lib.utzlist as utzlist

WEEKDAYS= ['Sun', 'Mon', 'Tue', 'Wed', 'Thr', 'Fri', 'Sat']


TZ = utzlist.America_Central

def setTimezone(tzname: str):
    """
    """
    global TZ
    if tzname not in utzlist.TIMEZONES.keys():
        raise Exception(f"Timezone {tzname} not found in utzlist.TIMEZONES list")
    TZ = utzlist.TIMEZONES[tzname]

def now() -> int:
    """
    return the Epoch int time adjusted for the setTimezone
    """
    return TZ.toLocal(time.time())

def gmnow() -> tuple:
    """
    returns the gm tupple time adjusted for the setTimezone
    (2022, 7, 13, 20, 41, 21, 2, 194)
    """
    return time.gmtime(now())


def weekday(year: int, month: int, day: int) :
    """
    Return a tuple of the dow[0] int, and it's string[1] value
    """
    offset = [0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334]
    afterFeb = 1
    if month > 2: afterFeb = 0
    aux = year - 1700 - afterFeb
    # dayOfWeek for 1700/1/1 = 5, Friday
    dayOfWeek  = 5
    # partial sum of days betweem current date and 1700/1/1
    dayOfWeek += (aux + afterFeb) * 365                  
    # leap year correction    
    dayOfWeek += aux / 4 - aux / 100 + (aux + 100) / 400     
    # sum monthly and day offsets
    dayOfWeek += offset[month - 1] + (day - 1)               
    dayOfWeek %= 7
    dayOfWeek = int(dayOfWeek)
    return dayOfWeek, WEEKDAYS[dayOfWeek]
    

def to12h(h: int, convert0to12: bool = False):
    """
    Simply converts a 0-23 hour value to a 0-11 value. 
    For presentation, pass true to convert0to12
    """
    h = h % 12
    if convert0to12 and h == 0:
        return 12
    return h

def toISO8601(t: int):
    """
    """
    g = time.gmtime(t)
    iso = f"{g[0]}-{g[1]}-{g[2]}T{g[3]}:{g[4]}:{g[5]}.{g[6]}"
    return iso