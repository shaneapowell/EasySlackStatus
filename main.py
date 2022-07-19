

try:
    import easyslack
    easyslack.start()
except Exception as e:
    print(e)

# log.info("Program Stopped. Resetting in 10 seconds")
# import time
# time.sleep(10)

# import machine
# machine.reset()