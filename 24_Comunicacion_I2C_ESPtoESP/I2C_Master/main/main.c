// Incluye las librerías necesarias para ESP-IDF y FreeRTOS
#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

// Dirección del esclavo I2C y parámetros de comunicación
#define I2C_SLAVE_ADDR 0x32
#define RX_BUFFER_LEN 255
#define TIMEOUT_MS 1000
#define DELAY_MS 1000

// Etiqueta para los logs
static const char *tag = "i2c-master";

// Función para copiar un arreglo de bytes de src a dest
static void copy_array(uint8_t *dest, uint8_t *src, size_t len)
{
    for (size_t i = 0; i < len; i++)
    {
        dest[i] = src[i];
    }
}

// Configura el bus I2C en modo master
static esp_err_t set_i2c(void)
{
    i2c_config_t i2c_config = {};
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = 21; // Pin SDA
    i2c_config.scl_io_num = 22; // Pin SCL
    i2c_config.sda_pullup_en = true;
    i2c_config.scl_pullup_en = true;
    i2c_config.master.clk_speed = 100000; // 100 kHz
    i2c_config.clk_flags = 0;

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_LEVEL1));

    return ESP_OK;
}

// Función principal de la aplicación
void app_main(void)
{
    uint8_t rx_data[RX_BUFFER_LEN]; // Buffer para datos recibidos del esclavo
    uint8_t command[12];            // Buffer para comando a enviar
    uint8_t command_led_r[12] = "LEDR"; // Comando para encender LED rojo
    uint8_t command_led_g[12] = "LEDG"; // Comando para encender LED verde
    uint8_t command_led_b[12] = "LEDB"; // Comando para encender LED azul
    uint8_t count = 0;                  // Contador para alternar comandos

    ESP_ERROR_CHECK(set_i2c()); // Inicializa el bus I2C

    while (1)
    {
        count++;
        if (count > 2)
        {
            count = 0;
        }

        // Selecciona el comando a enviar según el valor de count
        switch (count)
        {
        case 0:
            // Enviar comando para LED rojo
            copy_array(&command, command_led_r, 12);
            break;
        case 1:
            // Enviar comando para LED verde
            copy_array(&command, command_led_g, 12);
            break;
        case 2:
            // Enviar comando para LED azul
            copy_array(&command, command_led_b, 12);
            break;
        default:
            break;
        }
        // Envía el comando al esclavo y lee la respuesta (espera 2 bytes ADC + mensaje)
        // --- Comunicación I2C Master-Slave ---
        // El master envía un comando (ej: "LEDR", "LEDG", "LEDB") para controlar LEDs en el slave.
        // El slave responde con:
        //   - 2 bytes: valor analógico leído en el pin 34 (ADC1_CHANNEL_6)
        //   - Mensaje de texto (ej: "Hello from Slave!")
        int rx_len = 2 + 32; // 2 bytes ADC + hasta 32 bytes de mensaje
        memset(rx_data, 0, rx_len);
        i2c_master_write_read_device(I2C_NUM_0,
                                     I2C_SLAVE_ADDR,
                                     &command,
                                     sizeof(command),
                                     rx_data,
                                     rx_len,
                                     pdMS_TO_TICKS(TIMEOUT_MS));

        // --- Procesamiento de la respuesta ---
        // Reconstruye el valor ADC recibido (2 bytes, MSB primero)
        uint16_t adc_value = ((uint16_t)rx_data[0] << 8) | rx_data[1];
        ESP_LOGI(tag, "ADC Value from slave: %d", adc_value);
        // Imprime solo el mensaje recibido, deteniéndose en el primer caracter nulo o no imprimible
        char msg[33] = {0};
        for (int i = 0; i < 32; i++) {
            char c = rx_data[2 + i];
            if (c == '\0' || c < 32 || c > 126) {
                break;
            }
            msg[i] = c;
        }
        ESP_LOGI(tag, "Mensaje del slave: %s", msg);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}

/*
--- COMUNICACIÓN BIDIRECCIONAL Y MÁS VALORES ANALÓGICOS ---

1. Comunicación bidireccional:
   - El master puede enviar comandos al slave (como ya se hace).
   - El slave puede enviar datos al master en la respuesta (como el valor ADC y el mensaje).
   - Si quieres que el slave también pueda enviar comandos al master, deberías implementar una tarea en el master que escuche (read) datos del slave, o definir un protocolo donde el master periódicamente pregunte si hay comandos pendientes.

2. Enviar más valores analógicos:
   - En el slave, lee más canales ADC y colócalos en el buffer de respuesta:
     Ejemplo:
        int adc1 = adc1_get_raw(ADC1_CHANNEL_6); // pin 34
        int adc2 = adc1_get_raw(ADC1_CHANNEL_7); // pin 35
        tx_data[0] = (adc1 >> 8) & 0xFF;
        tx_data[1] = adc1 & 0xFF;
        tx_data[2] = (adc2 >> 8) & 0xFF;
        tx_data[3] = adc2 & 0xFF;
   - El master debe leer 4 bytes para los dos valores y reconstruirlos igual que antes.

3. Protocolo recomendado:
   - Define el orden y cantidad de bytes para cada dato (ej: 2 bytes por cada ADC, luego el mensaje).
   - El master debe saber cuántos bytes esperar y cómo interpretarlos.

4. Ejemplo de respuesta del slave con 3 valores ADC y mensaje:
   tx_data[0-1]: ADC1 (MSB, LSB)
   tx_data[2-3]: ADC2 (MSB, LSB)
   tx_data[4-5]: ADC3 (MSB, LSB)
   tx_data[6...]: Mensaje null-terminado

5. Para comunicación realmente bidireccional (ambos pueden iniciar comunicación):
   - Ambos ESP32 deben ser master y slave a la vez, o alternar roles, lo cual es más complejo y requiere coordinación.
*/