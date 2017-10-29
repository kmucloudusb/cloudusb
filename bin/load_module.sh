#/bin/bash

sudo modprobe usb_f_mass_storage
sudo modprobe g_mass_storage file=/piusb.bin stall=0
