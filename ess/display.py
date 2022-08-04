import sys
import uasyncio
from lib.sh1106 import SH1106_I2C
import time
import network
from machine import Pin, SoftI2C
import ess.const as const
import ess.log as log

# 60 minutes
SCREEN_OFF_INTERVAL_MS = (60 * 60 * 1000)

# Display Modes
DISPLAY_MODE_MAIN                    = 0
DISPLAY_MODE_WIFI_STATUS             = 1
DISPLAY_MODE_SLACK_EXPIRE_SELECT     = 2

# Text Size 1
TS1_FONT_HEIGHT = 8  # Default FreameBuffer font
TS1_FONT_WIDTH = 8
TS1_MARGIN = 1
TS1_STATUS_LINE_HEIGHT = 12
TS1_LINE_HEIGHT = 10

# Y:  H    font + margin
# 0:  12  (8 + 4) top aligned; plus space; line; space
# 12: 10  (8 + 2) v centered
# 22: 10  (8 + 2) v centered
# 32: 10  (8 + 2) v centered
# 42: 10  (8 + 2) v centered
# 52: 12  (8 + 4) bottom aligned; plus space; line; space
TS1_STATUS_LINE_TOP = 0
TS1_LINE0 = 12
TS1_LINE1 = 22
TS1_LINE2 = 32
TS1_LINE3 = 42
TS1_STATUS_LINE_BOTTOM = 52
TS1_LINES = [TS1_LINE0, TS1_LINE1, TS1_LINE2, TS1_LINE3]
TS1_LINE_COUNT = len(TS1_LINES)


# Wifi Icons
WIFI_ICONS = {
    const.WIFI_ERROR: [
        [0,0,0,1,1,0,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,0,0,0,0,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,0,1,1,0,0,0]
    ],
    const.WIFI_NOT_CONNECTED: [
        [0,0,1,1,1,1,0,0],
        [0,1,1,0,0,1,1,0],
        [1,1,1,1,0,0,1,1],
        [1,0,1,1,1,0,0,1],
        [1,0,0,1,1,1,0,1],
        [1,1,0,0,1,1,1,1],
        [0,1,1,0,0,1,1,0],
        [0,0,1,1,1,1,0,0]
    ],
    const.WIFI_CONNECTING: [
        [1,1,1,1,1,1,1,1],
        [0,1,0,0,0,0,1,0],
        [0,0,1,0,0,1,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,0,1,1,0,0,0],
        [0,0,1,0,0,1,0,0],
        [0,1,0,0,0,0,1,0],
        [1,1,1,1,1,1,1,1]
    ],
    const.WIFI_CONNECTED: [
        [0,0,0,0,0,0,0,0,0,1,1],
        [0,0,0,0,0,0,0,0,0,1,1],
        [0,0,0,0,0,0,1,1,0,1,1],
        [0,0,0,0,0,0,1,1,0,1,1],
        [0,0,0,1,1,0,1,1,0,1,1],
        [0,0,0,1,1,0,1,1,0,1,1],
        [1,1,0,1,1,0,1,1,0,1,1],
        [1,1,0,1,1,0,1,1,0,1,1],
    ],
}



# Shared Config
_config: dict = None

# Setup
_i2c: SoftI2C = SoftI2C(scl=Pin(const.PIN_OLED_CLOCK), sda=Pin(const.PIN_OLED_DATA), freq=400000)
_display: SH1106_I2C = SH1106_I2C(width=128, height=64, i2c=_i2c, rotate=180)
_display.sleep(False)
_display.fill(0)
_display.show()

# Input Status
_lastInputMillis = 0

# Main Body Status
_displayMode = DISPLAY_MODE_MAIN
_isScreenOn = True
_isBodyDirty = True

# Slack Status
_topStatusName = "EasySlackStatus"
_topStatusStatus = None
_isTopStatusDirty = True
_sendSlackStatusCallback = None

# Bottom Status bar
_bottomLeftWifiStatus = const.WIFI_NOT_CONNECTED
_bottomLeftWifiStrength = const.WIFI_STRONG
_bottomRightStatus = ""
_isBottomStatusDirty = True

# Encode State
_topStatusToRenderIndex = 0  # The status values within the config.json. We render 4, starting at this index.
_itemHighlightIndex = 0      # Which currently rendered status to highlight. This is a 0-4 offset of the above index

