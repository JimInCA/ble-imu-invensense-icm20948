/*
 * Copyright(c) 2021 - Jim Newman
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies
 * or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF
 * CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE
 * OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef IMU_H__
#define IMU_H__

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

#define IMU_ADDR                0X68

#define IMU_REG_BANK_SEL        0x7F    // common to all banks

// Most Significant Byte: bank, Least Significant Byte: register

#define IMU_WHO_AM_I            0x0000

#define IMU_USER_CTRL           0x0003
#define IMU_BIT_FIFO_EN                 0x40

#define IMU_LP_CONFIG           0x0005
#define IMU_I2C_MST_CYCLE               0x40
#define IMU_ACCEL_CYCLE                 0x20
#define IMU_GYRO_CYCLE                  0x10

#define IMU_PWR_MGMT_1          0x0006
#define IMU_BIT_DEVICE_RESET            0x80
#define IMU_BIT_SLEEP                   0x40

#define IMU_INT_ENABLE          0x0010
#define IMU_INT_ENABLE_1        0x0011
#define IMU_BIT_RAW_DATA_0_RDY_EN       0x01
#define IMU_INT_ENABLE_2        0x0012
#define IMU_INT_ENABLE_3        0x0013
#define IMU_INT_STATUS          0x0019
#define IMU_INT_STATUS_1        0x001A
#define IMU_BIT_RAW_DATA_0_RDY_INT      0X01
#define IMU_INT_STATUS_2        0x001B
#define IMU_INT_STATUS_3        0x001C

#define IMU_ACCEL_XOUT_H        0x002d
#define IMU_GYRO_XOUT_H         0x0033
#define IMU_TEMP_OUT_H          0x0039

#define IMU_FIFO_EN_1           0x0066
#define IMU_BIT_SLV_0_FIFO_EN           0x01
#define IMU_BIT_SLV_1_FIFO_EN           0x02
#define IMU_BIT_SLV_2_FIFO_EN           0x04
#define IMU_BIT_SLV_3_FIFO_EN           0x08
#define IMU_FIFO_EN_2           0x0067
#define IMU_BIT_ACCEL_FIFO_EN           0x10
#define IMU_BIT_GYRO_FIFO_EN            0x0E
#define IMU_BIT_TEMP_FIFO_EN            0x01
#define IMU_FIFO_RST            0x0068
#define IMU_FIFO_MODE           0x0069
#define IMU_FIFO_COUNTH         0x0070
#define IMU_FIFO_COUNTL         0x0071
#define IMU_FIFO_R_W            0x0072
#define IMU_DATA_RDY_STATUS     0x0074
#define IMU_BIT_RAW_DATA_RDY            0x0f
#define IMU_FIFO_CFG            0x0076
#define IMU_BIT_FIFO_CFG                0x01

#define IMU_GYRO_SMPLRT_DIV     0x0200
#define IMU_GYRO_CONFIG_1       0x0201
#define IMU_GYRO_CONFIG_2       0x0202
#define IMU_ACCEL_CONFIG        0x0214
#define IMU_ACCEL_CONFIG_2      0x0215

// WHOAMI value for ICM-20948
#define IMU_EXPECTED_WHOAMI     0xEA
#define IMU_ID                  IMU_EXPECTED_WHOAMI

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

extern IMU_DATA last_sample;

/*device enum */
typedef enum _INV_DEVICES {
        INV_ICM20948,
        INV_NUM_PARTS
} INV_DEVICES;

/**
 *  inv_icm20948_chip_config - chip configuration data.
 *    accl_fsr:          accelerometer full scale range
 *    gyro_fsr:          gyroscope full scale range
 *    magn_fsr:          magnetometer full scale range
 *    accl_fifo_enable:  enable accel data output
 *    gyro_fifo_enable:  enable gyro data output
 *    magn_fifo_enable:  enable magn data output
 *    temp_fifo_enable:  enable temperature data output
 *    enable:            master enable state
      bytes_per_datum:   number of bytes to read from fifo
 *    sample_rate:       sample update rate
 *    accel_dlpf:        accelerometer digital low pass filter
 *    gyro_dlpf:         gyroscope digital low pass filter
 */
typedef struct _inv_icm20948_chip_config {
        unsigned int accl_fsr:2;
        unsigned int gyro_fsr:2;
        unsigned int magn_fsr:2;
        unsigned int accl_fifo_enable:1;
        unsigned int gyro_fifo_enable:1;
        unsigned int magn_fifo_enable:1;
        unsigned int temp_fifo_enable:1;
        unsigned int enable:1;
        uint16_t bytes_per_datum;
        uint16_t sample_rate;
        uint8_t accel_dlpf;
        uint8_t gyro_dlpf;
} inv_icm20948_chip_config;

