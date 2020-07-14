/*
 * os.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_system.h"
#include "esp_smartconfig.h"

#include "freertos/event_groups.h"

#include "core/os.h"
#include "chip/wifi.h"

#include "user/gui.h"
#include "user/led.h"
#include "user/ntp.h"
#include "user/http_app_ota.h"
#include "user/http_app_status.h"

#define OS_SC_TAG "os_sc"

EventGroupHandle_t wifi_event_group;
EventGroupHandle_t user_event_group;

static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    switch (event_id) {
        case WIFI_EVENT_STA_START:
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case WIFI_EVENT_STA_CONNECTED:
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            xEventGroupClearBits(wifi_event_group, WIFI_READY_BIT);
            ESP_ERROR_CHECK(esp_wifi_connect());
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
            xEventGroupSetBits(wifi_event_group, WIFI_READY_BIT);
            ntp_sync_time();
            break;
        case IP_EVENT_STA_LOST_IP:
            break;
        default:
            break;
    }
}

static void sc_event_handler(void* arg, esp_event_base_t event_base,
                             int32_t event_id, void* event_data)
{
    switch (event_id) {
        case SC_EVENT_SCAN_DONE:
            ESP_LOGI(OS_SC_TAG, "scan done");
            break;
        case SC_EVENT_FOUND_CHANNEL:
            ESP_LOGI(OS_SC_TAG, "found channel");
            break;
        case SC_EVENT_GOT_SSID_PSWD:
            led_set_mode(7);
            ESP_LOGI(OS_SC_TAG, "got ssid and passwd");

            smartconfig_event_got_ssid_pswd_t *evt = (smartconfig_event_got_ssid_pswd_t *)event_data;
            memcpy(wifi_config.sta.ssid, evt->ssid, sizeof(wifi_config.sta.ssid));
            memcpy(wifi_config.sta.password, evt->password, sizeof(wifi_config.sta.password));
            wifi_config.sta.bssid_set = evt->bssid_set;
            if (wifi_config.sta.bssid_set == true) {
                memcpy(wifi_config.sta.bssid, evt->bssid, sizeof(wifi_config.sta.bssid));
            }
            ESP_LOGI(OS_SC_TAG, "ssid: %s", wifi_config.sta.ssid);
            ESP_LOGI(OS_SC_TAG, "pswd: %s", wifi_config.sta.password);

            ESP_ERROR_CHECK(esp_wifi_disconnect());
            ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
            ESP_ERROR_CHECK(esp_wifi_connect());
            break;
        case SC_EVENT_SEND_ACK_DONE:
            ESP_LOGI(OS_SC_TAG, "ack done");
            esp_smartconfig_stop();
            xEventGroupClearBits(wifi_event_group, WIFI_CONFIG_BIT);
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
    ESP_ERROR_CHECK(esp_event_handler_register(SC_EVENT, ESP_EVENT_ANY_ID, &sc_event_handler, NULL));
}
