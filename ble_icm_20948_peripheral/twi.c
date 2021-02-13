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

#include <stdio.h>
#include <string.h>

#include "twi.h"

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(0);

void twi_init (void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t config =
    {
       .scl                = SCL_PIN,
       .sda                = SDA_PIN,
       .frequency          = NRF_TWI_FREQ_250K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH
    };

    err_code = nrf_drv_twi_init(&m_twi, &config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void twi_enable(bool isEnabled)
{
	if (isEnabled)
		nrf_drv_twi_enable(&m_twi);
	else
		nrf_drv_twi_disable(&m_twi);
}

void twi_write_register(uint8_t addr, uint8_t reg, uint8_t value)
{
    //uint8_t data[2];
    //data[0] = reg;
    //data[1] = value;
    //nrf_drv_twi_tx(&m_twi, addr, data, 2, false);

    twi_write_register_block(addr, reg, &value, 1);
}

void twi_write_register_block(uint8_t addr, uint8_t reg, uint8_t *block, uint8_t count)
{
    uint8_t data[17];
    data[0] = reg;
    memcpy(&data[1], block, MIN(count, 16));
    nrf_drv_twi_tx(&m_twi, addr, &data[0], count+1, false);

    //nrf_drv_twi_tx(&m_twi, addr, &reg, 1, true);
    //nrf_drv_twi_tx(&m_twi, addr, block, count, false);
}

uint8_t twi_read_register(uint8_t addr, uint8_t reg)
{
    uint8_t block;
    twi_read_register_block(addr, reg, &block, 1);
    return block;
}

void twi_read_register_block(uint8_t addr, uint8_t reg, uint8_t *block, uint8_t count)
{
    nrf_drv_twi_tx(&m_twi,addr,&reg,1,true);
    nrf_drv_twi_rx(&m_twi,addr,block,count);
}

