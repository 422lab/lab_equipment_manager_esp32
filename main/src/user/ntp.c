/*
 * ntp.c
 *
 *  Created on: 2018-02-16 17:50
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <time.h>

#include "esp_log.h"
#include "esp_sntp.h"

#include "core/os.h"
#include "user/gui.h"
#include "user/led.h"

#define TAG "ntp"

static void ntp_time_sync_notification_cb(struct timeval *tv)
{
    time_t now = 0;
    struct tm timeinfo = {0};
    char strftime_buf[64] = {0};

    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    ESP_LOGW(TAG, "current timezone: %s", CONFIG_NTP_TIMEZONE);
    ESP_LOGW(TAG, "current date/time: %s", strftime_buf);

    xEventGroupSetBits(user_event_group, NTP_RDY_BIT);
}

static void ntp_task(void *pvParameter)
{
    portTickType xLastWakeTime;
    const int retry_count = 30;

    xEventGroupWaitBits(
        user_event_group,
        NTP_RUN_BIT,
        pdFALSE,
        pdFALSE,
        portMAX_DELAY
    );

    xEventGroupClearBits(user_event_group, KEY_RUN_BIT);

    led_set_mode(2);
    gui_set_mode(5);

    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, CONFIG_NTP_SERVER_URL);
    sntp_set_time_sync_notification_cb(ntp_time_sync_notification_cb);

    sntp_init();

    setenv("TZ", CONFIG_NTP_TIMEZONE, 1);
    tzset();

    ESP_LOGI(TAG, "started.");

    int retry = 1;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {
        xLastWakeTime = xTaskGetTickCount();

        ESP_LOGW(TAG, "waiting for system time to be set... (%d/%d)", retry, retry_count);

        if (++retry > retry_count) {
            ESP_LOGE(TAG, "time sync timeout");

            gui_set_mode(4);
            vTaskDelay(2000 / portTICK_RATE_MS);

            esp_restart();
        }

        vTaskDelayUntil(&xLastWakeTime, 1000 / portTICK_RATE_MS);
    }

    xEventGroupSetBits(user_event_group, MAN_RUN_BIT);

    vTaskDelete(NULL);
}

void ntp_sync_time(void)
{
    EventBits_t uxBits = xEventGroupGetBits(user_event_group);
    if (!(uxBits & NTP_RDY_BIT)) {
        xEventGroupSetBits(user_event_group, NTP_RUN_BIT);
    }
}

void ntp_init(void)
{
    xTaskCreatePinnedToCore(ntp_task, "ntpT", 2048, NULL, 5, NULL, 0);
}
