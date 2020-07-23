/*
 * wifi.c
 *
 *  Created on: 2018-02-11 06:56
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <string.h>

#include "esp_log.h"
#include "esp_wifi.h"

#define TAG "wifi"

char wifi_hostname[40] = {0};
char wifi_mac_string[18] = {0};
char wifi_mac_address[6] = {0};

wifi_config_t wifi_config = {
    .sta = {
        .scan_method = WIFI_ALL_CHANNEL_SCAN,
        .sort_method = WIFI_CONNECT_AP_BY_SIGNAL,
        .threshold.authmode = WIFI_AUTH_WPA2_PSK,
    },
};

void wifi_init(void)
{
    esp_netif_t *wifi_sta = esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_ps(WIFI_PS_MIN_MODEM));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    memcpy(wifi_config.sta.ssid, (char *)CONFIG_WIFI_SSID, strlen(CONFIG_WIFI_SSID));
    wifi_config.sta.ssid[strlen(CONFIG_WIFI_SSID)] = '\0';
    memcpy(wifi_config.sta.password, (char *)CONFIG_WIFI_PASSWORD, strlen(CONFIG_WIFI_PASSWORD));
    wifi_config.sta.password[strlen(CONFIG_WIFI_PASSWORD)] = '\0';
    ESP_LOGI(TAG, "use default wifi configuration, ssid: %s", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_ERROR_CHECK(esp_wifi_get_mac(ESP_IF_WIFI_STA, (uint8_t *)wifi_mac_address));
    snprintf(wifi_mac_string, sizeof(wifi_mac_string), MACSTR, MAC2STR(wifi_mac_address));

    snprintf(wifi_hostname, sizeof(wifi_hostname), "%s_%X%X%X", CONFIG_WIFI_HOSTNAME_PREFIX,
             wifi_mac_address[3], wifi_mac_address[4], wifi_mac_address[5]);
    ESP_ERROR_CHECK(esp_netif_set_hostname(wifi_sta, wifi_hostname));

    ESP_LOGI(TAG, "initialized.");
}
