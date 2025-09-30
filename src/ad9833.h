/**
 * @file ad9833.h
 * @brief Módulo de control del generador DDS AD9833
 * 
 * Este módulo implementa el protocolo SPI para configuración del AD9833
 * y proporciona funciones de alto nivel para generación de señales senoidales.
 */

#ifndef AD9833_H
#define AD9833_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Tipo de forma de onda del AD9833
 */
typedef enum {
    AD9833_WAVEFORM_SINE = 0,      ///< Onda senoidal
    AD9833_WAVEFORM_TRIANGLE = 1,  ///< Onda triangular
    AD9833_WAVEFORM_SQUARE = 2     ///< Onda cuadrada (MSB del DAC)
} ad9833_waveform_t;

/**
 * @brief Inicializa el módulo AD9833
 * 
 * Configura:
 * - Interfaz SPI
 * - Pines GPIO para CS
 * - Resetea el chip
 * - Configura modo senoidal por defecto
 * 
 * @return true si la inicialización fue exitosa, false en caso contrario
 */
bool ad9833_init(void);

/**
 * @brief Configura la frecuencia de salida del AD9833
 * 
 * Calcula la palabra de frecuencia de 28 bits y la programa en el chip.
 * La frecuencia se configura con precisión de ~0.09 Hz.
 * 
 * @param freq_hz Frecuencia deseada en Hz (rango: 0 - 12.5 MHz)
 */
void ad9833_set_frequency(float freq_hz);

/**
 * @brief Configura el tipo de forma de onda
 * 
 * @param waveform Tipo de forma de onda deseada
 */
void ad9833_set_waveform(ad9833_waveform_t waveform);

/**
 * @brief Habilita o deshabilita la salida del AD9833
 * 
 * @param enable true para habilitar, false para deshabilitar (salida a GND)
 */
void ad9833_enable_output(bool enable);

/**
 * @brief Resetea el AD9833 a estado inicial
 */
void ad9833_reset(void);

#endif // AD9833_H
