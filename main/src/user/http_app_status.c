/*
 * http_app_status.c
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "core/os.h"
#include "chip/wifi.h"
#include "board/relay.h"

#include "user/key.h"
#include "user/led.h"
#include "user/gui.h"
#include "user/man.h"
#include "user/audio_player.h"
#include "user/http_app_status.h"

#define TAG "http_app_status"

static bool response = false;
static req_code_t request = HTTP_REQ_CODE_DEV_UPD;

esp_err_t http_app_status_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        break;
    case HTTP_EVENT_ON_CONNECTED:
        break;
    case HTTP_EVENT_HEADER_SENT:
        break;
    case HTTP_EVENT_ON_HEADER:
        break;
    case HTTP_EVENT_ON_DATA: {
        cJSON *root = NULL;
        if (evt->data_len) {
            response = true;

            root = cJSON_Parse(evt->data);
            if (cJSON_HasObjectItem(root, "code")) {
                cJSON *code = cJSON_GetObjectItemCaseSensitive(root, "code");

                if (cJSON_IsNumber(code)) {
                    switch ((int)code->valuedouble) {
                        case HTTP_REQ_CODE_DEV_UPD: {
                            cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");
                            ESP_LOGW(TAG, "code: %d, status: %d", (int)code->valuedouble, cJSON_IsTrue(status));

                            if (relay_get_status() == RELAY_STATUS_IDX_ON) {
                                if (cJSON_IsTrue(status)) {
                                    cJSON *user_info = cJSON_GetObjectItemCaseSensitive(root, "user_info");
                                    char *c_user_info = cJSON_GetStringValue(user_info);

                                    if (c_user_info) {
                                        man_set_user_info(c_user_info);
                                    }

                                    cJSON *expire_time = cJSON_GetObjectItemCaseSensitive(root, "expire_time");
                                    char *c_expire_time = cJSON_GetStringValue(expire_time);

                                    if (c_expire_time) {
                                        int hour, minute, second;
                                        sscanf(c_expire_time, "%d:%d:%d", &hour, &minute, &second);

                                        man_set_expire_time(hour, minute, second);
                                    }

                                    gui_set_mode(GUI_MODE_IDX_TIMER);
                                } else {
                                    relay_set_status(RELAY_STATUS_IDX_OFF);
                                    ESP_LOGW(TAG, "relay is off");

                                    gui_set_mode(GUI_MODE_IDX_QRCODE);
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                                    audio_player_play_file(MP3_FILE_IDX_NOTIFY);
#endif
                                }
                            } else {
                                if (cJSON_IsTrue(status)) {
                                    cJSON *qrcode = cJSON_GetObjectItemCaseSensitive(root, "qrcode");
                                    char *c_qrcode = cJSON_GetStringValue(qrcode);

                                    if (c_qrcode) {
                                        man_set_qrcode(c_qrcode);
                                    }
                                }

                                gui_set_mode(GUI_MODE_IDX_QRCODE);
                            }
#ifdef CONFIG_ENABLE_LED
                            led_set_mode(LED_MODE_IDX_BLINK_S0);
#endif
                            break;
                        }
                        case HTTP_REQ_CODE_DEV_OFF: {
                            ESP_LOGW(TAG, "code: %d, result: 1", (int)code->valuedouble);

                            relay_set_status(RELAY_STATUS_IDX_OFF);
                            ESP_LOGW(TAG, "relay is off");

                            gui_set_mode(GUI_MODE_IDX_QRCODE);
#ifdef CONFIG_ENABLE_LED
                            led_set_mode(LED_MODE_IDX_BLINK_S0);
#endif
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                            audio_player_play_file(MP3_FILE_IDX_NOTIFY);
#endif
                            break;
                        }
                        case HTTP_REQ_CODE_DEV_ON: {
                            cJSON *result = cJSON_GetObjectItemCaseSensitive(root, "result");
                            ESP_LOGW(TAG, "code: %d, result: %d", (int)code->valuedouble, cJSON_IsTrue(result));

                            if (cJSON_IsTrue(result)) {
                                cJSON *user_info = cJSON_GetObjectItemCaseSensitive(root, "user_info");
                                char *c_user_info = cJSON_GetStringValue(user_info);

                                if (c_user_info) {
                                    man_set_user_info(c_user_info);
                                }

                                cJSON *expire_time = cJSON_GetObjectItemCaseSensitive(root, "expire_time");
                                char *c_expire_time = cJSON_GetStringValue(expire_time);

                                if (c_expire_time) {
                                    int hour, minute, second;
                                    sscanf(c_expire_time, "%d:%d:%d", &hour, &minute, &second);

                                    man_set_expire_time(hour, minute, second);
                                }

                                relay_set_status(RELAY_STATUS_IDX_ON);
                                ESP_LOGW(TAG, "relay is on");

                                gui_set_mode(GUI_MODE_IDX_TIMER);
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                                audio_player_play_file(MP3_FILE_IDX_AUTH_DONE);
#endif
                            } else {
                                gui_set_mode(GUI_MODE_IDX_QRCODE);
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                                audio_player_play_file(MP3_FILE_IDX_AUTH_FAIL);
#endif
                            }
#ifdef CONFIG_ENABLE_LED
                            led_set_mode(LED_MODE_IDX_BLINK_S0);
#endif
                            break;
                        }
                        default:
                            ESP_LOGE(TAG, "invalid code: %d", (int)code->valuedouble);
                            xEventGroupSetBits(user_event_group, HTTP_APP_STATUS_FAIL_BIT);
                            break;
                    }
                } else {
                    ESP_LOGE(TAG, "invalid code format");
                    xEventGroupSetBits(user_event_group, HTTP_APP_STATUS_FAIL_BIT);
                }
            } else {
                ESP_LOGE(TAG, "invalid response");
                xEventGroupSetBits(user_event_group, HTTP_APP_STATUS_FAIL_BIT);
            }
            cJSON_Delete(root);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH: {
        if (!response) {
            ESP_LOGE(TAG, "null response");
            xEventGroupSetBits(user_event_group, HTTP_APP_STATUS_FAIL_BIT);
        }

        if (xEventGroupGetBits(user_event_group) & HTTP_APP_STATUS_FAIL_BIT) {
            if (relay_get_status() == RELAY_STATUS_IDX_ON) {
                if (http_app_get_code() == HTTP_REQ_CODE_DEV_OFF) {
                    relay_set_status(RELAY_STATUS_IDX_OFF);
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
            led_set_mode(LED_MODE_IDX_BLINK_M1);
#endif
        }
        break;
    }
    case HTTP_EVENT_DISCONNECTED:
        break;
    default:
        break;
    }
    return ESP_OK;
}

void http_app_status_prepare_data(char *buf, int len)
{
    cJSON *root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "request", request);
    cJSON_AddStringToObject(root, "device_mac", wifi_mac_string);
    cJSON_AddStringToObject(root, "relay_status", (relay_get_status() == RELAY_STATUS_IDX_ON) ? "on" : "off");
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

req_code_t http_app_get_code(void)
{
    return request;
}

void http_app_update_status(req_code_t code)
{
    if (xEventGroupGetBits(user_event_group) & HTTP_APP_STATUS_RUN_BIT) {
        ESP_LOGW(TAG, "app is running");
        return;
    }

    if (!(xEventGroupGetBits(wifi_event_group) & WIFI_RDY_BIT)) {
        ESP_LOGW(TAG, "network is down");

        if (relay_get_status() == RELAY_STATUS_IDX_ON) {
            if (code == HTTP_REQ_CODE_DEV_OFF) {
                relay_set_status(RELAY_STATUS_IDX_OFF);
                ESP_LOGW(TAG, "relay is off");

                gui_set_mode(GUI_MODE_IDX_QRCODE);
#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                audio_player_play_file(MP3_FILE_IDX_NOTIFY);
            }
        } else {
            if (code == HTTP_REQ_CODE_DEV_ON) {
                audio_player_play_file(MP3_FILE_IDX_ERROR_REQ);
#endif
            }
        }

        vTaskDelay(1000 / portTICK_RATE_MS);

        key_set_scan_mode(KEY_SCAN_MODE_IDX_ON);

        return;
    }

    request = code;
    response = false;

    EventBits_t uxBits = xEventGroupSync(
        user_event_group,
        HTTP_APP_STATUS_RUN_BIT,
        HTTP_APP_STATUS_DONE_BIT,
        30000 / portTICK_RATE_MS
    );

    if (!(uxBits & HTTP_APP_STATUS_DONE_BIT)) {
        xEventGroupClearBits(user_event_group, HTTP_APP_STATUS_RUN_BIT);
    }
}
