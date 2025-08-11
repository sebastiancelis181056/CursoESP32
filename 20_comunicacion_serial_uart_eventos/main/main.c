#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/uart.h"
#include <string.h> // Incluir para usar memset
#include "freertos/queue.h"

#define ledR 33
#define ledG 25
#define ledB 26

#define UART_NUM UART_NUM_0
#define BUF_SIZE 1024
#define TASK_MEMORY 2048

static QueueHandle_t uart_queue;

const char *tag = "Main";

esp_err_t init_led(void);
esp_err_t init_uart(void);

void app_main(void)
{
    init_led();
    init_uart();
}

static void uart_task(void *pvParameters)
{
    uart_event_t event;
    uint8_t *data = (uint8_t *)malloc(BUF_SIZE);
    if (data == NULL)
    {
        ESP_LOGE(tag, "Failed to allocate memory for UART data buffer");
        vTaskDelete(NULL);
        return;
    }
    while (1)
    {
        if (xQueueReceive(uart_queue, &event, portMAX_DELAY))
        {
            memset(data, 0, BUF_SIZE); // Reemplazar bzero con memset
            switch (event.type)
            {
            case UART_DATA:
                uart_read_bytes(UART_NUM, data, event.size, pdMS_TO_TICKS(100));
                uart_write_bytes(UART_NUM, (const char *)data, event.size);
                uart_flush(UART_NUM);
                ESP_LOGI(tag, "Data received: %s", data);

                /*for (size_t i = 0; i < event.size; i++)
                {
                    char value = data[i];
                    switch (value)
                    {
                    case 'R':
                        gpio_set_level(ledR, 1);
                        gpio_set_level(ledG, 0);
                        gpio_set_level(ledB, 0);
                        break;

                    case 'G':
                        gpio_set_level(ledR, 0);
                        gpio_set_level(ledG, 1);
                        gpio_set_level(ledB, 0);
                        break;

                    case 'B':
                        gpio_set_level(ledR, 0);
                        gpio_set_level(ledG, 0);
                        gpio_set_level(ledB, 1);
                        break;
                    case 'O':
                        gpio_set_level(ledR, 0);
                        gpio_set_level(ledG, 0);
                        gpio_set_level(ledB, 0);
                        break;

                    default:
                        gpio_set_level(ledR, 0);
                        gpio_set_level(ledG, 0);
                        gpio_set_level(ledB, 0);
                        break;
                    }
                }*/
                data[event.size] = '\0'; // Asegura que sea un string válido
                if (strcmp((char *)data, "LedRojo") == 0)
                {
                    gpio_set_level(ledR, 1);
                    gpio_set_level(ledG, 0);
                    gpio_set_level(ledB, 0);
                }
                else if (strcmp((char *)data, "LedVerde") == 0)
                {
                    gpio_set_level(ledR, 0);
                    gpio_set_level(ledG, 1);
                    gpio_set_level(ledB, 0);
                }
                else if (strcmp((char *)data, "LedAzul") == 0)
                {
                    gpio_set_level(ledR, 0);
                    gpio_set_level(ledG, 0);
                    gpio_set_level(ledB, 1);
                }
                else
                {
                    gpio_set_level(ledR, 0);
                    gpio_set_level(ledG, 0);
                    gpio_set_level(ledB, 0);
                }
                break;

            default:
                break;
            }
        }
    }
    free(data); // Free allocated memory
}

esp_err_t init_led(void)
{
    gpio_reset_pin(ledR);
    gpio_set_direction(ledR, GPIO_MODE_OUTPUT);
    gpio_set_level(ledR, 0); // Ensure LED is off initially
    gpio_reset_pin(ledG);
    gpio_set_direction(ledG, GPIO_MODE_OUTPUT);
    gpio_set_level(ledG, 0); // Ensure LED is off initially
    gpio_reset_pin(ledB);
    gpio_set_direction(ledB, GPIO_MODE_OUTPUT);
    gpio_set_level(ledB, 0); // Ensure LED is off initially
    ESP_LOGI(tag, "LEDs initialized");
    return ESP_OK;
}

esp_err_t init_uart(void)
{
    uart_config_t uart_config = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_APB,
    };
    uart_param_config(UART_NUM, &uart_config);
    // uart_set_pin(UART_NUM, 5, 4, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_set_pin(UART_NUM, 1, 3, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    uart_driver_install(UART_NUM, BUF_SIZE, BUF_SIZE, 5, &uart_queue, 0);
    xTaskCreate(uart_task, "uart_task", TASK_MEMORY, NULL, 5, NULL);
    ESP_LOGI(tag, "init_uart completed!");

    return ESP_OK;
}