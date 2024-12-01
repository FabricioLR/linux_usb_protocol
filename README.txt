su

echo "2-1.6:1.0" > /sys/bus/usb/drivers/usbhid/unbind
echo "2-1.6:1.0" > /sys/bus/usb/drivers/mouse_test_driver/bind

rmmod mouse
insmod mouse.ko