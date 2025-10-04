# FRA RP2350 - Analizador de Respuesta en Frecuencia

Analizador de respuesta en frecuencia basado en Raspberry Pi Pico 2 W (RP2350) para caracterización de circuitos analógicos en el rango de audio (100 Hz - 20 kHz).

## Características

- Barrido de frecuencia con 200 puntos de medición (resolución de 100 Hz)
- Algoritmo de Goertzel para detección eficiente de tonos individuales
- Coherencia de muestreo para eliminación de fuga espectral sin ventanas
- Generación de señal mediante AD9833 DDS
- Transmisión de datos vía MQTT sobre WiFi
- Procesamiento DSP en punto flotante aprovechando FPU del Cortex-M33

## Requisitos del Sistema

### Hardware

- Raspberry Pi Pico 2 W (RP2350)
- Módulo generador AD9833 con cristal de 25 MHz
- Etapa de acondicionamiento analógico (ver documentación de diseño en `docs/`)
- Cable USB para programación y debug serial

### Software - Toolchain de Desarrollo

#### 1. Pico SDK

```bash
# Instalar dependencias
sudo pacman -S cmake gcc-arm-none-eabi libnewlib arm-none-eabi-gdb git

# Clonar Pico SDK (versión 2.0.0 o posterior)
cd ~
git clone https://github.com/raspberrypi/pico-sdk.git
cd pico-sdk
git checkout 2.0.0
git submodule update --init

# Configurar variable de entorno (agregar a ~/.bashrc)
export PICO_SDK_PATH=~/pico-sdk
```

#### 2. Broker MQTT (Mosquitto)

```bash
# Instalar Mosquitto
sudo pacman -S mosquitto

# Habilitar y arrancar el servicio
sudo systemctl enable mosquitto
sudo systemctl start mosquitto

# Verificar que está corriendo
mosquitto -v
```

#### 3. Herramientas Adicionales (Opcional)

```bash
# Cliente MQTT para testing
sudo pacman -S mosquitto-clients

# Herramienta screen para monitoreo serial
sudo pacman -S screen

# Picotool para flasheo avanzado (opcional)
sudo pacman -S picotool
```

## Configuración del Proyecto

### 1. Clonar el Repositorio

```bash
git clone <tu-repositorio>
cd fra_rp2350
```

### 2. Crear Archivo de Configuración

```bash
# Copiar template de configuración
cp config.h.example src/config.h

# Editar con tus credenciales WiFi y configuración
nano src/config.h
```

**IMPORTANTE:** Editar `src/config.h` con:
- Tu SSID y password de WiFi
- La IP de tu PC donde corre Mosquitto (ejemplo: `192.168.1.100`)
- Otros parámetros según tu setup

## Compilación y Flasheo

El proyecto incluye scripts automatizados para facilitar el desarrollo. Para detalles completos sobre cada script, consultar la carpeta `docs/`.

### Script Principal: build.sh

Script unificado para compilación, flasheo y monitoreo serial.

```bash
# Compilar el proyecto
./build.sh

# Limpiar y recompilar
./build.sh clean

# Flashear firmware al Pico (requiere modo BOOTSEL)
./build.sh flash

# Monitorear salida serial
./build.sh monitor
```

**Nota:** Ver `docs/build_script.md` para documentación detallada del script.

### Scripts Adicionales

```bash
# Flasheo avanzado con picotool (sin modo BOOTSEL manual)
./flash_pico.sh

# Reinicio y captura serial desde el inicio
./reset_and_monitor.sh
```

Consultar `docs/flash_pico.md` y `docs/reset_and_monitor.md` para más información.

## Estructura del Proyecto

```
fra_rp2350/
├── CMakeLists.txt              # Configuración de build
├── pico_sdk_import.cmake       # Import del SDK oficial
├── build.sh                    # Script principal de build
├── flash_pico.sh               # Script de flasheo con picotool
├── reset_and_monitor.sh        # Script de reinicio y monitoreo
├── config.h.example            # Template de configuración
│
├── src/                        # Código fuente (archivos .c)
│   ├── main.c                  # Punto de entrada, inicialización
│   ├── config.h                # Configuración (git-ignored)
│   ├── sweep.c                 # Orquestador del barrido
│   ├── ad9833.c                # Control del generador DDS
│   ├── adc_dma.c               # Adquisición ADC con DMA
│   ├── goertzel.c              # Algoritmo DSP
│   └── mqtt_client.c           # Cliente MQTT
│
├── include/                    # Headers del proyecto
│   ├── sweep.h                 # Header del orquestador
│   ├── ad9833.h                # Header del generador DDS
│   ├── adc_dma.h               # Header de ADC+DMA
│   ├── goertzel.h              # Header de DSP
│   ├── mqtt_client.h           # Header de MQTT
│   └── lwipopts.h              # Configuración lwIP
│
├── test/                       # Tests unitarios (futuro)
│   ├── test_goertzel.c
│   ├── test_adc_timing.c
│   └── test_coherence.c
│
└── docs/                       # Documentación técnica
    ├── build_script.md         # Doc del script build.sh
    ├── flash_pico.md           # Doc del script de flasheo
    ├── reset_and_monitor.md    # Doc del script de monitoreo
    ├── implementation_notes.md # Notas de implementación
    ├── arquitectura.md         # Diseño de arquitectura DSP
    └── acondicionamiento.md    # Diseño analógico
```

## Flujo de Desarrollo

### 1. Ciclo de Desarrollo Típico

```bash
# Editar código
nano src/main.c

# Compilar
./build.sh

# Flashear (poner Pico en modo BOOTSEL primero)
./build.sh flash

# Monitorear salida
./build.sh monitor
```