def setup(cfg: dict):
    """
    """
    global _config
    _config = cfg

def renderSplash():
    """
    """
    _display.fill(0)
    _display.text('Easy',   5, TS1_STATUS_LINE_TOP)
    _display.text('Slack',  15, TS1_LINE0)
    _display.text('Status', 25, TS1_LINE1)
    _display.text('Shane Powell', 0, TS1_LINE3)
    _display.text('v2.0', 0, TS1_STATUS_LINE_BOTTOM)
    _display.show()


def renderGeneralError(line1: str = "", line2: str = "", line3: str = "", line4 = ""):
    """
    """
    _display.fill(0)
    _display.text('ERROR:', 0, TS1_STATUS_LINE_TOP)
    _display.text(line1, 0, TS1_LINE0)
    _display.text(line2, 0, TS1_LINE1)
    _display.text(line3, 0, TS1_LINE2)
    _display.text(line4, 0, TS1_LINE3)
    _display.show()


def setTopStatus(name: str, status: str = None):
    """
    Should nearly always be the slack status.
    Name and Status.
    the Status is optional. If it's not provided, only the name is drawn with no '-' separator
    """
    global _topStatusName
    global _topStatusStatus
    global _isTopStatusDirty

    if _topStatusName != name or _topStatusStatus != status:
        _topStatusName = name.strip()
        _topStatusStatus = status
        _isTopStatusDirty = True


def setBottomStatus(wifiState: int, wifiStrength: int, rText: str = ""):
    """
    wifiState = const.WIFI_CONNECTED, const.WIFI_ERROR, const.WIFI_NOT_CONNTECTED, const.WIFI_CONNECTING
    wifiStrength = const.WIFI_WEAK, const.WIFI_MEDIUM, const.WIFI_STRONG
    rText is right justified. Timestamp
    """
    global _bottomLeftWifiStatus
    global _bottomLeftWifiStrength
    global _bottomRightStatus
    global _isBottomStatusDirty

    _bottomLeftWifiStatus = wifiState
    _bottomLeftWifiStrength = wifiStrength

    if rText != _bottomRightStatus:
        _bottomRightStatus = rText
        _isBottomStatusDirty = True


def onEncoderIncrease():
    """
    """
    global _lastInputMillis
    _lastInputMillis = time.ticks_ms()

    if not _isScreenOn:
        return

    if _displayMode == DISPLAY_MODE_MAIN:
        global _itemHighlightIndex
        _itemHighlightIndex += 1

        if _itemHighlightIndex > 3:
            _scrollStatusListDown()
            _itemHighlightIndex = 3

    if _displayMode == DISPLAY_MODE_SLACK_EXPIRE_SELECT:
        status = _config[const.CFG_KEY_STATUS_LIST][_topStatusToRenderIndex + _itemHighlightIndex]
        expiry  = status[const.CFG_KEY_STATUS_ITEM_EXPIRY] + 1
        status[const.CFG_KEY_STATUS_ITEM_EXPIRY] = min(const.SLACK_MAX_EXPIRY_MINUTES, expiry)

    global _isBodyDirty
    _isBodyDirty = True

def onEncoderDecrease():
    """
    """
    global _lastInputMillis
    _lastInputMillis = time.ticks_ms()

    if not _isScreenOn:
        return

    if _displayMode == DISPLAY_MODE_MAIN:
        global _itemHighlightIndex
        _itemHighlightIndex -= 1

        if _itemHighlightIndex < 0:
            _scrollStatusListUp()
            _itemHighlightIndex = 0

    if _displayMode == DISPLAY_MODE_SLACK_EXPIRE_SELECT:
        status = _config[const.CFG_KEY_STATUS_LIST][_topStatusToRenderIndex + _itemHighlightIndex]
        expiry  = status[const.CFG_KEY_STATUS_ITEM_EXPIRY] - 1
        status[const.CFG_KEY_STATUS_ITEM_EXPIRY] = max(0, expiry)

    global _isBodyDirty
    _isBodyDirty = True


