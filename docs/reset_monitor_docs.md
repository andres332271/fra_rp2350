# Script de Reinicio y Monitoreo del Puerto Serie

## Descripción

El script `reset_and_monitor.sh` permite reiniciar el Raspberry Pi Pico y abrir automáticamente el puerto serie para capturar todos los mensajes desde el inicio de la ejecución. Esto resuelve el problema de pérdida de mensajes iniciales que ocurre cuando se abre el puerto serie manualmente después del reinicio.

## Funcionamiento

El script ejecuta las siguientes operaciones en secuencia:

1. Reinicia el Pico utilizando `picotool reboot -f`
2. Realiza polling activo del dispositivo `/dev/ttyACM0` con intervalos de 100 ms
3. Una vez detectado el puerto serie, espera 200 ms adicionales para asegurar estabilidad
4. Abre `picocom` configurado a 115200 baudios

El timeout de espera está configurado en 5 segundos. Si el dispositivo no aparece en este período, el script termina con un mensaje de error.

## Uso

Para ejecutar el script:

```bash
chmod +x reset_and_monitor.sh
./reset_and_monitor.sh
```

Para salir de `picocom`, usar la secuencia `Ctrl+A` seguido de `Ctrl+X`.

## Requisitos

- `picotool`: Herramienta oficial de Raspberry Pi para interactuar con el Pico
- `picocom`: Terminal serie ligero para Linux

Ambas herramientas deben estar instaladas y disponibles en el PATH del sistema.

## Configuración

Las siguientes variables pueden modificarse al inicio del script según las necesidades:

- `DEVICE`: Puerto serie del Pico (por defecto `/dev/ttyACM0`)
- `BAUD_RATE`: Velocidad de comunicación (por defecto 115200)
- `TIMEOUT`: Tiempo máximo de espera en segundos (por defecto 5)