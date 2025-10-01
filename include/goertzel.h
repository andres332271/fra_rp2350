/**
 * @file goertzel.h
 * @brief Módulo de procesamiento DSP - Algoritmo de Goertzel
 * 
 * Implementa el algoritmo de Goertzel en punto flotante de 32 bits
 * para detección eficiente de tonos individuales.
 */

#ifndef GOERTZEL_H
#define GOERTZEL_H

#include <stdint.h>

/**
 * @brief Estructura de resultado del análisis de Goertzel
 */
typedef struct {
    float magnitude;        ///< Magnitud absoluta
    float magnitude_db;     ///< Magnitud en dB (20*log10)
    float phase_rad;        ///< Fase en radianes
    float phase_deg;        ///< Fase en grados
} goertzel_result_t;

/**
 * @brief Ejecuta el algoritmo de Goertzel sobre un buffer de muestras
 * 
 * Procesa el buffer de muestras ADC y calcula la magnitud y fase
 * de la frecuencia objetivo especificada.
 * 
 * IMPORTANTE: 
 * - Usar sufijo 'f' en todos los literales para forzar precisión simple
 * - Las muestras se normalizan internamente de [0, 4095] a [-1, +1]
 * 
 * @param samples Buffer de muestras ADC (uint16_t, rango 0-4095)
 * @param num_samples Número de muestras en el buffer (típicamente 480)
 * @param target_freq_hz Frecuencia objetivo a detectar (Hz)
 * @param sample_rate_hz Frecuencia de muestreo del ADC (típicamente 48000 Hz)
 * @param result Puntero a estructura donde se almacenará el resultado
 */
void goertzel_compute(
    const uint16_t *samples,
    uint16_t num_samples,
    float target_freq_hz,
    float sample_rate_hz,
    goertzel_result_t *result
);

/**
 * @brief Versión de testing con señal sintética
 * 
 * Genera internamente una senoide de frecuencia conocida y ejecuta
 * Goertzel sobre ella para validación del algoritmo.
 * 
 * @param test_freq_hz Frecuencia de la senoide de prueba
 * @param num_samples Número de muestras a generar
 * @param sample_rate_hz Frecuencia de muestreo
 * @param result Puntero a estructura donde se almacenará el resultado
 */
void goertzel_test_synthetic(
    float test_freq_hz,
    uint16_t num_samples,
    float sample_rate_hz,
    goertzel_result_t *result
);

#endif // GOERTZEL_H
