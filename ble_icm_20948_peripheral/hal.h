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
/* HAL                                                                 */
/*                                                                     */
/***********************************************************************/

#ifndef _HAL_H_
#define _HAL_H_


#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>


uint32_t inv_icm20948_get_time_us(void);
void inv_icm20948_sleep_us(uint32_t us);

int inv_icm20948_i2c_init(void);
int inv_icm20948_i2c_read_reg(uint8_t reg, uint8_t *value);
int inv_icm20948_i2c_read_reg_block(uint8_t reg, uint8_t * rbuffer, uint32_t rlen);
int inv_icm20948_i2c_write_reg(uint8_t reg, uint8_t value);
int inv_icm20948_i2c_write_reg_block(uint8_t reg, uint8_t * wbuffer, uint32_t wlen);

#endif // _HAL_H_
