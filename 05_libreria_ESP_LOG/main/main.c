#define LOG_COLOR_E  "\033[1;91m" // Rojo intenso
#define LOG_COLOR_W  "\033[1;93m" // Amarillo intenso
#define LOG_COLOR_I  "\033[1;94m" // Azul intenso
#define LOG_COLOR_D  "\033[0;36m" // Cian
#define LOG_COLOR_V  "\033[0;37m" // Gris claro

#include "esp_log.h"
#include <stdio.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#define led1 33
// uint8_t es un tipo de entero sin signo de 8 bits, que puede contener valores de 0 a 255.
uint8_t led_level = 0;
uint8_t count = 0;

static const char* TAG = "Main";

esp_err_t init_led(void);
esp_err_t blink(void);

void app_main(void)
{
    init_led();
    while(1){
        vTaskDelay(100/portTICK_PERIOD_MS);
        blink();
        count+=1;

        if(count<10)
        {
            ESP_LOGI(TAG, "Value: %u",count);
        }

        if(count>=10 && count<20)
        {
            ESP_LOGW(TAG, "Value: %u",count);
        }
        if(count>=20 && count<30)
        {
            ESP_LOGE(TAG, "Value: %u",count);
        }

        if (count>=30)
        {
            count = 0;
        }
        
        if (count == 1) {
            ESP_LOGE(TAG, "Este es un mensaje de ERROR");
            ESP_LOGW(TAG, "Este es un mensaje de WARNING");
            ESP_LOGI(TAG, "Este es un mensaje de INFO");
            ESP_LOGD(TAG, "Este es un mensaje de DEBUG");
            ESP_LOGV(TAG, "Este es un mensaje de VERBOSE");
            printf("\033[1;92mEste texto es verde intenso\033[0m y esto es color normal\n");
        }

        //printf("Led level: %u\n",led_level);
        

    }
}

esp_err_t init_led(void){
    gpio_reset_pin(led1);
    gpio_set_direction(led1,GPIO_MODE_OUTPUT);
    return ESP_OK;
}

esp_err_t blink(void){
    led_level = !led_level;
    gpio_set_level(led1,led_level);
    return ESP_OK;
}