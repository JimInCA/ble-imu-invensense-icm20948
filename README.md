BLE IMU with Invensence ICM20948
====================

This program was created for the sole purpose as a learning experiment for Bluetooth Low Energy (BLE).  I wanted to do something that had a real world application and this seemed to be a good fit.  I have a lot of experience with both the Nordic nRF51 and multiple Invensense IMU modules including the ICM-20948 used in this example.  The ICM-20948 as a build-in Digital Motion Processor (DMP) but in this experiment, I'm not using the DMP and instead I'm just processing the raw data from the Accelerometer and Gyroscope.  Sometime in the future, I may do an example utilizing the DMP.  As for the nRF51, the other projects needed a much higher bandwidth than what is possible with BLE, so one project used Nordic's proprietary RF protocol Gazell and another used Enhanced Shock Burst (ESB).  So I thought that it was time to actually learn something about BLE.

![alt text](./images/hardware_setup.jpg?raw=true "BLM Example Hardware Setup")

Here is a photo of the hardware that I'm using for this experiment.  I'll add more detail later as the project commences.  Here is a list of the various components shown in the photo:

<ul>
<li>SparkFun nRF52832 Breakout (used as ble_peripheral)</li>
<li>SparkFun 9DoF IMU Breakout - ICM-20948</li>
<li>Segger J-Link EDU Mini Programmer</li>
<li>Teensy 4.0 (set-up as a UART to USB bridge)</li>
<li>Nordic nRF52 PCA10040 DK (used as ble_central)</li>
<li>Nordic Semiconductor nRF52840 USB Dongle (used with Nordic nRF Connect)</li>
</ul>

Overview
========

There are two parts to this project, ble_icm_20948_central and ble_icm_20948_peripheral.  As the name implies, the firmware generated in ble_icm_20948_peripheral is intended to be flashed on the remote device.  As for the ble_icm_20948 firmware, this is intended to be used on a device that will be used strictly for testing the ble_icm_20948 firmware.  A more complete discussion follows.

Hardware
========

ble_icm_20948_peripheral
------------------------

The hardware for the peripheral consists of two major parts; a development board with the nRF52 and a breakout board with the ICM-20948 DMP.  As shown in the photo above, this consists of the SparkFun nRF52832 Breakout board and the SparkFun ICM-20948 Breakout board.  But the ble_icm_20948 directory also has support for the Nordic nRF52 PCA10040 DK.  So either board will work.  You'll just need to consult file <board>/s132/config/app_config.h for the appropriate pin outs for the board that you will be using.  app_config.h has the pin outs for the I2C and interrupt connections to the ICM-20948 breakout.  As for the SparkFun nRF52832 breakout, you'll need to consult ./sparkfun/s132/config/custom_board.h for the pins for the UART connection to the UART to USB bridge (Teensy 4.0 in photo above).  With some minor editing, you should be able to port this to other boards that use the the mcu and dmp.  

Just be aware that when using the SparkFun ICM-20948 Breakout, you'll need to use a SWD device in order to program the firmware.  The board comes with a SparkFun UART Bootloader, so you may be able to use that connection.  I prefer the Segger J-Link programmer for the in-line debugging capabilities that it offers, so I went directly to that option and skipped the UART bootloader.  The PCA10040 comes with a Segger SWD interface built in, so that is a simpler option from the get-go.  

ble_icm_20948_central
---------------------

The development board for the central can be either the Nordic nRF52 PCA10040 DK or the Nordic nRF52840 Dongle.  As for the PCA10040 DK, no modifications are required; just plug it in and load the firmware.  And as stated above, it comes with the Segger SWD interface build in, so debugging is a snap.  

You also have two options available for using the Nordic nRF52840 USB Dongle.  One option requires you to solder three wires to the edge connectors that will then go to a UART to USB bridge such as something from FTDI or the Teensy that supports 3.3V as I'm using.  This option sends all output to the UART pins.  The second option for the dongle actually uses the nRF52840 USB interface.  This option configures the dongle to enumerate as a USB to UART bridge directly.  I think that both of these configurations work with the Nordic bootloader on the dongle, but don't quote me on that.  I again soldered on a connector in order to connect up a Segger J-Link for debugging.  And also with the second option, the USB to UART bridge is used for testing the peripheral but you can still solder on the three uart wires for in this setup, the uart is used for debugging information.  So you have lots of options and lots of things to play around with when testing with the dongle. 

Software
========

The software is very simple.  I downloaded the latest Nordic SWD from their website.  For this project, all software should compile for SWD nRF5_SDK_17.0.2_d674dde.  From the SDK's root directory, I created a sub-directory that I like to call development and in the development directory, you can use git to clone this project.  

As for the IDE, I used Segger Embedded Studio which should be avalable on their website.   You can also build all of the firmware using gcc.  The appropriate configuration files and Makefiles for either Embedded Studio or gcc are part of this project and all firmware should build without any issues with either (at least I hope that they will but you never know). 

Testing
=======

TBD

To be continued...
