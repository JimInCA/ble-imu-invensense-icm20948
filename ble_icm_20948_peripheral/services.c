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

#include <stdint.h>
#include <string.h>
#include "nrf_gpio.h"
#include "services.h"
#include "ble_srv_common.h"
#include "app_error.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "imu.h"

void imu_dvid_characteristic_update(ble_os_t *p_service);

/**@brief Function for handling the @ref BLE_GATTS_EVT_WRITE event from the SoftDevice.
 *
 * @param[in] p_service     Nordic UART Service structure.
 * @param[in] p_ble_evt Pointer to the event received from BLE stack.
 */
static void on_write(ble_os_t * p_service, ble_evt_t const * p_ble_evt)
{
    ret_code_t                    err_code;

    ble_gatts_evt_write_t const * p_evt_write = &p_ble_evt->evt.gatts_evt.params.write;

    
    if ((p_evt_write->handle == p_service->char_handle_data.cccd_handle) &&
        (p_evt_write->len == 2))
    {
        NRF_LOG_INFO("data cccd write");
    }
    //else if (p_evt_write->handle == p_service->char_handle_dvid.value_handle)
    //{
    //    NRF_LOG_INFO("device id write");
    //    imu_dvid_characteristic_update(p_service);
    //}
    else
    {
        // Do Nothing. This event is not relevant for this service.
    }

}

// Declaration of a function that will take care of some housekeeping of ble connections 
// related to the service and characteristic.
void ble_service_on_ble_evt(ble_evt_t const * p_ble_evt, void * p_context)
{
    ble_os_t * p_service =(ble_os_t *) p_context;  
    // implement switch case handling BLE events related to the service.
    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_CONNECTED:
            p_service->conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            break;
        case BLE_GAP_EVT_DISCONNECTED:
            p_service->conn_handle = BLE_CONN_HANDLE_INVALID;
            break;
        case BLE_GATTS_EVT_WRITE:
            on_write(p_service, p_ble_evt);
            NRF_LOG_INFO("BLE_GATTS_EVT_WRITE");
            break;
        /*
        case BLE_GATTS_EVT_HVN_TX_COMPLETE:
            NRF_LOG_INFO("BLE_GATTS_EVT_HVN_TX_COMPLETE");
            break;
        */
        default:
            // no implementation needed
            break;
    }
}

// Function for adding the new characterstic to "Our service" that we initiated in the previous tutorial. 
//
//     p_service  our Service structure
//
static uint32_t char_add_data(ble_os_t * p_service)
{
    // add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_BASE_UUID;
    char_uuid.uuid      = BLE_UUID_CHARACTERISTC_IMU_DATA;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // add read/write properties to the characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 0;

    // configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 1;

    // configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // set read/write security levels to the characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // set characteristic length in number of bytes
    // This is where I need to adjust the size of the characteristic data.  JTN
    attr_char_value.max_len     = sizeof(IMU_DATA);
    attr_char_value.init_len    = sizeof(IMU_DATA);
    uint8_t value[sizeof(IMU_DATA)]            = {0x12,0x34,0x56,0x78};
    attr_char_value.p_value     = value;

    // add the new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &p_service->char_handle_data);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}

// Function for adding the new characterstic to "Our service" that we initiated in the previous tutorial. 
//
//     p_service  our Service structure
//
static uint32_t char_add_dvid(ble_os_t * p_service)
{
    // add a custom characteristic UUID
    uint32_t            err_code;
    ble_uuid_t          char_uuid;
    ble_uuid128_t       base_uuid = BLE_UUID_BASE_UUID;
    char_uuid.uuid      = BLE_UUID_CHARACTERISTC_IMU_DVID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &char_uuid.type);
    APP_ERROR_CHECK(err_code);

    // add read/write properties to the characteristic
    ble_gatts_char_md_t char_md;
    memset(&char_md, 0, sizeof(char_md));
    char_md.char_props.read = 1;
    char_md.char_props.write = 0;

    // configuring Client Characteristic Configuration Descriptor metadata and add to char_md structure
    ble_gatts_attr_md_t cccd_md;
    memset(&cccd_md, 0, sizeof(cccd_md));
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&cccd_md.write_perm);
    cccd_md.vloc                = BLE_GATTS_VLOC_STACK;    
    char_md.p_cccd_md           = &cccd_md;
    char_md.char_props.notify   = 0;

    // configure the attribute metadata
    ble_gatts_attr_md_t attr_md;
    memset(&attr_md, 0, sizeof(attr_md));
    attr_md.vloc        = BLE_GATTS_VLOC_STACK;

    // set read/write security levels to the characteristic
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.read_perm);
    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&attr_md.write_perm);

    // configure the characteristic value attribute
    ble_gatts_attr_t    attr_char_value;
    memset(&attr_char_value, 0, sizeof(attr_char_value));
    attr_char_value.p_uuid      = &char_uuid;
    attr_char_value.p_attr_md   = &attr_md;

    // set characteristic length in number of bytes
    // This is where I need to adjust the size of the characteristic data.  JTN
    attr_char_value.max_len     = sizeof(uint32_t);
    attr_char_value.init_len    = sizeof(uint32_t);
    //uint8_t value[sizeof(uint32_t)]            = {0xaa,0x55,0xaa,0x55};
    uint8_t value[sizeof(uint32_t)];
    for (int32_t i = 0, j = 0; i < sizeof(uint32_t); i++, j+=8)
    {
        value[i] = (p_service->deviceid >> j) & 0x000000ff;
    }
    attr_char_value.p_value     = value;

    // add the new characteristic to the service
    err_code = sd_ble_gatts_characteristic_add(p_service->service_handle,
                                               &char_md,
                                               &attr_char_value,
                                               &p_service->char_handle_dvid);
    APP_ERROR_CHECK(err_code);

    return NRF_SUCCESS;
}


