#include "esp_err.h"
#include "driver/gpio.h"

esp_err_t init_led(gpio_num_t led_gpio);
esp_err_t set_led(gpio_num_t led_gpio, uint32_t level);