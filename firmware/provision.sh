#!/bin/bash
HEX_FILE=joco_2018_1.9.hex
NRF5_CHIP_FAMILY=nrf52
: "${NRFJ_BIN:?Need to set NRFJ_BIN env variable (see project README)}"
: "${SDK_ROOT:?Need to set SDK_ROOT env variable (see project README)}"
echo Provisioning...

echo =============== RECOVER ===============
$NRFJ_BIN/nrfjprog/nrfjprog --recover -f $NRF5_CHIP_FAMILY

echo ============= FIRMWARE ================
echo Flashing $HEX_FILE
$NRFJ_BIN/nrfjprog/nrfjprog --program $HEX_FILE --verify --chiperase -f $NRF5_CHIP_FAMILY
echo Resetting . . . 
$NRFJ_BIN/nrfjprog/nrfjprog --reset -f nrf52
