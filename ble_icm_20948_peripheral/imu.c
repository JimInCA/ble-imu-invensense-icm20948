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

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "imu.h"
#include "twi.h"
#include "hal.h"

char *INV_ICM20948_ACCEL_FSR_ASCII [] = {
	"INV_ICM20948_ACCEL_FSR_02G",
	"INV_ICM20948_ACCEL_FSR_04G",
	"INV_ICM20948_ACCEL_FSR_08G",
	"INV_ICM20948_ACCEL_FSR_16G"
};


char *INV_ICM20948_GYRO_FSR_ASCII [] = {
	"INV_ICM20948_GYRO_FSR_250DPS",
	"INV_ICM20948_GYRO_FSR_500DPS",
	"INV_ICM20948_GYRO_FSR_1000DPS",
	"INV_ICM20948_GYRO_FSR_2000DPS"
};


inv_icm20948_chip_config chip_config_20948 = {
	.accl_fsr = INV_ICM20948_ACCEL_FSR_04G,
	.gyro_fsr = INV_ICM20948_GYRO_FSR_2000DPS,
	.magn_fsr = INV_ICM20948_MAGN_FSR_4900UT,
	.accl_fifo_enable = true,
	.gyro_fifo_enable = true,
	.magn_fifo_enable = false,
	.temp_fifo_enable = true,
	.enable = false,
	.bytes_per_datum = 0,
	.sample_rate = INV_ICM20948_INIT_SAMPLE_RATE,
	.accel_dlpf = INV_ICM20948_ACCEL_FILTER_246HZ,	// normal default
	.gyro_dlpf = INV_ICM20948_GYRO_FILTER_197HZ     // normal default
};

inv_icm20948_state st = {
        .chip_type   = INV_ICM20948,
        .chip_config = &chip_config_20948
};

int16_t inv_icm20948_set_power(inv_icm20948_state *st, bool power_on)
{
    int result;
    if (st->chip_config->enable != power_on) {
        result = inv_icm20948_set_sleep_mode(power_on == true ? false : true);
        if (result)
            return result;
        st->chip_config->enable = power_on;
        //if (power_on)
        //    msleep(INV_ICM20948_POWER_UP_TIME);
    }
    return 0;
}

int16_t inv_check_and_setup_chip(inv_icm20948_state *st)
{
    int16_t result;

    twi_init();

    result = inv_icm20948_init(st);
    if (result)
        return -1;

    result = inv_icm20948_set_power(st, true);
    if (result)
        return -1;

    if (   st->chip_config->accl_fifo_enable == true
        || st->chip_config->gyro_fifo_enable == true
        || st->chip_config->magn_fifo_enable == true
        || st->chip_config->temp_fifo_enable == true )
    {
        st->chip_config->bytes_per_datum = 0;
        if (st->chip_config->accl_fifo_enable == true)
            st->chip_config->bytes_per_datum += 6;
        if (st->chip_config->gyro_fifo_enable == true)
            st->chip_config->bytes_per_datum += 6;
        if (st->chip_config->magn_fifo_enable == true)
            st->chip_config->bytes_per_datum += 6;
        if (st->chip_config->temp_fifo_enable == true)
            st->chip_config->bytes_per_datum += 2;
        inv_icm20948_reset_fifo(st);
    }
    else
    {
        inv_icm20948_write_register(IMU_INT_ENABLE_1, IMU_BIT_RAW_DATA_0_RDY_EN);
    }

    return 0;
}

int16_t inv_icm20948_init(inv_icm20948_state *st)
{
     uint8_t result, counter, temp, imu_device_id;

    // let's start by resetting the device
    counter = 0;
    inv_icm20948_write_register(IMU_PWR_MGMT_1, IMU_BIT_DEVICE_RESET);
    do {
        inv_icm20948_sleep_us(10); // 10uS delay
        temp = inv_icm20948_read_register(IMU_PWR_MGMT_1);
    } while ((temp & IMU_BIT_DEVICE_RESET) && (counter++ < 1000));

    // device requires 100mS delay after power-up/reset
    inv_icm20948_sleep_us(100000);    // 100mS delay

    imu_device_id = inv_icm20948_get_device_id();
    if (imu_device_id != IMU_EXPECTED_WHOAMI)
        return -1;

    // clear the sleep enable bit
    result = inv_icm20948_set_sleep_mode(false);
    if (result)
        return result;

    // setup low pass filters 
    result = inv_icm20948_set_gyro_dlpf(st->chip_config->gyro_dlpf);
    result = inv_icm20948_set_accel_dlpf(st->chip_config->accel_dlpf);

    // setup sample rate
    result = inv_icm20948_set_sample_frequency(st->chip_config->sample_rate);
    if (result)
        return result;

    // set the clock source
    temp = inv_icm20948_read_register(IMU_PWR_MGMT_1);
    temp &= 0xf8;
    temp |= 0x01;
    inv_icm20948_write_register(IMU_PWR_MGMT_1, temp);

    // set the gyro full scale range
    inv_icm20948_config_gyro(st->chip_config->gyro_fsr);

    // set the accelerometer full scale range
    inv_icm20948_config_accel(st->chip_config->accl_fsr);

    // set the sleep enable bit
    inv_icm20948_set_sleep_mode(true);

    return 0;
}

