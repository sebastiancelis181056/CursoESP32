#include <stdio.h>
#include "leds.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "nvs.h"
#include "nvs_flash.h"

#define led_rojo 33
#define led_verde 25
#define led_azul 26

static char *tag = "NVS_TEST";
nvs_handle_t app_nvs_handle;

void setup(void)
{
    init_led(led_rojo);
    init_led(led_verde);
    init_led(led_azul);

}

static esp_err_t init_nvs(void)
{
    esp_err_t error;
    nvs_flash_init();
    error = nvs_open(tag, NVS_READWRITE, &app_nvs_handle);
    if (error != ESP_OK)
    {
        ESP_LOGE(tag, "Error opening NVS");
    }
    else 
    {
        ESP_LOGI(tag, "NVS opened successfully");
    }
    return error;
}

static esp_err_t read_nvs(char *key, uint8_t *value)
{
    esp_err_t error;
    error = nvs_get_u8(app_nvs_handle, key, value);
    if (error != ESP_OK)
    {
        ESP_LOGE(tag, "Error read NVS");
    }
    else 
    {
        ESP_LOGI(tag, "Value read > %u", *value);
    }
    return error;
}

static esp_err_t write_nvs(char *key, uint8_t value)
{
    esp_err_t error;
    error = nvs_set_u8(app_nvs_handle, key, value);
    if (error != ESP_OK)
    {
        ESP_LOGE(tag, "Error writing NVS");
    }
    else 
    {
        ESP_LOGI(tag, "Value written > %u", value);
    }
    return error;
}

uint8_t count = 0;
void app_main(void)
{
    setup();
    char *key = "count";
    ESP_ERROR_CHECK(init_nvs());
    
    read_nvs(key, &count);
    while (1)
    {
        if (count == 0)
        {
            set_led(led_rojo, 1);
            set_led(led_verde, 0);
            set_led(led_azul, 0);
            count++;
        }
        else if (count == 1)
        {
            set_led(led_rojo, 0);
            set_led(led_verde, 1);
            set_led(led_azul, 0);
            count++;
        }
        else if (count == 2)
        {
            set_led(led_rojo, 0);
            set_led(led_verde, 0);
            set_led(led_azul, 1);
            count++;
        }
        else if (count == 3)
        {
            set_led(led_rojo, 1);
            set_led(led_verde, 1);
            set_led(led_azul, 0);
            count++;
        }
        else if (count == 4)
        {
            set_led(led_rojo, 1);
            set_led(led_verde, 0);
            set_led(led_azul, 1);
            count++;
        }
        else if (count == 5)
        {
            set_led(led_rojo, 0);
            set_led(led_verde, 1);
            set_led(led_azul, 1);
            count++;
        }
        else if (count == 6)
        {
            set_led(led_rojo, 1);
            set_led(led_verde, 1);
            set_led(led_azul, 1);
            count++;
        }
        else if (count == 7)
        {
            set_led(led_rojo, 0);
            set_led(led_verde, 0);
            set_led(led_azul, 0);
            count = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
        write_nvs("count", count);
        
    }
}
