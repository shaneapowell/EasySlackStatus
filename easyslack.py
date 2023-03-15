"""
A root package level module to fire up the main program entrypoints.
easyslack.start()
easyslack.setup()
"""
import ess.run
import ess.log as log


def start(enableAmpyPing=False):
    """
    """
    log.info(__name__, "Start Easy Slack")
    ess.run.start(enableAmpyPing=enableAmpyPing)


def setup():
    """
    """
    log.info(__name__, "Setup Easy Slack: Not Yet Implemented")
