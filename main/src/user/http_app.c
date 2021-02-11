/*
 * http_app.c
 *
 *  Created on: 2018-02-17 18:51
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_http_client.h"

#include "core/os.h"
#include "chip/wifi.h"
#include "board/relay.h"

#include "user/key.h"
#include "user/led.h"
#include "user/gui.h"
#include "user/audio_player.h"
#include "user/http_app_status.h"

#define TAG "http_app"

static void http_app_task(void *pvParameter)
{
    char post_data[128] = {0};
    char server_url[80] = {0};
    esp_http_client_config_t config = {0};

    while (1) {
        EventBits_t uxBits = xEventGroupWaitBits(
            user_event_group,
            HTTP_APP_STATUS_RUN_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        key_set_scan_mode(KEY_SCAN_MODE_IDX_OFF);

#ifdef CONFIG_ENABLE_LED
        led_set_mode(LED_MODE_IDX_BLINK_F1);
#endif
        gui_set_mode(GUI_MODE_IDX_GIF_BUSY);

#ifdef CONFIG_TRANSFER_PROTOCOL_HTTP
        strcpy(server_url, "http://");
#else
        strcpy(server_url, "https://");
#endif
        strcat(server_url, CONFIG_SERVER_URL);
        config.url = (const char *)server_url;
#ifdef CONFIG_VERIFY_SERVER_CERT
        config.cert_pem = cert0_pem_ptr;
#endif

        if (uxBits & HTTP_APP_STATUS_RUN_BIT) {
            config.event_handler = http_app_status_event_handler;
            http_app_status_prepare_data(post_data, sizeof(post_data));
            xEventGroupClearBits(user_event_group, HTTP_APP_STATUS_DONE_BIT | HTTP_APP_STATUS_FAIL_BIT);
        }
        esp_http_client_handle_t client = esp_http_client_init(&config);

        esp_http_client_set_method(client, HTTP_METHOD_POST);
        esp_http_client_set_post_field(client, post_data, strlen(post_data));

        esp_err_t err = esp_http_client_perform(client);
        if (err != ESP_OK) {
            ESP_LOGE(TAG, "failed to perform http(s) request");
            if (config.event_handler == http_app_status_event_handler) {
                if (relay_get_status()) {
                    if (http_app_get_code() == HTTP_REQ_CODE_DEV_SET_OFFLINE) {
                        relay_set_status(0);

                        ESP_LOGW(TAG, "relay is off");

                        gui_set_mode(GUI_MODE_IDX_QRCODE);
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                        audio_player_play_file(MP3_FILE_IDX_NOTIFY);
#endif
                    } else {
                        gui_set_mode(GUI_MODE_IDX_TIMER);
                    }
                } else {
                    gui_set_mode(GUI_MODE_IDX_QRCODE);
                }
#ifdef CONFIG_ENABLE_LED
                led_set_mode(LED_MODE_IDX_BLINK_M0);
#endif
            }
        }
        esp_http_client_cleanup(client);

        vTaskDelay(1000 / portTICK_RATE_MS);

        if (uxBits & HTTP_APP_STATUS_RUN_BIT) {
            xEventGroupSetBits(user_event_group, HTTP_APP_STATUS_DONE_BIT);
            xEventGroupClearBits(user_event_group, HTTP_APP_STATUS_RUN_BIT);
        }

        key_set_scan_mode(KEY_SCAN_MODE_IDX_ON);
    }
}

void http_app_init(void)
{
    xTaskCreatePinnedToCore(http_app_task, "httpAppT", 5120, NULL, 7, NULL, 0);
}
