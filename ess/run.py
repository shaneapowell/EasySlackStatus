# A quick and easy entry point into the primary run of EasySlackStatus
# Generally simply called by adding
#   import ess.run
# into the main.py file.
# Can also be easily manually run from the ampy command line for debugging and development
import uasyncio
import time
import micropython
import esp
import json
import ntptime
import network
import ess.log as log
import ess.time
import ess.const as const
import ess.display as display
import ess.encoder as encoder
import ess.slack as slack
import lib.utimezone as tz
import lib.utelnetserver

DEBUG_ENABLE_AMPY_PING  = False     # Ampy times out without a regular output to it's stdin
DEBUG_ENABLE_MEMINFO    = False     # Dump the current mem info
DEBUG_ENABLE_WIFIINFO   = False     # Dump the wifi info

_config = None
_wifi = None

def _loadConfig() -> bool:
    """
    """
    global _config
    try:
        f = open(const.CFG_FILENAME, 'r')
    except Exception as e:
        log.error(e)
        display.renderGeneralError(const.CFG_FILENAME, const.ERR_NOT_FOUND)
        return False

    try:
        _config = json.load(f)
    except Exception as e:
        log.error(e)
        display.renderGeneralError(const.CFG_FILENAME, 'json error')
        return False

    # Look for required keys
    if const.CFG_KEY_WIFI_SSID not in _config.keys():
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_WIFI_SSID, const.ERR_NOT_FOUND)
        return False

    if const.CFG_KEY_WIFI_PASS not in _config.keys():
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_WIFI_PASS, const.ERR_NOT_FOUND)
        return False

    if const.CFG_KEY_SLACK_TOKEN not in _config.keys():
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_SLACK_TOKEN, const.ERR_NOT_FOUND)
        return False

    if _config[const.CFG_KEY_SLACK_TOKEN] == None:
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_SLACK_TOKEN, const.ERR_NOT_SET)
        return False

    if const.CFG_KEY_STATUS_LIST not in _config.keys():
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_STATUS_LIST, const.ERR_NOT_FOUND)
        return False

    if const.CFG_KEY_TIMEZONE not in _config.keys():
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_TIMEZONE, const.ERR_NOT_FOUND)
        return False

    try:
        ess.time.setTimezone(_config[const.CFG_KEY_TIMEZONE])
    except Exception as e:
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_TIMEZONE, f"{_config[const.CFG_KEY_TIMEZONE]}", const.ERR_NOT_FOUND)
        return False

    if len(_config[const.CFG_KEY_STATUS_LIST]) == 0:
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_STATUS_LIST, "empty")
        return False

    if len(_config[const.CFG_KEY_STATUS_LIST]) > const.CFG_MAX_STATUS_COUNT:
        display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_STATUS_LIST, f"Too Many: {len(_config[const.CFG_KEY_STATUS_LIST])}", f"Max {const.CFG_MAX_STATUS_COUNT}")
        return False

    for index, status in enumerate(_config[const.CFG_KEY_STATUS_LIST]):

        # status.name
        if not const.CFG_KEY_STATUS_ITEM_STATUS in status.keys() or status[const.CFG_KEY_STATUS_ITEM_STATUS] == None:
            display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_STATUS_LIST, f"Item {index}", f"missing {const.CFG_KEY_STATUS_ITEM_STATUS}")
            return False

        # status.emoji
        if not const.CFG_KEY_STATUS_ITEM_EMOJI in status.keys() or status[const.CFG_KEY_STATUS_ITEM_EMOJI] == None:
            display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_STATUS_LIST, f"Item {index}", f"missing {const.CFG_KEY_STATUS_ITEM_EMOJI}")
            return False

        # status.expiry
        if not const.CFG_KEY_STATUS_ITEM_EXPIRY in status.keys() or status[const.CFG_KEY_STATUS_ITEM_EXPIRY] == None:
            display.renderGeneralError(const.CFG_FILENAME, const.CFG_KEY_STATUS_LIST, f"Item {index}", f"missing {const.CFG_KEY_STATUS_ITEM_EXPIRY}")
            return False

    return True



