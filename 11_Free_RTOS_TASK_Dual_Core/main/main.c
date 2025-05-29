#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#define ledR 33
#define ledG 25
#define ledB 26
#define STACK_SIZE 1024*2
#define R_delay 4000
#define G_delay 66
#define B_delay 1000

const char *tag = "Main";

esp_err_t init_led(void);
esp_err_t create_task(void);
void vtaskR(void *pvParameters);
void vtaskG(void *pvParameters);
void vtaskB(void *pvParameters);

void app_main()
{
    init_led();
    create_task();
    ESP_LOGI(tag, "Task created");
    ESP_LOGI(tag, "Número de procesadores: %i",portNUM_PROCESSORS);
    while(1)
    {
        printf("Hello World from main\n");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}

esp_err_t init_led(void)
{
    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);
    gpio_reset_pin(ledB);
    gpio_set_direction(ledB, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

esp_err_t create_task(void)
{
    static uint8_t ucParameterToPass;
    TaskHandle_t xHandle = NULL;

    xTaskCreatePinnedToCore(vtaskR, "vtaskR", STACK_SIZE, &ucParameterToPass, 1, &xHandle, 0);
    xTaskCreatePinnedToCore(vtaskG, "vtaskG", STACK_SIZE, &ucParameterToPass, 1, &xHandle, 1);
    xTaskCreatePinnedToCore(vtaskB, "vtaskB", STACK_SIZE, &ucParameterToPass, 1, &xHandle, tskNO_AFFINITY);

    return ESP_OK;
}

void vtaskR(void *pvParameters)
{
    while (1)
    {
        // Task code goes here.
        ESP_LOGE(tag, "Task R Core 0");
        gpio_set_level(ledR, 1);
        vTaskDelay(pdMS_TO_TICKS(R_delay));
        gpio_set_level(ledR, 0);
        vTaskDelay(pdMS_TO_TICKS(R_delay));
    }
}

void vtaskG(void *pvParameters)
{
    while (1)
    {
        // Task code goes here.
        ESP_LOGI(tag, "Task G Core 1");
        gpio_set_level(ledG, 1);
        vTaskDelay(pdMS_TO_TICKS(G_delay));
        gpio_set_level(ledG, 0);
        vTaskDelay(pdMS_TO_TICKS(G_delay));
    }
}

void vtaskB(void *pvParameters)
{
    while (1)
    {
        // Task code goes here.
        ESP_LOGW(tag, "Task B Core Any");
        gpio_set_level(ledB, 1);
        vTaskDelay(pdMS_TO_TICKS(B_delay));
        gpio_set_level(ledB, 0);
        vTaskDelay(pdMS_TO_TICKS(B_delay));
    }
}