/*
 * relay.c
 *
 *  Created on: 2020-07-12 18:28
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "driver/gpio.h"

#include "board/relay.h"

#define TAG "relay"

static relay_status_t relay_status = RELAY_STATUS_IDX_OFF;

void relay_set_status(relay_status_t status)
{
    relay_status = status;

    if (relay_status == RELAY_STATUS_IDX_ON) {
#ifdef CONFIG_RELAY_PIN_ACTIVE_LOW
        gpio_set_level(CONFIG_RELAY_PIN, 0);
#else
        gpio_set_level(CONFIG_RELAY_PIN, 1);
#endif
    } else {
#ifdef CONFIG_RELAY_PIN_ACTIVE_LOW
        gpio_set_level(CONFIG_RELAY_PIN, 1);
#else
        gpio_set_level(CONFIG_RELAY_PIN, 0);
#endif
    }
}

relay_status_t relay_get_status(void)
{
    return relay_status;
}

void relay_init(void)
{
    relay_set_status(RELAY_STATUS_IDX_OFF);

    gpio_config_t io_conf = {
        .pin_bit_mask = BIT64(CONFIG_RELAY_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = false,
        .pull_down_en = false,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&io_conf);

    ESP_LOGI(TAG, "initialized, pin: %d", CONFIG_RELAY_PIN);
}
