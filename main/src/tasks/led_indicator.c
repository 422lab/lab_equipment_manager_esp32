/*
 * led_indicator.c
 *
 *  Created on: 2018-02-13 15:43
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/led.h"

xTaskHandle led_indicator_task_handle;

static const portTickType led_delay_mode[5] = {1000, 500, 250, 100, 25};
static portTickType led_delay = 1000;

static const uint16_t led_count_mode[5] = {25, 50, 100};
static uint16_t led_count = 2;

#define TAG "led_indicator"

void led_indicator_set_mode(uint8_t mode_index)
{
    ESP_LOGD(TAG, "set mode %u", mode_index);
    switch (mode_index) {
        case 0:
            vTaskSuspend(led_indicator_task_handle);
            led_indicator_off();
            break;
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
            led_delay = led_delay_mode[mode_index-1];
            led_count = 2;
            vTaskResume(led_indicator_task_handle);
            break;
        case 6:
        case 7:
        case 8:
            led_delay = 25;
            led_count = led_count_mode[mode_index-6];
            vTaskResume(led_indicator_task_handle);
            break;
        case 9:
            vTaskSuspend(led_indicator_task_handle);
            led_indicator_on();
            break;
        default:
            ESP_LOGE(TAG, "set mode failed");
            break;
    }
}

void led_indicator_task(void *pvParameter)
{
    portTickType xLastWakeTime = 0;  
    uint16_t i = 0;

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        if (i++ % led_count) {
            led_indicator_off();
        }
        else {
            led_indicator_on();
        }

        vTaskDelayUntil(&xLastWakeTime, led_delay);
    }
}