def _setup() -> bool:
    """
    """
    # Splash screen while we load everything
    display.renderSplash()

    # The display module needs the config for the status values to render
    display.setup(_config)

    # Load config.json
    if not _loadConfig():
        return False

    # Connect encoder to Display
    encoder.setOnIncreaseCallback(display.onEncoderIncrease)
    encoder.setOnDecreaseCallback(display.onEncoderDecrease)
    encoder.setOnClickCallback(display.onEncoderClick)
    encoder.setOnDoubleClickCallback(display.onEncoderDoubleClick)
    encoder.setOnLongClickCallback(display.onEncoderLongClick)

    # The display module needs the config for the status values to render
    display.setup(_config)

    # Connect Slack to Display
    slack.setup(_config[const.CFG_KEY_SLACK_TOKEN])
    slack.setOnStatusUpdateCallback(display.setTopStatus)

    # Connect the Display back to Slack
    display.setOnSendSlackStatusCallback(slack.sendNewStatus)

    # Pause a moment for the splash
    time.sleep(1)

    # Start wifi connection loop
    global _wifi
    _wifi = network.WLAN(network.STA_IF)
    try:
        log.info(__name__, "Get Wifi Interface")
        _wifi = network.WLAN(network.STA_IF)
        log.info(__name__, "Activate Wifi Interface")
        _wifi.active(True)
        _wifi.disconnect()
        log.info(__name__, f"Connect Wifi to {_config[const.CFG_KEY_WIFI_SSID]}")
        _wifi.connect(_config[const.CFG_KEY_WIFI_SSID], _config[const.CFG_KEY_WIFI_PASS])
        log.info(__name__, "Connection Complete")
    except Exception as e:
        log.error(__name__, "Unknown Wifi Error")
        log.error(__name__, str(e))
        return False


    # Enable the telnet server for debugging?
    if const.CFG_KEY_ENABLE_TELNET in _config and _config[const.CFG_KEY_ENABLE_TELNET] == True:
        lib.utelnetserver.start()


    return True



async def _dumpDebugInfoLoop():

    log.info(__name__, "Start Dump Debugging Loop...")

    ampyPing = DEBUG_ENABLE_AMPY_PING

    while True:
        await uasyncio.sleep_ms(5000)

        if DEBUG_ENABLE_WIFIINFO:
            ampyPing = False
            try:

                if _wifi != None:
                    log.info(__name__, f"{_wifi.ifconfig()} rssi[{_wifi.status('rssi')}]")
                else:
                    log.info(__name__, "No Wifi")

            except Exception as e:
                log.warn(__name__, "Failed to obtain wifi debug info - " + str(e))
                pass

        if DEBUG_ENABLE_MEMINFO:
            ampyPing = False
            micropython.mem_info()

        if ampyPing:
            print(".")



async def _wifiStatusLoop():
    """
    Check the network status and send to the display.
    This is done every 1 second, since we over-use this
    function to also send the clock to the bottom status line
    """
    log.info(__name__, "Start Network/Time Status Loop ...")

    while True:

        # -1 = error
        # 0  = not connected
        # 1  = connecting
        # 2  = connected
        wifiStrength = const.WIFI_STRONG
        wifiState = const.WIFI_NOT_CONNECTED
        status = _wifi.status()

        if status == network.STAT_IDLE:
            wifiState = const.WIFI_NOT_CONNECTED

        if status == network.STAT_CONNECTING:
            wifiState = const.WIFI_CONNECTING

        if status == network.STAT_WRONG_PASSWORD:
            wifiState = const.WIFI_ERROR

        if status == network.STAT_NO_AP_FOUND:
            wifiState = const.WIFI_ERROR

        # TODO: Complete list for esp32?
        # if status == network.STAT_CONNECT_FAIL:
        #     display.setBottomStatus("Wifi Error")

        if status == network.STAT_GOT_IP:
            wifiState = const.WIFI_CONNECTED
            wifiStrength = _wifi.status('rssi')

            if wifiStrength < -75:
                wifiStrength = const.WIFI_WEAK
            elif wifiStrength < -60:
                wifiStrength = const.WIFI_MEDIUM
            else:
                wifiStrength = const.WIFI_STRONG

        # Date/Time
        now = ess.time.gmnow()
        day = ess.time.weekday(now[0], now[1], now[2])
        hour = ess.time.to12h(now[3], True)

        display.setBottomStatus(wifiState, wifiStrength, f"{day[1]} {hour}:{now[4]:02}:{now[5]:02}")

        # Enable / Disable slack processing
        slack.enable(wifiState == const.WIFI_CONNECTED)

        await uasyncio.sleep_ms(1000)


async def _timeSyncLoop():
    """
    Run very infrequently to re-sync the ntp time
    """
    sleepInterval = 2000

    while True:

        try:
            if _wifi.status() == network.STAT_GOT_IP:
                ntptime.settime()
                sleepInterval = (1000 * 60 * 60 * 6) # Every 6 hours
        except Exception as e:
            log.warn(__name__, f"ntp sync error {e}. Retrying")
            pass

        await uasyncio.sleep_ms(sleepInterval)


async def _main():
    """
    """
    if not _setup():
        return

    await uasyncio.gather(
        _dumpDebugInfoLoop(),
        _wifiStatusLoop(),
        _timeSyncLoop(),
        encoder.loop(),
        slack.loop(),
        display.loop(),
    )



def start():
    """
    """
    log.info(__name__, "EasySlackStatus Starting....")
    uasyncio.get_event_loop().run_until_complete( _main() )
    log.info(__name__, "EasySlackStatus Stopped")
