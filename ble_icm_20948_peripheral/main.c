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

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "nordic_common.h"
#include "nrf.h"
#include "app_error.h"
#include "ble.h"
#include "ble_hci.h"
#include "ble_srv_common.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "app_timer.h"
#include "fds.h"
#include "peer_manager.h"
#include "bsp_btn_ble.h"
#include "sensorsim.h"
#include "ble_conn_state.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_gpiote.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "services.h"
#include "imu.h"
#include "twi.h"
#include "hal.h"

#define DEVICE_NAME                     "BLE-IMU"                               // Name of device. Will be included in the advertising data
#define MANUFACTURER_NAME               "JTNEnterprises"                        // Manufacturer. Will be passed to Device Information Service
#define APP_ADV_INTERVAL                300                                     // The advertising interval (in units of 0.625 ms. This value corresponds to 187.5 ms)

#define APP_ADV_DURATION                18000                                   // The advertising duration (180 seconds) in units of 10 milliseconds
#define APP_BLE_OBSERVER_PRIO           3                                       // Application's BLE observer priority. You shouldn't need to modify this value
#define APP_BLE_CONN_CFG_TAG            1                                       // A tag identifying the SoftDevice BLE configuration

#define MIN_CONN_INTERVAL               MSEC_TO_UNITS(100, UNIT_1_25_MS)        // Minimum acceptable connection interval (0.1 seconds)
#define MAX_CONN_INTERVAL               MSEC_TO_UNITS(200, UNIT_1_25_MS)        // Maximum acceptable connection interval (0.2 second)
#define SLAVE_LATENCY                   0                                       // Slave latency
#define CONN_SUP_TIMEOUT                MSEC_TO_UNITS(4000, UNIT_10_MS)         // Connection supervisory timeout (4 seconds)

#define FIRST_CONN_PARAMS_UPDATE_DELAY  APP_TIMER_TICKS(5000)                   // Time from initiating event (connect or start of notification) to first time sd_ble_gap_conn_param_update is called (5 seconds)
#define NEXT_CONN_PARAMS_UPDATE_DELAY   APP_TIMER_TICKS(30000)                  // Time between each call to sd_ble_gap_conn_param_update after the first call (30 seconds)
#define MAX_CONN_PARAMS_UPDATE_COUNT    3                                       // Number of attempts before giving up the connection parameter negotiation

#define SEC_PARAM_BOND                  1                                       // Perform bonding
#define SEC_PARAM_MITM                  0                                       // Man In The Middle protection not required
#define SEC_PARAM_LESC                  0                                       // LE Secure Connections not enabled
#define SEC_PARAM_KEYPRESS              0                                       // Keypress notifications not enabled
#define SEC_PARAM_IO_CAPABILITIES       BLE_GAP_IO_CAPS_NONE                    // No I/O capabilities
#define SEC_PARAM_OOB                   0                                       // Out Of Band data not available
#define SEC_PARAM_MIN_KEY_SIZE          7                                       // Minimum encryption key size
#define SEC_PARAM_MAX_KEY_SIZE          16                                      // Maximum encryption key size

#define DEAD_BEEF                       0xDEADBEEF                              // Value used as error code on stack dump, can be used to identify stack location on stack unwind


NRF_BLE_GATT_DEF(m_gatt);                                                       // GATT module instance
NRF_BLE_QWR_DEF(m_qwr);                                                         // Context for the Queued Write module
BLE_ADVERTISING_DEF(m_advertising);                                             // Advertising module instance

static int16_t imu_init(void);
void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action);

static uint16_t m_conn_handle = BLE_CONN_HANDLE_INVALID;                        // Handle of the current connection
ble_os_t m_service;   // declare a service structure for the application
extern inv_icm20948_state st;
static volatile uint32_t deviceid;

// Declare an app_timer id variable and define the timer interval and define a timer interval.
//APP_TIMER_DEF(m_char_timer_id);
#define CHAR_TIMER_INTERVAL     APP_TIMER_TICKS(1000) // 1000 ms intervals


// Use UUIDs for service(s) used in your application.
static ble_uuid_t m_adv_uuids[] =                                               // Universally unique service identifiers
{
    {BLE_UUID_SERVICE_UUID, BLE_UUID_TYPE_VENDOR_BEGIN}
};


