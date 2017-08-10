#!/bin/bash
HEX_FILE=andnxor_dc25_1.6.hex
NRF5_CHIP_FAMILY=nrf52
NRFJPROG_PATH=../bin/nrfjprog
SDK_ROOT=../nRF5x
echo Provisioning...

echo =============== RECOVER ===============
$NRFJPROG_PATH/nrfjprog --recover -f $NRF5_CHIP_FAMILY

echo ============= FIRMWARE ================
echo Flashing $HEX_FILE
$NRFJPROG_PATH/nrfjprog --program $HEX_FILE --verify --chiperase -f $NRF5_CHIP_FAMILY
