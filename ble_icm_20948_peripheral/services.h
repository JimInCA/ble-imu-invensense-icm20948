/**
 * Copyright (c) 2017 - 2020, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

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

#ifndef SERVICES_H__
#define SERVICES_H__

#include <stdint.h>
#include "ble.h"
#include "ble_srv_common.h"

#include "imu.h"

// Defining 16-bit service and 128-bit base UUIDs
// 5c1aa4bc-0e70-4a20-a88e-3259e2e8bad9
#define BLE_UUID_BASE_UUID              {{0xd9, 0xba, 0xe8, 0xe2, 0x59, 0x32, 0x8e, 0xa8, 0x20, 0x4a, 0x70, 0x0e, 0xbc, 0xa4, 0x1a, 0x5c}} // 128-bit base UUID

// Defining 16-bit service UUID
#define BLE_UUID_SERVICE_UUID                    0xface // Just a random, but recognizable value

// Defining 16-bit characteristic UUID
#define BLE_UUID_CHARACTERISTC_IMU_DATA          0xfade // IMU Data
#define BLE_UUID_CHARACTERISTC_IMU_DEVICEID      0xfeed // IMU Device ID
#define BLE_UUID_CHARACTERISTC_IMU_RESOLUTION    0xbead // IMU MEMS Resolution

// This structure contains various status information for the service. 
// The name is based on the naming convention used in Nordics SDKs. 
// 'ble' indicates that it is a Bluetooth Low Energy relevant structure and 
// 'os' is short for Our Service). 
typedef struct
{
    uint16_t                    conn_handle;    // Handle of the current connection (as provided by the BLE stack, is BLE_CONN_HANDLE_INVALID if not in a connection).
    uint16_t                    service_handle; // Handle of Our Service (as provided by the BLE stack).
    // add handles for the characteristic attributes to the struct
    ble_gatts_char_handles_t    char_handle_data;
    ble_gatts_char_handles_t    char_handle_deviceid;
    ble_gatts_char_handles_t    char_handle_resolution;
    bool                        is_imu_data_notification_enabled;
    bool                        is_imu_data_transfer_complete;
    uint32_t                    deviceid;
} ble_os_t;

// Function for handling BLE Stack events related to the service and characteristic.
//
// Handles all events from the BLE stack of interest to Our Service.
//
//     p_service  our Service structure
//     p_ble_evt  event received from the BLE stack
//
void ble_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context);

// Function for initializing the new service.
//
//     p_service  pointer to Our Service structure
//
void service_init(ble_os_t * p_service);

// Function for updating and sending new characteristic values
//
// The application calls this function whenever the timer_timeout_handler triggers
//
//     p_service       our Service structure
//     imu_data        new characteristic value
//     length          length of characteristic value
//
void characteristic_update_imu_data(ble_os_t *p_service, IMU_DATA *imu_data, int16_t length);

void characteristic_update_imu_deviceid(ble_os_t *p_service);
void characteristic_update_imu_resolution(ble_os_t *p_service, uint32_t resolution);

#endif  // _SERVICES_H__
