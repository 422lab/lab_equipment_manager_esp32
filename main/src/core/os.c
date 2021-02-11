/*
 * os.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "core/os.h"
#include "chip/wifi.h"

#include "user/ntp.h"
#include "user/key.h"
#include "user/led.h"
#include "user/gui.h"
#include "user/man.h"
#include "user/http_app_status.h"

#define OS_PWR_TAG "os_pwr"

EventGroupHandle_t wifi_event_group;
EventGroupHandle_t user_event_group;

static EventBits_t reset_wait_bits = OS_PWR_DUMMY_BIT;
static EventBits_t sleep_wait_bits = OS_PWR_DUMMY_BIT;

static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_CONNECTED:
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            xEventGroupClearBits(wifi_event_group, WIFI_RDY_BIT);
            esp_wifi_connect();
            break;
        default:
            break;
    }
}

static void ip_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, WIFI_RDY_BIT);

            key_set_scan_mode(KEY_SCAN_MODE_IDX_OFF);

            ntp_sync_time();
            http_app_update_status(HTTP_REQ_CODE_DEV_GET_INFO);
            man_set_sync_mode(MAN_SYNC_MODE_IDX_ON);

            break;
        default:
            break;
    }
}

static void os_pwr_task_handle(void *pvParameters)
{
    ESP_LOGI(OS_PWR_TAG, "started.");

    while (1) {
        xEventGroupWaitBits(
            user_event_group,
            OS_PWR_RESET_BIT | OS_PWR_SLEEP_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY
        );

        EventBits_t uxBits = xEventGroupGetBits(user_event_group);
        if (uxBits & OS_PWR_RESET_BIT) {
            if (reset_wait_bits) {
                ESP_LOGW(OS_PWR_TAG, "waiting for unfinished jobs....");

                vTaskDelay(500 / portTICK_RATE_MS);

                xEventGroupWaitBits(
                    user_event_group,
                    reset_wait_bits,
                    pdFALSE,
                    pdTRUE,
                    portMAX_DELAY
                );
            }

            ESP_LOGW(OS_PWR_TAG, "reset now");
            esp_restart();
        } else if (uxBits & OS_PWR_SLEEP_BIT) {
            if (sleep_wait_bits) {
                ESP_LOGW(OS_PWR_TAG, "waiting for unfinished jobs....");

                vTaskDelay(500 / portTICK_RATE_MS);

                xEventGroupWaitBits(
                    user_event_group,
                    sleep_wait_bits,
                    pdFALSE,
                    pdTRUE,
                    portMAX_DELAY
                );
            }

            ESP_LOGW(OS_PWR_TAG, "sleep now");
            esp_deep_sleep_start();
        }
    }
}

void os_pwr_reset_wait(EventBits_t bits)
{
    reset_wait_bits = bits;
    xEventGroupSetBits(user_event_group, OS_PWR_RESET_BIT);
}

void os_pwr_sleep_wait(EventBits_t bits)
{
    sleep_wait_bits = bits;
    xEventGroupSetBits(user_event_group, OS_PWR_SLEEP_BIT);
}

void os_init(void)
{
    wifi_event_group = xEventGroupCreate();
    user_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));

    xTaskCreatePinnedToCore(os_pwr_task_handle, "osPwrT", 2048, NULL, 5, NULL, 0);
}
