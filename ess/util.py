"""
A few common util functions
"""
import utime as time


class Interval:
    """
    A simple interval checker utility class.
    Tracks the previous "trigger" and when the "next" trigger has passed.
    Used by the asyncio looper functions to control when to actually fire off a block of code.
    """

    _prevMs = None
    _intervalMs = None
    _enabled = True

    def __init__(self, intervalMs, startEnabled: bool = True):
        self._intervalMs = intervalMs
        self._enabled = startEnabled

    def hasPassed(self) -> bool:
        """
        Checks if the interval has passed.
        If it has, resets the interval check, and returns true.
        If not, just returns false

        Uses the time.ticks_ms() value and checks if
        the interval has elapsed. This is able to handle the ticks_ms rollover from max back to 0.
        Unfortunately, since I dont actually know the int value that the chip will rollover at, a rollover will simply always return a value.
        prevMs: The previously triggered time in ms.  This is the value returned by this function.  You can pass None if this is a first call. A first call will ALWAYS return a non -1.
        intervalMs: The interval to check 
        returns: the new value for prevMs to be used if the interval passed.  -1 if the interval has not passed
        """
        if self._enabled == False:
            return False

        now = time.ticks_ms()

        # first time call?
        if self._prevMs == None:
            self._prevMs = now
            return True

        # RollOver?
        if self._prevMs > now:
            self._prevMs = now
            return True

        # Has the interval passed?
        if self._prevMs + self._intervalMs < now:
            self._prevMs = now
            return True
        
        return False

    def reset(self):
        """
        Does a reset that will ensure the next call to hasPassed is true
        """
        self._prevMs = None

    def enable(self, enable: bool, shouldReset: bool = False):
        """
        Enable or Disable this interval.
        This does not do any sort of reset, just ensures that this interval will
        always return False with hasPassed.
        Unless you pass true to shouldReset. This only resets if the enabled state
        actually changes.  In other words, if this module is already enabled, the reset will
        have no affect.
        Finally, this will return not teh enable status, but if the status actually changed.
        """
        changed: bool = self._enabled != enable
        self._enabled = enable
        if changed:
            self._prevMs = None
        return changed

    def isEnabled(self) -> bool:
        return self._enabled