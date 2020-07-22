/*
 * os.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include "core/os.h"
#include "chip/wifi.h"
#include "user/ntp.h"

EventGroupHandle_t wifi_event_group;
EventGroupHandle_t user_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
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

static void ip_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            xEventGroupSetBits(wifi_event_group, WIFI_RDY_BIT);
            ntp_sync_time();
            break;
        case IP_EVENT_STA_LOST_IP:
            break;
        default:
            break;
    }
}

void os_init(void)
{
    wifi_event_group = xEventGroupCreate();
    user_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &ip_event_handler, NULL));
}
