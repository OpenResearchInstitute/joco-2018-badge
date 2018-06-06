# joco-2018-badge

Code and documentation for the unofficial electronic badge for the JoCo Cruise 2018.

This is based on the excellent work that the AND!XOR team produced for the DEFCON 25 Bender Badge.

All code and hardware design files are closed until Feb 26th, 2018 and then are licensed under the Apache license 2.0 unless superceded by notices in individual files or directories.

## Getting Started

Some resources must be installed outside of this source tree. One of these is the Nordic SDK.

Choose a place to install these on your file system. For example, we'll use "/src/joco-support/".

Download the version 12.3 of the Nordic SDK [from here](https://developer.nordicsemi.com/nRF5_SDK/nRF5_SDK_v12.x.x/nRF5_SDK_12.3.0_d7731ad.zip) and unzip it into the directory you chose.

It's known that later versions of the SDK are not compatible with the existing badge code. We may work on fixing that, which is why we don't install the SDK in the source tree for the project.

Now edit your shell init or rc file to set the following environment variable when you create a shell, substituting your chosen path:

```
export SDK_ROOT=/src/joco-support/nRF5_SDK_12.3.0_d7731ad
```

### Ubuntu Installation (non-ARM host)

Download the GNU ARM Embedded Toolchain. Choose one of these methods:

* Download the latest from <https://developer.arm.com/open-source/gnu-toolchain/gnu-rm/downloads> and unpack it into its own directory in `/usr/local`. Or,

* Download the version that Nordic used to test their SDK, 4.9 2015q3, from <https://launchpad.net/gcc-arm-embedded/+milestone/4.9-2015-q3-update> and unpack it into its own directory in `/usr/local`. Or,

* `sudo apt-get install gcc-arm-none-eabi` and take your chances on whatever version is current for your Linux. This may be your only option if you're running Linux on a non-x86 platform.

Then edit `$SDK_ROOT/components/toolchain/gcc/Makefile.posix` and change the value for `GNU_INSTALL_ROOT` to the directory where your toolchain was installed.

If you plan to hook up hardware for programming and debugging, you'll need the Segger JLink software. It's available from <https://www.segger.com/downloads/jlink/> for a variety of host platforms. We used version V6.18b, but the latest version is probably fine. To install on Ubuntu on an Intel PC:

`sudo dpkg -i <path-to-downloads>/JLink_Linux_V618b_x86_64.deb`

You'll also need a couple of command line tools from Nordic. These are available from <http://infocenter.nordicsemi.com/topic/com.nordic.infocenter.tools/dita/tools/nrf5x_command_line_tools/nrf5x_installation.html?cp=5_1_1>. We used version 9.7.0, but the latest version is probably fine. Install them in a location outside this project, like the Nordic SDK. For example using the location "/src/joco-support", you can do the following:

```
mkdir -p /src/joco-support/bin
cd /src/joco-support/bin
tar xvf <path-to-downloads>/nRF5x-Command-Line-Tools_9_7_0_Linux-x86_64.tar
```

Now edit your shell init or rc file to set the following environment variable when you create a shell, substituting your chosen path:

```
export NRFJ_BIN=/src/joco-support/bin
```

#### Ubuntu on an ARM Host

If you want to develop on an ARM-based host, most of the components are available.

The Nordic SDK is the same.

You should be able to `sudo apt-get install gcc-arm-none-eabi` to get the toolchain.

You can get JLink software [from Segger here](https://www.segger.com/downloads/jlink/#J-LinkSoftwareAndDocumentationPack)

The command line tools nrfjprog and mergehex are not currently available for ARM hosts. You can use the JLink tools to program the device, so you can probably work around nrfjprog.

### Build the code

`cd firmware/manbearpig`

`make`

`cd ..`

`./update.sh`


### JTAG interface hardware

To flash the code to the badge and use the Ozone debugger, you'll need a JTAG hardware interface.
The one that was used by the development team is the Segger [J-Link EDU Mini](https://www.segger.com/products/debug-probes/j-link/models/j-link-edu-mini/).

It's much easier to connect to the board if you use a short flat cable and a mating header on the board end. All these can be purchased from these retailers:

* [J-Link EDU Min from Adafruit](https://www.adafruit.com/product/3571)
* [Ten pin JTAG cable from Adafruit](https://www.adafruit.com/product/1675) (not the same as the one that comes with the debugger)
* [Header from Mouser](https://www.mouser.com/ProductDetail/Harwin/M50-3500542?qs=9fQaSFfsqsyXI0P9tFOVoQ%3D%3D)

### Flashing the target

Two ways, either:

`cd firmware`

`./provision.sh`

or

`cd firmware/manbearpig`

`make flash_softdevice`

`make flash`

The `make flash_softdevice` can be omitted after the first time.

### Setting up the Segger Ozone debugger

This was correct for Ubuntu 14.04.5 64 bit

Install the Ozone package [from here](https://www.segger.com/downloads/jlink/#Ozone)

The manual is useful (same link)

IMPORTANT NOTE: 

Ozone version 2.56 would not work for me. It would halt execution in seemingly random places and not load source code.

Version 2.54b works fine

If you're using the Segger edu device, Select the device as Nordic nRF52832_xxAA

Choose SWD for the debugger device and NOT USB.

open the badge .out file to bring in source files


