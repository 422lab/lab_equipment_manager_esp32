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

#include "user/gui.h"
#include "user/led.h"
#include "user/audio_player.h"
#include "user/http_app_status.h"

#define TAG "http_app_status"

static char token_string[33] = {0};
static http_req_t req_code = HTTP_REQ_IDX_UPD;

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
        cJSON *root    = NULL;
        cJSON *request = NULL;
        cJSON *status  = NULL;
        http_req_t req = 0;
        if (evt->data_len) {
            root = cJSON_Parse(evt->data);
            if (cJSON_HasObjectItem(root, "request")) {
                request = cJSON_GetObjectItemCaseSensitive(root, "request");
                status  = cJSON_GetObjectItemCaseSensitive(root, "status");

                if (cJSON_IsNumber(request)) {
                    req = request->valuedouble;

                    ESP_LOGW(TAG, "req: %d, sta: %d", req, cJSON_IsTrue(status));

                    switch (req) {
                        case HTTP_REQ_IDX_UPD:
                            if (cJSON_IsTrue(status)) {
                                cJSON *token = cJSON_GetObjectItemCaseSensitive(root, "token");
                                char *c_token = cJSON_GetStringValue(token);

                                memset(token_string, 0x00, sizeof(token_string));
                                if (c_token) {
                                    ESP_LOGW(TAG, "token: %s", c_token);
                                    memcpy(token_string, c_token, strlen(c_token));
                                }

                                cJSON *user_info = cJSON_GetObjectItemCaseSensitive(root, "user_info");
                                char *c_user_info = cJSON_GetStringValue(user_info);

                                if (c_user_info) {
                                    gui_set_user_info(c_user_info);
                                }

                                cJSON *timer_time = cJSON_GetObjectItemCaseSensitive(root, "timer_time");
                                char *c_timer_time = cJSON_GetStringValue(timer_time);

                                if (c_timer_time) {
                                    int hour, minute, second;
                                    sscanf(c_timer_time, "%d:%d:%d", &hour, &minute, &second);

                                    gui_set_timer_time(hour, minute, second);
                                }

                                if (relay_get_status()) {
                                    led_set_mode(1);
                                    gui_set_mode(GUI_MODE_IDX_TIMER);
                                }
                            } else {
                                if (relay_get_status()) {
                                    relay_set_status(0);

                                    ESP_LOGW(TAG, "relay is off");

                                    led_set_mode(1);
                                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                                    audio_player_play_file(0);
                                }
                            }

                            led_set_mode(1);

                            if (!relay_get_status()) {
                                if (!strncmp(http_app_get_token(), "CCCC", 4)) {
                                    gui_set_mode(6);
                                } else {
                                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                                }
                            }
                            break;
                        case HTTP_REQ_IDX_OFF:
                            relay_set_status(0);

                            ESP_LOGW(TAG, "relay is off");

                            led_set_mode(1);
                            gui_set_mode(GUI_MODE_IDX_QR_CODE);
                            audio_player_play_file(0);
                            break;
                        case HTTP_REQ_IDX_ON:
                            if (cJSON_IsTrue(status)) {
                                cJSON *user_info = cJSON_GetObjectItemCaseSensitive(root, "user_info");
                                char *c_user_info = cJSON_GetStringValue(user_info);

                                if (c_user_info) {
                                    gui_set_user_info(c_user_info);
                                }

                                cJSON *timer_time = cJSON_GetObjectItemCaseSensitive(root, "timer_time");
                                char *c_timer_time = cJSON_GetStringValue(timer_time);

                                if (c_timer_time) {
                                    int hour, minute, second;
                                    sscanf(c_timer_time, "%d:%d:%d", &hour, &minute, &second);

                                    gui_set_timer_time(hour, minute, second);
                                }

                                relay_set_status(1);

                                ESP_LOGW(TAG, "relay is on");

                                led_set_mode(1);
                                gui_set_mode(GUI_MODE_IDX_TIMER);
                                audio_player_play_file(1);
                            } else {
                                led_set_mode(1);
                                gui_set_mode(GUI_MODE_IDX_QR_CODE);
                                audio_player_play_file(2);
                            }
                            break;
                        default:
                            break;
                    }
                }
            } else {
                ESP_LOGE(TAG, "invalid response");
                xEventGroupSetBits(user_event_group, HTTP_APP_STATUS_FAILED_BIT);
            }
            cJSON_Delete(root);
        }
        break;
    }
    case HTTP_EVENT_ON_FINISH: {
        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & HTTP_APP_STATUS_FAILED_BIT) {
            if (!relay_get_status()) {
                if (!strncmp(http_app_get_token(), "CCCC", 4)) {
                    gui_set_mode(6);
                } else {
                    gui_set_mode(GUI_MODE_IDX_QR_CODE);
                }
            }
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
    cJSON_AddNumberToObject(root, "request", req_code);
    cJSON_AddStringToObject(root, "status", relay_get_status() ? "on" : "off");
    cJSON_AddStringToObject(root, "mac", wifi_mac_string);
    cJSON_PrintPreallocated(root, buf, len, 0);
    cJSON_Delete(root);
}

char *http_app_get_token(void)
{
    return token_string;
}

void http_app_update_status(http_req_t req)
{
    req_code = req;

    EventBits_t uxBits = xEventGroupSync(
        user_event_group,
        HTTP_APP_STATUS_RUN_BIT,
        HTTP_APP_STATUS_READY_BIT,
        30000 / portTICK_RATE_MS
    );
    if ((uxBits & HTTP_APP_STATUS_READY_BIT) == 0) {
        xEventGroupClearBits(user_event_group, HTTP_APP_STATUS_RUN_BIT);
    }

    if (strlen(token_string) == 0) {
        memset(token_string, 0x43, sizeof(token_string)-1);
    }
}
