# Notas de Implementación - FRA RP2350

## Estado Actual del Proyecto

### Estructura Completa ✓
- [x] Estructura de directorios
- [x] Sistema de build (CMake)
- [x] Configuración de proyecto
- [x] Headers de todos los módulos
- [x] Stubs compilables de todos los módulos

### Módulos Pendientes de Implementación

#### 1. ADC + DMA (Prioridad: ALTA)
**Archivo:** `src/adc_dma.c`

**Estado:** Stub básico - Genera datos sintéticos

**Tareas pendientes:**
- [ ] Configurar ADC con clock divider para 48 kHz exactos
- [ ] Configurar FIFO del ADC para DMA
- [ ] Configurar canal DMA para transferir 480 muestras
- [ ] Implementar interrupciones de DMA
- [ ] Validar timing con osciloscopio

**Referencias de implementación:**
- Ejemplo oficial del SDK: `pico-examples/adc/dma_capture`
- Cornell ECE4760: https://people.ece.cornell.edu/land/courses/ece4760/RP2350/

**Cálculo del Clock Divider:**
```c
// ADC clock = 48 MHz
// Desired sample rate = 48 kHz
// ADC requires 96 cycles per conversion
// clkdiv = 48MHz / (48kHz * 96) = 10.417
adc_set_clkdiv(10.417f * 96.0f);
```

#### 2. AD9833 DDS Generator (Prioridad: ALTA)
**Archivo:** `src/ad9833.c`

**Estado:** Stub - No implementado

**Tareas pendientes:**
- [ ] Configurar SPI a 1-5 MHz
- [ ] Implementar protocolo de escritura de 16 bits
- [ ] Calcular palabra de frecuencia de 28 bits
- [ ] Implementar secuencia de inicialización
- [ ] Implementar cambio de frecuencia
- [ ] Validar frecuencias con contador de frecuencia

**Protocolo SPI del AD9833:**
```c
// Frecuency register calculation:
// freq_word = (f_out * 2^28) / f_mclk
// f_mclk = 25 MHz

uint32_t freq_word = (uint32_t)((freq_hz * 268435456.0f) / 25000000.0f);
uint16_t lsb = (freq_word & 0x3FFF) | 0x4000;  // FREQ0 register
uint16_t msb = ((freq_word >> 14) & 0x3FFF) | 0x4000;

// Write sequence:
// 1. Control word with B28=1 (0x2100 for reset off, sine output)
// 2. LSB word
// 3. MSB word
```

#### 3. Algoritmo de Goertzel (Prioridad: MEDIA)
**Archivo:** `src/goertzel.c`

**Estado:** Stub - Retorna valores ficticios

**Tareas pendientes:**
- [ ] Implementar precálculo de coeficientes
- [ ] Implementar bucle IIR
- [ ] Implementar cálculo de magnitud y fase
- [ ] Usar sufijo 'f' en todos los literales
- [ ] Validar con señales sintéticas antes de integrar con ADC

**Pseudocódigo del algoritmo:**
```c
// Precompute coefficients
float omega = 2.0f * M_PI * target_freq / sample_rate;
float coeff = 2.0f * cosf(omega);
float cos_omega = cosf(omega);
float sin_omega = sinf(omega);

// IIR filter loop
float s_prev = 0.0f;
float s_prev2 = 0.0f;

for (int n = 0; n < N; n++) {
    float x = (samples[n] - 2048.0f) / 2048.0f;  // Normalize
    float s = x + coeff * s_prev - s_prev2;
    s_prev2 = s_prev;
    s_prev = s;
}

// Calculate result
float real = s_prev - s_prev2 * cos_omega;
float imag = s_prev2 * sin_omega;
float magnitude = sqrtf(real*real + imag*imag);
float phase = atan2f(imag, real);
```

#### 4. Cliente MQTT (Prioridad: BAJA inicialmente)
**Archivo:** `src/mqtt_client.c`

**Estado:** Stub - Imprime pero no transmite

**Tareas pendientes:**
- [ ] Integrar biblioteca lwIP MQTT
- [ ] Implementar callbacks de conexión
- [ ] Implementar callbacks de publicación
- [ ] Manejar reconexiones automáticas
- [ ] Implementar retry con backoff exponencial

**Notas:**
- Puede dejarse como stub inicialmente para validar el resto del sistema
- Los mensajes se imprimen por serial y pueden monitorearse
- Una vez validado el DSP, implementar MQTT es directo

## Orden de Implementación Recomendado

### Fase 1: Validación Básica del Hardware
1. **ADC básico sin DMA** (1-2 horas)
   - Implementar lectura simple del ADC
   - Validar que se obtienen valores razonables
   - Verificar conexión del circuito de acondicionamiento

2. **AD9833 sin frecuencias precisas** (2-3 horas)
   - Implementar SPI básico
   - Configurar una frecuencia fija (ej: 1 kHz)
   - Validar con osciloscopio que genera señal

### Fase 2: DSP Aislado
3. **Goertzel con señales sintéticas** (2-3 horas)
   - Implementar algoritmo completo
   - Probar con senoides generadas por software
   - Validar resultados contra MATLAB/Python
   - Verificar que detecta correctamente frecuencias coherentes

### Fase 3: Integración ADC + Goertzel
4. **ADC + DMA completo** (3-4 horas)
   - Implementar DMA con timing preciso
   - Validar frecuencia de muestreo con osciloscopio
   - Capturar datos reales del circuito

5. **Integración ADC → Goertzel** (1-2 horas)
   - Conectar salida del AD9833 al ADC
   - Verificar que Goertzel detecta la frecuencia correctamente
   - Validar coherencia de muestreo

