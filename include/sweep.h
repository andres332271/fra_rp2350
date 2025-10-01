/**
 * @file sweep.h
 * @brief Módulo orquestador del barrido de frecuencia
 * 
 * Coordina todos los subsistemas para ejecutar el barrido completo
 * de frecuencias desde 100 Hz hasta 20 kHz con resolución de 100 Hz.
 */

#ifndef SWEEP_H
#define SWEEP_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Estadísticas del barrido
 */
typedef struct {
    uint32_t total_points;          ///< Número total de puntos medidos
    uint32_t successful_points;     ///< Puntos con medición exitosa
    uint32_t failed_points;         ///< Puntos con error
    uint32_t total_time_ms;         ///< Tiempo total del barrido (ms)
    float avg_time_per_point_ms;    ///< Tiempo promedio por punto (ms)
} sweep_stats_t;

/**
 * @brief Ejecuta un barrido completo de frecuencia
 * 
 * Realiza medición secuencial de 200 puntos desde 100 Hz hasta 20 kHz.
 * Para cada punto:
 * 1. Configura AD9833 a frecuencia objetivo
 * 2. Espera estabilización
 * 3. Adquiere 480 muestras con ADC+DMA
 * 4. Procesa con Goertzel
 * 5. Transmite resultado via MQTT
 * 
 * Esta función es bloqueante y toma aproximadamente 3-4 segundos.
 */
void frequency_sweep_execute(void);

/**
 * @brief Ejecuta un barrido con recolección de estadísticas
 * 
 * Igual que frequency_sweep_execute() pero recolecta estadísticas
 * de rendimiento y las almacena en la estructura proporcionada.
 * 
 * @param stats Puntero a estructura donde se almacenarán las estadísticas
 */
void frequency_sweep_execute_with_stats(sweep_stats_t *stats);

/**
 * @brief Ejecuta medición de un solo punto de frecuencia
 * 
 * Útil para testing o mediciones aisladas.
 * 
 * @param frequency_hz Frecuencia a medir
 * @return true si la medición fue exitosa, false en caso contrario
 */
bool frequency_sweep_single_point(float frequency_hz);

#endif // SWEEP_H
