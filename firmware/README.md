# JoCo Cruise 2018 pirate badge firmware

based on the AND!XOR DC25 Bender badge

## Building ##

Firmware is built using the Nordic 12.2 SDK and arm-non-eabi-gcc 6.3.1.

cd manbearpig
make

That's it.

## Flashing ##

Youll need to install nrfjprog and poin tto it usng environment variables. See the Makefile.

make flash_softdevice
make flash

flash_softdevice only needs to be run once after a full recover or full erase.

flash will only update user code.