static void advertising_start(bool erase_bonds);


// Callback function for asserts in the SoftDevice.
//
// This function will be called in case of an assert in the SoftDevice.
//
// Warning: This handler is an example only and does not fit a final product. You need to analyze
//          how your product is supposed to react in case of Assert.
// Warning: On assert from the SoftDevice, the system can only recover on reset.
//
//    line_num   line number of the failing ASSERT call
//    file_name  file name of the failing ASSERT call
//
void assert_nrf_callback(uint16_t line_num, const uint8_t * p_file_name)
{
    app_error_handler(DEAD_BEEF, line_num, p_file_name);
}


// Function for handling Peer Manager events.
//
//     p_evt  peer Manager event
//
static void pm_evt_handler(pm_evt_t const * p_evt)
{
    ret_code_t err_code;

    switch (p_evt->evt_id)
    {
        case PM_EVT_BONDED_PEER_CONNECTED:
        {
            NRF_LOG_INFO("Connected to a previously bonded device.");
        } break;

        case PM_EVT_CONN_SEC_SUCCEEDED:
        {
            NRF_LOG_INFO("Connection secured: role: %d, conn_handle: 0x%x, procedure: %d.",
                         ble_conn_state_role(p_evt->conn_handle),
                         p_evt->conn_handle,
                         p_evt->params.conn_sec_succeeded.procedure);
        } break;

        case PM_EVT_CONN_SEC_FAILED:
        {
            // Often, when securing fails, it shouldn't be restarted, for security reasons.
            // Other times, it can be restarted directly.
            // Sometimes it can be restarted, but only after changing some Security Parameters.
            // Sometimes, it cannot be restarted until the link is disconnected and reconnected.
            // Sometimes it is impossible, to secure the link, or the peer device does not support it.
            // How to handle this error is highly application dependent
        } break;

        case PM_EVT_CONN_SEC_CONFIG_REQ:
        {
            // reject pairing request from an already bonded peer
            pm_conn_sec_config_t conn_sec_config = {.allow_repairing = false};
            pm_conn_sec_config_reply(p_evt->conn_handle, &conn_sec_config);
        } break;

        case PM_EVT_STORAGE_FULL:
        {
            // run garbage collection on the flash
            err_code = fds_gc();
            if (err_code == FDS_ERR_NO_SPACE_IN_QUEUES)
            {
                // retry
            }
            else
            {
                APP_ERROR_CHECK(err_code);
            }
        } break;

        case PM_EVT_PEERS_DELETE_SUCCEEDED:
        {
            advertising_start(false);
        } break;

        case PM_EVT_PEER_DATA_UPDATE_FAILED:
        {
            // assert
            APP_ERROR_CHECK(p_evt->params.peer_data_update_failed.error);
        } break;

        case PM_EVT_PEER_DELETE_FAILED:
        {
            // assert
            APP_ERROR_CHECK(p_evt->params.peer_delete_failed.error);
        } break;

        case PM_EVT_PEERS_DELETE_FAILED:
        {
            // assert
            APP_ERROR_CHECK(p_evt->params.peers_delete_failed_evt.error);
        } break;

        case PM_EVT_ERROR_UNEXPECTED:
        {
            // assert
            APP_ERROR_CHECK(p_evt->params.error_unexpected.error);
        } break;

        case PM_EVT_CONN_SEC_START:
        case PM_EVT_PEER_DATA_UPDATE_SUCCEEDED:
        case PM_EVT_PEER_DELETE_SUCCEEDED:
        case PM_EVT_LOCAL_DB_CACHE_APPLIED:
        case PM_EVT_LOCAL_DB_CACHE_APPLY_FAILED:
            // This can happen when the local DB has changed.
        case PM_EVT_SERVICE_CHANGED_IND_SENT:
        case PM_EVT_SERVICE_CHANGED_IND_CONFIRMED:
        default:
            break;
    }
}