def onEncoderClick():
    """
    """
    global _lastInputMillis
    _lastInputMillis = time.ticks_ms()

    if not _isScreenOn:
        return

    log.info(__name__, "Encoder Click")

    global _displayMode
    if _displayMode == DISPLAY_MODE_MAIN or _displayMode == DISPLAY_MODE_SLACK_EXPIRE_SELECT:
        status = _config[const.CFG_KEY_STATUS_LIST][_topStatusToRenderIndex + _itemHighlightIndex]
        _sendSlackStatusCallback(status[const.CFG_KEY_STATUS_ITEM_STATUS], status[const.CFG_KEY_STATUS_ITEM_EMOJI], status[const.CFG_KEY_STATUS_ITEM_EXPIRY])
        _displayMode = DISPLAY_MODE_MAIN

    global _isBodyDirty
    _isBodyDirty = True

def onEncoderDoubleClick():
    """
    """
    global _lastInputMillis
    _lastInputMillis = time.ticks_ms()

    if not _isScreenOn:
        return

    log.info(__name__, "Encoder Double Click")

    global _displayMode

    if _displayMode == DISPLAY_MODE_MAIN:
        _displayMode = DISPLAY_MODE_SLACK_EXPIRE_SELECT

    global _isBodyDirty
    _isBodyDirty = True


def onEncoderLongClick():
    """
    """
    global _lastInputMillis
    _lastInputMillis = time.ticks_ms()

    if not _isScreenOn:
        return

    log.info(__name__, "Encoder Long Click.")
    global _isBodyDirty
    global _displayMode

    if _displayMode == DISPLAY_MODE_MAIN:
        _displayMode = DISPLAY_MODE_WIFI_STATUS
    elif _displayMode == DISPLAY_MODE_WIFI_STATUS:
        _displayMode = DISPLAY_MODE_MAIN

    _isBodyDirty = True



def _renderTopStatusLine() -> bool:
    """
    """
    global _isTopStatusDirty

    if _isTopStatusDirty:

        # Clear
        _display.fill_rect(0, TS1_STATUS_LINE_TOP, _display.width, TS1_STATUS_LINE_HEIGHT, 0)

        line = _topStatusName
        if _topStatusStatus != None and _topStatusStatus != "":
            line = f"{_topStatusName[:7]}={_topStatusStatus}"

        # Render Name at Left
        y = TS1_STATUS_LINE_TOP
        _display.text(line, 0, y)
        y += TS1_FONT_HEIGHT + 1
        _display.line(0, y, _display.width, y, 1)

        _isTopStatusDirty = False
        return True

    return False


def _renderBottomStatusLine() -> bool:
    """
    """

    global _isBottomStatusDirty

    if _isBottomStatusDirty:

        # Clear
        _display.fill_rect(0, TS1_STATUS_LINE_BOTTOM, _display.width, TS1_STATUS_LINE_HEIGHT, 0)

        # Render, on the bottom
        y = _display.height - TS1_FONT_HEIGHT

        # Left Wifi Status and strength
        x = 1
        iconArray = WIFI_ICONS[_bottomLeftWifiStatus]
        for rowIndex, row in enumerate(iconArray):
            for colIndex, col in enumerate(row):
                # Adjust the icon based on strength. The strength should ONLY be not const.WIFI_STRONG when actually connected with an IP
                # Only not strong should be adjusted. And we never affect the bottom 2 rows.
                if  rowIndex <= 6 and _bottomLeftWifiStrength != const.WIFI_STRONG:
                    if _bottomLeftWifiStrength == const.WIFI_MEDIUM and colIndex >= 8:
                        col = 0
                    elif _bottomLeftWifiStrength == const.WIFI_WEAK and colIndex >= 6:
                        col = 0

                _display.pixel(x + colIndex, y + rowIndex, col)

        # Right Status
        x = _display.width - (len(_bottomRightStatus) * TS1_FONT_WIDTH)
        _display.text(_bottomRightStatus, x, y)

        # Sep Line
        y = TS1_STATUS_LINE_BOTTOM + 1
        _display.line(0, y, _display.width, y, 1)

        _isBottomStatusDirty = False
        return True

    return False

def _renderItemLine(y: int, item: str, highlighted: bool = False ):
    """
    """
    _display.fill_rect(0, y, _display.width, TS1_LINE_HEIGHT, highlighted)
    _display.text(item, 1, y + 1, not highlighted)


