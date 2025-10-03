#!/bin/bash

DEVICE="/dev/ttyACM0"
BAUD_RATE=115200
TIMEOUT=5

echo "Reiniciando Pico..."
picotool reboot -f

echo "Esperando a que el puerto serie esté disponible..."
elapsed=0
while [ ! -e "$DEVICE" ] && [ $elapsed -lt $TIMEOUT ]; do
  sleep 0.1
  elapsed=$((elapsed + 1))
done

if [ ! -e "$DEVICE" ]; then
  echo "Error: El dispositivo $DEVICE no apareció después de ${TIMEOUT}s"
  exit 1
fi

echo "Puerto serie detectado. Conectando con picocom..."
sleep 0.2
picocom -b $BAUD_RATE $DEVICE
