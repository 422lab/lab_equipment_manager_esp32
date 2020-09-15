/*
 * key_handle.c
 *
 *  Created on: 2019-07-06 10:35
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"

#include "core/os.h"
#include "board/relay.h"
#include "user/http_app_status.h"

#define PWR_KEY_TAG "pwr_key"

void power_key_handle(void)
{
    if (relay_get_status()) {
        ESP_LOGW(PWR_KEY_TAG, "power off");

        http_app_update_status(HTTP_REQ_IDX_OFF);
    } else {
        ESP_LOGW(PWR_KEY_TAG, "power on");

        http_app_update_status(HTTP_REQ_IDX_ON);
    }
}