int16_t inv_icm20948_set_sleep_mode(bool sleep_mode)
{
    uint8_t temp, temp2;

    // get the sleep enable bit
    temp = inv_icm20948_read_register(IMU_PWR_MGMT_1);
    if (sleep_mode == false)
        temp &= ~(IMU_BIT_SLEEP);    // clear the sleep bit
    else
        temp |= IMU_BIT_SLEEP;       // set the sleep bit
    inv_icm20948_write_register(IMU_PWR_MGMT_1, temp);
    temp2 = inv_icm20948_read_register(IMU_PWR_MGMT_1);
    if (temp != temp2)
    {
        return -1;
    }

    return 0;
}

int16_t inv_icm20948_set_sample_frequency(uint16_t rate)
{
    uint8_t divider;
    divider = (uint8_t)(1100 / rate) - 1;    // from ICM-20948 data sheet
    inv_icm20948_write_register(IMU_GYRO_SMPLRT_DIV, divider);
    return 0;
}

int16_t inv_icm20948_set_gyro_dlpf(inv_icm20948_gyro_filter_e rate)
{
    uint8_t temp;
    temp = inv_icm20948_read_register(IMU_GYRO_CONFIG_1);  // get current reg value
    temp &= 0xc6;                                          // clear all dlpf bit
    if (rate != INV_ICM20948_GYRO_FILTER_12106HZ_NOLPF) {
        temp |= 0x01;                                      // set FCHOICE bit
        temp |= (rate & 0x07) << 3;                        // set DLPFCFG bits
    }
    inv_icm20948_write_register(IMU_GYRO_CONFIG_1, temp);           // set new value
    return 0;
}

int16_t inv_icm20948_set_accel_dlpf(inv_icm20948_accel_filter_e rate)
{
    uint8_t temp;
    temp = inv_icm20948_read_register(IMU_ACCEL_CONFIG);            // get current reg value
    temp &= 0xc6;                                          // clear all dlpf bits
    if (rate != INV_ICM20948_ACCEL_FILTER_1209HZ_NOLPF) {
        temp |= 0x01;                                      // set FCHOICE bit
        temp |= (rate & 0x07) << 3;                        // set DLPFCFG bits
    }
    inv_icm20948_write_register(IMU_ACCEL_CONFIG, temp);            // set new value
    return 0;
}

void inv_icm20948_config_gyro(uint8_t full_scale_select)
{
    uint8_t temp;
    // set the gyro full scale range
    temp = inv_icm20948_read_register(IMU_GYRO_CONFIG_1);   // get current value of register
    temp &= 0xf9;                                  // clear bit4 and bit3
    temp |= ((full_scale_select & 0x03) << 1);     // set desired bits to set range
    inv_icm20948_write_register(IMU_GYRO_CONFIG_1, temp);
    NRF_LOG_INFO("Gyroscope FSR: %s", INV_ICM20948_GYRO_FSR_ASCII[(full_scale_select & 0x03)]);
}

void inv_icm20948_config_accel(uint8_t full_scale_select)
{
    uint8_t temp;
    // set the accelerometer full scale range
    temp = inv_icm20948_read_register(IMU_ACCEL_CONFIG);    // get current value of register
    temp &= 0xf9;                                  // clear bit2 and bit1
    temp |= ((full_scale_select & 0x03) << 1);     // set desired bits to set range
    inv_icm20948_write_register(IMU_ACCEL_CONFIG, temp);
    NRF_LOG_INFO("Accelerometer FSR: %s", INV_ICM20948_ACCEL_FSR_ASCII[(full_scale_select & 0x03)]);
}

uint8_t inv_icm20948_get_device_id(void)
{
    return (inv_icm20948_read_register(IMU_WHO_AM_I));
}

void inv_icm20948_read_accel_xyz(int16_t *ax, int16_t *ay, int16_t *az)
{
  uint8_t  data_blk[6];

    inv_icm20948_read_register_block(IMU_ACCEL_XOUT_H, data_blk, 6);
    *ax = (data_blk[0] << 8) + data_blk[1];
    *ay = (data_blk[2] << 8) + data_blk[3];
    *az = (data_blk[4] << 8) + data_blk[5];
}

