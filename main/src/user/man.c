/*
 * man.c
 *
 *  Created on: 2020-07-15 18:14
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <time.h>
#include <string.h>

#include "esp_log.h"

#include "core/os.h"
#include "user/man.h"
#include "user/gui.h"
#include "user/audio_player.h"
#include "user/http_app_status.h"

#define TAG "man"

static int32_t remain = 0;
static man_info_t info = {0};
static struct tm timeinfo = {0};

static man_sync_mode_t man_sync_mode = MAN_SYNC_MODE_IDX_OFF;

static void man_task(void *pvParameter)
{
    portTickType xLastWakeTime;
    const int update_sec = esp_random() % 30 + 30;

    ESP_LOGI(TAG, "started.");

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            MAN_SYNC_RUN_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        xLastWakeTime = xTaskGetTickCount();

        switch (gui_get_mode()) {
            case GUI_MODE_IDX_QRCODE:
                xEventGroupSetBits(user_event_group, GUI_RLD_MODE_BIT);
                /* fall through */
            case GUI_MODE_IDX_GIF_FAIL:
                man_update_info();

                if (timeinfo.tm_sec == update_sec) {
                    http_app_update_status(HTTP_REQ_CODE_DEV_GET_INFO);
                }

                break;
            case GUI_MODE_IDX_TIMER:
                xEventGroupClearBits(user_event_group, GUI_RLD_SYNC_BIT);
                xEventGroupSetBits(user_event_group, GUI_RLD_MODE_BIT);

                EventBits_t uxBits = xEventGroupWaitBits(
                    user_event_group,
                    GUI_RLD_SYNC_BIT,
                    pdFALSE,
                    pdFALSE,
                    1000 / portTICK_RATE_MS
                );

                if (!(uxBits & GUI_RLD_SYNC_BIT)) {
                    continue;
                }

#ifdef CONFIG_ENABLE_AUDIO_PROMPT
                if (remain / 60 <= 4 && remain % 60 == 10) {
                    audio_player_play_file(MP3_FILE_IDX_NOTIFY);
                }
#endif
                if (remain == 0) {
                    http_app_update_status(HTTP_REQ_CODE_DEV_SET_OFFLINE);
                } else if (remain % 60 == update_sec) {
                    http_app_update_status(HTTP_REQ_CODE_DEV_GET_INFO);
                }

                break;
            default:
                break;
        }

        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);
    }
}

void man_set_qrcode(const char *qrcode)
{
    strncpy(info.qrcode, qrcode, sizeof(info.qrcode) - 1);

    ESP_LOGI(TAG, "qrcode: %s", info.qrcode);
}

char *man_get_qrcode(void)
{
    return info.qrcode;
}

void man_set_user_info(const char *user_info)
{
    strncpy(info.user_info, user_info, sizeof(info.user_info) - 1);

    ESP_LOGI(TAG, "user info: %s", info.user_info);
}

void man_set_expire_time(int hour, int min, int sec)
{
    info.expire_hour = hour;
    info.expire_min  = min;
    info.expire_sec  = sec;

    ESP_LOGI(TAG, "expire time: %02d:%02d:%02d", info.expire_hour, info.expire_min, info.expire_sec);
}

man_info_t *man_update_info(void)
{
    time_t now = 0;

    time(&now);
    localtime_r(&now, &timeinfo);

    info.current_hour = timeinfo.tm_hour;
    info.current_min  = timeinfo.tm_min;
    info.current_sec  = timeinfo.tm_sec;

    remain = info.expire_hour * 3600 + info.expire_min * 60 + info.expire_sec -
             info.current_hour * 3600 - info.current_min * 60 - info.current_sec;
    if (remain <= 0) {
        remain = 0;
    }

    info.remain_hour = remain / 3600;
    info.remain_min  = remain / 60 % 60;
    info.remain_sec  = remain % 60;

    return &info;
}

void man_set_sync_mode(man_sync_mode_t idx)
{
    man_sync_mode = idx;

    if (man_sync_mode == MAN_SYNC_MODE_IDX_ON) {
        xEventGroupSetBits(user_event_group, MAN_SYNC_RUN_BIT);
    } else {
        xEventGroupClearBits(user_event_group, MAN_SYNC_RUN_BIT);
    }
}

man_sync_mode_t man_get_sync_mode(void)
{
    return man_sync_mode;
}

void man_init(void)
{
    xTaskCreatePinnedToCore(man_task, "manT", 2048, NULL, 5, NULL, 0);
}
