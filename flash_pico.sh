#!/bin/bash

# Script para flashear Raspberry Pi Pico 2W
# Autor: Script de automatización para fra_rp2350
# Uso: ./flash_pico.sh

set -e # Salir si hay algún error

# Colores para output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # Sin color

UF2_FILE="build/fra_rp2350.uf2"

echo "========================================="
echo "  Flasheo automático Raspberry Pi Pico 2W"
echo "========================================="
echo ""

# 1. Verificar que existe el archivo .uf2
echo "[1/5] Verificando archivo .uf2..."
if [ ! -f "$UF2_FILE" ]; then
  echo -e "${RED}ERROR: No se encuentra el archivo $UF2_FILE${NC}"
  echo "Asegúrate de haber compilado el proyecto primero."
  exit 1
fi
echo -e "${GREEN}✓ Archivo encontrado: $UF2_FILE${NC}"
FILE_SIZE=$(du -h "$UF2_FILE" | cut -f1)
echo -e "  Tamaño: $FILE_SIZE"
echo ""

# 2. Verificar que el Pico está conectado y mostrar información
echo "[2/5] Detectando Raspberry Pi Pico..."
if ! picotool info &>/dev/null; then
  echo -e "${YELLOW}ADVERTENCIA: No se detectó el Pico en modo normal.${NC}"
  echo "Intentando detectar en modo bootloader..."
  if ! lsusb | grep -q "2e8a:0003\|2e8a:0004"; then
    echo -e "${RED}ERROR: No se detectó ningún Pico conectado.${NC}"
    echo "Conecta el Pico o mantenlo presionado BOOTSEL al conectarlo."
    exit 1
  fi
  echo -e "${GREEN}✓ Pico detectado en modo bootloader${NC}"
else
  echo -e "${GREEN}✓ Pico detectado${NC}"
fi

# Mostrar información del dispositivo
echo ""
echo -e "${BLUE}Información del dispositivo:${NC}"
picotool info 2>/dev/null || echo "  (Información no disponible en este modo)"
echo ""

# 3. Verificar si el binario actual es idéntico al que se va a flashear
echo "[3/6] Verificando si es necesario reflashear..."
NEEDS_FLASH=true

if picotool verify "$UF2_FILE" &>/dev/null; then
  echo -e "${YELLOW}⚠ El binario actual es idéntico al archivo a flashear${NC}"
  echo -e "${YELLOW}  No es necesario reflashear (evita desgaste de memoria)${NC}"
  echo ""
  read -p "¿Deseas flashear de todos modos? (s/N): " -n 1 -r
  echo ""
  if [[ ! $REPLY =~ ^[Ss]$ ]]; then
    echo -e "${BLUE}Flasheo cancelado por el usuario${NC}"
    NEEDS_FLASH=false
  fi
else
  echo -e "${GREEN}✓ El binario es diferente o el dispositivo está en bootloader${NC}"
  echo -e "  Procediendo con el flasheo..."
fi
echo ""

# 4. Flashear el dispositivo (solo si es necesario)
if [ "$NEEDS_FLASH" = true ]; then
  echo "[4/6] Flasheando dispositivo..."
  if picotool load -f "$UF2_FILE"; then
    echo -e "${GREEN}✓ Flasheo completado exitosamente${NC}"
  else
    echo -e "${RED}ERROR: Fallo el flasheo${NC}"
    exit 1
  fi
  echo ""
else
  echo "[4/6] Flasheo omitido"
  echo ""
  echo -e "${GREEN}=========================================${NC}"
  echo -e "${GREEN}  Operación completada (sin cambios)${NC}"
  echo -e "${GREEN}=========================================${NC}"
  exit 0
fi

# 5. Verificar que el contenido flasheado coincide con el archivo
echo "[5/6] Verificando integridad del flasheo..."
if picotool verify "$UF2_FILE" 2>/dev/null; then
  echo -e "${GREEN}✓ Verificación exitosa: el contenido coincide${NC}"
else
  echo -e "${YELLOW}ADVERTENCIA: No se pudo verificar (el dispositivo pudo haber reiniciado)${NC}"
  echo "Esto es normal si el firmware no tiene USB stdio habilitado."
fi
echo ""

# 6. Verificar que el dispositivo está presente después del flasheo
echo "[6/6] Verificando presencia del dispositivo..."
sleep 2

if lsusb | grep -q "2e8a"; then
  echo -e "${GREEN}✓ Dispositivo detectado después del flasheo${NC}"
  echo ""

  # Intentar obtener info del dispositivo reiniciado
  if picotool info &>/dev/null; then
    echo -e "${BLUE}Estado del dispositivo:${NC}"
    picotool info
  fi

  echo ""
  echo -e "${GREEN}=========================================${NC}"
  echo -e "${GREEN}  ¡Flasheo completado con éxito!${NC}"
  echo -e "${GREEN}=========================================${NC}"
else
  echo -e "${YELLOW}ADVERTENCIA: No se detectó el dispositivo después del flasheo.${NC}"
  echo "El firmware puede estar corriendo pero sin enumerar USB."
  echo "Verifica manualmente el funcionamiento."
fi