void inv_icm20948_read_gyro_xyz(int16_t *gx, int16_t *gy, int16_t *gz)
{
    uint8_t  data_blk[6];

    inv_icm20948_read_register_block(IMU_GYRO_XOUT_H, data_blk, 6);
    *gx = (data_blk[0] << 8) + data_blk[1];
    *gy = (data_blk[2] << 8) + data_blk[3];
    *gz = (data_blk[4] << 8) + data_blk[5];
}

void inv_icm20948_read_magn_xyz(int16_t *mx, int16_t *my, int16_t *mz)
{
    uint8_t  data_blk[6];

    inv_icm20948_read_register_block(IMU_GYRO_XOUT_H, data_blk, 6);
    *mx = (data_blk[0] << 8) + data_blk[1];
    *my = (data_blk[2] << 8) + data_blk[3];
    *mz = (data_blk[4] << 8) + data_blk[5];
}

void inv_icm20948_temperature(int16_t *temperature)
{
    uint8_t  data_blk[2];

    inv_icm20948_read_register_block(IMU_TEMP_OUT_H, data_blk, 2);
    *temperature = (data_blk[0] << 8) + data_blk[1];
}

void inv_icm20948_read_imu(IMU_DATA *imu_data)
{
    // This function reads the axis data directly from the registers.
    uint8_t data_blk[20];

    // burst read starts at register ACCEL_XOUT_H for 14 8 bit registers
    inv_icm20948_read_register_block(IMU_ACCEL_XOUT_H, data_blk, 20);
    imu_data->time_stamp = inv_icm20948_get_time_us();
    //imu_data->deviceid = (uint32_t)inv_icm20948_get_device_id();

    imu_data->ax = (data_blk[0] << 8) + data_blk[1];
    imu_data->ay = (data_blk[2] << 8) + data_blk[3];
    imu_data->az = (data_blk[4] << 8) + data_blk[5];

    imu_data->gx = (data_blk[6]  << 8) + data_blk[7];
    imu_data->gy = (data_blk[8]  << 8) + data_blk[9];
    imu_data->gz = (data_blk[10] << 8) + data_blk[11];

    imu_data->temperature = (data_blk[12] << 8) + data_blk[13];

    imu_data->mx = 1; //(data_blk[14] << 8) + data_blk[15];
    imu_data->my = 2; //(data_blk[16] << 8) + data_blk[17];
    imu_data->mz = 3; //(data_blk[18] << 8) + data_blk[19];

    //printk("ax %d ay %d az %d\n", imu_data->ax, imu_data->ay, imu_data->az);
    //printk("gx %d gy %d gz %d\n", imu_data->gx, imu_data->gy, imu_data->gz);
    //printk("mx %d my %d mz %d\n", imu_data->mx, imu_data->my, imu_data->mz);
}

int16_t inv_icm20948_get_fifo_counter(void)
{
    uint8_t data_blk[2];
    inv_icm20948_read_register_block(IMU_FIFO_COUNTH, data_blk, 2);
    return ((data_blk[0] << 8) | data_blk[1]);
}

void inv_icm20948_read_imu_fifo(inv_icm20948_state *st, IMU_DATA *imu_data)
{
    uint8_t data_blk[32];
    uint16_t i, fifo_count, bytes_per_datum;

    fifo_count = inv_icm20948_get_fifo_counter();

    bytes_per_datum = st->chip_config->bytes_per_datum;
    if (fifo_count >= bytes_per_datum) {
        inv_icm20948_read_register_block(IMU_FIFO_R_W, data_blk, bytes_per_datum);
        imu_data->time_stamp = inv_icm20948_get_time_us();
        fifo_count -= bytes_per_datum;
    }
    if (fifo_count) {    // I only want the first set of data
        // reset FIFO
        inv_icm20948_write_register(IMU_FIFO_RST, 0x1F);
        inv_icm20948_write_register(IMU_FIFO_RST, 0x00);
    }

    i = 0;
    if (st->chip_config->accl_fifo_enable) {
        imu_data->ax = (data_blk[i+0] << 8) + data_blk[i+1];
        imu_data->ay = (data_blk[i+2] << 8) + data_blk[i+3];
        imu_data->az = (data_blk[i+4] << 8) + data_blk[i+5];
        i += 6;
    }
    if (st->chip_config->gyro_fifo_enable) {
        imu_data->gx = (data_blk[i+0] << 8) + data_blk[i+1];
        imu_data->gy = (data_blk[i+2] << 8) + data_blk[i+3];
        imu_data->gz = (data_blk[i+4] << 8) + data_blk[i+5];
        i += 6;
    }
    if (st->chip_config->temp_fifo_enable) {
        imu_data->temperature = (data_blk[i+0] << 8) + data_blk[i+1];
        i += 2;
    }
    if (st->chip_config->magn_fifo_enable) {
        imu_data->mx = 1; //(data_blk[i+0] << 8) + data_blk[i+1];
        imu_data->my = 2; //(data_blk[i+2] << 8) + data_blk[i+3];
        imu_data->mz = 3; //(data_blk[i+4] << 8) + data_blk[i+5];
        i += 6;
    }

    //printk("ax %d ay %d az %d\n", imu_data->ax, imu_data->ay, imu_data->az);
    //printk("gx %d gy %d gz %d\n", imu_data->gx, imu_data->gy, imu_data->gz);
    //printk("mx %d my %d mz %d\n", imu_data->mx, imu_data->my, imu_data->mz);
}

