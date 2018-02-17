#!/bin/bash
: "${NRFJ_BIN:?Need to set NRFJ_BIN env variable (see project README)}"
: "${SDK_ROOT:?Need to set SDK_ROOT env variable (see project README)}"

cp manbearpig/_build/nrf52832_xxaa.hex .
cp $SDK_ROOT/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex .

$NRFJ_BIN/mergehex/mergehex -m nrf52832_xxaa.hex s132_nrf52_3.0.0_softdevice.hex -o joco_2018_1.9.hex

rm nrf52832_xxaa.hex
rm s132_nrf52_3.0.0_softdevice.hex
