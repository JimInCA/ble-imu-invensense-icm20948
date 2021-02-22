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

#include "sdk_config.h"
#include "nordic_common.h"
#include "app_error.h"
#include "app_uart.h"
#include "ble_db_discovery.h"
#include "app_timer.h"
#include "app_util.h"
#include "bsp_btn_ble.h"
//#include "ble.h"
//#include "ble_gap.h"
//#include "ble_hci.h"
//#include "nrf_sdh.h"
//#include "nrf_sdh_ble.h"
//#include "nrf_sdh_soc.h"
#include "ble_nus_c.h"
//#include "nrf_ble_gatt.h"
//#include "nrf_pwr_mgmt.h"
//#include "nrf_ble_scan.h"

#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#include "uart.h"

#define UART_TX_BUF_SIZE       1024                                     /**< UART TX buffer size. */
#define UART_RX_BUF_SIZE       1024                                     /**< UART RX buffer size. */

ble_process_input_string_handler_t ble_process_input_string;

/**@brief   Function for handling app_uart events.
 *
 * @details This function receives a single character from the app_uart module and appends it to
 *          a string. The string is sent over BLE when the last character received is a
 *          'new line' '\n' (hex 0x0A) or if the string reaches the maximum data length.
 */
void uart_event_handle(app_uart_evt_t * p_event)
{
    static uint8_t data_array[BLE_NUS_MAX_DATA_LEN];
    static uint16_t index = 0;

    switch (p_event->evt_type)
    {
        /**@snippet [Handling data from UART] */
        case APP_UART_DATA_READY:
            UNUSED_VARIABLE(app_uart_get(&data_array[index]));
            index++;

            if (   (data_array[index - 1] == '\n')
                || (data_array[index - 1] == '\r')
                || (index >= BLE_NUS_MAX_DATA_LEN) )
            {
                // call function to process input string
                ble_process_input_string(data_array, index);
                index = 0;  // reset index for next string
            }
            break;

        /**@snippet [Handling data from UART] */
        case APP_UART_COMMUNICATION_ERROR:
            NRF_LOG_ERROR("Communication error occurred while handling UART.");
            APP_ERROR_HANDLER(p_event->data.error_communication);
            break;

        case APP_UART_FIFO_ERROR:
            NRF_LOG_ERROR("Error occurred in FIFO module used by UART.");
            APP_ERROR_HANDLER(p_event->data.error_code);
            break;

        default:
            break;
    }
}


void output_string(uint8_t *data_array, uint32_t length)
{
    ret_code_t ret_val;
    uint32_t i;

    for (i = 0; i < length; i++)
    {
        ret_val = app_uart_put(data_array[i]);
        if ((ret_val != NRF_SUCCESS) && (ret_val != NRF_ERROR_BUSY))
        {
            NRF_LOG_ERROR("Error occured processing output string.");
        }
    }

    //return ret_val;
}


/**@brief Function for initializing the UART. */
void uart_init(ble_process_input_string_handler_t ble_process_input_string_handler)
{
    ret_code_t err_code;

    ble_process_input_string = ble_process_input_string_handler;

    app_uart_comm_params_t const comm_params =
    {
        .rx_pin_no    = RX_PIN_NUMBER,
        .tx_pin_no    = TX_PIN_NUMBER,
        .rts_pin_no   = RTS_PIN_NUMBER,
        .cts_pin_no   = CTS_PIN_NUMBER,
        .flow_control = APP_UART_FLOW_CONTROL_DISABLED,
        .use_parity   = false,
        .baud_rate    = UART_BAUDRATE_BAUDRATE_Baud115200
    };
                       
    app_uart_buffers_t buffers;
    static uint8_t     rx_buf[UART_RX_BUF_SIZE];
    static uint8_t     tx_buf[UART_TX_BUF_SIZE];

    buffers.rx_buf      = rx_buf;
    buffers.rx_buf_size = sizeof (rx_buf);
    buffers.tx_buf      = tx_buf;
    buffers.tx_buf_size = sizeof (tx_buf);
    err_code = app_uart_init(&comm_params, &buffers, uart_event_handle, APP_IRQ_PRIORITY_LOWEST);

    APP_ERROR_CHECK(err_code);
}
