/*
SPDX-License-Identifier: GPL-3.0-or-later

smart-shower-timer
Copyright (C) 2022  dxcfl

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ei_device_thingy53.h"
#include "edge-impulse-sdk/porting/ei_classifier_porting.h"
#include "inference/ei_run_impulse.h"
#include "sensors/ei_microphone.h"
#include <drivers/uart.h>
#include <logging/log.h>
#include <nrfx_clock.h>
#include <zephyr.h>
#include "ei_result_aggregation.h"
#include <dk_buttons_and_leds.h>
#include "beacon.h"
#include "buzzer.h"

#define LOG_MODULE_NAME main
LOG_MODULE_REGISTER(LOG_MODULE_NAME);

#define LED_RED DK_LED1
#define LED_GREEN DK_LED2
#define LED_BLUE DK_LED3
#define USER_BUTTON DK_BTN1_MSK

#define SHOWER_TIME_MINUTES_WARNING CONFIG_SHOWER_TIME_MINUTES_WARNING

static const struct device *uart;

void ei_putchar(char c)
{
    // it is ~30% faster than printf
    uart_fifo_fill(uart, (const uint8_t *)&c, 1);
}

// Beep and blink red LED multiple times
static void beep(const uint8_t repeat, uint16_t length)
{
    for (uint8_t i = 0; i < repeat; i++)
    {
        buzzer_set_frequency(440, 100);
        dk_set_led(LED_RED, 1);
        ei_sleep(length);
        dk_set_led(LED_RED, 0);
    }
    buzzer_set_frequency(0, 0);
    ei_sleep(length * repeat);
}

// Button handler ...
static void button_changed(uint32_t button_state, uint32_t has_changed)
{
    static uint32_t usrbtnprsd_tm = 0;
    const static uint32_t lngprs_tm = 3000;

    if (has_changed & USER_BUTTON)
    {
        uint32_t user_button_state = button_state & USER_BUTTON;
        if (user_button_state)
        {
            usrbtnprsd_tm = k_uptime_get_32();
        }
        else
        {
            // Button pressed (released) long: Reset aggregation ...
            if (k_uptime_get_32() - usrbtnprsd_tm >= lngprs_tm)
            {
                LOG_INF("User button pressed (long)!\n");
                if (is_inference_running())
                {
                    ei_stop_impulse();
                }
                ei_result_aggregation_init(0.5);
                beacon_update_with_time(0);
                dk_set_led(LED_GREEN, 1);
                dk_set_led(LED_RED, 1);
                dk_set_led(LED_BLUE, 1);
                ei_sleep(1000);
                dk_set_led(LED_GREEN, 0);
                dk_set_led(LED_RED, 0);
                dk_set_led(LED_BLUE, 0);
            }
            // Button pressed (released) short: Start / stop detection ...
            else
            {
                LOG_INF("Button pressed!\n");
                if (is_inference_running())
                {
                    ei_stop_impulse();
                }
                else
                {
                    ei_start_impulse(false, false);
                }
            }
        }
    }
}

// Setup button handler
static int init_button(void)
{
    int err;

    err = dk_buttons_init(button_changed);
    if (err)
    {
        printk("Cannot init buttons (err: %d)\n", err);
    }

    return err;
}

int main(void)
{
    /* output of printf is output immediately without buffering */
    setvbuf(stdout, NULL, _IONBF, 0);

    /* Switch CPU core clock to 128 MHz */
    nrfx_clock_divider_set(NRF_CLOCK_DOMAIN_HFCLK, NRF_CLOCK_HFCLK_DIV_1);

    /* Initialize board uart */
    uart = device_get_binding("CDC_ACM_0");
    if (!uart)
    {
        LOG_ERR("Failed to init CDC_ACM_0\n");
    }

    /* Setup the microphone sensor */
    if (ei_microphone_init() == false)
    {
        LOG_ERR("Microphone init failed!");
    }

    /* Setup BLE beacon */
    beacon_start();

    /* Setup buzzer */
    buzzer_init();

    /* Setup button */
    init_button();

    // Beep!
    beep(1, 5000);

    // Reset result aggregation
    ei_result_aggregation_init(0.5);

    uint32_t previous_meassured_time_millis = 0;
    while (1)
    {
        if (is_inference_running())
        {
            uint32_t meassured_time_millis = ei_result_aggregation_get_time(1);

            if (meassured_time_millis != previous_meassured_time_millis)
            {
                dk_set_led(LED_GREEN, 1);
                ei_sleep(500);
                dk_set_led(LED_GREEN, 0);
                beacon_update_with_time(meassured_time_millis);
                previous_meassured_time_millis = meassured_time_millis;
            }

            uint32_t meassured_time_minutes = meassured_time_millis / 60000;
            if (meassured_time_minutes >= SHOWER_TIME_MINUTES_WARNING)
            {
                beep(meassured_time_minutes, 500);
            }
        }
        ei_sleep(1);
    }
}
