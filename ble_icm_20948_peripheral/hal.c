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

/***********************************************************************/
/*                                                                     */
/* HAL -- These are the callback functions required by the InvenSense  */
/*        IMC-20948 driver.                                            */
/*                                                                     */
/***********************************************************************/

#include "nrf_delay.h"
#include "nrf_drv_rtc.h"

#include "imu.h"
#include "hal.h"
#include "twi.h"


static bool verbose = false;	// For debugging I2C issues.

const nrf_drv_rtc_t rtc = NRF_DRV_RTC_INSTANCE(0); /**< Declaring an instance of nrf_drv_rtc for RTC0. */

uint32_t inv_icm20948_get_time_us(void)
{
    return nrf_drv_rtc_counter_get(&rtc);
}

void inv_icm20948_sleep_us(uint32_t us)
{
    nrf_delay_us(us);
}

int inv_icm20948_i2c_init(void)
{
    return 0;
}

int inv_icm20948_i2c_read_reg(uint8_t reg, uint8_t *value)
{
    if (verbose)
        printf("Executing %s(0x%02x)\r\n", __func__, reg);

    *value = twi_read_register(IMU_ADDR, reg);

    if (verbose)
    {
        printf("0x%02x\r\n", *value);
    }

    return 0;
}

int inv_icm20948_i2c_read_reg_block(uint8_t reg, uint8_t * rbuffer, uint32_t rlen)
{
    int i;

    if (verbose)
        printf("Executing %s(0x%02x, %ld)\r\n", __func__, reg, rlen);

    twi_read_register_block(IMU_ADDR, reg, rbuffer, rlen);

    if (verbose)
    {
        for (i = 0; i < rlen; i++)
            printf(" val[%d]:0x%02x%s", i, rbuffer[i], (i%4==3?"\r\n":", "));
        printf("\r\n");
    }

    return 0;
}

int inv_icm20948_i2c_write_reg(uint8_t reg, uint8_t value)
{
    if (verbose)
    {
        printf("Executing %s(0x%02x, 0x%02x)\r\n", __func__, reg, value);
    }

    twi_write_register(IMU_ADDR, reg, value);

    return 0;
}

int inv_icm20948_i2c_write_reg_block(uint8_t reg, uint8_t *wbuffer, uint32_t wlen)
{
    int i;

    if (verbose)
    {
        printf("Executing %s(0x%02x, 0x%02x, %ld)", __func__, reg, wbuffer[0], wlen);
        if (wlen > 1)
        {
            printf("\r\n");
            for (i = 0; i < wlen; i++)
                printf(" val[%d]:0x%02x%s", i, wbuffer[i], (i%4==3?"\r\n":((i==(wlen-1)?"\r\n":","))));
        }
    }

    twi_write_register_block(IMU_ADDR, reg, wbuffer, wlen);

    return 0;
}
