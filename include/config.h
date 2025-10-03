/**
 * @file config.h.example
 * @brief Template de configuración del proyecto FRA
 * 
 * INSTRUCCIONES:
 * 1. Copiar este archivo como src/config.h
 * 2. Editar los valores según tu configuración local
 * 3. NUNCA hacer commit de src/config.h (está en .gitignore)
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURACIÓN WiFi
// ============================================================================

#define WIFI_SSID "Fibertel WiFi316 2.4GHz"
#define WIFI_PASSWORD "0146198437"

// Timeout de conexión WiFi en milisegundos
#define WIFI_CONNECT_TIMEOUT_MS 20000

// ============================================================================
// CONFIGURACIÓN MQTT
// ============================================================================

// Dirección del broker MQTT (Mosquitto local)
#define MQTT_BROKER_ADDR "10.57.116.71"  // Cambiar por IP de tu PC
#define MQTT_BROKER_PORT 1883

// Identificador único del cliente (cambiar si usas múltiples dispositivos)
#define MQTT_CLIENT_ID "fra_pico2w_001"

// Topic para publicar mediciones
#define MQTT_TOPIC_MEASUREMENTS "fra/measurements"

// Topic para publicar estado del sistema
#define MQTT_TOPIC_STATUS "fra/status"

// QoS para mensajes MQTT (0, 1 o 2)
// 0 = At most once (sin confirmación)
// 1 = At least once (con confirmación)
#define MQTT_QOS 0

// ============================================================================
// PARÁMETROS DEL SISTEMA DSP
// ============================================================================

// Frecuencia de muestreo del ADC (Hz)
#define SAMPLE_RATE 48000.0f

// Tamaño de ventana para Goertzel (muestras)
#define WINDOW_SIZE 480

// Resolución en frecuencia (Hz) - derivada de SAMPLE_RATE/WINDOW_SIZE
#define FREQ_RESOLUTION 100.0f

// Frecuencia mínima y máxima del barrido (Hz)
#define SWEEP_FREQ_MIN 100.0f
#define SWEEP_FREQ_MAX 20000.0f

// Número de puntos de medición en el barrido
#define SWEEP_NUM_POINTS 200

// ============================================================================
// CONFIGURACIÓN HARDWARE AD9833
// ============================================================================

// Pin SPI para AD9833
#define AD9833_SPI_INSTANCE spi0
#define AD9833_PIN_SCK  2
#define AD9833_PIN_MOSI 3
#define AD9833_PIN_CS   5

// Frecuencia del cristal del AD9833 (Hz)
#define AD9833_MCLK 25000000.0f

// ============================================================================
// CONFIGURACIÓN ADC
// ============================================================================

// Pin ADC para canal de referencia
#define ADC_PIN_REFERENCE 26  // ADC0

// Pin ADC para canal de respuesta (futuro, para 2 canales)
#define ADC_PIN_RESPONSE 27   // ADC1

// Canal DMA para transferencias ADC
#define ADC_DMA_CHANNEL 0

// ============================================================================
// DEBUGGING
// ============================================================================

// Habilitar debug serial (comentar para producción)
#define DEBUG_ENABLED

// Nivel de verbosidad del debug (0-3)
// 0 = Solo errores
// 1 = Errores + warnings
// 2 = Errores + warnings + info
// 3 = Todo (incluye debug detallado)
#define DEBUG_LEVEL 2

// Habilitar instrumentación con GPIO para osciloscopio
// #define DEBUG_GPIO_ENABLED

#ifdef DEBUG_GPIO_ENABLED
    #define DEBUG_PIN_SWEEP_START   15
    #define DEBUG_PIN_ADC_ACQUIRE   16
    #define DEBUG_PIN_DSP_PROCESS   17
    #define DEBUG_PIN_MQTT_TX       18
#endif

#endif // CONFIG_H
