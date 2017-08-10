# AND!XOR DC25 Firmware #

And the firmware itself...

Everything is here, Master Badge mode, C2, Botnet, Bling, everything. 

## Building ##

Firmware is built using the Nordic 12.2 SDK and arm-non-eabi-gcc 6.3.1.

cd manbearpig
make

That's it.

## Flashing ##

nrfjprog has been provided in ../bin. This is used to recover the NRF52, flash the soft device, and firmware. All of this has been scripted in provision.sh.

Or use make.

make flash_softdevice
make flash

flash_softdevice only needs to be run once after a full recover or full erase.

flash will only update user code.
