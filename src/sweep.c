/**
 * @file sweep.c
 * @brief Implementación del orquestador de barrido
 * 
 * ESTADO: STUB - Implementación mínima funcional
 * TODO: Optimizar timing y manejo de errores
 */

#include "sweep.h"
#include "config.h"
#include "ad9833.h"
#include "adc_dma.h"
#include "goertzel.h"
#include "mqtt_client.h"
#include <stdio.h>
#include "pico/stdlib.h"

#ifdef DEBUG_GPIO_ENABLED
#include "hardware/gpio.h"
#endif

void frequency_sweep_execute(void) {
    printf("\n========================================\n");
    printf("  INICIANDO BARRIDO DE FRECUENCIA\n");
    printf("========================================\n\n");
    
#ifdef DEBUG_GPIO_ENABLED
    gpio_put(DEBUG_PIN_SWEEP_START, 1);
#endif
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    uint32_t successful_points = 0;
    
    // Iterar sobre todas las frecuencias
    for (uint16_t k = 1; k <= SWEEP_NUM_POINTS; k++) {
        // Calcular frecuencia objetivo
        float freq = SWEEP_FREQ_MIN + (k - 1) * FREQ_RESOLUTION;
        
        printf("[SWEEP] Punto %d/%d: %.0f Hz\n", k, SWEEP_NUM_POINTS, freq);
        
        // 1. Configurar generador AD9833
        ad9833_set_frequency(freq);
        sleep_ms(100);  // Esperar estabilización
        
        // 2. Adquirir datos con ADC+DMA
#ifdef DEBUG_GPIO_ENABLED
        gpio_put(DEBUG_PIN_ADC_ACQUIRE, 1);
#endif
        
        adc_dma_start_capture();
        adc_dma_wait_complete();
        
#ifdef DEBUG_GPIO_ENABLED
        gpio_put(DEBUG_PIN_ADC_ACQUIRE, 0);
#endif
        
        // 3. Validar muestras
        if (!adc_dma_validate_samples(adc_sample_buffer, WINDOW_SIZE)) {
            printf("[SWEEP] WARNING: Muestras inválidas en %.0f Hz\n", freq);
            // Continuar de todos modos en modo stub
        }
        
        // 4. Procesar con Goertzel
#ifdef DEBUG_GPIO_ENABLED
        gpio_put(DEBUG_PIN_DSP_PROCESS, 1);
#endif
        
        goertzel_result_t result;
        goertzel_compute(
            adc_sample_buffer,
            WINDOW_SIZE,
            freq,
            SAMPLE_RATE,
            &result
        );
        
#ifdef DEBUG_GPIO_ENABLED
        gpio_put(DEBUG_PIN_DSP_PROCESS, 0);
#endif
        
        // 5. Transmitir via MQTT
#ifdef DEBUG_GPIO_ENABLED
        gpio_put(DEBUG_PIN_MQTT_TX, 1);
#endif
        
        if (mqtt_publish_measurement(freq, result.magnitude_db, result.phase_deg)) {
            successful_points++;
        } else {
            printf("[SWEEP] ERROR: Fallo en transmisión MQTT\n");
        }
        
#ifdef DEBUG_GPIO_ENABLED
        gpio_put(DEBUG_PIN_MQTT_TX, 0);
#endif
        
        // Pequeña pausa entre puntos para no saturar el broker
        sleep_ms(5);
    }
    
#ifdef DEBUG_GPIO_ENABLED
    gpio_put(DEBUG_PIN_SWEEP_START, 0);
#endif
    
    uint32_t elapsed_ms = to_ms_since_boot(get_absolute_time()) - start_time;
    float avg_time = (float)elapsed_ms / SWEEP_NUM_POINTS;
    
    printf("\n========================================\n");
    printf("  BARRIDO COMPLETADO\n");
    printf("========================================\n");
    printf("  Puntos exitosos: %lu/%d\n", successful_points, SWEEP_NUM_POINTS);
    printf("  Tiempo total: %lu ms (%.2f s)\n", elapsed_ms, elapsed_ms / 1000.0f);
    printf("  Tiempo por punto: %.2f ms\n", avg_time);
    printf("========================================\n\n");
    
    // Publicar mensaje de finalización
    mqtt_publish_status("sweep_complete");
}

void frequency_sweep_execute_with_stats(sweep_stats_t *stats) {
    printf("[SWEEP] Ejecutando con recolección de estadísticas...\n");
    
    uint32_t start_time = to_ms_since_boot(get_absolute_time());
    
    stats->total_points = SWEEP_NUM_POINTS;
    stats->successful_points = 0;
    stats->failed_points = 0;
    
    // Ejecutar barrido (versión simplificada con conteo de errores)
    for (uint16_t k = 1; k <= SWEEP_NUM_POINTS; k++) {
        float freq = SWEEP_FREQ_MIN + (k - 1) * FREQ_RESOLUTION;
        
        if (frequency_sweep_single_point(freq)) {
            stats->successful_points++;
        } else {
            stats->failed_points++;
        }
    }
    
    stats->total_time_ms = to_ms_since_boot(get_absolute_time()) - start_time;
    stats->avg_time_per_point_ms = (float)stats->total_time_ms / stats->total_points;
    
    printf("[SWEEP] Estadísticas:\n");
    printf("  - Total: %lu puntos\n", stats->total_points);
    printf("  - Exitosos: %lu\n", stats->successful_points);
    printf("  - Fallidos: %lu\n", stats->failed_points);
    printf("  - Tiempo total: %lu ms\n", stats->total_time_ms);
    printf("  - Tiempo promedio: %.2f ms/punto\n", stats->avg_time_per_point_ms);
}

bool frequency_sweep_single_point(float frequency_hz) {
    printf("[SWEEP] Midiendo punto único: %.2f Hz\n", frequency_hz);
    
    // Configurar generador
    ad9833_set_frequency(frequency_hz);
    sleep_ms(100);
    
    // Adquirir
    adc_dma_start_capture();
    adc_dma_wait_complete();
    
    // Procesar
    goertzel_result_t result;
    goertzel_compute(
        adc_sample_buffer,
        WINDOW_SIZE,
        frequency_hz,
        SAMPLE_RATE,
        &result
    );
    
    // Transmitir
    return mqtt_publish_measurement(frequency_hz, result.magnitude_db, result.phase_deg);
}
