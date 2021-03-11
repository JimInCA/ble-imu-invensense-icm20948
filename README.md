BLE IMU with Invensence ICM20948
====================

This program was created for the sole purpose as a learning experiment for Bluetooth Low Energy (BLE).  I wanted to do something that had a real world application and this seemed to be a good fit.  I have a lot of experience with both the Nordic nRF51 and multiple Invensense IMU modules including the ICM-20948 used in this example.  The ICM-20948 has a build-in Digital Motion Processor (DMP) but in this experiment, I'm not using the DMP and instead I'm just processing the raw data from the Accelerometer and Gyroscope.  Sometime in the future, I may do an project utilizing the DMP.  As for the nRF51, the other projects needed a much higher bandwidth than what is possible with BLE, so one project used Nordic's proprietary RF protocol Gazell and another used Enhanced Shock Burst (ESB).  So I thought that it was time to actually learn something about BLE.

![alt text](./images/hardware_setup.jpg?raw=true "BLM Example Hardware Setup")

Here is a photo of the hardware that I'm using for this experiment. What follows is a list of the various components shown in the photo:

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

There are two parts to this project, ble_icm_20948_central and ble_icm_20948_peripheral.  As the name implies, the firmware generated in ble_icm_20948_peripheral is intended to be flashed on the remote device.  As for the ble_icm_20948_central firmware, this is intended to be used on a device that will be used strictly for testing the ble_icm_20948 firmware.  A more complete discussion follows.

Hardware
========

ble_icm_20948_peripheral
------------------------

The hardware for the peripheral consists of two major parts; a development board with the nRF52 and a breakout board with the ICM-20948 DMP.  As shown in the photo above, this consists of the SparkFun nRF52832 Breakout board and the SparkFun ICM-20948 Breakout board.  But the ble_icm_20948_peripheral directory also has support for the Nordic nRF52 PCA10040 DK.  So either board will work.  You'll just need to consult file ./\<board\>/s132/config/app_config.h for the appropriate pin outs for the board that you will be using.  Header file app_config.h has the pin outs for the I2C and interrupt connections to the ICM-20948 breakout.  As for the SparkFun nRF52832 breakout, you'll need to consult ./sparkfun/s132/config/custom_board.h for the pins for the UART connection to the UART to USB bridge (Teensy 4.0 in photo above).  With some minor editing, you should be able to port this to other boards that use the the mcu and dmp.  

Just be aware that when using the SparkFun ICM-20948 Breakout, you'll need to use a SWD device in order to program the firmware.  The board comes with a SparkFun UART Bootloader, so you may be able to use that connection.  I prefer the Segger J-Link programmer for the in-line debugging capabilities that it offers, so I went directly to that option and skipped the UART bootloader.  The PCA10040 comes with a Segger SWD interface built in, so that is a simpler option from the get-go.  

ble_icm_20948_central
---------------------

The development board for the central can be either the Nordic nRF52 PCA10040 DK or the Nordic nRF52840 Dongle.  As for the PCA10040 DK, no modifications are required; just plug it in and load the firmware.  And as stated above, it comes with the Segger SWD interface build in, so debugging is a snap.  

You also have two options available for using the Nordic nRF52840 USB Dongle.  One option requires you to solder three wires to the edge connectors that will then go to a UART to USB bridge such as something from FTDI or the Teensy that supports 3.3V as I'm using.  This option sends all output to the UART pins.  The second option for the dongle actually uses the nRF52840's USB interface.  This option configures the dongle to enumerate as a USB to UART bridge directly.  I think that both of these configurations work with the Nordic bootloader on the dongle, but don't quote me on that.  I again soldered on a connector in order to connect up a Segger J-Link for debugging.  And also with the second option, the USB to UART bridge is used for testing the peripheral but you can still solder on the three uart wires for in this setup, the uart is used for debugging information.  So you have lots of options and lots of things to play around with when testing with the dongle. 

Software
========

The software is very simple.  I downloaded the latest Nordic SWD from their website.  For this project, all software should compile for SWD nRF5_SDK_17.0.2_d674dde, which is the version that I'm currently using.  From the SDK's root directory, I created a sub-directory that I like to call development and in the development directory, you can use git to clone this project.  

As for the IDE, I used Segger Embedded Studio which should be avalable on their website.   You can also build all of the firmware using gcc.  The appropriate configuration files and Makefiles for either Embedded Studio or gcc respectively are part of this project and all firmware should build without any issues with either (at least I hope that they will but you never know). 

Testing
=======

Load the firmware onto the devices that you have chosen as central and peripheral.  Hopefully, all of the appropriate wiring has been completed on both devices.  When you power up the peripheral, an LED should begin to flash indicating that the peripheral is advertising.  Assuming that you have a UART to USB device connected to the peripheral, you can now bring up a terminal window and connect it to the peripheral uart output.  I like to use Tera Term.  Now power up the central.  Depending on the device that you are using, an LED should flash briefly and then stay lit.  At the same time, the LED on the peripheral should stop flashing by staying lit as well.  This indicates that the a connection has been completed between the two devices.  You should also see the following in the terminal window that's connected to the peripheral:

