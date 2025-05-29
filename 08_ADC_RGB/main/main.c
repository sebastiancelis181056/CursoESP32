#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "freertos/timers.h"
#include "driver/adc.h"

#define led1 26
// uint8_t es un tipo de entero sin signo de 8 bits, que puede contener valores de 0 a 255.
uint8_t led_level = 0;

TimerHandle_t xTimers;
int interval = 1000;
int timerId = 1;
int adc_val=0;

static const char *TAG = "Main";

// Declaración de funciones
esp_err_t init_led(void);
esp_err_t blink(void);
esp_err_t set_timer(void);
esp_err_t set_adc(void);

// Callback del temporizador
void vTimerCallback(TimerHandle_t pxTimer)
{
    blink();
    adc_val=adc1_get_raw(ADC1_CHANNEL_6);
    ESP_LOGI(TAG, "ADC Value: %d", adc_val);
}

// Función principal
void app_main(void)
{
    init_led();
    set_timer();
}

// Inicialización del LED
esp_err_t init_led(void)
{
    gpio_reset_pin(led1);
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

// Función para parpadear el LED
esp_err_t blink(void)
{
    led_level = !led_level;
    gpio_set_level(led1, led_level);
    return ESP_OK;
}

// Configuración del temporizador
esp_err_t set_timer(void)
{
    ESP_LOGI(TAG, "Timer init configuration");
    xTimers = xTimerCreate("Timer",                   // Nombre del temporizador
                           (pdMS_TO_TICKS(interval)), // Periodo del temporizador en ticks
                           pdTRUE,                    // El temporizador se recarga automáticamente
                           (void *)timerId,           // ID único del temporizador
                           vTimerCallback             // Callback del temporizador
    );

    if (xTimers == NULL)
    {
        // El temporizador no fue creado
        ESP_LOGE(TAG, "The timer was not created.");
    }
    else
    {
        // Iniciar el temporizador
        if (xTimerStart(xTimers, 0) != pdPASS)
        {
            // El temporizador no pudo ser activado
            ESP_LOGE(TAG, "The timer could not be set into the Active state.");
        }
    }
    return ESP_OK;
}

esp_err_t set_adc(void)
{
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_MAX);
    adc1_config_width(ADC_WIDTH_BIT_12);
    return ESP_OK;
}