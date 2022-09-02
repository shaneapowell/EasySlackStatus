# Troubleshooting

## MacOS Users

Finding the `/dev` mount for the USB device can be a little tricky, so here are the steps to find them. 

In a new terminal, run the command `system_profiler SPUSBDataType`. 
This will spit out all of the connected USB devices.
You should see something that looks like this. Pay close attention to the serial number.
  
![Screen Shot 2022-09-02 at 1 25 49 PM](https://user-images.githubusercontent.com/2481437/188215356-4bd5acdb-274d-4a63-a3b5-6d3a60ab47d8.png)

Run the command `ls /dev/ | grep tty | grep <serial number>` eg: `ls /dev/ | grep tty | grep 024A9AC9`

You should see a list of the devices:

![Screen Shot 2022-09-02 at 1 31 45 PM](https://user-images.githubusercontent.com/2481437/188216236-15b09861-8e32-465c-a354-d4b4c77c3ccc.png)

When using the commands that reference a `/dev/<device name>` resource, use the value like `/dev/tty.usbserial-024A9AC9` 
