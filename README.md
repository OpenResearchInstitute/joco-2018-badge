# joco-2018-badge

Code and documentation for the unofficial electronic badge for the JoCo Cruise 2018.

This is based on the excellent work that the AND!XOR team produced for the DEFCON 25 Bender Badge.

All code and hardware design files are closed until Feb 26th, 2018 and then are licensed under the Apache license 2.0 unless superceded by notices in individual files or directories.

## Getting Started

Download the Nordic SDK from https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v12.x.x/nRF5_SDK_12.3.0_d7731ad.zip and unzip it into the top level directory. Then link it as follows:

`ln -s nRF5_SDK_12.3.0_d7731ad nRF5x`

### Ubuntu Dependencies

Download the latest GNU ARM Embedded Toolchain from https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads and unpack it into its own directory in `/usr/local`. Or, if you prefer, `sudo apt-get install gcc-arm-none-eabi`. Then edit `nRF5x/components/toolchain/gcc/Makefile.posix` and change the value for `GNU_INSTALL_ROOT` to the directory where your toolchain was installed.

`sudo dpkg -i ubuntu-dependencies/JLink_Linux_V618b_x86_64.deb`

`cd bin`

`tar xvf ../ubuntu-dependencies/nRF5x-Command-Line-Tools_9_7_0_Linux-x86_64.tar`

### Build the code

`cd firmware/manbearpig`

`make`

`cd ..`

`./update.sh`

### Flashing the target

Two ways, either:

`cd firmware`

`./provision.sh`

or

`cd firmware/manbearpig`

`make flash_softdevice`

`make flash`

The `make flash_softdevice` can be omitted after the first time.
