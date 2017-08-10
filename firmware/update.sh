#!/bin/bash

cp manbearpig/_build/nrf52832_xxaa.hex .
cp ../nRF5x/components/softdevice/s132/hex/s132_nrf52_3.0.0_softdevice.hex .

../bin/mergehex/mergehex -m nrf52832_xxaa.hex s132_nrf52_3.0.0_softdevice.hex -o andnxor_dc25_1.6.hex

rm nrf52832_xxaa.hex
rm s132_nrf52_3.0.0_softdevice.hex
