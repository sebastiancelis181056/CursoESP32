#include <stdio.h>
#include "leds.h"
#include "esp_log.h"


/**
 * @brief Inicializa la configuración del LED.
 * @param led_gpio GPIO del LED a inicializar.
 * @return ESP_OK si la inicialización fue exitosa, de lo contrario un error.
 * Esta función configura el GPIO del LED como salida y lo establece en un estado bajo (apagado).
 * @note Asegúrate de que el GPIO especificado sea válido y no esté en uso por otro periférico.
 */
esp_err_t init_led(gpio_num_t led_gpio)
{
    gpio_reset_pin(led_gpio);
    gpio_set_direction(led_gpio, GPIO_MODE_OUTPUT);
    gpio_set_level(led_gpio, 0); // Set initial state to low (off)
    ESP_LOGI("LED", "LED initialized on GPIO %d", led_gpio);
    return ESP_OK;
}

/**
 * @brief Establece el nivel del LED.
 * @param led_gpio GPIO del LED a controlar.
 * @param level Nivel a establecer (0 para apagado, 1 para encendido).
 * @return ESP_OK si la operación fue exitosa, de lo contrario un error.
 * Esta función cambia el estado del LED al nivel especificado.
 * @note Asegúrate de que el GPIO del LED haya sido inicializado previamente con init_led().
 */
esp_err_t set_led(gpio_num_t led_gpio, uint32_t level)
{
    gpio_set_level(led_gpio, level);
    return ESP_OK;
}