import uasyncio
import sys
import time
from machine import Pin
from rotary_irq_esp import RotaryIRQ
import ess.const as const


BTN_MIN_RELEASE_INTERVAL_MS = 350
LONG_PRESS_INTERVAL_MS      = 1500
DEBOUNCE_INTERVAL_MS        = 20

# Rotary
_rotary = RotaryIRQ(
    pin_num_clk = const.PIN_ROTARY_CLOCK,
    pin_num_dt = const.PIN_ROTARY_DATA,
    min_val = -999999,
    max_val = +999999,
    reverse = True,
    range_mode=RotaryIRQ.RANGE_WRAP)
_rotary.set(0)

_prevRotaryValue = _rotary.value()

_onIncreaseCallback: None
_onDecreaseCallback: None

# Button
_button = Pin(const.PIN_ROTARY_BUTTON, Pin.IN, Pin.PULL_UP)

# Callbacks
_onClickCallback: None
_onDoubleClickCallback: None
_onLongClickCallback: None


class ButtonClickEvent:
    isDown: bool
    timestamp: int
    timeSincePrevious: int = 0

    def __init__(self, d, eventQueue = None):
        """
        d = is the button down
        eventQueue = the current event queue. the element at [0] is used to calculate the timeSincePrevious
        """
        self.isDown = d
        self.timestamp = time.ticks_ms()

        if eventQueue != None and len(eventQueue) > 0:
            e = eventQueue[0]
            self.timeSincePrevious = self.timestamp - e.timestamp

    def __str__(self):
        return f"[{self.isDown}/{self.timestamp}/{self.timeSincePrevious}]"


def setOnIncreaseCallback(func):
    """
    """
    global _onIncreaseCallback
    _onIncreaseCallback = func


def setOnDecreaseCallback(func):
    """
    """
    global _onDecreaseCallback
    _onDecreaseCallback = func


def setOnClickCallback(func):
    """
    """
    global _onClickCallback
    _onClickCallback = func


def setOnDoubleClickCallback(func):
    """
    """
    global _onDoubleClickCallback
    _onDoubleClickCallback = func


def setOnLongClickCallback(func):
    """
    """
    global _onLongClickCallback
    _onLongClickCallback = func


def _onRotaryChange():
    """
    """

    global _prevRotaryValue
    global _onIncreaseCallback
    global _onDecreaseCallback

    newRotaryVal = _rotary.value()

    if _onIncreaseCallback != None and newRotaryVal > _prevRotaryValue:
        _onIncreaseCallback()

    if _onDecreaseCallback != None and newRotaryVal < _prevRotaryValue:
        _onDecreaseCallback()
        
    _prevRotaryValue = newRotaryVal




async def loop():
    """
    An async loop to do our debounce and press length tracking
    """
    # stateAtMillis = None
    # buttonState = 0

    # A short sequence array of button states.
    # A state is both the state const, as well as the millis it was triggered.
    # This array is a simple FIFO.  element [0] is the youngest, [1] next oldest, etc.
    # We'll track only the last 4 events for now, even though it seems like only 1 - 3
    # are needed.
    btnEvtQueue = []

    while True:

        await uasyncio.sleep_ms(DEBOUNCE_INTERVAL_MS)

        # Button is pulled up 1 when not pressed
        btnDown: bool = _button.value() == 0
        now = time.ticks_ms()

        # Add to our queue if the queue is empty, or if the current is a different state type
        if len(btnEvtQueue) == 0 or btnEvtQueue[0].isDown != btnDown:
            btnEvtQueue.insert(0, ButtonClickEvent(btnDown, btnEvtQueue))
            btnEvtQueue = btnEvtQueue[:4]

        # Sanity Check
        if len(btnEvtQueue) > 4:
            raise Exception("Button State Queue grew beyond 4.  Oops")
        

        # Double Click - 4 events
        if len(btnEvtQueue) >= 4:
            e0: ButtonClickEvent = btnEvtQueue[0]
            e1: ButtonClickEvent = btnEvtQueue[1]
            e2: ButtonClickEvent = btnEvtQueue[2]
            e3: ButtonClickEvent = btnEvtQueue[3]
            # DOWN -> UP -> DOWN -> UP
            if e3.isDown == True and e2.isDown == False and e1.isDown == True and e0.isDown == False:
                if e2.timeSincePrevious < BTN_MIN_RELEASE_INTERVAL_MS:
                    if e1.timeSincePrevious < BTN_MIN_RELEASE_INTERVAL_MS:
                        if e0.timeSincePrevious < BTN_MIN_RELEASE_INTERVAL_MS:
                            if _onDoubleClickCallback != None:
                                _onDoubleClickCallback()
                            btnEvtQueue.clear()


        ### Single Click - 2 events
        if len(btnEvtQueue) >= 2:
            e0: ButtonClickEvent = btnEvtQueue[0]
            e1: ButtonClickEvent = btnEvtQueue[1]
            # DOWN -> UP
            if e1.isDown == True and e0.isDown == False:
                if e0.timeSincePrevious < BTN_MIN_RELEASE_INTERVAL_MS:
                    if now - e0.timestamp > BTN_MIN_RELEASE_INTERVAL_MS:
                        if _onClickCallback != None:
                            _onClickCallback()
                        btnEvtQueue.clear()


        ### Long Press - 1 event
        if len(btnEvtQueue) >= 1:
            e0: ButtonClickEvent = btnEvtQueue[0]
            if e0.isDown == True:
                if now - e0.timestamp > LONG_PRESS_INTERVAL_MS:
                    if _onLongClickCallback != None:
                        _onLongClickCallback()
                    btnEvtQueue.clear()

        

# Final Setup
_rotary.add_listener(_onRotaryChange)
