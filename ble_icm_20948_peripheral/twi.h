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

#ifndef TWI_H__
#define TWI_H__

#include <stdint.h>
#include <stdbool.h>
#include "nrf_drv_twi.h"
#include "app_util_platform.h"

typedef union{
    uint8_t raw;
    int8_t  conv;
} elem_t;

typedef struct
{
    elem_t  x;
    elem_t  y;
    elem_t  z;
    uint8_t tilt;
} sample_t;

static volatile bool m_xfer_done = true;
static volatile bool m_set_mode_done = false;

void twi_handler(nrf_drv_twi_evt_t const * p_event, void * p_context);
void twi_init(void);

void    twi_write_register(uint8_t addr, uint8_t reg, uint8_t value);
void    twi_write_register_block(uint8_t addr, uint8_t reg, uint8_t *block, uint8_t count);
uint8_t twi_read_register(uint8_t addr, uint8_t reg);
void    twi_read_register_block(uint8_t addr, uint8_t reg, uint8_t *block, uint8_t count);

#endif // TWI_H__
