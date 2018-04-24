# Setting up an NFC reader to verify the badge functionality

Disclaimer: These are not very good notes


Install headers on the PN532 breakout
Use an FTDI Friend, standard jumpers (5V power supply, 3.3V signals)

TL;DR

```
$ sudo apt-get install libusb-dev dh-autoreconf libusb-0.1-4
$ git clone https://github.com/nfc-tools/libnfc.git
$ cd libnfc
$ git checkout libnfc-1.7.1
$ git clean -d -f -x
$ git remote|grep -q anonscm||git remote add anonscm
$ git://anonscm.debian.org/collab-maint/libnfc.git
$ git fetch anonscm
$ git checkout remotes/anonscm/master debian
$ git reset
$ dpkg-buildpackage -uc -us -b
$ sudo mkdir -p /etc/nfc/devices.d
$ sudo emacs /etc/nfc/devices.d/pn532_via_uart2usb.conf
```

insert the following:

```
## Typical configuration file for PN532 board (ie. microbuilder.eu / Adafruit) device
name = "Adafruit PN532 board via UART"
connstring = pn532_uart:/dev/ttyUSB0
allow_intrusive_scan = true
log_level = 3
< end of contents of pn532_via_uart2usb.conf>
```

```
$ sudo emacs /etc/nfc/libnfc.conf
```

Uncomment the “allow_autoscan = true” line

```
$ sudo dpkg -i ../libnfc*.deb
```

[Or if force reinstall needed]

```
sudo dpkg --force-all -i ../libnfc*.deb
```

That should be all

If it’s working, then the command ‘nfc-list’ should return something like the following:

```
nfc-list uses libnfc libnfc-1.7.1
NFC device: pn532_uart:/dev/ttyUSB0 opened
```

And the command ‘nfc-poll’ should block until you bring a tag near the board, and then print information about the tag and exit.


## Here are ALL the details, probably not in the right order

Install libnfc from the git repo like this, but stop short of the build and install:
http://nfc-tools.org/index.php?title=Libnfc


Add configuration files like this:
http://brokeragesdaytrading.com/article/8395128793/how-to-configure-libnfc-on-linux-for-pn532-breakout-board-uart-ftdi-/

Then go back to the libnfc source directory and do this:
```
./configure --sysconfdir=/etc --prefix=/usr --with-drivers=pn532_uart
```
