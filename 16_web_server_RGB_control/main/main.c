#include "esp_wifi.h" // Librería para manejo de WiFi
#include <esp_event.h> // Librería para manejo de eventos
#include <esp_log.h> // Librería para logs
#include <esp_system.h> // Funciones del sistema ESP
#include <nvs_flash.h> // Manejo de memoria no volátil
#include <sys/param.h> // Parámetros del sistema
#include "esp_netif.h" // Interfaz de red
#include "esp_eth.h" // Ethernet
#include "protocol_examples_common.h" // Ejemplos de conexión
#include <esp_https_server.h> // Servidor HTTPS
#include "esp_tls.h" // Seguridad TLS
#include <string.h> // Manejo de cadenas
#include "driver/gpio.h" // Manejo de pines GPIO
#include <stdio.h> // Funciones estándar de C

#define ledR 33 // Pin GPIO para LED Rojo
#define ledG 25 // Pin GPIO para LED Verde
#define ledB 26 // Pin GPIO para LED Azul

int8_t led_r_state = 0; // Estado del LED Rojo (0: apagado, 1: encendido)
int8_t led_g_state = 0; // Estado del LED Verde
int8_t led_b_state = 0; // Estado del LED Azul

static const char *TAG = "main"; // Etiqueta para logs

// Prototipos de funciones
esp_err_t init_led(void); // Inicializa los pines de los LEDs
esp_err_t toggle_led(int led); // Cambia el estado de un LED

/* Handler para peticiones HTTP GET */
static esp_err_t root_get_handler(httpd_req_t *req)
{

    // Carga el HTML embebido en el binario
    extern unsigned char view_start[] asm("_binary_view_html_start");
    extern unsigned char view_end[] asm("_binary_view_html_end");
    size_t view_len = view_end - view_start;
    char viewHtml[view_len];
    memcpy(viewHtml, view_start, view_len);
    ESP_LOGI(TAG, "URI: %s", req->uri); // Muestra la URI recibida

    // Cambia el estado del LED correspondiente según la URI
    if (strcmp(req->uri, "/?led-r") == 0)
    {
        toggle_led(ledR);
    }
    if (strcmp(req->uri, "/?led-g") == 0)
    {
        toggle_led(ledG);
    }
    if (strcmp(req->uri, "/?led-b") == 0)
    {
        toggle_led(ledB);
    }


    // Inserta el estado de los LEDs en el HTML
    char *viewHtmlUpdated;
    int formattedStrResult = asprintf(&viewHtmlUpdated, viewHtml, led_r_state ? "ON" : "OFF", led_g_state ? "ON" : "OFF", led_b_state ? "ON" : "OFF");


    // Indica que la respuesta es HTML
    httpd_resp_set_type(req, "text/html");


    // Envía el HTML actualizado o el original si hubo error
    if (formattedStrResult > 0)
    {
        httpd_resp_send(req, viewHtmlUpdated, view_len);
        free(viewHtmlUpdated);
    }
    else
    {
        ESP_LOGE(TAG, "Error updating variables");
        httpd_resp_send(req, viewHtml, view_len);
    }

    return ESP_OK;
}

// Estructura que define el endpoint principal del servidor web
static const httpd_uri_t root = {
    .uri = "/",
    .method = HTTP_GET,
    .handler = root_get_handler};

// Inicia el servidor web
static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;


    // Inicia el servidor httpd
    ESP_LOGI(TAG, "Starting server");

    httpd_ssl_config_t conf = HTTPD_SSL_CONFIG_DEFAULT();
    conf.transport_mode = HTTPD_SSL_TRANSPORT_INSECURE;
    esp_err_t ret = httpd_ssl_start(&server, &conf);
    if (ESP_OK != ret)
    {
        ESP_LOGI(TAG, "Error starting server!");
        return NULL;
    }


    // Registra los handlers de las URIs
    ESP_LOGI(TAG, "Registering URI handlers");
    httpd_register_uri_handler(server, &root);
    return server;
}

// Detiene el servidor web
static void stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    httpd_ssl_stop(server);
}

// Handler para evento de desconexión WiFi
static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        stop_webserver(*server);
        *server = NULL;
    }
}

// Handler para evento de conexión WiFi
static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        *server = start_webserver();
    }
}

// Inicializa los pines de los LEDs como salida
esp_err_t init_led(void)
{
    gpio_config_t pGPIOConfig;
    // Configura los pines de los LEDs como salida
    pGPIOConfig.pin_bit_mask = (1ULL << ledR) | (1ULL << ledG) | (1ULL << ledB);
    pGPIOConfig.mode = GPIO_MODE_DEF_OUTPUT;
    pGPIOConfig.pull_up_en = GPIO_PULLUP_DISABLE;
    pGPIOConfig.pull_down_en = GPIO_PULLDOWN_DISABLE;
    pGPIOConfig.intr_type = GPIO_INTR_DISABLE;

    gpio_config(&pGPIOConfig);

    ESP_LOGI(TAG, "init led completed");
    return ESP_OK;
}

// Cambia el estado de un LED (enciende/apaga)
esp_err_t toggle_led(int led)
{
    int8_t state = 0;
    // Cambia el estado de la variable y el pin correspondiente
    switch (led)
    {
    case ledR:
        led_r_state = !led_r_state;
        state = led_r_state;
        break;
    case ledG:
        led_g_state = !led_g_state;
        state = led_g_state;
        break;
    case ledB:
        led_b_state = !led_b_state;
        state = led_b_state;
        break;

    default:
        gpio_set_level(ledR, 0);
        gpio_set_level(ledG, 0);
        gpio_set_level(ledB, 0);
        led_r_state = 0;
        led_g_state = 0;
        led_b_state = 0;
        break;
    }
    gpio_set_level(led, state); // Actualiza el pin físico
    return ESP_OK;
}

// Función principal del programa
void app_main(void)
{
    ESP_ERROR_CHECK(init_led()); // Inicializa los LEDs
    static httpd_handle_t server = NULL;
    ESP_ERROR_CHECK(nvs_flash_init()); // Inicializa la memoria NVS
    ESP_ERROR_CHECK(esp_netif_init()); // Inicializa la red
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // Crea el loop de eventos
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server)); // Registra handler para conexión WiFi
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server)); // Registra handler para desconexión WiFi
    ESP_ERROR_CHECK(example_connect()); // Conecta a la red WiFi
}
