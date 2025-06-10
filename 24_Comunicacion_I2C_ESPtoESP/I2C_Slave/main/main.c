#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "driver/adc.h"

// Dirección del esclavo I2C
#define I2C_SLAVE_ADDR 0x32
// Tamaño de los buffers de recepción y transmisión
#define RX_BUFFER_LEN 255
#define TX_BUFFER_LEN 255

// Pines de los LEDs
#define LED_R 33
#define LED_G 25
#define LED_B 26
#define LED_BOARD 2

// Etiqueta para los logs
static const char *tag = "i2c-slave";

// Tarea que maneja la comunicación I2C como esclavo
// Esta tarea espera comandos del master por I2C, controla los LEDs según el comando recibido,
// lee un valor analógico (ADC) y responde al master con el valor y un mensaje de texto.
static void i2c_handle_task(void *pvParameters)
{
    ESP_LOGI(tag, "I2C slave task started");
    uint8_t rx_data[RX_BUFFER_LEN] = {0}; // Buffer para datos recibidos del master
    uint8_t tx_data[32] = "Hello from Slave!"; // Buffer de respuesta (mensaje al master)
    int adc_value = 0;
    // Inicializa ADC para leer del pin 34 (ADC1_CHANNEL_6)
    adc1_config_width(ADC_WIDTH_BIT_12); // Resolución de 12 bits (0-4095)
    adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11); // Atenuación para rango completo

    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(100)); // Espera 100 ms entre iteraciones
        gpio_set_level(LED_BOARD, 0);   // Apaga LED de estado

        // Lee datos recibidos por I2C (comando del master)
        int len = i2c_slave_read_buffer(I2C_NUM_0, &rx_data, RX_BUFFER_LEN, pdMS_TO_TICKS(100));
        if (len > 0)
        {
            gpio_set_level(LED_BOARD, 1); // Enciende LED de estado si hay datos
            ESP_LOG_BUFFER_CHAR(tag, rx_data, 32); // Muestra los datos recibidos por consola

            i2c_reset_rx_fifo(I2C_NUM_0); // Limpia el FIFO de recepción
            // Procesa comandos recibidos para controlar los LEDs
            if (strncmp((const char *)rx_data, "LEDR", 4) == 0)
            {
                gpio_set_level(LED_R, 1); // Enciende LED rojo
                gpio_set_level(LED_G, 0);
                gpio_set_level(LED_B, 0);
            }
            if (strncmp((const char *)rx_data, "LEDG", 4) == 0)
            {
                gpio_set_level(LED_R, 0);
                gpio_set_level(LED_G, 1); // Enciende LED verde
                gpio_set_level(LED_B, 0);
            }
            if (strncmp((const char *)rx_data, "LEDB", 4) == 0)
            {
                gpio_set_level(LED_R, 0);
                gpio_set_level(LED_G, 0);
                gpio_set_level(LED_B, 1); // Enciende LED azul
            }

            // Leer valor analógico del pin 34 (ADC1_CHANNEL_6)
            adc_value = adc1_get_raw(ADC1_CHANNEL_6);
            ESP_LOGI(tag, "ADC value: %d", adc_value); // Log para depuración
            // Prepara la respuesta: 2 bytes para el valor ADC (MSB primero) + mensaje
            tx_data[0] = (adc_value >> 8) & 0xFF;
            tx_data[1] = adc_value & 0xFF;
            // Copia el mensaje después de los 2 bytes de ADC
            const char *msg = "Hello from Slave!";
            size_t msg_len = strlen(msg);
            memcpy(&tx_data[2], msg, msg_len);
            // Envía respuesta al master: 2 bytes ADC + mensaje
            i2c_slave_write_buffer(I2C_NUM_0, tx_data, 2 + msg_len, pdMS_TO_TICKS(100));
            bzero(rx_data, RX_BUFFER_LEN); // Limpia el buffer de recepción
        }
        // Si no hay datos, espera y repite
    }
}

// Configura el periférico I2C en modo esclavo
static esp_err_t set_i2c(void)
{
    i2c_config_t i2c_config = {};
    i2c_config.mode = I2C_MODE_SLAVE;
    i2c_config.sda_io_num = 21;
    i2c_config.scl_io_num = 22;
    i2c_config.sda_pullup_en = true;
    i2c_config.scl_pullup_en = true;
    i2c_config.slave.addr_10bit_en = 0;
    i2c_config.slave.slave_addr = I2C_SLAVE_ADDR;
    i2c_config.clk_flags = 0;

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config)); // Aplica la configuración
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_SLAVE, RX_BUFFER_LEN, TX_BUFFER_LEN, ESP_INTR_FLAG_LEVEL1)); // Instala el driver

    return ESP_OK;
}

// Inicializa los pines de los LEDs como salidas y los apaga
static esp_err_t init_led(void)
{
    gpio_reset_pin(LED_R);
    gpio_set_direction(LED_R, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_R, 0);

    gpio_reset_pin(LED_G);
    gpio_set_direction(LED_G, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_G, 0);

    gpio_reset_pin(LED_B);
    gpio_set_direction(LED_B, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_B, 0);

    gpio_reset_pin(LED_BOARD);
    gpio_set_direction(LED_BOARD, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_BOARD, 0);

    ESP_LOGI(tag, "Initialized LEDs");

    return ESP_OK;
}

// Crea la tarea que maneja la comunicación I2C
static esp_err_t create_i2c_handle_task(void)
{
    xTaskCreate(i2c_handle_task, "i2c_handle_task", 1024 * 4, NULL, 1, NULL);
    return ESP_OK;
}

// Función principal de la aplicación
void app_main(void)
{

    
    ESP_ERROR_CHECK(set_i2c());      // Configura I2C esclavo
    ESP_ERROR_CHECK(init_led());     // Inicializa LEDs
    ESP_ERROR_CHECK(create_i2c_handle_task()); // Crea la tarea de manejo I2C
}

/*
--- ¿CÓMO ENVIAR MÁS VALORES ANALÓGICOS? ---
// 1. Lee más canales ADC y colócalos en el buffer de respuesta:
//    int adc1 = adc1_get_raw(ADC1_CHANNEL_6); // pin 34
//    int adc2 = adc1_get_raw(ADC1_CHANNEL_7); // pin 35
//    tx_data[0] = (adc1 >> 8) & 0xFF;
//    tx_data[1] = adc1 & 0xFF;
//    tx_data[2] = (adc2 >> 8) & 0xFF;
//    tx_data[3] = adc2 & 0xFF;
//    memcpy(&tx_data[4], msg, msg_len);
// 2. El master debe leer 4 bytes para los dos valores y reconstruirlos igual que antes.

--- ¿CÓMO HACER COMUNICACIÓN BIDIRECCIONAL REAL? ---
// - El master siempre inicia la comunicación en I2C estándar.
// - El slave responde con datos (como ya se hace).
// - Si quieres que el slave también envíe comandos al master, puedes:
//   a) Hacer que el master pregunte periódicamente si hay comandos pendientes.
//   b) Usar interrupciones o un canal adicional (ej: UART, GPIO) para avisar al master.
// - Para que ambos puedan iniciar comunicación, ambos ESP32 deben ser master y slave a la vez, o alternar roles (más complejo).
*/