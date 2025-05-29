#include <stdio.h>
#include "string.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_now.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define ESP_CHANNEL 1
#define LED_STRIP 25
#define LED_STRIP_MAX_LEDS 1

static uint8_t peer_mac [ESP_NOW_ETH_ALEN] = {0x24, 0x0A, 0xC4, 0x00, 0x5E, 0x3C};

const char *TAG = "esp_now_init";

static esp_err_t init_wifi(void)
{
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();

    esp_netif_init();
    esp_event_loop_create_default();
    esp_wifi_init(&wifi_init_config);
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_storage(WIFI_STORAGE_FLASH);
    esp_wifi_start();
    ESP_LOGI(TAG, "WiFi init completed");
    return ESP_OK;
}

void recv_cb(const uint8_t *mac_addr, const uint8_t *data, int data_len)
{
    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "MAC address is NULL");
        return;
    }
    if (data == NULL || data_len <= 0) {
        ESP_LOGE(TAG, "Invalid data received");
        return;
    }
    ESP_LOGI(TAG, "Data received from MAC: " MACSTR ", Data: %.*s", MAC2STR(mac_addr), data_len, (char *)data);
}

void send_cb(const uint8_t *mac_addr, esp_now_send_status_t status)
{
    if (mac_addr == NULL) {
        ESP_LOGE(TAG, "Invalid arguments in send_cb");
        return;
    }
    ESP_LOGI(TAG, "Send status to " MACSTR ": %s", MAC2STR(mac_addr), status == ESP_NOW_SEND_SUCCESS ? "Success" : "Failure");
}

static esp_err_t init_esp_now(void)
{
    esp_err_t ret = esp_now_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "ESP-NOW init failed: %s", esp_err_to_name(ret));
        return ret;
    }
    esp_now_register_recv_cb(recv_cb);
    esp_now_register_send_cb(send_cb);
    ESP_LOGI(TAG, "ESPNOW init completed");
    return ESP_OK;
}

static esp_err_t register_peer(uint8_t *peer_addr)
{
    if (peer_addr == NULL) {
        ESP_LOGE(TAG, "Peer address is NULL");
        return ESP_ERR_INVALID_ARG;
    }

    esp_now_peer_info_t peer_info = {};
    memcpy(peer_info.peer_addr, peer_addr, ESP_NOW_ETH_ALEN);
    peer_info.channel = ESP_CHANNEL;
    peer_info.ifidx = ESP_IF_WIFI_STA;
    peer_info.encrypt = false;

    esp_err_t ret = esp_now_add_peer(&peer_info);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add peer: %s", esp_err_to_name(ret));
        return ret;
    }
    ESP_LOGI(TAG, "Peer registered successfully");
    return ESP_OK;
}

void app_main(void)
{
    ESP_ERROR_CHECK(init_wifi());
    ESP_ERROR_CHECK(init_esp_now());
    ESP_ERROR_CHECK(register_peer(peer_mac));
    ESP_LOGI(TAG, "Initialization complete");
}