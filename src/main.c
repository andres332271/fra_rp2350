/**
 * @file main.c
 * @brief Punto de entrada principal del analizador de respuesta en frecuencia
 * 
 * Este archivo contiene la inicialización del sistema y el loop principal.
 */

#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/adc.h"

#include "config.h"
#include "adc_dma.h"
#include "ad9833.h"
#include "goertzel.h"
#include "mqtt_client.h"
#include "sweep.h"

// Macros de debug
#ifdef DEBUG_ENABLED
    #define DEBUG_PRINT(level, ...) do { \
        if (level <= DEBUG_LEVEL) { \
            printf(__VA_ARGS__); \
        } \
    } while(0)
#else
    #define DEBUG_PRINT(level, ...)
#endif

/**
 * @brief Inicializa el hardware del sistema
 * @return true si la inicialización fue exitosa, false en caso contrario
 */
static bool init_hardware(void) {
    DEBUG_PRINT(2, "[INIT] Iniciando hardware...\n");
    
    // Inicializar stdio para USB serial
    stdio_init_all();
    sleep_ms(2000);  // Dar tiempo para que se establezca la conexión serial
    
    DEBUG_PRINT(2, "[INIT] FRA RP2350 v1.0\n");
    DEBUG_PRINT(2, "[INIT] Compilado: %s %s\n", __DATE__, __TIME__);
    
    // Inicializar WiFi/Bluetooth
    DEBUG_PRINT(2, "[INIT] Inicializando CYW43...\n");
    if (cyw43_arch_init()) {
        DEBUG_PRINT(0, "[ERROR] Fallo al inicializar WiFi\n");
        return false;
    }
    
    cyw43_arch_enable_sta_mode();
    DEBUG_PRINT(2, "[INIT] WiFi habilitado en modo estación\n");
    
    // Inicializar GPIO de debug si está habilitado
#ifdef DEBUG_GPIO_ENABLED
    DEBUG_PRINT(2, "[INIT] Configurando GPIO de debug...\n");
    gpio_init(DEBUG_PIN_SWEEP_START);
    gpio_init(DEBUG_PIN_ADC_ACQUIRE);
    gpio_init(DEBUG_PIN_DSP_PROCESS);
    gpio_init(DEBUG_PIN_MQTT_TX);
    
    gpio_set_dir(DEBUG_PIN_SWEEP_START, GPIO_OUT);
    gpio_set_dir(DEBUG_PIN_ADC_ACQUIRE, GPIO_OUT);
    gpio_set_dir(DEBUG_PIN_DSP_PROCESS, GPIO_OUT);
    gpio_set_dir(DEBUG_PIN_MQTT_TX, GPIO_OUT);
    
    gpio_put(DEBUG_PIN_SWEEP_START, 0);
    gpio_put(DEBUG_PIN_ADC_ACQUIRE, 0);
    gpio_put(DEBUG_PIN_DSP_PROCESS, 0);
    gpio_put(DEBUG_PIN_MQTT_TX, 0);
#endif
    
    return true;
}

/**
 * @brief Conecta a la red WiFi
 * @return true si la conexión fue exitosa, false en caso contrario
 */