/*
 *  inv_icm20948_state - Driver state variables
 *    chip_config:       cached attribute information
 *    chip_type:         chip type
 */
typedef struct _inv_icm20948_state {
        inv_icm20948_chip_config *chip_config;
        INV_DEVICES  chip_type;
} inv_icm20948_state;

#define INV_ICM20948_INIT_SAMPLE_RATE         10

typedef enum _inv_icm20948_accl_fsr_e {
	INV_ICM20948_ACCEL_FSR_02G = 0,
	INV_ICM20948_ACCEL_FSR_04G,
	INV_ICM20948_ACCEL_FSR_08G,
	INV_ICM20948_ACCEL_FSR_16G,
	NUM_ICM20948_ACCEL_FSR
} inv_icm20948_accl_fsr_e;

typedef enum _inv_icm20948_gyro_fsr_e {
	INV_ICM20948_GYRO_FSR_250DPS = 0,
	INV_ICM20948_GYRO_FSR_500DPS,
	INV_ICM20948_GYRO_FSR_1000DPS,
	INV_ICM20948_GYRO_FSR_2000DPS,
	NUM_ICM20948_GYRO_FSR
} inv_icm20948_gyro_fsr_e;

typedef enum inv_icm20948_magn_fsr_e {
	INV_ICM20948_MAGN_FSR_4900UT = 0,
	NUM_ICM20948_MAGN_FSR
} inv_icm20948_magn_fsr_e;

typedef enum _inv_icm20948_gyro_filter_e {
        INV_ICM20948_GYRO_FILTER_197HZ = 0,
        INV_ICM20948_GYRO_FILTER_152HZ,
        INV_ICM20948_GYRO_FILTER_120HZ,
        INV_ICM20948_GYRO_FILTER_51HZ,
        INV_ICM20948_GYRO_FILTER_24HZ,
        INV_ICM20948_GYRO_FILTER_12HZ,
        INV_ICM20948_GYRO_FILTER_6HZ,
        INV_ICM20948_GYRO_FILTER_361HZ,
        INV_ICM20948_GYRO_FILTER_12106HZ_NOLPF,
        NUM_ICM20948_GYRO_FILTER
} inv_icm20948_gyro_filter_e;
 
typedef enum _inv_icm20948_accel_filter_e {
        INV_ICM20948_ACCEL_FILTER_246HZ = 0,
        INV_ICM20948_ACCEL_FILTER_246HZ_DUP,
        INV_ICM20948_ACCEL_FILTER_111HZ,
        INV_ICM20948_ACCEL_FILTER_50HZ,
        INV_ICM20948_ACCEL_FILTER_24HZ,
        INV_ICM20948_ACCEL_FILTER_12HZ,
        INV_ICM20948_ACCEL_FILTER_6HZ,
        INV_ICM20948_ACCEL_FILTER_473HZ,
        INV_ICM20948_ACCEL_FILTER_1209HZ_NOLPF,
        NUM_ICM20948_ACCEL_FILTER
} inv_icm20948_accel_filter_e;

int16_t inv_check_and_setup_chip(inv_icm20948_state *st);

int16_t inv_icm20948_init(inv_icm20948_state *st);
int16_t inv_icm20948_set_sleep_mode(bool sleep_mode);
int16_t inv_icm20948_set_sample_frequency(uint16_t rate);
uint8_t inv_icm20948_get_device_id(void);
int16_t inv_icm20948_reset_fifo(inv_icm20948_state *st);
void inv_icm20948_read_accel_xyz(int16_t *x, int16_t *y, int16_t *z);
void inv_icm20948_read_gyro_xyz(int16_t *gx, int16_t *gy, int16_t *gz);
void inv_icm20948_read_magn_xyz(int16_t *mx, int16_t *my, int16_t *mz);
void inv_icm20948_temperature(int16_t *temperature);
void inv_icm20948_read_imu(IMU_DATA *imu_data);
int16_t inv_icm20948_set_gyro_dlpf(uint8_t rate);
int16_t inv_icm20948_set_accel_dlpf(uint8_t rate);
void inv_icm20948_config_gyro(uint8_t full_scale_select);
void inv_icm20948_config_accel(uint8_t full_scale_select);

int16_t inv_icm20948_get_fifo_counter(void);
void inv_icm20948_read_imu_fifo(inv_icm20948_state *st, IMU_DATA *imu_data);

uint8_t inv_icm20948_read_register(uint16_t reg);
void inv_icm20948_read_register_block(uint16_t reg, uint8_t *block, uint8_t count);
void inv_icm20948_write_register(uint16_t reg, uint8_t value);
void inv_icm20948_write_register_block(uint16_t reg, uint8_t *block, uint8_t count);

#endif // IMU_H__
