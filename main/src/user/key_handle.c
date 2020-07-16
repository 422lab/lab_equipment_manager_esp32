/*
 * key_handle.c
 *
 *  Created on: 2019-07-06 10:35
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_smartconfig.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "core/os.h"
#include "board/relay.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/audio_player.h"
#include "user/http_app_status.h"

#define SC_KEY_TAG  "sc_key"
#define PWR_KEY_TAG "pwr_key"

#ifdef CONFIG_ENABLE_SC_KEY
void sc_key_handle(void)
{
    ESP_LOGI(SC_KEY_TAG, "start smartconfig");
    xEventGroupSetBits(wifi_event_group, WIFI_CFG_BIT);

    led_set_mode(5);
    gui_set_mode(GUI_MODE_IDX_GIF_CFG);
    audio_player_play_file(7);

    ESP_ERROR_CHECK(esp_wifi_disconnect());

    ESP_ERROR_CHECK(esp_smartconfig_set_type(SC_TYPE_ESPTOUCH));
    smartconfig_start_config_t sc_cfg = SMARTCONFIG_START_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_smartconfig_start(&sc_cfg));
}
#endif // CONFIG_ENABLE_SC_KEY

void pwr_key_handle(void)
{
    if (relay_get_status()) {
        ESP_LOGW(PWR_KEY_TAG, "power off");

        http_app_update_status(HTTP_REQ_IDX_OFF);
    } else {
        ESP_LOGW(PWR_KEY_TAG, "power on");

        http_app_update_status(HTTP_REQ_IDX_ON);
    }
}
