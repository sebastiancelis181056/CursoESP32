// Incluye la librería estándar de entrada/salida
#include <stdio.h>
// Incluye el controlador para manejo de pines GPIO
#include "driver/gpio.h"
// Incluye las librerías de FreeRTOS para manejo de tareas
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// Define el número de pin GPIO donde está conectado el LED
#define led1 10

// Variable global para almacenar el estado del LED (encendido/apagado)
uint8_t led_level = 0;

// Declaración de la función para inicializar el LED
esp_err_t init_led(void);
// Declaración de la función para hacer parpadear el LED
esp_err_t blink_led(void);

// Función principal de la aplicación
void app_main(void)
{
    // Inicializa el LED configurando el pin correspondiente
    init_led();
    // Bucle infinito
    while (1)
    {
        // Espera 1 segundo (1000 ms)
        vTaskDelay(1000 / portTICK_PERIOD_MS);
        // Cambia el estado del LED (enciende/apaga)
        blink_led();
        // Imprime el estado actual del LED por consola
        printf("Led level: %u\n", led_level);
    }
}

// Función para inicializar el pin del LED
esp_err_t init_led(void)
{
    // Restablece la configuración del pin del LED
    gpio_reset_pin(led1);
    // Configura el pin como salida
    gpio_set_direction(led1, GPIO_MODE_OUTPUT);
    return ESP_OK;
}

// Función para alternar el estado del LED
esp_err_t blink_led(void)
{
    // Invierte el valor de led_level (0 -> 1, 1 -> 0)
    led_level = !led_level;
    // Establece el nivel del pin según el valor de led_level
    gpio_set_level(led1, led_level);
    return ESP_OK;
}
