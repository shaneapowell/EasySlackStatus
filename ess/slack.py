"""
Access the slack WebSocket, and provide a call-back hook for dissplay
"""
import _thread
import sys
import network
import uasyncio
import json
import time
import urequests as requests
import ess.util
import ess.const as const
import ess.log as log

STATUS_GETTING  = "Retrieving..."
STATUS_SENDING  = "Sending..."
STATUS_ERROR    = "Error"

SLACK_URL_GET_PROFILE = 'https://slack.com/api/users.profile.get'
SLACK_URL_SET_PROFILE = 'https://slack.com/api/users.profile.set'

SLACK_STATUS_FETCH_INTERVAL_MS = 1000 * 60

_token: str = "xoxo-NOT-SET"
_statusCallback = None
_newStatus = None

_shutdownCountdown = 5

# Updated by the fetch function. Can be re-set any time to force a new fetch.
# Start disabled since we need wifi to be ready before we can fetch anything
_lastStatusFetchInterval = ess.util.Interval(SLACK_STATUS_FETCH_INTERVAL_MS, False)


def _showStatus(name: str, status: str = None):
    """
    Shows the current slack status info line on the display
    """
    if _statusCallback != None:
        _statusCallback(name, status)



def _fetchCurrentStatus():
    """
    A blocking call that will fetch and update the current slack status.
    """

    if _lastStatusFetchInterval.hasPassed():

        try:
            log.info(__name__, "Fetching current status...")
            _showStatus(STATUS_GETTING)

            req = requests.get(SLACK_URL_GET_PROFILE, headers=_headers)
            
            if req.status_code != 200:
                _showStatus(f"HTTP[{req.status_code}]")
                log.error(__name__, f"Slack Profile Fetch failed with HTTP code [{req.status_code}]")
                return

            req = req.json()
            if req['ok'] == False:
                log.error(__name__, str(req))
                _showStatus(req['error'])
                return

            # First name only. Max
            userName = req['profile']['display_name_normalized'].split(' ')[0]
            userStatus = req['profile']['status_text']

            # Send the status to the display
            _showStatus(userName, userStatus)

            log.info(__name__, f"Fetch Complete [{userName}][{userStatus}]")

        except Exception as e:
            log.error(__name__, f"Fetch Current Status Error: {str(e)}")
            _showStatus(str(e))


def _sendNewStatus():
    """
    If a status is waiting in the status dict, send it.
    """
    global _newStatus
    newStatus = _newStatus
    _newStatus = None

    # If fetching is disabled, then so is sending
    if _lastStatusFetchInterval.isEnabled() and newStatus != None:

        _lastStatusFetchInterval.reset()
        
        log.info(__name__, f"Sending New Status: {str(newStatus)}")
        _showStatus(STATUS_SENDING)
        req = requests.post(SLACK_URL_SET_PROFILE, json=newStatus, headers=_headers)

        if req.status_code != 200:
            _showStatus(f"HTTP[{req.status_code}]")
            log.error(__name__, f"Slack Profile Send Failed with HTTP code [{req.status_code}]")
            return

        log.info(__name__, str(req.json()))


def _slackThread():
    """
    seems urequests is blocking, so lets try it in a dedicated thread.
    """
    log.debug(__name__, "Slack API Thread Start...")
    global _shutdownCountdown
    while _shutdownCountdown > 0:
        try:
            _sendNewStatus()
            _fetchCurrentStatus()
            _shutdownCountdown -= 1
            time.sleep(0.1)
        except Exception as e:
            _showStatus(const.ERR_GENERAL)
            log.warn(__name__, f"Slack Thread unexpected error: {str(e)}")

    log.debug(__name__, "Slack API Thread shutdown")



def setup(token: str):
    """
    Provide the slack token.  
    """
    global _token 
    global _headers

    _token = token
    _headers = { 
        'Authorization': f'Bearer {_token}'
    }



def enable(enable: bool):
    """
    Enable or Disable slack processing.
    This is done mainly by the wifi being online or offline.
    """
    if _lastStatusFetchInterval.enable(enable=enable, shouldReset=True):
        log.debug(__name__, f"Enable Slack API Module [{enable}]")



def setOnStatusUpdateCallback(func):
    """
    """
    global _statusCallback
    _statusCallback = func


def sendNewStatus(status: str, emoji: str, expiryInMinutes: int):
    """
    Set the status of your user.
    This is a non-blocking call that will rely on the internal slack thread to send 
    the status, and also be sure to do a fresh fetch too.
    """
    global _newStatus 

    expiry = 0
    if expiryInMinutes > 0:
        # Expiry is the epoch timestamp of when to expire.
        # The micropython epoch however is jan 1 2000 (946684800), instead of the unix jan 1 1970
        expiry = int(time.time() + 946684800 + (expiryInMinutes * 60))

    _newStatus = {
        'profile': {
            'status_text': status,
            'status_emoji': emoji,
            'status_expiration': expiry
        }
    }


async def loop():
    """
    Because we're using a thread, and not async, lets have 
    an async looper be tasked with always resetting the _shutdown
    flag.  If the reset fails, the thread will self-stop
    """
    
    global _shutdownCountdown

    # Fire up the thread
    slackThreadId = _thread.start_new_thread( _slackThread, ())

    # Keep the thread alive
    while True:
        _shutdownCountdown = 25
        await uasyncio.sleep_ms(100)