def _renderStatusSelectionBody() -> bool:
    """
    Render the main status selection list body on the screen.
    """
    if _displayMode == DISPLAY_MODE_MAIN:

        global _isBodyDirty

        if _isBodyDirty:
            for i in range(len(TS1_LINES)):
                stat = _config[const.CFG_KEY_STATUS_LIST][_topStatusToRenderIndex + i]
                if const.CFG_KEY_STATUS_ITEM_DISPLAY in stat.keys():
                    text = stat[const.CFG_KEY_STATUS_ITEM_DISPLAY]
                else:
                    text = stat[const.CFG_KEY_STATUS_ITEM_STATUS]
                _renderItemLine(TS1_LINES[i], text, _itemHighlightIndex == i)
            _isBodyDirty = False
            return True

    return False

def _renderSelectExpiryBody() -> bool:
    """
    Render the Expiry Minutes selection screen in the main body
    """
    if _displayMode == DISPLAY_MODE_SLACK_EXPIRE_SELECT:

        global _isBodyDirty
        if _isBodyDirty:

            status = _config[const.CFG_KEY_STATUS_LIST][_topStatusToRenderIndex + _itemHighlightIndex]
            expiry = status[const.CFG_KEY_STATUS_ITEM_EXPIRY]
            if const.CFG_KEY_STATUS_ITEM_DISPLAY in status.keys():
                status = status[const.CFG_KEY_STATUS_ITEM_DISPLAY]
            else:
                status = status[const.CFG_KEY_STATUS_ITEM_STATUS]

            _renderItemLine(TS1_LINE0, status)
            _renderItemLine(TS1_LINE1, f"  Expire in {expiry}", True)
            _renderItemLine(TS1_LINE2,  "   Minutes")
            _renderItemLine(TS1_LINE3,  "")

            _isBodyDirty = False
            return True

    return False


def _renderWifiInfoBody() -> bool:
    """
    Render the wifi-status info screen into the body area.
    """
    if _displayMode == DISPLAY_MODE_WIFI_STATUS:

        global _isBodyDirty

        if _isBodyDirty:

            wifi = network.WLAN(network.STA_IF)
            ssid = _config[const.CFG_KEY_WIFI_SSID]
            ip = wifi.ifconfig()[0]
            gw = wifi.ifconfig()[2]
            rssi = 0
            try:
                rssi = wifi.status('rssi')
            except:
                pass

            _renderItemLine(TS1_LINE0, ssid, False)
            _renderItemLine(TS1_LINE1, ip, False)
            _renderItemLine(TS1_LINE2, gw, False)
            _renderItemLine(TS1_LINE3, f"RSSI: {rssi}", False)

        _isBodyDirty = False
        return True

    return False


def _renderMainScreen():
    """
    """
    show = _renderTopStatusLine()
    show = _renderStatusSelectionBody() or show
    show = _renderSelectExpiryBody() or show
    show =_renderWifiInfoBody() or show
    show = _renderBottomStatusLine() or show

    if show:
       _display.show()


def _scrollStatusListDown():
    """
    """
    global _topStatusToRenderIndex
    _topStatusToRenderIndex += 1
    _topStatusToRenderIndex = min(_topStatusToRenderIndex, len(_config[const.CFG_KEY_STATUS_LIST]) - TS1_LINE_COUNT)


def _scrollStatusListUp():
    """
    """
    global _topStatusToRenderIndex
    _topStatusToRenderIndex -= 1
    _topStatusToRenderIndex = max(_topStatusToRenderIndex, 0)

def setOnSendSlackStatusCallback(func):
    """
    The status subsystem callback to send a new status to
    """
    global _sendSlackStatusCallback
    _sendSlackStatusCallback = func


async def loop():
    """
    Watch for the isDirty to be set, and render when it is
    """
    global _isBodyDirty
    global _lastInputMillis
    global _isScreenOn

    screenSleepLogMs = 0

    while True:

        now = time.ticks_ms()

        # Screen Sleep or not
        if _lastInputMillis + SCREEN_OFF_INTERVAL_MS < now:
            if _isScreenOn != False:
                log.info(__name__, "Screen Sleep")
            _isScreenOn = False
            _display.sleep(True)
            await uasyncio.sleep_ms(250)
        else:
            if _isScreenOn != True:
                log.info(__name__, "Screen Wake")
            _isScreenOn = True
            _display.sleep(False)
            _renderMainScreen()
            await uasyncio.sleep_ms(10)

            if screenSleepLogMs < now:
                screenSleepLogMs = now + 1000 * 60 * 5
                m = int(_lastInputMillis + SCREEN_OFF_INTERVAL_MS - now) / 1000 / 60
                log.debug(__name__, f"Screen Sleep in [{m}] minutes")
