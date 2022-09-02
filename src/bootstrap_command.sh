device=${args[device]}
force=${args[--force]}

if ! [ -z "$device" ]; then
  echo "Pushing ess folder to device '${device}' (Might take a while)"
  ampy -p $device put ess
  echo "Pushing lib folder to device '${device}'"
  ampy -p $device put lib
  echo "Pushing easyslack.py to device '${device}'"
  ampy -p $device put easyslack.py
  echo "Pushing config.json to device '${device}'"
  ampy -p $device put config.json

  echo "Testing configuration..."
  if ampy -p $device run test.py; then
    echo "Test successful. Installing auto-start script"
    ampy -p $device put main.py
  else
    echo "Test failed."
    if ! [ -z "$force" ]; then
      echo "--force is used. Pushing auto-start anyway..."
      ampy -p $device put main.py
      echo "Done."
    fi
  fi
else
  echo "Error: missing device"
fi
