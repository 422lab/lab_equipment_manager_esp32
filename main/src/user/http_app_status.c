/*
 * http_app_status.c
 *
 *  Created on: 2018-04-06 15:09
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"

#include "cJSON.h"

#include "core/os.h"
#include "chip/wifi.h"
#include "board/relay.h"

#include "user/man.h"
#include "user/gui.h"
#include "user/led.h"
#include "user/audio_player.h"
#include "user/http_app_status.h"

#define TAG "http_app_status"

static bool response = false;
static req_code_t req_code = HTTP_REQ_IDX_UPD;

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
                cJSON *status = cJSON_GetObjectItemCaseSensitive(root, "status");

                if (cJSON_IsNumber(code)) {
                    ESP_LOGW(TAG, "code: %d, status: %d", (int)code->valuedouble, cJSON_IsTrue(status));

                    switch ((int)code->valuedouble) {
                        case HTTP_REQ_IDX_UPD:
                            if (relay_get_status()) {
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

                                        man_set_exp_time(hour, minute, second);
                                    }

                                    gui_set_mode(GUI_MODE_IDX_TIMER);
                                } else {
                                    relay_set_status(0);

                                    ESP_LOGW(TAG, "relay is off");

                                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                                    audio_player_play_file(0);
                                }
                            } else {
                                if (cJSON_IsTrue(status)) {
                                    cJSON *token = cJSON_GetObjectItemCaseSensitive(root, "token");
                                    char *c_token = cJSON_GetStringValue(token);

                                    if (c_token) {
                                        man_set_token(c_token);
                                    }
                                }

                                if (*man_get_token() == 0x00) {
                                    gui_set_mode(6);
                                } else {
                                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                                }
                            }

                            led_set_mode(1);

                            break;
                        case HTTP_REQ_IDX_OFF:
                            relay_set_status(0);

                            ESP_LOGW(TAG, "relay is off");

                            gui_set_mode(GUI_MODE_IDX_QR_CODE);
                            audio_player_play_file(0);

                            led_set_mode(1);

                            break;
                        case HTTP_REQ_IDX_ON:
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

                                    man_set_exp_time(hour, minute, second);
                                }

                                relay_set_status(1);

                                ESP_LOGW(TAG, "relay is on");

                                gui_set_mode(GUI_MODE_IDX_TIMER);
                                audio_player_play_file(1);
                            } else {
                                gui_set_mode(GUI_MODE_IDX_QR_CODE);
                                audio_player_play_file(2);
                            }

                            led_set_mode(1);

                            break;
                        default:
                            ESP_LOGE(TAG, "invalid code");
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

        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & HTTP_APP_STATUS_FAIL_BIT) {
            if (relay_get_status()) {
                if (http_app_get_code() == HTTP_REQ_IDX_OFF) {
                    relay_set_status(0);

                    ESP_LOGW(TAG, "relay is off");

                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                    audio_player_play_file(0);
                } else {
                    gui_set_mode(GUI_MODE_IDX_TIMER);
                }
            } else {
                if (*man_get_token() == 0x00) {
                    gui_set_mode(6);
                } else {
                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                }
            }

            led_set_mode(2);
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
    cJSON_AddNumberToObject(root, "code", req_code);
    cJSON_AddStringToObject(root, "status", relay_get_status() ? "on" : "off");
    cJSON_AddStringToObject(root, "wifi_mac", wifi_mac_string);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

req_code_t http_app_get_code(void)
{
    return req_code;
}

void http_app_update_status(req_code_t code)
{
    EventBits_t uxBits = xEventGroupGetBits(user_event_group);
    if (uxBits & HTTP_APP_STATUS_RUN_BIT) {
        ESP_LOGW(TAG, "app is running");
        return;
    }

    uxBits = xEventGroupGetBits(wifi_event_group);
    if (!(uxBits & WIFI_RDY_BIT)) {
        ESP_LOGW(TAG, "network is down");

        if (relay_get_status()) {
            if (code == HTTP_REQ_IDX_OFF) {
                relay_set_status(0);

                ESP_LOGW(TAG, "relay is off");

                gui_set_mode(GUI_MODE_IDX_QR_CODE);
                audio_player_play_file(0);
            }
        } else {
            if (code == HTTP_REQ_IDX_ON) {
                audio_player_play_file(5);
            }
        }

        vTaskDelay(1000 / portTICK_RATE_MS);

        xEventGroupSetBits(user_event_group, KEY_RUN_BIT);

        return;
    }

    req_code = code;
    response = false;

    uxBits = xEventGroupSync(
        user_event_group,
        HTTP_APP_STATUS_RUN_BIT,
        HTTP_APP_STATUS_RDY_BIT,
        30000 / portTICK_RATE_MS
    );
    if (!(uxBits & HTTP_APP_STATUS_RDY_BIT)) {
        xEventGroupClearBits(user_event_group, HTTP_APP_STATUS_RUN_BIT);
    }
}