static bool connect_wifi(void) {
    DEBUG_PRINT(2, "[WIFI] Conectando a SSID: %s\n", WIFI_SSID);
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    
    int result = cyw43_arch_wifi_connect_timeout_ms(
        WIFI_SSID,
        WIFI_PASSWORD,
        CYW43_AUTH_WPA2_AES_PSK,
        WIFI_CONNECT_TIMEOUT_MS
    );
    
    if (result != 0) {
        DEBUG_PRINT(0, "[ERROR] WiFi connect failed: %d\n", result);
        return false;
    }
    
    uint32_t elapsed = to_ms_since_boot(get_absolute_time()) - start_time;
    DEBUG_PRINT(2, "[WIFI] Conectado en %lu ms\n", elapsed);
    
    // Imprimir información de la conexión
    uint8_t mac[6];
    cyw43_wifi_get_mac(&cyw43_state, CYW43_ITF_STA, mac);
    DEBUG_PRINT(2, "[WIFI] MAC: %02X:%02X:%02X:%02X:%02X:%02X\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    
    return true;
}

/**
 * @brief Inicializa los módulos del sistema
 * @return true si la inicialización fue exitosa, false en caso contrario
 */
static bool init_modules(void) {
    DEBUG_PRINT(2, "[INIT] Inicializando módulos del sistema...\n");
    
    // Inicializar ADC+DMA
    DEBUG_PRINT(2, "[INIT] Configurando ADC+DMA...\n");
    if (!adc_dma_init()) {
        DEBUG_PRINT(0, "[ERROR] Fallo al inicializar ADC+DMA\n");
        return false;
    }
    
    // Inicializar AD9833
    DEBUG_PRINT(2, "[INIT] Configurando AD9833...\n");
    if (!ad9833_init()) {
        DEBUG_PRINT(0, "[ERROR] Fallo al inicializar AD9833\n");
        return false;
    }
    
    // Inicializar cliente MQTT
    DEBUG_PRINT(2, "[INIT] Configurando MQTT...\n");
    mqtt_config_t mqtt_cfg = {
        .broker_addr = MQTT_BROKER_ADDR,
        .broker_port = MQTT_BROKER_PORT,
        .client_id = MQTT_CLIENT_ID,
        .topic = MQTT_TOPIC_MEASUREMENTS
    };
    
    if (!mqtt_init(&mqtt_cfg)) {
        DEBUG_PRINT(0, "[ERROR] Fallo al inicializar MQTT\n");
        return false;
    }
    
    DEBUG_PRINT(2, "[INIT] Todos los módulos inicializados correctamente\n");
    return true;
}

/**
 * @brief Función principal
 */
int main(void) {
    // Inicializar hardware base
    if (!init_hardware()) {
        printf("[FATAL] Fallo en inicialización de hardware\n");
        while (1) {
            tight_loop_contents();
        }
    }
    
    // Conectar a WiFi
    if (!connect_wifi()) {
        printf("[FATAL] No se pudo conectar a WiFi\n");
        while (1) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(250);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(250);
        }
    }
    
    // Indicar conexión exitosa con LED
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(1000);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
    
    // Inicializar módulos del sistema
    if (!init_modules()) {
        printf("[FATAL] Fallo en inicialización de módulos\n");
        while (1) {
            tight_loop_contents();
        }
    }
    
    DEBUG_PRINT(1, "\n");
    DEBUG_PRINT(1, "========================================\n");
    DEBUG_PRINT(1, "  Sistema listo para iniciar barrido\n");
    DEBUG_PRINT(1, "========================================\n");
    DEBUG_PRINT(1, "  Frecuencia: %.0f Hz - %.0f Hz\n", SWEEP_FREQ_MIN, SWEEP_FREQ_MAX);
    DEBUG_PRINT(1, "  Resolución: %.0f Hz\n", FREQ_RESOLUTION);
    DEBUG_PRINT(1, "  Puntos: %d\n", SWEEP_NUM_POINTS);
    DEBUG_PRINT(1, "========================================\n\n");
    
    // Esperar un momento antes de iniciar
    sleep_ms(2000);
    
    // Loop principal: ejecutar barrido
    while (true) {
        DEBUG_PRINT(1, "[MAIN] Iniciando barrido de frecuencia...\n");
        
        // Ejecutar barrido completo
        frequency_sweep_execute();
        
        DEBUG_PRINT(1, "[MAIN] Barrido completado\n");
        DEBUG_PRINT(1, "[MAIN] Esperando 10 segundos antes del próximo barrido...\n\n");
        
        // Parpadear LED para indicar barrido completo
        for (int i = 0; i < 3; i++) {
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
            sleep_ms(100);
            cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
            sleep_ms(100);
        }
        
        // Esperar antes del próximo barrido
        sleep_ms(10000);
    }
    
    // Cleanup (nunca alcanzado en este diseño)
    cyw43_arch_deinit();
    return 0;
}
