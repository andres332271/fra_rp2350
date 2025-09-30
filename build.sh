#!/bin/bash
# Script de build para FRA RP2350
# Uso: ./build.sh [clean|flash|monitor]

set -e # Exit on error

PROJECT_NAME="fra_rp2350"
BUILD_DIR="build"
PICO_MOUNT="/run/media/$USER/RPI-RP2"
SERIAL_PORT="/dev/ttyACM0"

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

print_info() {
  echo -e "${GREEN}[INFO]${NC} $1"
}

print_warn() {
  echo -e "${YELLOW}[WARN]${NC} $1"
}

print_error() {
  echo -e "${RED}[ERROR]${NC} $1"
}

# Verificar que PICO_SDK_PATH está definido
if [ -z "$PICO_SDK_PATH" ]; then
  print_error "PICO_SDK_PATH no está definido"
  print_info "Agregar a ~/.bashrc:"
  print_info "  export PICO_SDK_PATH=~/pico-sdk"
  exit 1
fi

print_info "PICO_SDK_PATH: $PICO_SDK_PATH"

# Verificar que config.h existe
if [ ! -f "src/config.h" ]; then
  print_error "src/config.h no existe"
  print_info "Crear desde template:"
  print_info "  cp config.h.example src/config.h"
  print_info "  nano src/config.h"
  exit 1
fi

# Función de limpieza
clean_build() {
  print_info "Limpiando directorio de build..."
  rm -rf "$BUILD_DIR"
  print_info "Build directory limpio"
}

# Función de configuración y build
build_project() {
  print_info "Configurando proyecto con CMake..."

  if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
  fi

  cd "$BUILD_DIR"
  cmake .. || {
    print_error "CMake falló"
    exit 1
  }

  print_info "Compilando proyecto..."
  make -j$(nproc) || {
    print_error "Compilación falló"
    exit 1
  }

  cd ..

  print_info "Build exitoso!"
  print_info "Archivo generado: $BUILD_DIR/${PROJECT_NAME}.uf2"

  # Mostrar tamaño del binario
  if [ -f "$BUILD_DIR/${PROJECT_NAME}.elf" ]; then
    print_info "Tamaño del binario:"
    arm-none-eabi-size "$BUILD_DIR/${PROJECT_NAME}.elf"
  fi
}

# Función de flasheo
flash_device() {
  UF2_FILE="$BUILD_DIR/${PROJECT_NAME}.uf2"

  if [ ! -f "$UF2_FILE" ]; then
    print_error "No se encontró $UF2_FILE"
    print_info "Ejecutar './build.sh' primero para compilar"
    exit 1
  fi

  print_info "Esperando que el Pico entre en modo BOOTSEL..."
  print_info "1. Desconectar el Pico"
  print_info "2. Mantener presionado BOOTSEL"
  print_info "3. Conectar USB mientras mantienes BOOTSEL"
  print_info "4. Soltar BOOTSEL"
  echo ""

  # Esperar a que aparezca el dispositivo
  timeout=30
  while [ $timeout -gt 0 ]; do
    if [ -d "$PICO_MOUNT" ]; then
      print_info "Pico detectado en $PICO_MOUNT"
      break
    fi
    sleep 1
    timeout=$((timeout - 1))
    echo -ne "\rEsperando... $timeout segundos restantes  "
  done
  echo ""

  if [ ! -d "$PICO_MOUNT" ]; then
    print_error "Timeout esperando el Pico"
    print_warn "Verificar que el Pico está en modo BOOTSEL"
    exit 1
  fi

  print_info "Copiando firmware al Pico..."
  cp "$UF2_FILE" "$PICO_MOUNT/" || {
    print_error "Falló la copia del firmware"
    exit 1
  }

  print_info "Firmware flasheado exitosamente!"
  print_info "El Pico se reiniciará automáticamente"

  # Esperar un momento para que se reinicie
  sleep 2
}

# Función de monitoreo serial
monitor_serial() {
  if [ ! -e "$SERIAL_PORT" ]; then
    print_error "Puerto serial $SERIAL_PORT no encontrado"
    print_info "Verificar que el Pico está conectado"
    print_info "Puertos disponibles:"
    ls /dev/ttyACM* 2>/dev/null || print_warn "No se encontraron puertos ttyACM"
    exit 1
  fi

  print_info "Conectando a $SERIAL_PORT (115200 baud)"
  print_info "Para salir: Ctrl+A luego K (confirmar con Y)"
  echo ""

  # Verificar si screen está instalado
  if command -v screen &>/dev/null; then
    screen "$SERIAL_PORT" 115200
  else
    print_error "screen no está instalado"
    print_info "Instalar con: sudo pacman -S screen"
    exit 1
  fi
}

# Procesar argumentos
case "$1" in
clean)
  clean_build
  ;;
flash)
  flash_device
  ;;
monitor)
  monitor_serial
  ;;
"")
  # Sin argumentos: build normal
  build_project
  ;;
*)
  echo "Uso: $0 [clean|flash|monitor]"
  echo ""
  echo "Comandos:"
  echo "  (ninguno)  - Compilar el proyecto"
  echo "  clean      - Limpiar archivos de build"
  echo "  flash      - Flashear firmware al Pico (requiere modo BOOTSEL)"
  echo "  monitor    - Conectar a serial para ver debug output"
  echo ""
  echo "Ejemplos:"
  echo "  $0              # Compilar"
  echo "  $0 clean        # Limpiar y compilar"
  echo "  $0 flash        # Flashear después de compilar"
  echo "  $0 monitor      # Ver output serial"
  exit 1
  ;;
esac