// Function for initiating the new service.
//
//    p_service  service structure
//
void service_init(ble_os_t * p_service)
{
    uint32_t   err_code; // variable to hold return codes from library and softdevice functions

    // declare 16-bit service and 128-bit base UUIDs and add them to the BLE stack
    ble_uuid_t        service_uuid;
    ble_uuid128_t     base_uuid = BLE_UUID_BASE_UUID;
    service_uuid.uuid = BLE_UUID_SERVICE_UUID;
    err_code = sd_ble_uuid_vs_add(&base_uuid, &service_uuid.type);
    APP_ERROR_CHECK(err_code);    

    // Set the service connection handle to default value. I.e. an invalid handle since we are not yet in a connection.
    p_service->conn_handle = BLE_CONN_HANDLE_INVALID;

    // add the service
    err_code = sd_ble_gatts_service_add(BLE_GATTS_SRVC_TYPE_PRIMARY,
                                        &service_uuid,
                                        &p_service->service_handle);

    APP_ERROR_CHECK(err_code);

    // call the function char_add_[x]() to add the new characteristics to the service.
    char_add_data(p_service);
    char_add_dvid(p_service);
}

// Function to be called when updating characteristic value with IMU data
void imu_characteristic_update(ble_os_t *p_service, IMU_DATA *imu_data, int16_t length)
{
    uint32_t err_code;
    // update characteristic value
    if (p_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = length;
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_service->char_handle_data.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)imu_data;  

        err_code = sd_ble_gatts_hvx(p_service->conn_handle, &hvx_params);
        if (err_code == NRF_SUCCESS)
        {
            nrf_gpio_pin_clear(PIN_OUT);
        }
        else
        {
            NRF_LOG_INFO("sd_ble_gatts_hvx() returned error code %d", err_code);
        }
    }
}


// Function to be called when updating characteristic value with IMU data
void imu_dvid_characteristic_update(ble_os_t *p_service)
{
    uint32_t err_code;
    // update characteristic value
    if (p_service->conn_handle != BLE_CONN_HANDLE_INVALID)
    {
        uint16_t               len = sizeof(uint32_t);
        ble_gatts_hvx_params_t hvx_params;
        memset(&hvx_params, 0, sizeof(hvx_params));

        hvx_params.handle = p_service->char_handle_dvid.value_handle;
        hvx_params.type   = BLE_GATT_HVX_NOTIFICATION;
        hvx_params.offset = 0;
        hvx_params.p_len  = &len;
        hvx_params.p_data = (uint8_t*)&(p_service->deviceid);  

        err_code = sd_ble_gatts_hvx(p_service->conn_handle, &hvx_params);
        if (err_code != NRF_SUCCESS)
        {
            NRF_LOG_INFO("sd_ble_gatts_hvx() returned error code %d", err_code);
        }
    }
}
