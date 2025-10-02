# Script de Flasheo Automático para Raspberry Pi Pico 2W

## Descripción

El script `flash_pico.sh` automatiza el proceso de flasheo del firmware `fra_rp2350` en la Raspberry Pi Pico 2W, eliminando la necesidad de presionar manualmente el botón BOOTSEL y proporcionando verificación automática de integridad.

## Requisitos Previos

### Dependencias del Sistema

- **Sistema Operativo**: Linux (probado en Manjaro)
- **picotool**: Herramienta oficial de Raspberry Pi para interacción con dispositivos RP2040/RP2350

### Instalación de picotool en Manjaro/Arch

```bash
yay -S picotool
# o
paru -S picotool
```

Para otras distribuciones, consultar la documentación oficial en el repositorio de Raspberry Pi.

### Configuración del Firmware

El firmware debe estar compilado con soporte USB stdio para aprovechar todas las funcionalidades del script. En el archivo `CMakeLists.txt` del proyecto, asegurarse de incluir:

```cmake
pico_enable_stdio_usb(fra_rp2350 1)
```

Esta configuración permite la comunicación USB bidireccional y habilita funcionalidades avanzadas como la verificación sin necesidad de modo bootloader.

## Instalación del Script

1. Copiar el script `flash_pico.sh` en la raíz del repositorio del proyecto
2. Otorgar permisos de ejecución:

```bash
chmod +x flash_pico.sh
```

## Uso

### Ejecución Básica

Desde la raíz del repositorio, después de compilar el proyecto:

```bash
./flash_pico.sh
```

### Flujo de Operación

El script ejecuta los siguientes pasos automáticamente:

1. **Verificación del archivo UF2**: Confirma la existencia del binario compilado en `build/fra_rp2350.uf2`
2. **Detección del dispositivo**: Identifica la Raspberry Pi Pico 2W conectada
3. **Comparación de binarios**: Verifica si el firmware actual es idéntico al que se desea flashear
4. **Flasheo**: Carga el nuevo firmware en el dispositivo
5. **Verificación de integridad**: Compara byte a byte el contenido flasheado con el archivo fuente
6. **Confirmación**: Verifica que el dispositivo reinició correctamente y está operativo

### Comportamiento Interactivo

Cuando el script detecta que el binario a flashear es idéntico al que ya está en el dispositivo, presenta un mensaje de advertencia y solicita confirmación:

```
⚠ El binario actual es idéntico al archivo a flashear
  No es necesario reflashear (evita desgaste de memoria)

¿Deseas flashear de todos modos? (s/N):
```

**Opciones de respuesta:**
- `N` o `Enter`: Cancela el flasheo (opción por defecto, recomendada)
- `s`: Procede con el flasheo a pesar de ser idéntico

Esta funcionalidad evita escrituras innecesarias en la memoria flash, prolongando su vida útil y reduciendo el tiempo de operación.

## Características Principales

### Flasheo sin BOOTSEL Manual

El script utiliza el comando `picotool load -f` que fuerza automáticamente el reinicio del dispositivo en modo bootloader, eliminando la necesidad de presionar manualmente el botón BOOTSEL en la mayoría de los casos. Esta funcionalidad requiere que el firmware tenga habilitado USB stdio.

### Verificación de Integridad

Implementa dos niveles de verificación:

- **Pre-flasheo**: Compara el firmware actual con el archivo a flashear para evitar operaciones innecesarias
- **Post-flasheo**: Verifica byte a byte que el contenido flasheado coincide exactamente con el archivo fuente

### Manejo Robusto de Errores

El script detecta y reporta claramente diversos escenarios de error:

- Ausencia del archivo UF2 compilado
- Dispositivo no conectado o no detectado
- Fallos durante el proceso de flasheo
- Problemas en la verificación de integridad

### Información Detallada del Dispositivo

Muestra información técnica relevante del dispositivo antes y después del flasheo, incluyendo:

- Nombre del programa
- Características habilitadas (USB stdio)
- Direcciones de memoria del binario
- Tipo de chip (RP2350)
- Tipo de imagen (ARM Secure)

## Casos de Uso Comunes

### Desarrollo Iterativo

