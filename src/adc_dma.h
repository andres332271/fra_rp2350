/**
 * @file adc_dma.h
 * @brief Módulo de adquisición de datos ADC con DMA
 * 
 * Este módulo maneja la configuración del ADC para muestreo a 48 kHz
 * y utiliza DMA para transferir automáticamente las muestras a un buffer
 * en memoria sin carga significativa del CPU.
 */

#ifndef ADC_DMA_H
#define ADC_DMA_H

#include <stdint.h>
#include <stdbool.h>
#include "config.h"

// Buffer de muestras ADC (global, accesible desde otros módulos)
extern uint16_t adc_sample_buffer[WINDOW_SIZE];

/**
 * @brief Inicializa el sistema ADC+DMA
 * 
 * Configura:
 * - ADC en modo free-running a 48 kHz
 * - FIFO del ADC para DMA
 * - Canal DMA para transferir WINDOW_SIZE muestras
 * 
 * @return true si la inicialización fue exitosa, false en caso contrario
 */
bool adc_dma_init(void);

/**
 * @brief Inicia la captura de un bloque de datos
 * 
 * Reinicia el canal DMA y comienza la adquisición de WINDOW_SIZE muestras.
 * Esta función retorna inmediatamente; usar adc_dma_wait_complete() para
 * esperar la finalización.
 */
void adc_dma_start_capture(void);

/**
 * @brief Espera a que se complete la captura actual
 * 
 * Función bloqueante que espera hasta que el DMA haya transferido
 * todas las WINDOW_SIZE muestras al buffer.
 */
void adc_dma_wait_complete(void);

/**
 * @brief Verifica si hay captura en progreso
 * @return true si el DMA está activo, false si completó
 */
bool adc_dma_is_busy(void);

/**
 * @brief Valida la calidad de las muestras capturadas
 * 
 * Verifica que no haya saturación excesiva (valores en 0 o 4095)
 * 
 * @param samples Buffer de muestras a validar
 * @param n Número de muestras
 * @return true si las muestras son válidas, false si hay problemas
 */
bool adc_dma_validate_samples(const uint16_t *samples, uint16_t n);

#endif // ADC_DMA_H
