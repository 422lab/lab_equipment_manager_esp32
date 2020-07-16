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
#include "user/led.h"
#include "user/audio_player.h"
#include "user/http_app_ota.h"
#include "user/http_app_status.h"

#define TAG "man"

static uint32_t t_rem = 0;
static man_info_t info = {0};
static struct tm timeinfo = {0};

static void man_task(void *pvParameter)
{
    portTickType xLastWakeTime;
    const int update_sec = esp_random() % 30 + 30;

    xEventGroupWaitBits(
        user_event_group,
        MAN_RUN_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    ESP_LOGI(TAG, "started.");

#ifdef CONFIG_ENABLE_OTA
    http_app_check_for_updates();
#endif

    http_app_update_status(HTTP_REQ_IDX_UPD);

    while (1) {
        xLastWakeTime = xTaskGetTickCount();

        switch (gui_get_mode()) {
            case GUI_MODE_IDX_TIMER:
                xEventGroupSync(
                    user_event_group,
                    GUI_RLD_BIT,
                    GUI_DONE_BIT,
                    portMAX_DELAY
                );

                if (t_rem / 60 <= 4 && t_rem % 60 == 10) {
                    audio_player_play_file(0);
                }

                if (t_rem == 0) {
                    http_app_update_status(HTTP_REQ_IDX_OFF);
                } else if (t_rem % 60 == update_sec) {
                    http_app_update_status(HTTP_REQ_IDX_UPD);
                }

                break;
            case GUI_MODE_IDX_QR_CODE:
                xEventGroupSync(
                    user_event_group,
                    GUI_RLD_BIT,
                    GUI_DONE_BIT,
                    portMAX_DELAY
                );

                if (timeinfo.tm_sec == update_sec) {
                    http_app_update_status(HTTP_REQ_IDX_UPD);
                }

                break;
            default:
                break;
        }

        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);
    }
}

void man_set_token(const char *s_token)
{
    strncpy(info.s_token, s_token, sizeof(info.s_token)-1);

    ESP_LOGI(TAG, "token: %s", info.s_token);
}

void man_set_user_info(const char *u_info)
{
    strncpy(info.u_info, u_info, sizeof(info.u_info)-1);

    ESP_LOGI(TAG, "user info: %s", info.u_info);
}

void man_set_exp_time(int hour, int min, int sec)
{
    info.e_hour = hour;
    info.e_min  = min;
    info.e_sec  = sec;

    ESP_LOGI(TAG, "expire time: %02d:%02d:%02d", info.e_hour, info.e_min, info.e_sec);
}

char *man_get_token(void)
{
    return info.s_token;
}

man_info_t *man_get_info(void)
{
    time_t now = 0;

    time(&now);
    localtime_r(&now, &timeinfo);

    info.n_hour = timeinfo.tm_hour;
    info.n_min  = timeinfo.tm_min;
    info.n_sec  = timeinfo.tm_sec;

    t_rem = info.e_hour * 3600 + info.e_min * 60 + info.e_sec -
            info.n_hour * 3600 - info.n_min * 60 - info.n_sec;
    if (t_rem <= 0) {
        t_rem = 0;
    }

    info.r_hour = t_rem / 3600;
    info.r_min  = t_rem / 60 % 60;
    info.r_sec  = t_rem % 60;

    return &info;
}

void man_init(void)
{
    xTaskCreatePinnedToCore(man_task, "manT", 2048, NULL, 5, NULL, 0);
}
