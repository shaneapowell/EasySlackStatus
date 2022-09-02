latest_esp32="https://micropython.org/resources/firmware/esp32-20220618-v1.19.1.bin"
device=${args[device]}
force=${args[--force]}

if ! [ -z "$force" ]; then
  echo "downloading $latest_esp32"
  curl -o esp32.bin $latest_esp32
  echo "Flashing on micropython to $device"
  esptool.py --chip esp32 --port "$device" --baud 460800 write_flash -z 0x1000 esp32.bin
else
  echo "Flashing $device with --force"
  echo "Running erase_flash on $device"
  esptool.py --chip esp32 --port "$device" erase_flash
  echo "downloading $latest_esp32"
  curl -o esp32.bin $latest_esp32
  echo "Flashing on micropython to $device"
  esptool.py --chip esp32 --port "$device" --baud 460800 write_flash -z 0x1000 esp32.bin
fi
