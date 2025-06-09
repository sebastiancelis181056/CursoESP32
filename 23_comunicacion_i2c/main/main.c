#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_SLAVE_ADDR 0x68
#define TIMEOUT_MS 1000
#define DELAY_MS 1000

static const char *tag = "i2c";

static esp_err_t set_i2c(void)
{
    i2c_config_t i2c_config = {};
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = 21;
    i2c_config.scl_io_num = 22;
    i2c_config.sda_pullup_en = true;
    i2c_config.scl_pullup_en = true;
    i2c_config.master.clk_speed = 100000;
    i2c_config.clk_flags = 0;

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_LEVEL1));

    return ESP_OK;
}

void app_main(void)
{
    uint8_t rx_data[8];
    uint8_t command = 0x00;
    ESP_ERROR_CHECK(set_i2c());
    while (1)
    {
        i2c_master_write_read_device(I2C_NUM_0,
                                     I2C_SLAVE_ADDR,
                                     &command,
                                     1,
                                     rx_data,
                                     8,
                                     pdMS_TO_TICKS(TIMEOUT_MS));

        ESP_LOG_BUFFER_HEX(tag, rx_data, 8);
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}


/*#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "driver/i2c.h"

#define I2C_SLAVE_ADDR 0x68
#define TIMEOUT_MS 1000
#define DELAY_MS 1000

static const char *tag = "i2c";

static esp_err_t set_i2c(void)
{
    i2c_config_t i2c_config = {};
    i2c_config.mode = I2C_MODE_MASTER;
    i2c_config.sda_io_num = 21;
    i2c_config.scl_io_num = 22;
    i2c_config.sda_pullup_en = true;
    i2c_config.scl_pullup_en = true;
    i2c_config.master.clk_speed = 100000;
    i2c_config.clk_flags = 0;

    ESP_ERROR_CHECK(i2c_param_config(I2C_NUM_0, &i2c_config));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_LEVEL1));

    return ESP_OK;
}

// --- Conversión BCD <-> Decimal ---
static uint8_t bcd_to_dec(uint8_t val) {
    return ((val >> 4) * 10) + (val & 0x0F);
}

static uint8_t dec_to_bcd(uint8_t val) {
    return ((val / 10) << 4) | (val % 10);
}

// --- Estructura para la hora/fecha ---
typedef struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t day_of_week;
    uint8_t day;
    uint8_t month;
    uint8_t year;
} ds1307_time_t;

// --- Escribir hora y fecha en el DS1307 ---
esp_err_t ds1307_set_time(const ds1307_time_t *time) {
    uint8_t data[8];
    data[0] = 0x00; // Dirección inicial
    // El bit 7 (CH) debe estar en 0 para que el reloj funcione
    data[1] = dec_to_bcd(time->seconds) & 0x7F;
    data[2] = dec_to_bcd(time->minutes);
    data[3] = dec_to_bcd(time->hours);
    data[4] = dec_to_bcd(time->day_of_week);
    data[5] = dec_to_bcd(time->day);
    data[6] = dec_to_bcd(time->month);
    data[7] = dec_to_bcd(time->year);
    return i2c_master_write_to_device(I2C_NUM_0, I2C_SLAVE_ADDR, data, 8, pdMS_TO_TICKS(TIMEOUT_MS));
}

// --- Leer hora y fecha del DS1307 ---
esp_err_t ds1307_read_time(ds1307_time_t *time) {
    uint8_t reg = 0x00; // Dirección inicial de los registros de tiempo
    uint8_t data[7];
    esp_err_t ret = i2c_master_write_read_device(I2C_NUM_0, I2C_SLAVE_ADDR, &reg, 1, data, 7, pdMS_TO_TICKS(TIMEOUT_MS));
    if (ret != ESP_OK) return ret;
    time->seconds     = bcd_to_dec(data[0] & 0x7F);
    time->minutes     = bcd_to_dec(data[1]);
    time->hours       = bcd_to_dec(data[2] & 0x3F);
    time->day_of_week = bcd_to_dec(data[3]);
    time->day         = bcd_to_dec(data[4]);
    time->month       = bcd_to_dec(data[5]);
    time->year        = bcd_to_dec(data[6]);
    return ESP_OK;
}

void print_ds1307_time(const ds1307_time_t *time) {
    printf("Fecha: %02d/%02d/20%02d  Hora: %02d:%02d:%02d  DíaSemana: %d\n",
        time->day, time->month, time->year, time->hours, time->minutes, time->seconds, time->day_of_week);
}

void app_main(void)
{
    ESP_ERROR_CHECK(set_i2c());
    ds1307_time_t rtc_time;
    // --- SOLO UNA VEZ: Inicializa la hora/fecha actual ---
    // Descomenta y ajusta la siguiente sección para poner en hora el DS1307Z
    *
    ds1307_time_t new_time = {
        .seconds = 0,
        .minutes = 30,
        .hours = 2,
        .day_of_week = 1, // 1=Domingo, 2=Lunes, ...
        .day = 9,
        .month = 6,
        .year = 25 // 2025
    };
    if (ds1307_set_time(&new_time) == ESP_OK) {
        printf("Hora/fecha inicializada en el DS1307\n");
    } else {
        printf("Error al inicializar hora/fecha\n");
    }
    *
    while (1)
    {
        if (ds1307_read_time(&rtc_time) == ESP_OK) {
            print_ds1307_time(&rtc_time);
        } else {
            printf("Error leyendo DS1307\n");
        }
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));
    }
}
*/