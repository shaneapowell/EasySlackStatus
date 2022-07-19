# ESP8266 Slack Status Updater
Inspired by [Slack Status Updater With ESP8266](https://www.instructables.com/Slack-Status-Updater-With-ESP8266/) by Becky Stern

![EasySlackStatus](doc/easyslackstatus.jpg)
![EasySlackStatus](doc/easyslackstatusB.jpg)

# Required Hardware 
- Wemos D1 Mini ESP32 [Amzon](https://www.amazon.com/MELIFE-Bluetooth-Development-Functional-Compatible/dp/B08FWXFSVN/)
- 128x64 SH1106 I2C OLED Display [Amazon](https://www.amazon.com/gp/product/B08V97FYD2/)
- Rotary encoder with push button [Amazon](https://www.amazon.com/gp/product/B07T3672VK/ref=ppx_yo_dt_b_search_asin_image?ie=UTF8&psc=1)
    - If you can find a rotary encoder without the header pins, it will be easier to build.
- 5-pin header wire [Amazon](https://www.amazon.com/gp/product/B0789F2Y1T/ref=ppx_yo_dt_b_asin_title_o09_s00?ie=UTF8&psc=1)
    - I used these 8-pin and cut off 3 pins.
- 3D Printed Housing [ThingiVerse](https://www.thingiverse.com/thing:4879479)
- 2x M2x6mm HexHead screws

# Features
- 100% OpenSource MIT License.
- Set your current slack status with optional emjoi.
- Set an Expiring (in x minutes) status.
- Shows your current slack status (updates every 60 seconds)
- Configure your own status text and icons.
- Room for up to 25 unique status.
- ScreenSaver triggers after 60 minutes of no input.  Simply turn the rotary dial to wake back up.  
- Simple json Based Configuration.

# Future Plans
- Command line file configuration.
- Automatic Softwrae Updates
- A wifi configurator. 

# Obtain a **unique to you** Slack-Token
In order for the status to be correctly sent to your account, you must first obtain a unique `Slack-Token`. This token binds your EasySlackStatus device to your specific user and workspace.

Original Instructions I used are here https://github.com/witnessmenow/arduino-slack-api


Steps:
- Create a [new Slack App Here](https://api.slack.com/apps)
  - `Create New App`
  - Name it something appropriate for your workspace. eg `EasySlackStatus`
  - On the next screen, on the left side menu, select `OAuth & Permissions`
  - Scroll down to `Scopes`. Under `User Token Scopes`, click the `Add an OAuth Scope` button.
  - Add the scopes you'll need for this app
     - users.profile:write
     - users.profile:read
     - users:write
  - Scroll back to the top and click `Install app to Workspace`
  - Click `Allow`
  - You will now have an `OAuth Access Token` that you must copy to the `config.json` file. The toke will be something like `xoxp-17986256612-407223851215-376290712391-48df5e2e6083745488da8823455afe67` 

Ask your Slack Admin for help getting a slack token if your slack has restrictions.


# Flashing the firmware and EasySlackStatus software
`EasySlackStatus` runs on an ESP32 running Micropython.  The following steps will get you up and running.
1. Install the tools [esptool](https://docs.espressif.com/projects/esptool/en/latest/esp32/) and [ampy](https://github.com/scientifichackers/ampy) to your computer.
   1. `pip3 install esptool`
   2. `pip3 install adafruit-ampy`
2. Download the most recent [micropython firmware](https://micropython.org/download/esp32/) for esp32 boards. eg `esp32-20220618-v1.19.1.bin`
3. Flash the python firmware.
   1. `esptool.py  --chip esp32 --port /dev/ttyUSB0 erase_flash`
   2. `esptool.py  --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 esp32-20220618-v1.19.1.bin `
4. Clone this repo
   1. `git clone https://github.com/shaneapowell/EasySlackStatus.git`
5. Modify the config.json to fit your needs.
   1. Set the `Wifi SSID` for your wifi network. These are usually case sensitive.
   2. Set the `Wifi Password`. Also case sensitive.
   3. Set your `slack token` .
   4. Set your `TimeZone`. See the `lib/utzlist.py` file for a list of available timezones.  Add yours if it's not there.
   5. Configure/Modify/Update the slack status list to fit your needs. The default list should get you started.
6. Install the EasySlack python files
   1. `ampy -p /dev/ttyUSB0 put ess`
   2. `ampy -p /dev/ttyUSB0 put lib`
   3. `ampy -p /dev/ttyUSB0 put easyslack.py`
   4. `ampy -p /dev/ttyUSB0 put config.json`
7. Test your configuration at the terminal before adding the auto-start script. This will allow you to watch the logs.
   1. `ampy -p /dev/ttyUSB0 run test.py`
8. Install the auto-start script that will start EasySlackStatus on resets.
   1. `ampy -p /dev/ttyUSB0 put main.py`
9. To Disable the auto-start script if you need to do more testing.
   1.  `ampy -p /dev/ttyUSB0 rm main.py`

# Items on the Screen
- Bottom Left shows the wifi status and strength.  An hourglass indicates the wifi is connecting.  A circle with a line indicates not-connected.
- Bottom Right shows the current day and time in your timezone.
- Top Left shows your name from Slack. This value will be only your first name, and a max of 7 characters only when a status is set.  When your status is clear, your entire first name (within the max of 16 characters) is displayed.
- Top Right shows the current slack status text when there is one.

# Send a Slack Status
1. Rotate the dial until the status you want is highlighted.
2. Single Press the dial.
    * The top should show a `Sending...`
    * Followed by a `Retrieving...`.
    * ERROR simply means.. well. .something went wrong.  Try again.


# Send an Expiring status
1. Rotate the dial until the desired status is highlighted.
2. Double-Press (quickly) the rotary dial to enter `Expire In` selection screen.
3. Rotate the dial to set the minutes to expire.  The default minutes is pulled from the `Default Expire In Minutes` field on the webconfig.
4. Press the rotary dial once to send the status.
5. The expiry minutes will be now set as the new default for that status,  until the next reboot.  The expiry value is not set into your `config.json` file.

# View WiFi Info
1. Long Press the rotary dial to switch back-and-forth between the main status selection screen, and the WiFi Info Screen.

# Setup your `config.json` file with new status values.
Each entry in the `status_list` within yoru config.json file is made up of:
- `status`: The text to send. Can't be empty. Except if you want to use a `clear` status.
- `emoji`: The slack emoji code you want.  Can be empty.
- `expiry`: The number of minutes to expire this status.
- `display`: An optional display text value. This i what you see on the EasySlackStatus screen, but the `status` text above is what is actually sent.  This is handy if you want to show a short status line on the EasySlackStatus, but send a longer one to Slack. Or, if you want a `clear` status.
  
```
{
    "display": "Available",
    "emoji": ":here:",
    "expiry": 0
}
```

# FAQ
* *How can I view the logs while it's running*
  * You can use a serial terminal to connect to the `/dev/ttyUSB0` port of your device.  You'll gain access to the `repl` interface, which should also now be outputting the log messages to your screen.
  
* *How do I find the current IP address and WiFi settings?*
    * Long Press the rotary dial.
    * Should show the current WiFi status, ip, config URL and Time.
 

# Build your own

## Wemos D1 Mini ESP32
Solder a pair of 5-pin headers to the Wemos D1. See pic below. Solder on the inside rows on both side.  Between `3.3v-IO18` and `GND-IO22`. 

![WemosPins](doc/wemosPins.jpg)
![WemosPins](doc/wemosPinsB.jpg)

The colors below indicate the wire color used on the 5 pin header wires.

### 5-pin (Right)
* Wemos-GND -> **Black** - OLED & Rotary GND 
* Wemos-D4 -> Not Used
* Wemos-D3 -> Not Used
* Wemos-D2 -> **Blue** - OLED-SDA
* Wemos-D1 -> **Green** - OLED-SCL
### 5-pin (Left)
* Wemos-3.3v -> **Red** - OLED & Rotary VCC
* Wemos-D8 -> Not Used
* Wemos-D7 -> **Yellow** - Roatry-CLK
* Wemos-D6 -> **Blue** - Rotary-DT
* Wemos-D5 ->  **Green** -Rotary-SW

## 5-pin Connectors
![5PinCables](doc/5pinCables.jpg)

* White,Brown & Gray discarded if using 8pin connectors.
* Red wire removed from right connector spliced to left red connector for 2 red wire outputs.
* Black wire removed from left connector, spliced to right connector for 2 ground wires.
* The Left Connector holds the 3.3v and the rotary encoder wires
* The Right connector holds the ground and the I2C Display wires.


## Rotary Encoder
Reference the above pins list to know which goes where on your particular encoder.
![RotaryWired](doc/rotaryWired.jpg)

## OLED
![OledWired](doc/oledWired.jpg)

## Connected 
Note: The MCU you see is the 1st generation esp8266 based.  Couldn't handle SSL in Micropython, so the project switched to the vastly better ESP32.
![Connected](doc/allPartsConnected.jpg)

## Installed
There is a small groove in the top of the case for the rotary encoders index pin. 
A small dab of HotGlue on each of the 4 posts holds the OLED in place.  You can still see some of the got-glue string.

![Connected](doc/installedA.jpg)

Slide the D1-Mini into the rail slots. The back-plate holds it in place.
![Connected](doc/installed.jpg)

# License
[MIT license](license.txt)
 


# How the project was assembled & configured. 
Plans are underway to perhaps make this into an automatically built micropython image.  Or, some sort of upip package.  In the mean time, this is how the `/lib` folder was populated.
 - cd lib
 - wget https://raw.githubusercontent.com/robert-hh/SH1106/master/sh1106.py
   - Line 200 in sh1106.py. change from fbuf.height to self.height
 - wget https://raw.githubusercontent.com/miketeachman/micropython-rotary/master/rotary_irq_esp.py
   - Modify import at top of rotary.py  (from lib.rotary import Rotary)
 - wget https://raw.githubusercontent.com/miketeachman/micropython-rotary/master/rotary.py
 - wget https://raw.githubusercontent.com/shaneapowell/utimezone/master/utimezone.py
 - wget https://raw.githubusercontent.com/shaneapowell/utimezone/master/utzlist.py


## mpfshell scripted (TBD)
 - pip3 install mpfshell=0.9.1
 - mpfshell -s make.mpf


## Notes
https://randomnerdtutorials.com/micropython-wi-fi-manager-esp32-esp8266/


## Build Micro Python (esp8266)
- git clone https://github.com/micropython/micropython-lib.git
- git clone --recursive https://github.com/micropython/micropython.git
- cd micropython
- git submodule update --init
- make -C mpy-cross
- docker pull larsks/esp-open-sdk
- cd ports/esp8266
- cd modules
  - wget https://raw.githubusercontent.com/robert-hh/SH1106/master/sh1106.py
   - Line 200 in sh1106.py. change from fbuf.height to self.height
  - wget https://raw.githubusercontent.com/miketeachman/micropython-rotary/master/rotary_irq_esp.py
  - wget https://raw.githubusercontent.com/miketeachman/micropython-rotary/master/rotary.py
 - wget https://raw.githubusercontent.com/shaneapowell/utimezone/master/utimezone.py
 - wget https://raw.githubusercontent.com/shaneapowell/utimezone/master/utzlist.py
- docker run --rm -v $HOME:$HOME -u $UID -w $PWD larsks/esp-open-sdk make
- esptool.py --port /dev/ttyUSB0 erase_flash
- esptool.py --port /dev/ttyUSB0 --baud 1000000  write_flash --verify --flash_size=detect -fm dio 0 build-GENERIC/firmware-combined.bin
- make PORT=/dev/ttyUSB0 FLASH_MODE=dio deploy