### 2. Workflow Alternativo con flash_pico.sh

```bash
# Editar código
nano src/main.c

# Compilar
./build.sh

# Flashear sin necesidad de BOOTSEL manual (requiere picotool)
./flash_pico.sh

# Ver salida desde el reinicio
./reset_and_monitor.sh
```

## Arquitectura del Firmware

### Flujo de Ejecución

1. **Inicialización** (`main.c`)
   - Configurar hardware (ADC, SPI, GPIO)
   - Inicializar WiFi con sistema de reintentos
   - Conectar a broker MQTT
   - Configurar AD9833 en modo senoidal

2. **Barrido de Frecuencia** (`sweep.c`)
   - Para cada frecuencia objetivo (100 Hz a 20 kHz, paso 100 Hz):
     - Configurar AD9833 a la frecuencia deseada
     - Esperar tiempo de asentamiento
     - Adquirir 480 muestras con ADC+DMA
     - Procesar con algoritmo de Goertzel
     - Calcular magnitud en dB y fase en grados
     - Transmitir resultado via MQTT

3. **Visualización**
   - Los datos se publican en el topic `fra/measurements`
   - Formato JSON: `{"freq":1000.0,"mag":-3.45,"phase":-87.3}`
   - Servidor web Node.js + D3.js/Chart.js para diagrama de Bode

## Debugging y Desarrollo

### Logs de Debug

El sistema imprime logs via USB serial según el nivel configurado en `config.h`:

```c
#define DEBUG_LEVEL 2  // 0=errores, 1=+warnings, 2=+info, 3=todo
```

### Instrumentación con GPIO

Si se habilita `DEBUG_GPIO_ENABLED` en `config.h`, los pines GPIO togglean durante eventos clave:
- GPIO 15: Inicio de barrido
- GPIO 16: Adquisición ADC activa
- GPIO 17: Procesamiento DSP activo
- GPIO 18: Transmisión MQTT activa

Útil para análisis de timing con osciloscopio.

### Recomendaciones de Desarrollo

1. **Desarrollo incremental**: Implementar y validar un módulo antes de continuar
2. **Testing aislado**: Usar señales sintéticas para validar Goertzel antes de integrar ADC
3. **Coherencia de muestreo**: Verificar con osciloscopio que frecuencias del AD9833 son exactas

## Troubleshooting

### El Pico no aparece como /dev/ttyACM0

```bash
# Verificar que el device está conectado
lsusb | grep "Raspberry Pi"

# Verificar permisos (agregar tu usuario al grupo uucp o dialout)
sudo usermod -a -G uucp $USER
# Logout y login para aplicar cambios
```

### Error de compilación: "PICO_SDK_PATH not defined"

```bash
# Verificar que la variable está definida
echo $PICO_SDK_PATH

# Si no está definida, agregar a ~/.bashrc:
export PICO_SDK_PATH=~/pico-sdk
source ~/.bashrc
```

### Problemas de Conexión WiFi

El Pico 2 W incluye mejoras para estabilidad de conexión WiFi. Si experimentas problemas:

**Verificaciones básicas:**
- Confirmar SSID y password en `src/config.h`
- El Pico 2 W solo soporta WiFi 2.4 GHz (no 5 GHz)
- Verificar que el router permite nuevas conexiones

**Errores comunes del chip CYW43:**

El firmware implementa un sistema robusto de conexión WiFi con:
- Configuración automática de país/región para acceso completo a canales
- Sistema de 3 reintentos con timeouts de 20 segundos
- Diagnóstico específico de errores CYW43

**Códigos de error:**
- `Error -1` (LINK_FAIL): Fallo general de conexión. Verificar hardware y antena.
- `Error -2` (LINK_NONET): Red no encontrada. Verificar SSID y que la red está disponible.
- `Error -3` (LINK_BADAUTH): Autenticación fallida. Verificar password WiFi.

**Depuración adicional:**
- Monitorear salida serial durante conexión con `./build.sh monitor`
- Los tiempos de conexión varían entre 300ms y 12s (normal)
- Si falla después de 3 intentos, verificar distancia al router y posibles interferencias
- Asegurar que el router no tiene filtrado MAC activo

**Configuración del router:**
- Preferir canales WiFi 1, 6 u 11 (menos interferencia)
- Deshabilitar temporalmente firewall/filtros para testing
- Verificar que DHCP está habilitado

### MQTT no conecta

```bash
# Verificar que Mosquitto está corriendo
systemctl status mosquitto

# Verificar que acepta conexiones
mosquitto_pub -h localhost -t test -m "hello"

# Si hay firewall activo, verificar reglas
sudo firewall-cmd --list-all
```

### El script build.sh no ejecuta

```bash
# Dar permisos de ejecución
chmod +x build.sh flash_pico.sh reset_and_monitor.sh
```

## Próximos Pasos

- Ver `docs/implementation_notes.md` para detalles de implementación de cada módulo
- Consultar `docs/arquitectura.md` para especificación completa del sistema DSP
- Revisar `docs/acondicionamiento.md` para diseño del circuito analógico

## Referencias

- [Pico SDK Documentation](https://www.raspberrypi.com/documentation/pico-sdk/)
- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)
- [AD9833 Datasheet](https://www.analog.com/en/products/ad9833.html)
- Documentación técnica completa del proyecto en `docs/`

## Licencia

Este proyecto está licenciado bajo la Licencia MIT - ver el archivo [LICENSE](LICENSE) para más detalles.

## Autores

- Tomás Alfonzo
- Andrés Villarreal