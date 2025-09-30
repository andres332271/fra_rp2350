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

- Raspberry Pi Pico 2 W
- Módulo generador AD9833 con cristal de 25 MHz
- Etapa de acondicionamiento analógico (ver documentación de diseño)
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

#### 3. Herramientas de Monitoring MQTT (Opcional)

```bash
# Cliente de línea de comandos para testing
sudo pacman -S mosquitto-clients

# Suscribirse a todos los topics para ver mensajes
mosquitto_sub -h localhost -t "fra/#" -v
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

### 3. Compilar el Proyecto

```bash
# Crear directorio de build
mkdir build
cd build

# Configurar CMake
cmake ..

# Compilar
make -j$(nproc)
```

Si la compilación es exitosa, obtendrás `fra_rp2350.uf2` en el directorio `build/`.

## Programación del Pico 2 W

### Modo BOOTSEL (Programación Inicial)

1. Desconectar el Pico 2 W del USB
2. Mantener presionado el botón BOOTSEL
3. Conectar el USB mientras mantienes el botón
4. Soltar el botón - el Pico aparece como dispositivo de almacenamiento masivo
5. Copiar el archivo `.uf2`:

```bash
# El dispositivo se monta automáticamente, usualmente en /run/media/$USER/RPI-RP2
cp build/fra_rp2350.uf2 /run/media/$USER/RPI-RP2/

# El Pico se reinicia automáticamente y ejecuta el firmware
```

### Debug Serial via USB

Para ver la salida de debug:

```bash
# Verificar el puerto asignado (usualmente /dev/ttyACM0)
ls /dev/ttyACM*

# Conectar con screen (instalar si no lo tienes: sudo pacman -S screen)
screen /dev/ttyACM0 115200

# Alternativamente con minicom
sudo pacman -S minicom
minicom -D /dev/ttyACM0 -b 115200

# Para salir de screen: Ctrl+A luego K, confirmar con Y
```

## Arquitectura del Código

```
src/
├── main.c           - Inicialización y loop principal
├── config.h         - Configuración del sistema (git-ignored)
├── adc_dma.c/h      - Adquisición ADC con DMA
├── ad9833.c/h       - Control del generador DDS
├── goertzel.c/h     - Algoritmo DSP
├── mqtt_client.c/h  - Cliente MQTT
└── sweep.c/h        - Orquestador del barrido
```

### Flujo de Ejecución

1. **Inicialización** (`main.c`)
   - Configurar hardware (ADC, SPI, WiFi)
   - Conectar a WiFi y broker MQTT
   - Inicializar AD9833 en modo senoidal

2. **Barrido de Frecuencia** (`sweep.c`)
   - Para cada frecuencia objetivo (100 Hz a 20 kHz, paso 100 Hz):
     - Configurar AD9833
     - Adquirir 480 muestras con ADC+DMA
     - Procesar con Goertzel
     - Transmitir resultado via MQTT

3. **Visualización**
   - Los datos se publican en el topic `fra/measurements`
   - Formato JSON: `{"freq":1000.0,"mag":-3.45,"phase":-87.3}`

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

Conectar osciloscopio a estos pines para análisis de timing.

### Recomendaciones de Desarrollo

1. **Desarrollo incremental**: Implementar y validar un módulo antes de continuar
2. **Testing aislado**: Usar señales sintéticas para validar Goertzel antes de integrar ADC
3. **Coherencia**: Verificar con osciloscopio que frecuencias del AD9833 son exactas

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

# Si no está, agregar a ~/.bashrc:
export PICO_SDK_PATH=~/pico-sdk
source ~/.bashrc
```

### No conecta a WiFi

- Verificar SSID y password en `src/config.h`
- El Pico 2 W solo soporta WiFi 2.4 GHz (no 5 GHz)
- Verificar que el router permite nuevas conexiones

### MQTT no conecta

```bash
# Verificar que Mosquitto está corriendo
systemctl status mosquitto

# Verificar que acepta conexiones
mosquitto_pub -h localhost -t test -m "hello"

# Verificar firewall (si aplica)
sudo firewall-cmd --list-all
```

## Próximos Pasos

Ver `docs/implementation_notes.md` para detalles de implementación de cada módulo.

## Referencias

- [Pico SDK Documentation](https://www.raspberrypi.com/documentation/pico-sdk/)
- [RP2350 Datasheet](https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf)
- Documentación técnica del proyecto en `docs/`

## Licencia

[Especificar licencia]

## Autores

- Tomás Alfonzo
- Andrés Villarreal
