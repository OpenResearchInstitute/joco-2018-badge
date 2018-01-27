# Wall of Joco 2018

Brought to you by Abraxas3D and Skunkwrx.

Based on the [Wall of Bender](https://github.com/MustBeArt/wallofbender)


## Introduction

The **Wall of Joco** is a logger and display board for the
JOCO Cruise 2018 Pirate Monkey Badge. The badge has a Bluetooth Low Energy (BLE)
radio, which it uses to implement a game. Each badge is constantly transmitting its
unique ID, player name, and other game info in BLE advertisement format.
The Wall listens for these advertisements, logs them all to a file, and
displays them in a friendly format for the amusement and edification of
passers-by.

Three windows are displayed. A smaller window displays just the names and
unique IDs of each and every badge advertisement received, in real time,
even if they go by too fast to read. A bigger window displays the same
information plus a time-since-last-heard indication, and smooth scrolls
the entire stored list, after removing duplicates based on the BLE
advertising address transmitted. In between, another smooth-scrolled
window shows just the names received, regardless of which badge sent them.

The badge also has NFC capabilities, and the Wall includes an NFC reader.
When a badge is detected by the Wall via NFC, the Wall checks its game
status in the BLE database. If the badge is eligible for a prize based on
its game status, the Wall commands the trinket dispenser to dispense one.

## Hardware

We deployed the Wall of Joco on a
[Raspberry Pi Zero W](https://www.raspberrypi.org/products/raspberry-pi-zero-w/),
which has its own built-in Bluetooth hardware. Any Linux-based computer
with a Bluetooth adapter capable of supporting BLE under the standard
[BlueZ Bluetooth stack](http://www.bluez.org) should theoretically work fine.

We used a [LG 24M38H](http://www.lg.com/us/monitors/lg-24M38H-B-led-monitor)
HDMI monitor at 1920x1080 ("Full HD" 1080P) resolution. Note that the Wall
of Joco is hard-coded for this resolution and will look terrible at any
other resolution.

The NFC hardware was an Adafruit PN532 breakout board, connected to the
Raspberry Pi Zero W's serial port.

An Adafruit 3013 DS3231 realtime clock breakout was connected to the
Raspberry Pi Zero W and configured according to Adafruit's
[Adding a Real Time Clock to Raspberry Pi](https://learn.adafruit.com/adding-a-real-time-clock-to-raspberry-pi).

## Deployment Notes

### Operating System

We originally used
[NOOBS](https://www.raspberrypi.org/blog/introducing-noobs/) 2.4.2 to install
[Raspbian](http://raspbian.org) Jessie on an 8GB SD card, and updated the
system regularly thereafter.

### Permissions

The Wall of Joco needs networking permissions to operate the Bluetooth
interface. This could be accomplished by running as `root` but that's a
bad idea. Instead, we want to grant ourselves the appropriate capabilities.
But we're a script running under the Python interpreter, so actually we
needed to grant those capabilities to the interpreter. We didn't want to do
that for every Python program, so we made a private copy of the interpreter
and granted the capabilities to that. Like so:

```
	cp /usr/bin/python3 capython3
	sudo setcap 'cap_net_raw,cap_net_admin+eip' capython3
```

We then put that private interpreter in the shebang line at the top of
`wallofjoco.py`, so it gets the needed permissions if run like so:

```
	./wallofjoco.py
```

### Dependencies

```
	sudo apt-get install python3-pil.imagetk
```

Everything else needed was already included in the NOOBS install of
Raspbian.

### Connecting the NFC Hardware

See [Adafruit NFC/RFID on Raspberrry Pi](https://learn.adafruit.com/adafruit-nfc-rfid-on-raspberry-pi)
for info on connecting the NFC hardware to the Raspberry Pi Zero W.

### Screen Blanking

We turned off screen blanking as suggested on
[this web page](http://www.geeks3d.com/hacklab/20160108/how-to-disable-the-blank-screen-on-raspberry-pi-raspbian/)
by adding

```
    [SeatDefaults]
    xserver-command=X -s 0 -dpms
```

in the file `/etc/lightdm/lightdm.conf `.

### Crash Recovery

The Wall of Joco is designed to clean up after itself when stopped with
control-C or `kill`, but if it crashes after initializing the Bluetooth
interface it may leave the interface in an unusable condition. It can
usually be recovered like so:

```
	sudo hciconfig hci0 down
	sudo hciconfig hci0 up
```

If that doesn't work, a reboot probably will.

