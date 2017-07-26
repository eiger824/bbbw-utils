#!/bin/bash

if [[ $UID -ne 0 ]]
then
	exit 1
fi

dmesg | grep -q BB-ADC
if [ $? -ne 0 ]
then
	sh -c "echo 'BB-ADC' > /sys/devices/platform/bone_capemgr/slots"
fi

# Just make sure that module is loaded
lsmod | grep -q "ti_am335x_adc"
if [ $? -ne 0 ]
then
	modprobe ti_am335x_adc
fi

# Insmod the ledtoggle driver
if [[ -f /home/debian/bbbw-utils/kernel-mods/tl-led/tl-led.ko ]]
then
	lsmod | grep -q "tl_led"
	if [ $? -ne 0 ]
	then
		insmod /home/debian/bbbw-utils/kernel-mods/tl-led/tl-led.ko
	fi
fi

# And run the program
ps ax | grep -q test_tl-led_tmp36 | head -1
if [ $? -ne 0 ]
then
	/home/debian/bbbw-utils/ledtest/3-led/test_tl-led_tmp36 -s &
fi
