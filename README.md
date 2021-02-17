BLE IMU with Invensence ICM20948
====================

This program was created for the sole purpose as a learning experiment for Bluetool Low Energy (BLE).  I wanted to do something that had a real world application and this seemed to be a good fit.  I have a lot of experience with both the Nordic nRF51 and multiple Invensense IMU modules including the ICM-20948 used in this example.  The ICM-20948 as a build-in Digital Motion Processor (DMP) but in this experiment, I'm not using the DMP and instead I'm just processing the raw data from the Accelerometer and Gyroscope.  Sometime in the future, I may do an example utilizing the DMP.  As for the nRF51, the other projects needed a much higher bandwidth than what is possible with BLE, so one project used Nordic's proprietary RF protocol Gazell and another used Enhanced Shock Burst (ESB).  So I thought that it was time to actually learn something about BLE.

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

To be continued...