```
<info> app: Connected.
<info> app: data cccd write
<info> app: notification disabled
```
If you see this, than all is well, else you'll have some debugging to do... 

At this point, you should be able to bring up a terminal window and connect it to the central's uart output.  In the central's terminal window, type 'r' followed by return/enter.  By doing so, you should then see the data that's being transferred from the peripheral to the central.  It should look something like this:

```
0xe6 0xe3 0x9e 0xe9 0xae 0xaf 0xce 0x00 0xee 0xff 0x00 0x00 0x66 0x20 0x00 0x00 0x00 0x00 0x00 0x00 0xd7 0x93 0x7b 0xea 0x06 0x00 0x90 0x08
0xe6 0xe3 0x9e 0xe9 0xe7 0xaf 0xce 0x00 0xfc 0xff 0xf2 0xff 0x60 0x20 0xf1 0xff 0xfa 0xff 0x0a 0x00 0xd7 0x93 0x7b 0xea 0x06 0x00 0x80 0x08
0xe6 0xe3 0x9e 0xe9 0x5a 0xbc 0xce 0x00 0x22 0x00 0x16 0x00 0x22 0x20 0x07 0x00 0x0d 0x00 0xfb 0xff 0xd7 0x93 0x7b 0xea 0x06 0x00 0x80 0x08
0xe6 0xe3 0x9e 0xe9 0xcc 0xc8 0xce 0x00 0x12 0x00 0xfc 0xff 0x5a 0x20 0x06 0x00 0x0c 0x00 0xff 0xff 0xd7 0x93 0x7b 0xea 0x06 0x00 0x90 0x08
0xe6 0xe3 0x9e 0xe9 0x3e 0xd5 0xce 0x00 0x20 0x00 0xfe 0xff 0x84 0x20 0x07 0x00 0x08 0x00 0x01 0x00 0xd7 0x93 0x7b 0xea 0x06 0x00 0xa0 0x08
0xe6 0xe3 0x9e 0xe9 0xb0 0xe1 0xce 0x00 0x26 0x00 0x30 0x00 0x3c 0x20 0x08 0x00 0x0b 0x00 0xff 0xff 0xd7 0x93 0x7b 0xea 0x06 0x00 0x80 0x08
```

This is twenty eight bytes of data representing the following IMU structure:

```
typedef struct _IMU_DATA {
        uint32_t deviceid;
        uint32_t time_stamp;
        int16_t ax;
        int16_t ay;
        int16_t az;
        int16_t gx;
        int16_t gy;
        int16_t gz;
        int16_t mx;
        int16_t my;
        int16_t mz;
        int16_t temperature;
} IMU_DATA;
```

The first four bytes of data represent the device ID and are in little endian format.  So the device's ID is actually 0xe99ee3e6.   The other fields in the structure follow.  

To stop the data collection, just type in 's' and hit enter/return.  What's happening is that with the 'r' the central is setting the notify flag in the peripheral which tells it to send data whenever new data is available and the 's' clears the notify flag to instruct the peripheral to stop sending data.

This same signalling is used to set and retrieve features in the peripheral and the imu from the central.  Here is the full list of commands:

| Character | Description |
| --------- | ----------- |
| 'r' or 'R'   | Run - start IMU sending data |
| 's' or 'S'   | Stop - stop imu from sending data |
| 'id' or 'ID' | Get device ID |
| 'a0' or 'A0' | Set Accel FSR to 2G |
| 'a1' or 'A1' | Set Accel FSR to 4G |
| 'a2' or 'A2' | Set Accel FSR to 8G |
| 'a3' or 'A3' | Set Accel FSR to 16G |
| 'g0' or 'G0' | Set Gyro FSR to 250DPS |
| 'g1' or 'G1' | Set Gyro FSR to 500DPS |
| 'g2' or 'G2' | Set Gyro FSR to 1000DPS |
| 'g3' or 'G3' | Set Gyro FSR to 2000DPS |
| 'd' or 'D'   | Get last IMU data sample |

For this testing, the central is converting the twenty eight bytes that it is receiving from the peripheral to ascii and then outputting the ascii string to the uart.  It was done this way to simplify testing.  But the central could had just as easily output the data as bytes, which would be the more appropriate solution if the data was being used by an application.

Conclusion
==========

This was an interesting project.  It did help to clarify that differences within Bluetooth Low Energy such as GAP, GATT, Services, Characteristics, and descriptor.  It also gave me a better understanding of the Nordic BLE Driver.  It's not that simple to understand but by developing this example program, thinks have begun to become much clearer.  I just hope that I can retain what I've learned ;-)

Update 3/11/2021
================

I added support for the Sparkfun MicroMod ATP Carrier Board configured with the MicroMod nRF5840 Processor Board.  Support was added so that the MicroMod can be used as either the central or peripheral.  