### Fase 4: Sistema Completo
6. **Barrido completo funcional** (1-2 horas)
   - Integrar todos los módulos
   - Ejecutar barrido de 200 puntos
   - Verificar tiempos de ejecución

7. **MQTT y visualización** (2-3 horas)
   - Implementar cliente MQTT real
   - Configurar broker Mosquitto
   - Desarrollar visualización web básica

## Herramientas de Testing y Debugging

### Testing del ADC
```bash
# Conectar a serial
screen /dev/ttyACM0 115200

# Verificar que el ADC lee valores entre 0-4095
# Con entrada en GND: valores cercanos a 0
# Con entrada en 3.3V: valores cercanos a 4095
# Con entrada en 1.65V: valores cercanos a 2048
```

### Testing del AD9833
```bash
# Usar osciloscopio para validar:
# - Frecuencia generada (contador de frecuencia para precisión)
# - Amplitud de salida (~0.65 Vpp)
# - Forma de onda (senoidal limpia)

# Probar coherencia:
# Configurar AD9833 a múltiplos de 100 Hz
# Verificar que la señal capturada tiene ciclos completos
```

### Testing de Goertzel
```python
# Script Python de validación
import numpy as np
import matplotlib.pyplot as plt

fs = 48000  # Frecuencia de muestreo
N = 480     # Tamaño de ventana
f_target = 1000  # Frecuencia objetivo

# Generar señal
t = np.arange(N) / fs
signal = 0.5 + 0.4 * np.sin(2 * np.pi * f_target * t)

# DFT manual del bin objetivo
k = int(f_target * N / fs)
omega = 2 * np.pi * k / N
coeff = 2 * np.cos(omega)

s_prev = 0
s_prev2 = 0

for x in signal:
    s = x + coeff * s_prev - s_prev2
    s_prev2 = s_prev
    s_prev = s

real = s_prev - s_prev2 * np.cos(omega)
imag = s_prev2 * np.sin(omega)
magnitude = np.sqrt(real**2 + imag**2)

print(f"Magnitud detectada: {magnitude:.4f}")
print(f"Magnitud esperada: ~0.4 (amplitud de la senoide)")
```

### Instrumentación con GPIO
```c
// En el código, toggle GPIO antes/después de eventos clave
gpio_put(DEBUG_PIN_ADC_ACQUIRE, 1);
adc_dma_start_capture();
// ... operación ...
gpio_put(DEBUG_PIN_ADC_ACQUIRE, 0);

// Conectar osciloscopio a los DEBUG_PINs para ver timing real
```

## Problemas Conocidos y Soluciones

### 1. ADC con ENOB degradado
**Problema:** ENOB real de ~8.8 bits vs 12 bits nominales

**Solución implementada:** Etapa de acondicionamiento analógico con ganancia de 4.5

**Validación:**
- Medir SNR de la señal amplificada
- Comparar con señal sin amplificar
- Debe mejorarse la resolución efectiva

### 2. Jitter en frecuencia de muestreo
**Problema:** Timer interrupt puede tener jitter

**Solución:** Usar DMA paced by ADC, no por timer de software

**Validación:**
- Capturar señal de trigger con osciloscopio
- Medir variación entre disparos
- Debe ser < 1 µs para considerar aceptable

### 3. Fuga espectral si no hay coherencia
**Problema:** Si frecuencias no son múltiplos exactos de 100 Hz

**Solución:** Validar configuración del AD9833 con contador de frecuencia

**Validación:**
- Configurar AD9833 a 1000 Hz
- Medir con contador de frecuencia
- Error debe ser < 1 Hz

## Checklist de Validación por Módulo

### ADC + DMA ✓
- [ ] Frecuencia de muestreo medida = 48 kHz ± 100 Hz
- [ ] Buffer de 480 muestras se llena en 10 ms ± 0.5 ms
- [ ] No hay pérdida de muestras (verificar continuidad)
- [ ] Valores ADC en rango esperado según voltaje de entrada

### AD9833 ✓
- [ ] Frecuencia configurada = frecuencia medida ± 1 Hz
- [ ] Amplitud de salida ≈ 0.65 Vpp
- [ ] Forma de onda senoidal limpia (THD < 1%)
- [ ] Cambio de frecuencia estable en < 100 ms

### Goertzel ✓
- [ ] Detecta frecuencias coherentes con error < 1%
- [ ] Magnitud proporcional a amplitud de entrada
- [ ] Fase correcta dentro de ±5 grados
- [ ] No detecta falsas frecuencias (espurias < -40 dB)

### Sistema Integrado ✓
- [ ] Barrido de 200 puntos completa en 3-5 segundos
- [ ] Datos MQTT recibidos en servidor
- [ ] Diagrama de Bode se visualiza correctamente
- [ ] Caracterización de filtro RC de referencia coincide con teoría

## Referencias Útiles

- **Pico SDK Documentation:** https://www.raspberrypi.com/documentation/pico-sdk/
- **RP2350 Datasheet:** https://datasheets.raspberrypi.com/rp2350/rp2350-datasheet.pdf
- **Cornell ECE4760 Examples:** https://people.ece.cornell.edu/land/courses/ece4760/RP2350/
- **AD9833 Datasheet:** https://www.analog.com/media/en/technical-documentation/data-sheets/AD9833.pdf
- **Goertzel Algorithm:** https://en.wikipedia.org/wiki/Goertzel_algorithm

## Próximos Pasos Inmediatos

1. Crear archivo `src/config.h` basado en `config.h.example`
2. Compilar proyecto para verificar que todo está configurado correctamente
3. Comenzar implementación del módulo ADC+DMA
4. Validar cada módulo antes de continuar con el siguiente
