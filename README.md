# FreeRTOS Ported to Raspberry Pi

This is a forked branch from James' [RaspberryPi-FreeRTOS](https://github.com/jameswalmsley/RaspberryPi-FreeRTOS)
James provides a very basic port of FreeRTOS to Raspberry pi.

## Howto Build

Type make! -- If you get an error then:

    $ cd .dbuild/pretty
    $ chmod +x *.py

Currently the makefile expect an arm-none-eabi- toolchain in the path. Either export the path to yours or
modify the TOOLCHAIN variable in dbuild.config.mk file.

You may also need to modify the library locations in the Makefile:

    kernel.elf: LDFLAGS += -L "/usr/lib/gcc/arm-none-eabi/9.2.1" -lgcc
    kernel.elf: LDFLAGS += -L "/usr/lib/arm-none-eabi/lib" -lc

The build system also expects find your python interpreter by using /usr/bin/env python,
if this doesn't work you will get problems.

To resolve this, modify the #! lines in the .dbuild/pretty/\*.py files; or add a soft link - python which links a python executable (e.g. python3.8)

Hope this helps.

I'm currently porting my BitThunder project to the Pi, which is a OS based on FreeRTOS
but with a comprehensive driver model, and file-systems etc.

http://github.com/jameswalmsley/bitthunder/

James

## Howto Prepare the Bootable SD Card

The key RTOS image is the kernel.img built from above "make". However, it can not run itself, a bootloader is requried to load it. We can use Raspberry Pi distro bootloader to do so. The following steps are needed to prepare a bootable mini SD card:
1. Collect the following files from a Raspberry Pi boot partition and store them to a temporary folder.
    * bootcode.bin
    * start.elf
2. Copy the kernel.img from the output of the "make" to the same temporary folder
3. Create a file with name as config.txt and have the following content. Note: since the default kernel name is "kernel.img", this file can be ignored if you only use "kernel.img" as the name.
    > kernel=kernel.img
4. Format a mini SD card in FAT32 (no need to use a big one, in my case, I just use 2G SD card) and copy all the 4 files in the temporary folder to it.
5. Insert this mini SD card to a Raspberry Pi before power up.

## Try on Raspberry Pi Zero W

I tried this on a Raspberry Pi Zero W and it works. As soon as the power is turn on, the LED connected to GPIO 16 blinks. I then did a minor modification on main.c that have two LEDs (adding another one on GPIO 12) blinks together. Also, I add a TCRT5000 Infrared Reflective Sensor on GPIO 21, as soon as it detect an object it will trigger the siren LEDs to blink for a while. This is to simulate a police car siren lights with red and blue color. Like below picture:

![Raspberry Pi Zero W with FreeRTOS](/demo_rpi0w.png)

At beginning, I thougth it could not work on Raspberry Pi Zero, since it's processor has ARM1176JZF-S core while Raspberry Pi 2(B) has ARM Cortex-A7. Looks like source code that James provided applies to both of them. Thanks James, I can enjoy the bare-metal world with Raspberry Pi so quickly!