Durante el desarrollo activo, el script permite ciclos rápidos de compilación-flasheo-prueba sin intervención manual del botón BOOTSEL.

### Verificación de Builds

Útil para confirmar que el firmware compilado se flasheó correctamente y coincide exactamente con el binario generado.

### Prevención de Flasheos Redundantes

Evita automáticamente reflashear el mismo firmware, ahorrando tiempo y ciclos de escritura en la memoria flash.

## Limitaciones Conocidas

### Primera Vez o Modo Bootloader

Si el dispositivo está en modo bootloader (botón BOOTSEL presionado al conectar) o es la primera vez que se flashea, la comparación pre-flasheo no será posible. En estos casos, el script procederá directamente con el flasheo sin solicitar confirmación.

### Firmware sin USB Stdio

Si el firmware actual no tiene habilitado USB stdio, algunas funcionalidades avanzadas pueden no estar disponibles:

- La comparación pre-flasheo no funcionará
- La información post-flasheo puede ser limitada
- Será necesario usar el botón BOOTSEL manualmente para futuros flasheos

### Múltiples Dispositivos

El script asume que solo hay un dispositivo Raspberry Pi Pico conectado. Si hay múltiples dispositivos conectados simultáneamente, el comportamiento puede ser impredecible.

## Solución de Problemas

### Error de Permisos USB

Si el script reporta errores de permisos al intentar acceder al dispositivo, puede ser necesario configurar reglas udev apropiadas para el acceso sin privilegios de superusuario.

### Dispositivo No Detectado

Verificar que:
- El cable USB soporta transferencia de datos (no solo carga)
- El dispositivo está correctamente conectado
- picotool está instalado y accesible en el PATH del sistema

### Verificación Falla Post-Flasheo

Si la verificación posterior al flasheo falla consistentemente, pero el dispositivo funciona correctamente, es posible que el firmware reinicie muy rápidamente. Este es un comportamiento esperado y no indica un problema.

## Salida de Ejemplo

### Flasheo Exitoso con Binario Diferente

```
=========================================
  Flasheo automático Raspberry Pi Pico 2W
=========================================

[1/5] Verificando archivo .uf2...
✓ Archivo encontrado: build/fra_rp2350.uf2
  Tamaño: 744K

[2/5] Detectando Raspberry Pi Pico...
✓ Pico detectado

Información del dispositivo:
Program Information
 name:          fra_rp2350
 features:      USB stdin / stdout
 binary start:  0x10000000
 binary end:    0x1005c834
 target chip:   RP2350
 image type:    ARM Secure

[3/6] Verificando si es necesario reflashear...
✓ El binario es diferente o el dispositivo está en bootloader
  Procediendo con el flasheo...

[4/6] Flasheando dispositivo...
Family ID 'rp2350-arm-s' can be downloaded in absolute space:
  00000000->02000000
Loading into Flash:   [==============================]  100%
✓ Flasheo completado exitosamente

[5/6] Verificando integridad del flasheo...
Verifying Flash:      [==============================]  100%
  OK
✓ Verificación exitosa: el contenido coincide

[6/6] Verificando presencia del dispositivo...
✓ Dispositivo detectado después del flasheo

Estado del dispositivo:
Program Information
 name:          fra_rp2350
 features:      USB stdin / stdout
 binary start:  0x10000000
 binary end:    0x1005c834
 target chip:   RP2350
 image type:    ARM Secure

=========================================
  ¡Flasheo completado con éxito!
=========================================
```

### Detección de Binario Idéntico

```
[3/6] Verificando si es necesario reflashear...
⚠ El binario actual es idéntico al archivo a flashear
  No es necesario reflashear (evita desgaste de memoria)

¿Deseas flashear de todos modos? (s/N): n
Flasheo cancelado por el usuario

[4/6] Flasheo omitido

=========================================
  Operación completada (sin cambios)
=========================================
```

## Referencias

- [Documentación oficial de picotool](https://github.com/raspberrypi/picotool)
- [Raspberry Pi Pico SDK Documentation](https://www.raspberrypi.com/documentation/microcontrollers/c_sdk.html)
- [Getting Started with Raspberry Pi Pico](https://datasheets.raspberrypi.com/pico/getting-started-with-pico.pdf)
