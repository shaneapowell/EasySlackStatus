import ess.time as time

COLOR_END       = '\033[0m'
COLOR_BLUE      = '\033[94m'
COLOR_GREEN     = '\033[92m'
COLOR_YELLOW    = '\033[93m'
COLOR_RED       = '\033[91m'
BG_RED          = '\u001b[41m'
UNDERLINE       = '\033[4m'

DEBUG  = COLOR_BLUE     + "DEBUG"   + COLOR_END
INFO   = COLOR_GREEN    + "INFO"    + COLOR_END
WARN   = COLOR_YELLOW   + "WARN"    + COLOR_END
ERROR  = COLOR_RED      + "ERROR"   + COLOR_END
FATAL  = BG_RED         + "FATAL"   + COLOR_END

def log(type: str, tag: str, msg: str):
    """
    Output a raw log messages.  
    Prefer to use one of the other functions for better formatting output.
    """
    t = time.now()
    dt = UNDERLINE + time.toISO8601(t) + COLOR_END
    print(f"\n[{dt}] {type} - {tag}\n{msg}")

def debug(tag, msg):
    log(DEBUG, tag, msg)

def info(tag, msg):
    log(INFO, tag, msg)

def warn(tag, msg):
    log(WARN, tag, msg)

def error(tag, msg):
    log(ERROR, tag, msg)

def fatal(tag, msg):
    log(FATAL, tag, msg)
