/**
 * @file lwipopts.h
 * @brief Configuración de lwIP para FRA RP2350
 * 
 * Configuración mínima para habilitar:
 * - TCP (requerido para MQTT)
 * - DHCP (asignación automática de IP)
 * - DNS (resolución de nombres)
 */

#ifndef LWIPOPTS_H
#define LWIPOPTS_H

// --- Configuración de threading ---
// Usar el modo background thread (compatible con pico_cyw43_arch_lwip_threadsafe_background)
#define NO_SYS                      1
#define LWIP_SOCKET                 0
#define LWIP_NETCONN                0
#define LWIP_TCPIP_CORE_LOCKING     0

// --- Protocolos habilitados ---
#define LWIP_IPV4                   1
#define LWIP_IPV6                   0
#define LWIP_TCP                    1
#define LWIP_UDP                    1
#define LWIP_DHCP                   1
#define LWIP_DNS                    1

// --- Configuración de memoria ---
#define MEM_LIBC_MALLOC             0
#define MEMP_NUM_TCP_PCB            8
#define MEMP_NUM_TCP_PCB_LISTEN     8
#define MEMP_NUM_TCP_SEG            32
#define MEMP_NUM_PBUF               24
#define PBUF_POOL_SIZE              24

// --- Tamaños de buffers ---
#define TCP_MSS                     1460
#define TCP_WND                     (8 * TCP_MSS)
#define TCP_SND_BUF                 (8 * TCP_MSS)
#define LWIP_WND_SCALE              1
#define TCP_RCV_SCALE               0
#define PBUF_POOL_BUFSIZE           LWIP_MEM_ALIGN_SIZE(TCP_MSS + 40 + PBUF_LINK_ENCAPSULATION_HLEN + PBUF_LINK_HLEN)

// --- Timeouts ---
#define LWIP_NETIF_LINK_CALLBACK    1
#define LWIP_NETIF_STATUS_CALLBACK  1

// --- DNS ---
#define DNS_TABLE_SIZE              4
#define DNS_MAX_NAME_LENGTH         256

// --- DHCP ---
#define LWIP_DHCP_CHECK_LINK_UP     1

// --- Estadísticas y debug (deshabilitado para producción) ---
#define LWIP_STATS                  0
#define LWIP_STATS_DISPLAY          0

// --- Checksums (dejar en hardware si es posible) ---
#define CHECKSUM_GEN_IP             1
#define CHECKSUM_GEN_UDP            1
#define CHECKSUM_GEN_TCP            1
#define CHECKSUM_CHECK_IP           1
#define CHECKSUM_CHECK_UDP          1
#define CHECKSUM_CHECK_TCP          1

#endif // LWIPOPTS_H
