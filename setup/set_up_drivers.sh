#!/bin/bash

if [[ $UID -ne 0 ]]
then
	echo "Run script as sudo"
	exit 1
fi

dmesg | grep -q BB-ADC
if [ $? -eq 0 ]
then
	echo "IIO cape enabled"
else
	echo "Enabling IIO cape"
	sh -c "echo 'BB-ADC' > /sys/devices/platform/bone_capemgr/slots"
fi

# Just make sure that module is loaded
lsmod | grep -q "ti_am335x_adc"
if [ $? -ne 0 ]
then
	echo "Modprobing ti_am335x_adc"
	modprobe ti_am335x_adc
fi

# Insmod the ledtoggle driver
if [[ -f /home/debian/bbbw-utils/kernel-mods/gled01/gled01.ko ]]
then
	lsmod | grep -q "gled01"
	if [ $? -ne 0 ]
	then
		echo "Insmoding gled01.ko"
		insmod /home/debian/bbbw-utils/kernel-mods/gled01/gled01.ko
	else
		echo "gled01 already insmoded"
	fi
else
	echo "Compile gled01 module and run this script again"
fi

STATUS=$?
echo "done"
exit $STATUS