int16_t inv_icm20948_reset_fifo(inv_icm20948_state *st)
{
    uint8_t temp;

    // disable interrupts
    inv_icm20948_write_register(IMU_INT_ENABLE, 0x00);
    inv_icm20948_write_register(IMU_INT_ENABLE_1, 0x00);
    inv_icm20948_write_register(IMU_INT_ENABLE_2, 0x00);
    inv_icm20948_write_register(IMU_INT_ENABLE_3, 0x00);


    // disable the sensor output to FIFO
    inv_icm20948_write_register(IMU_FIFO_EN_1, 0x00);
    inv_icm20948_write_register(IMU_FIFO_EN_2, 0x00);


    // disable fifo reading
    inv_icm20948_write_register(IMU_USER_CTRL, 0x00);


    // reset FIFO
    inv_icm20948_write_register(IMU_FIFO_RST, 0x1F);
    inv_icm20948_write_register(IMU_FIFO_RST, 0x00);

    // enable interrupt
    if (   st->chip_config->accl_fifo_enable
        || st->chip_config->gyro_fifo_enable
        || st->chip_config->magn_fifo_enable
        || st->chip_config->temp_fifo_enable) {
        inv_icm20948_write_register(IMU_INT_ENABLE_1, IMU_BIT_RAW_DATA_0_RDY_EN);
    }
    //inv_icm20948_write_register(IMU_INT_ENABLE_2, 0x01);    // enable for testing

    // enable FIFO reading and I2C master interface
    inv_icm20948_write_register(IMU_USER_CTRL, IMU_BIT_FIFO_EN);

    // enable sensor output to FIFO
    temp = 0;
    if (st->chip_config->gyro_fifo_enable)
        temp |= IMU_BIT_GYRO_FIFO_EN;
    if (st->chip_config->accl_fifo_enable)
        temp |= IMU_BIT_ACCEL_FIFO_EN;
    if (st->chip_config->temp_fifo_enable)
        temp |= IMU_BIT_TEMP_FIFO_EN;
    inv_icm20948_write_register(IMU_FIFO_EN_2, temp);

    // need to add support for temperature and magnetometer

    return 0;
}

/***********************************************************************/
/*                                                                     */
/* Support functions                                                   */
/*                                                                     */
/***********************************************************************/

static uint8_t current_bank = 0xff;

static void inv_icm20948_set_bank(uint16_t reg)
{
    uint8_t bank = (reg & 0xff00) >> 4;
    if (bank != current_bank) {
        inv_icm20948_i2c_write_reg(IMU_REG_BANK_SEL, bank);
        current_bank = bank;
    }
}

void inv_icm20948_write_register(uint16_t reg, uint8_t value)
{
    inv_icm20948_set_bank(reg);
    inv_icm20948_i2c_write_reg((uint8_t)(reg&0xff), value);
}

void inv_icm20948_write_register_block(uint16_t reg, uint8_t *block, uint8_t count)
{
    inv_icm20948_set_bank(reg);
    inv_icm20948_i2c_write_reg_block((uint8_t)(reg&0xff), block, count);
}

uint8_t inv_icm20948_read_register(uint16_t reg)
{
    uint8_t value;
    inv_icm20948_set_bank(reg);
    inv_icm20948_i2c_read_reg((uint8_t)(reg&0xff), &value);
    return value;
}

void inv_icm20948_read_register_block(uint16_t reg, uint8_t *block, uint8_t count)
{
    inv_icm20948_set_bank(reg);
    inv_icm20948_i2c_read_reg_block((uint8_t)(reg&0xff), block, count);
}
