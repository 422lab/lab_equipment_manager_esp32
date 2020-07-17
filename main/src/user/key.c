/*
 * key.c
 *
 *  Created on: 2018-05-31 14:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "core/os.h"
#include "user/key_handle.h"

#define TAG "key"

static const uint8_t gpio_pin[] = {
    CONFIG_PWR_KEY_PIN,
};

static const uint8_t gpio_val[] = {
#ifdef CONFIG_PWR_KEY_ACTIVE_LOW
    0,
#else
    1,
#endif
};

static const uint16_t gpio_hold[] = {
    CONFIG_PWR_KEY_HOLD_TIME,
};

static void (*key_handle[])(void) = {
    pwr_key_handle,
};

static void key_task(void *pvParameter)
{
    portTickType xLastWakeTime;
    gpio_config_t io_conf = {0};
    uint16_t count[sizeof(gpio_pin)] = {0};

    for (int i=0; i<sizeof(gpio_pin); i++) {
        io_conf.pin_bit_mask = BIT64(gpio_pin[i]);
        io_conf.mode = GPIO_MODE_INPUT;

        if (gpio_val[i] == 0) {
            io_conf.pull_up_en = true;
            io_conf.pull_down_en = false;
        } else {
            io_conf.pull_up_en = false;
            io_conf.pull_down_en = true;
        }

        io_conf.intr_type = GPIO_INTR_DISABLE;

        gpio_config(&io_conf);
    }

    ESP_LOGI(TAG, "started.");

    vTaskDelay(2000 / portTICK_RATE_MS);

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            KEY_RUN_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        xLastWakeTime = xTaskGetTickCount();

        for (int i=0; i<sizeof(gpio_pin); i++) {
            if (gpio_get_level(gpio_pin[i]) == gpio_val[i]) {
                if (++count[i] == gpio_hold[i] / 10) {
                    count[i] = 0;

                    xEventGroupClearBits(user_event_group, KEY_RUN_BIT);

                    key_handle[i]();
                }
            } else {
                count[i] = 0;
            }
        }

        vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_RATE_MS);
    }
}

void key_init(void)
{
    xEventGroupSetBits(user_event_group, KEY_RUN_BIT);

    xTaskCreatePinnedToCore(key_task, "keyT", 1280, NULL, 5, NULL, 1);
}