// Function for the Timer initialization.
//
// Initializes the timer module. This creates and starts application timers.
// Application timers are required by softdevice.
//
static void timers_init(void)
{
    // initialize timer module
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


// Function for the GAP initialization.
//
// This function sets up all the necessary GAP (Generic Access Profile) parameters of the
// device including the device name, appearance, and the preferred connection parameters.
//
static void gap_params_init(void)
{
    ret_code_t              err_code;
    ble_gap_conn_params_t   gap_conn_params;
    ble_gap_conn_sec_mode_t sec_mode;

    BLE_GAP_CONN_SEC_MODE_SET_OPEN(&sec_mode);

    err_code = sd_ble_gap_device_name_set(&sec_mode,
                                          (const uint8_t *)DEVICE_NAME,
                                          strlen(DEVICE_NAME));
    APP_ERROR_CHECK(err_code);

    memset(&gap_conn_params, 0, sizeof(gap_conn_params));

    gap_conn_params.min_conn_interval = MIN_CONN_INTERVAL;
    gap_conn_params.max_conn_interval = MAX_CONN_INTERVAL;
    gap_conn_params.slave_latency     = SLAVE_LATENCY;
    gap_conn_params.conn_sup_timeout  = CONN_SUP_TIMEOUT;

    err_code = sd_ble_gap_ppcp_set(&gap_conn_params);
    APP_ERROR_CHECK(err_code);
}


// Function for initializing the GATT module.
static void gatt_init(void)
{
    ret_code_t err_code = nrf_ble_gatt_init(&m_gatt, NULL);
    APP_ERROR_CHECK(err_code);
}


// Function for handling Queued Write Module errors.
//
// A pointer to this function will be passed to each service which may need to inform the
// application about an error.
//
//     nrf_error   error code containing information about what went wrong
//
static void nrf_qwr_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


// Function for initializing services that will be used by the application.
static void services_init(void)
{
    uint32_t         err_code;
    nrf_ble_qwr_init_t qwr_init = {0};

    // initialize Queued Write Module
    qwr_init.error_handler = nrf_qwr_error_handler;

    err_code = nrf_ble_qwr_init(&m_qwr, &qwr_init);
    APP_ERROR_CHECK(err_code);

    // add code to initialize the services used by the application
    service_init(&m_service);
}


// Function for handling the Connection Parameters Module.
//
// This function will be called for all events in the Connection Parameters Module which
// are passed to the application.
//
// Note: All this function does is to disconnect. This could have been done by simply
//       setting the disconnect_on_fail config parameter, but instead we use the event
//       handler mechanism to demonstrate its use.
//
//    p_evt  event received from the Connection Parameters Module
//
static void on_conn_params_evt(ble_conn_params_evt_t * p_evt)
{
    ret_code_t err_code;

    if (p_evt->evt_type == BLE_CONN_PARAMS_EVT_FAILED)
    {
        err_code = sd_ble_gap_disconnect(m_conn_handle, BLE_HCI_CONN_INTERVAL_UNACCEPTABLE);
        APP_ERROR_CHECK(err_code);
    }
}


// Function for handling a Connection Parameters error.
//
//     nrf_error  Error code containing information about what went wrong.
//
static void conn_params_error_handler(uint32_t nrf_error)
{
    APP_ERROR_HANDLER(nrf_error);
}


// Function for initializing the Connection Parameters module.
static void conn_params_init(void)
{
    ret_code_t             err_code;
    ble_conn_params_init_t cp_init;

    memset(&cp_init, 0, sizeof(cp_init));

    cp_init.p_conn_params                  = NULL;
    cp_init.first_conn_params_update_delay = FIRST_CONN_PARAMS_UPDATE_DELAY;
    cp_init.next_conn_params_update_delay  = NEXT_CONN_PARAMS_UPDATE_DELAY;
    cp_init.max_conn_params_update_count   = MAX_CONN_PARAMS_UPDATE_COUNT;
    cp_init.start_on_notify_cccd_handle    = BLE_GATT_HANDLE_INVALID;
    cp_init.disconnect_on_fail             = false;
    cp_init.evt_handler                    = on_conn_params_evt;
    cp_init.error_handler                  = conn_params_error_handler;

    err_code = ble_conn_params_init(&cp_init);
    APP_ERROR_CHECK(err_code);
}


// Function for starting timers.
//static void application_timers_start(void)
//{
//    // start the timer
//    app_timer_start(m_char_timer_id, CHAR_TIMER_INTERVAL, NULL);
//}


// Function for putting the chip into sleep mode.
//
// Note: This function will not return.
//
static void sleep_mode_enter(void)
{
    ret_code_t err_code;

    err_code = bsp_indication_set(BSP_INDICATE_IDLE);
    APP_ERROR_CHECK(err_code);

    // prepare wakeup buttons
    err_code = bsp_btn_ble_sleep_mode_prepare();
    APP_ERROR_CHECK(err_code);

    // go to system-off mode (this function will not return; wakeup will cause a reset)
    err_code = sd_power_system_off();
    APP_ERROR_CHECK(err_code);
}


// Function for handling advertising events.
//
// This function will be called for advertising events which are passed to the application.
//
//     ble_adv_evt  advertising event
//
static void on_adv_evt(ble_adv_evt_t ble_adv_evt)
{
    ret_code_t err_code;

    switch (ble_adv_evt)
    {
        case BLE_ADV_EVT_FAST:
            NRF_LOG_INFO("Fast advertising.");
            err_code = bsp_indication_set(BSP_INDICATE_ADVERTISING);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_ADV_EVT_IDLE:
            sleep_mode_enter();
            break;

        default:
            break;
    }
}


// Function for handling BLE events.
//
//     p_ble_evt   bluetooth stack event
//     p_context   unused
//
static void ble_evt_handler(ble_evt_t const * p_ble_evt, void * p_context)
{
    ret_code_t err_code = NRF_SUCCESS;
        

    switch (p_ble_evt->header.evt_id)
    {
        case BLE_GAP_EVT_DISCONNECTED:
            NRF_LOG_INFO("Disconnected.");
            // LED indication will be changed when advertising starts
            break;

        case BLE_GAP_EVT_CONNECTED:
            NRF_LOG_INFO("Connected.");
            err_code = bsp_indication_set(BSP_INDICATE_CONNECTED);
            APP_ERROR_CHECK(err_code);
            m_conn_handle = p_ble_evt->evt.gap_evt.conn_handle;
            err_code = nrf_ble_qwr_conn_handle_assign(&m_qwr, m_conn_handle);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GAP_EVT_PHY_UPDATE_REQUEST:
        {
            NRF_LOG_DEBUG("PHY update request.");
            ble_gap_phys_t const phys =
            {
                .rx_phys = BLE_GAP_PHY_AUTO,
                .tx_phys = BLE_GAP_PHY_AUTO,
            };
            err_code = sd_ble_gap_phy_update(p_ble_evt->evt.gap_evt.conn_handle, &phys);
            APP_ERROR_CHECK(err_code);
        } break;

        case BLE_GATTC_EVT_TIMEOUT:
            // disconnect on GATT Client timeout event
            NRF_LOG_DEBUG("GATT Client Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gattc_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        case BLE_GATTS_EVT_TIMEOUT:
            // disconnect on GATT Server timeout event
            NRF_LOG_DEBUG("GATT Server Timeout.");
            err_code = sd_ble_gap_disconnect(p_ble_evt->evt.gatts_evt.conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            APP_ERROR_CHECK(err_code);
            break;

        default:
            // no implementation needed
            break;
    }
}


// Function for initializing the BLE stack.
//
// Initializes the SoftDevice and the BLE event interrupt.
//
static void ble_stack_init(void)
{
    ret_code_t err_code;

    err_code = nrf_sdh_enable_request();
    APP_ERROR_CHECK(err_code);

    // Configure the BLE stack using the default settings.
    // Fetch the start address of the application RAM.
    uint32_t ram_start = 0;
    err_code = nrf_sdh_ble_default_cfg_set(APP_BLE_CONN_CFG_TAG, &ram_start);
    APP_ERROR_CHECK(err_code);

    // enable BLE stack
    err_code = nrf_sdh_ble_enable(&ram_start);
    APP_ERROR_CHECK(err_code);

    // register a handler for BLE events
    NRF_SDH_BLE_OBSERVER(m_ble_observer, APP_BLE_OBSERVER_PRIO, ble_evt_handler, NULL);

    // call ble_service_on_ble_evt() to do housekeeping of ble connections related to the service and characteristics
    NRF_SDH_BLE_OBSERVER(m_service_observer, APP_BLE_OBSERVER_PRIO, ble_service_on_ble_evt, (void*) &m_service);
}


// Function for the Peer Manager initialization.
static void peer_manager_init(void)
{
    ble_gap_sec_params_t sec_param;
    ret_code_t           err_code;

    err_code = pm_init();
    APP_ERROR_CHECK(err_code);

    memset(&sec_param, 0, sizeof(ble_gap_sec_params_t));

    // Security parameters to be used for all security procedures.
    sec_param.bond           = SEC_PARAM_BOND;
    sec_param.mitm           = SEC_PARAM_MITM;
    sec_param.lesc           = SEC_PARAM_LESC;
    sec_param.keypress       = SEC_PARAM_KEYPRESS;
    sec_param.io_caps        = SEC_PARAM_IO_CAPABILITIES;
    sec_param.oob            = SEC_PARAM_OOB;
    sec_param.min_key_size   = SEC_PARAM_MIN_KEY_SIZE;
    sec_param.max_key_size   = SEC_PARAM_MAX_KEY_SIZE;
    sec_param.kdist_own.enc  = 1;
    sec_param.kdist_own.id   = 1;
    sec_param.kdist_peer.enc = 1;
    sec_param.kdist_peer.id  = 1;

    err_code = pm_sec_params_set(&sec_param);
    APP_ERROR_CHECK(err_code);

    err_code = pm_register(pm_evt_handler);
    APP_ERROR_CHECK(err_code);
}


// Clear bond information from persistent storage.
static void delete_bonds(void)
{
    ret_code_t err_code;

    NRF_LOG_INFO("Erase bonds!");

    err_code = pm_peers_delete();
    APP_ERROR_CHECK(err_code);
}


// Function for handling events from the BSP module.
//
//     event   event generated when button is pressed
//
static void bsp_event_handler(bsp_event_t event)
{
    ret_code_t err_code;

    switch (event)
    {
        case BSP_EVENT_SLEEP:
            sleep_mode_enter();
            break; // BSP_EVENT_SLEEP

        case BSP_EVENT_DISCONNECT:
            err_code = sd_ble_gap_disconnect(m_conn_handle,
                                             BLE_HCI_REMOTE_USER_TERMINATED_CONNECTION);
            if (err_code != NRF_ERROR_INVALID_STATE)
            {
                APP_ERROR_CHECK(err_code);
            }
            break; // BSP_EVENT_DISCONNECT

        case BSP_EVENT_WHITELIST_OFF:
            if (m_conn_handle == BLE_CONN_HANDLE_INVALID)
            {
                err_code = ble_advertising_restart_without_whitelist(&m_advertising);
                if (err_code != NRF_ERROR_INVALID_STATE)
                {
                    APP_ERROR_CHECK(err_code);
                }
            }
            break; // BSP_EVENT_KEY_0

        default:
            break;
    }
}


// Function for initializing the Advertising functionality
static void advertising_init(void)
{
    ret_code_t             err_code;
    ble_advertising_init_t init;

    memset(&init, 0, sizeof(init));

    init.advdata.name_type               = BLE_ADVDATA_FULL_NAME;
    init.advdata.include_appearance      = true;
    init.advdata.flags                   = BLE_GAP_ADV_FLAGS_LE_ONLY_GENERAL_DISC_MODE;

    init.srdata.uuids_complete.uuid_cnt = sizeof(m_adv_uuids) / sizeof(m_adv_uuids[0]);
    init.srdata.uuids_complete.p_uuids = m_adv_uuids;

    init.config.ble_adv_fast_enabled  = true;
    init.config.ble_adv_fast_interval = APP_ADV_INTERVAL;
    init.config.ble_adv_fast_timeout  = APP_ADV_DURATION;

    init.evt_handler = on_adv_evt;

    err_code = ble_advertising_init(&m_advertising, &init);
    APP_ERROR_CHECK(err_code);

    ble_advertising_conn_cfg_tag_set(&m_advertising, APP_BLE_CONN_CFG_TAG);
}


// Function for initializing buttons and leds.
//
//     p_erase_bonds  will be true if the clear bonding button was pressed to wake the application up
//
static void buttons_leds_init(bool * p_erase_bonds)
{
    ret_code_t err_code;
    bsp_event_t startup_event;

    err_code = bsp_init(BSP_INIT_LEDS | BSP_INIT_BUTTONS, bsp_event_handler);
    APP_ERROR_CHECK(err_code);

    err_code = bsp_btn_ble_init(NULL, &startup_event);
    APP_ERROR_CHECK(err_code);

    *p_erase_bonds = (startup_event == BSP_EVENT_CLEAR_BONDING_DATA);
}


// Function for initializing the nrf log module.
static void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


// Function for initializing power management.
static void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


// Function for handling the idle state (main loop).
//
// If there is no pending log operation, then sleep until next the next event occurs.
//
static void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
    }
}


// Function for starting advertising.
static void advertising_start(bool erase_bonds)
{
    if (erase_bonds == true)
    {
        delete_bonds();
        // Advertising is started by PM_EVT_PEERS_DELETED_SUCEEDED event
    }
    else
    {
        ret_code_t err_code = ble_advertising_start(&m_advertising, BLE_ADV_MODE_FAST);
        APP_ERROR_CHECK(err_code);
    }
}


static int16_t imu_init(void)
{
    int16_t result;
    uint8_t counter = 0;
    uint8_t device_id;

    NRF_LOG_INFO("calling inv_check_and_setup_chip()");
    do 
    {
        result = inv_check_and_setup_chip(&st);
        if (result)
        {
            NRF_LOG_INFO("IMU failed to initialize");
            return result;
        }
    } while (result);

    NRF_LOG_INFO("calling inv_icm20948_get_device_id()");
    do
    {
        device_id = inv_icm20948_get_device_id();
    } while ((device_id != IMU_ID) && (counter++ < 10));

    if (device_id == IMU_ID)
    {
        NRF_LOG_INFO("IMU is responding on I2C bus");
    }
    else
    {
        NRF_LOG_INFO("IMU failed to respond on I2C bus");
        return -1;
    }

    return 0;
}

bool data_ready = false;

void in_pin_handler(nrf_drv_gpiote_pin_t pin, nrf_gpiote_polarity_t action)
{
    if (data_ready == false)
    {
        data_ready = true;
    }
}


// Function for configuring: INV_INT_PIN pin for input, PIN_OUT pin for output,
// and configures GPIOTE to give an interrupt on pin change.
static void gpio_init(void)
{
    ret_code_t err_code;

    if (!nrf_drv_gpiote_is_init())
    {
        err_code = nrf_drv_gpiote_init();
        APP_ERROR_CHECK(err_code);
    }

    nrf_drv_gpiote_out_config_t out_config = GPIOTE_CONFIG_OUT_SIMPLE(false);

    err_code = nrf_drv_gpiote_out_init(PIN_OUT, &out_config);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_config_t in_config = GPIOTE_CONFIG_IN_SENSE_LOTOHI(true);
    in_config.pull = NRF_GPIO_PIN_PULLUP;

    err_code = nrf_drv_gpiote_in_init(INV_INT_PIN, &in_config, in_pin_handler);
    APP_ERROR_CHECK(err_code);

    nrf_drv_gpiote_in_event_enable(INV_INT_PIN, true);
}


// Function for application main entry.
int main(void)
{
    bool erase_bonds;

    // initialize
    log_init();
    timers_init();
    buttons_leds_init(&erase_bonds);
    power_management_init();
    imu_init();
    ble_stack_init();
    gap_params_init();
    gatt_init();

    services_init();
    advertising_init();

    conn_params_init();
    peer_manager_init();

    deviceid = NRF_FICR->DEVICEID0;

    // start execution
    NRF_LOG_INFO("BLE IMU evaluation started.");
    //application_timers_start();

    advertising_start(erase_bonds);

    gpio_init();

    // enter main loop
    while (1)
    {
        if (data_ready == true)
        {
            IMU_DATA imu_data;
            //inv_icm20948_read_imu(&imu_data);
            inv_icm20948_read_imu_fifo(&st, &imu_data);
            data_ready = false;
            imu_data.deviceid = deviceid;
            nrf_gpio_pin_set(PIN_OUT);
            imu_characteristic_update(&m_service, &imu_data, sizeof(IMU_DATA));
        }
        idle_state_handle();
    }
}
