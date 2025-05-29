#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#define ledR 33
#define ledG 25

#define STACK_SIZE 1024 * 2
#define R_delay 1000
#define G_delay 2000

xSemaphoreHandle GlobalKey = 0;

const char *tag = "Main";

esp_err_t init_led(void);
esp_err_t create_task(void);
esp_err_t shared_resource(int led);
void vtaskR(void *pvParameters);
void vtaskG(void *pvParameters);

void app_main()
{
    GlobalKey = xSemaphoreCreateMutex();

    init_led();
    create_task();
}

esp_err_t init_led(void)
{
    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);

    return ESP_OK;
}

esp_err_t create_task(void)
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreatePinnedToCore(vtaskR, "vtaskR", STACK_SIZE, &ucParameterToPass, 1, &xHandle, 0);
    xTaskCreatePinnedToCore(vtaskG, "vtaskG", STACK_SIZE, &ucParameterToPass, 1, &xHandle, 1);

    return ESP_OK;
}

esp_err_t shared_resource(int led)
{
    for (size_t i=0;i<8;i++)
    {
        vTaskDelay(pdMS_TO_TICKS(400));
        gpio_set_level(led, 1);
        vTaskDelay(pdMS_TO_TICKS(400));
        gpio_set_level(led, 0);
    }
    return ESP_OK;
}

void vtaskR(void *pvParameters)
{
    while (1)
    {
        if(xSemaphoreTake(GlobalKey,pdMS_TO_TICKS(100)))
        {
            ESP_LOGI(tag, "Task R tomo el recurso compartido");
            shared_resource(ledR);
            xSemaphoreGive(GlobalKey);
        }
        vTaskDelay(pdMS_TO_TICKS(R_delay));
    }
}

void vtaskG(void *pvParameters)
{
    

    while (1)
    {
        if(xSemaphoreTake(GlobalKey,pdMS_TO_TICKS(100)))
        {
            ESP_LOGI(tag, "Task G tomo el recurso compartido");
            shared_resource(ledG);
            xSemaphoreGive(GlobalKey);
        }
        vTaskDelay(pdMS_TO_TICKS(G_delay));
    }
}
