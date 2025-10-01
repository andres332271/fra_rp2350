/**
 * @file mqtt_client.h
 * @brief Módulo de cliente MQTT para transmisión de datos
 * 
 * Implementa cliente MQTT sobre lwIP para transmisión inalámbrica
 * de mediciones al servidor de visualización.
 */

#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Estructura de configuración MQTT
 */
typedef struct {
    const char *broker_addr;    ///< Dirección IP del broker
    uint16_t broker_port;       ///< Puerto del broker (típicamente 1883)
    const char *client_id;      ///< Identificador único del cliente
    const char *topic;          ///< Topic para publicación de mediciones
} mqtt_config_t;

/**
 * @brief Inicializa el cliente MQTT y conecta al broker
 * 
 * @param config Puntero a estructura de configuración
 * @return true si la conexión fue exitosa, false en caso contrario
 */
bool mqtt_init(const mqtt_config_t *config);

/**
 * @brief Publica una medición en formato JSON
 * 
 * Serializa los datos de medición a JSON y los publica en el topic configurado.
 * Formato: {"freq":1000.0,"mag":-3.45,"phase":-87.3}
 * 
 * @param frequency_hz Frecuencia medida (Hz)
 * @param magnitude_db Magnitud en dB
 * @param phase_deg Fase en grados
 * @return true si la publicación fue exitosa, false en caso contrario
 */
bool mqtt_publish_measurement(
    float frequency_hz,
    float magnitude_db,
    float phase_deg
);

/**
 * @brief Publica mensaje de estado del sistema
 * 
 * @param status_msg Mensaje de estado a publicar
 * @return true si la publicación fue exitosa, false en caso contrario
 */
bool mqtt_publish_status(const char *status_msg);

/**
 * @brief Verifica si el cliente está conectado al broker
 * @return true si está conectado, false en caso contrario
 */
bool mqtt_is_connected(void);

/**
 * @brief Intenta reconectar al broker MQTT
 * @return true si la reconexión fue exitosa, false en caso contrario
 */
bool mqtt_reconnect(void);

/**
 * @brief Desconecta del broker MQTT limpiamente
 */
void fra_mqtt_disconnect(void);

#endif // MQTT_CLIENT_H
