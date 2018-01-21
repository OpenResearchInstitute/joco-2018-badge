#!./capython3
# To avoid running as root, we use a copy of the Python3 interpreter.
# Give it the needed capabilities with this command:
# sudo setcap 'cap_net_raw,cap_net_admin+eip' capython3

# Portions of the Bluetooth interaction parts of this script have been
# taken from https://stackoverflow.com/questions/23788176/finding-bluetooth-low-energy-with-python


import sys
import os
import struct
import signal
import time
import errno
from ctypes import (CDLL, get_errno)
from ctypes.util import find_library
from socket import (
    socket,
    AF_BLUETOOTH,
    SOCK_RAW,
    BTPROTO_HCI,
    SOL_HCI,
    HCI_FILTER,
)
from tkinter import *
from PIL import ImageTk, Image
from collections import deque
import threading

BADGE_YEAR = "yr"     # year (Appearance field) in most recent advertisement
BADGE_YEARS = "yrs"   # list of years seen for this address
BADGE_NAME = "nm"     # badge name (Complete Local Name) in most recent
BADGE_NAMES = "nms"   # list of names seen for this address
BADGE_ID = "id"       # badge ID (first two octets of Manufacturer Specific Data)
BADGE_IDS = "ids"     # list of badge IDs seen for this address
BADGE_TIME = "tm"     # time of most recent advertisement received
BADGE_ADDR = "ad"     # Advertising Address for this badge (assumed constant)
BADGE_CNT = "n"       # number of advertisements received from this address
BADGE_ID_FAKED = "faked"    # present if multiple IDs seen for this address


class BTAdapter (threading.Thread):
    def __init__(self, master, btQueue):
        threading.Thread.__init__(self)
        self.btQueue = btQueue
        
        self.stop_event = threading.Event()
        
        btlib = find_library("bluetooth")
        if not btlib:
            raise Exception(
                "Can't find required bluetooth libraries"
                " (need to install bluez)"
            )
        self.bluez = CDLL(btlib, use_errno=True)
    
        dev_id = self.bluez.hci_get_route(None)
        
        self.sock = socket(AF_BLUETOOTH, SOCK_RAW, BTPROTO_HCI)
        if not self.sock:
            print("Failed to open Bluetooth")
            sys.exit(1)
            
        self.sock.bind((dev_id,))

        err = self.bluez.hci_le_set_scan_parameters(self.sock.fileno(), 0, 0x10, 0x10, 0, 0, 1000);
        if err < 0:
            raise Exception("Set scan parameters failed")
            # occurs when scanning is still enabled from previous call
        
        # allows LE advertising events
        hci_filter = struct.pack(
            "<IQH", 
            0x00000010, 
            0x4000000000000000, 
            0
        )
        self.sock.setsockopt(SOL_HCI, HCI_FILTER, hci_filter)
        
        err = self.bluez.hci_le_set_scan_enable(
            self.sock.fileno(),
            1,  # 1 - turn on;  0 - turn off
            0, # 0-filtering disabled, 1-filter out duplicates
            1000  # timeout
        )
        if err < 0:
            errnum = get_errno()
            raise Exception("{} {}".format(
                errno.errorcode[errnum],
                os.strerror(errnum)
            ))

    def stop(self):
        self.stop_event.set()
    
    def stopped(self):
        return self.stop_event.is_set()

    def clean_up(self):
        if self.sock is None:
            print("Double clean_up", flush=True)
            return
            
        err = self.bluez.hci_le_set_scan_enable(
            self.sock.fileno(),
            0,  # 1 - turn on;  0 - turn off
            0, # 0-filtering disabled, 1-filter out duplicates
            1000  # timeout
            )
        if err < 0:
            errnum = get_errno()
            print("{} {}".format(
                errno.errorcode[errnum],
                os.strerror(errnum)
                ))

        self.sock.close()
        self.sock = None
        
    def run(self):
        while True:
            data = self.sock.recv(1024)
            badge_time = time.time()
            self.btQueue.appendleft((badge_time,data))
            if self.stopped():
                self.clean_up()
                break

class Logger:
    def __init__(self):
        self.intercepts = []
        self.count = 0
        
    def _writeout(self):
        filename = time.strftime("%Y%m%d%H%M%S", time.gmtime(time.time())) + ".log"
        with open(filename, "w+") as f:
            for ts,data in self.intercepts:
                hex = ''.join('{0:02x}'.format(x) for x in data)
                print("%f %s" % (ts, hex), file=f)

    def intercept(self, cept):
        self.intercepts.append(cept)
        self.count += 1
        if self.count >= 1000:
            self._writeout()
            self.intercepts = []
            self.count = 0

    def closeout(self):
        self._writeout()
                
        
class LiveDisplay:
    def __init__(self, master):
        self.live_canvas = Canvas(master, width=494, height=430, bg=tablebg, borderwidth=0, highlightthickness=0)
        self.live_text = self.live_canvas.create_text(tmargin, tmargin, anchor=NW, text="", font=("Droid Sans Mono", 44))
        self.live_canvas.place(x=screenw-margin, y=screenh-margin, anchor=SE)
        self.lines = deque()
        
    def intercept(self, badge):
        line = "%s %s" % (badge[BADGE_ID], badge[BADGE_NAME])
        if len(self.lines) >= 6:
            self.lines.popleft()
        self.lines.append(line)
        self.live_canvas.itemconfigure(self.live_text, text="\n".join(self.lines))


class SmoothScroller:
    def __init__(self, master, width, height, x, y, wait):
        self.master = master
        self.wait = wait
        self.height = height
        self.canvas = Canvas(master, width=width, height=height, bg=tablebg, borderwidth=0, highlightthickness=0)
        self.text = self.canvas.create_text(tmargin, tmargin, anchor=NW, text="", font=("Droid Sans Mono", 44))
        self.canvas.place(x=x, y=y, anchor=NW)
        self.scroll()
             
    def scroll(self):
        left,top,right,bottom = self.canvas.bbox(ALL)
        if bottom > self.height:
            self.canvas.move(self.text, 0, -1)
        elif top < 0:
            if bottom > 0:
                self.canvas.move(self.text, 0, -1)
            else:
                self.canvas.move(self.text, 0, -top + self.height)
        self.master.after(self.wait, self.scroll)
        

class NamesDisplay (SmoothScroller):
    def __init__(self, master):
        SmoothScroller.__init__(self, master, width=304, height=680, x=margin+912+margin, y=350, wait=20)
        self.lines = deque()
        self.scroll()
        
    def intercept(self, badge):
        if badge[BADGE_NAME] not in self.lines:
            #print("BADGE NAME .%s." % badge[BADGE_NAME])
            #line = badge[BADGE_NAME] + " "*(8-len(badge[BADGE_NAME]))
            #print("LINE .%s." % line)
            self.lines.append(badge[BADGE_NAME])
            self.canvas.itemconfigure(self.text, text="\n".join(self.lines))

        
class BadgeDisplay (SmoothScroller):
    def __init__(self, master):
        self.master = master
        self.badges = {}
        SmoothScroller.__init__(self, master, width=912, height=680, x=margin, y=350, wait=30)
        self.lines = deque()
        self.scroll()
        self.updater()
    
    def updater(self):
        self.update_display()
        self.master.after(5000, self.updater)        
        
    def format_time_ago(self, t, timenow):
        age = timenow - t
        if age < 5.0:
            return " just now"
        else:
            hours = int(age / (60*60))
            age -= hours * 60*60
            minutes = int(age / 60)
            age -= minutes * 60
            secs = int(age/5) * 5
            if hours > 0:
                return "%3d:%02d:%02d" % (hours, minutes, secs)
            else:
                return "    %2d:%02d" % (minutes, secs)            
        
    def update_display(self):
        timenow = time.time()
        self.lines = []
        for _,b in self.badges.items():
            if BADGE_ID_FAKED in b:
                flag = "*"
            else:
                flag = " "
            ident = b[BADGE_ID]
            name = b[BADGE_NAME]
            t = self.format_time_ago(b[BADGE_TIME], timenow)
            #line = "%s %s %s %s" % (flag, ident, name, t)
            line = flag + " " + ident + " " + name + " "*(8-len(name)) + t
            self.lines.append(line)
            self.canvas.itemconfigure(self.text, text="\n".join(self.lines))
            
    def intercept(self, badge):
        if badge[BADGE_ADDR] not in self.badges:
            badge[BADGE_IDS] = [badge[BADGE_ID]]
            badge[BADGE_NAMES] = [badge[BADGE_NAME]]
            badge[BADGE_YEARS] = [badge[BADGE_YEAR]]
            badge[BADGE_CNT] = 1
            self.badges[badge[BADGE_ADDR]] = badge
            #self.update_display()

        else:
            b = self.badges[badge[BADGE_ADDR]]
            b[BADGE_CNT] += 1
            b[BADGE_NAME] = badge[BADGE_NAME]
            b[BADGE_ID] = badge[BADGE_ID]
            b[BADGE_TIME] = badge[BADGE_TIME]
            b[BADGE_YEAR] = badge[BADGE_YEAR]
            if badge[BADGE_NAME] not in b[BADGE_NAMES]:
                b[BADGE_NAMES].append(badge[BADGE_NAME])
            if badge[BADGE_ID] not in b[BADGE_IDS]:
                b[BADGE_IDS].append(badge[BADGE_ID])
            if badge[BADGE_YEAR] not in b[BADGE_YEARS]:
                b[BADGE_YEARS].append(badge[BADGE_YEAR])
            if len(b[BADGE_IDS]) > 1:
                b[BADGE_ID_FAKED] = True
            #self.update_display()

            
margin = 50
tmargin = 5
screenh = 1080
screenw = 1920
bgcolor = "#ffe298"
tablebg = "#eed288"


root = Tk()
root.overrideredirect(True)
root.overrideredirect(False)
root.attributes("-fullscreen", True)
root.configure(background=bgcolor)

heading = Label(root, text="Wall of Bender", bg=bgcolor, font=("Droid Sans Mono", 120))
heading.place(x=margin, y=margin-40, anchor=NW)
credit = Label(root, text="Brought to you by Abraxas3D and Skunkwrx with thanks to AND!XOR and DEFCON Group San Diego",
    fg="#888888", bg=bgcolor, font=("Droid Sans Mono", 9))
credit.place(x=margin+18, y=170, anchor=NW)
badges_label = Label(root, text="Badges Seen", bg=bgcolor, font=("Droid Sans Mono", 50))
badges_label.place(x=margin, y=250, anchor=NW)
names_label = Label(root, text="Names", bg=bgcolor, font=("Droid Sans Mono", 50))
names_label.place(x=margin+912+margin, y=250, anchor=NW)
live_label = Label(root, text="Intercepts", bg=bgcolor, font=("Droid Sans Mono", 60))
live_label.place(x=margin+912+margin+304+margin, y=480, anchor=NW)

img = ImageTk.PhotoImage(Image.open("badge_photo.png").convert("RGBA"))
photo_panel = Label(root, image = img, borderwidth=0, bg=bgcolor)
photo_panel.place(x=screenw-margin, y=margin, anchor=NE)

badge_display = BadgeDisplay(root)
names_display = NamesDisplay(root)
live_display = LiveDisplay(root)
log = Logger()

def badgeParse(data):
    """ If the advertisement data contains a valid badge beacon,
    return the parsed badge data structure. If not, return None."""
    
    badge_address = ':'.join('{0:02x}'.format(x) for x in data[12:6:-1])

    index = 14
    badge = False
    while (index < len(data)-1):
        packet_len = data[index]
        packet_type = data[index+1]
        packet_payload = data[index+2:index+2+packet_len-1]
        index += packet_len+1
        if packet_type == 0x01:     # Flags
            if int(packet_payload[0]) != 0x06:
                badge = False
        elif packet_type == 0x09:   # Local Name
            badge_name = packet_payload.decode("utf-8")
        elif packet_type == 0x19:   # Appearance
            badge_year = "%02X%d" % (packet_payload[0], packet_payload[1])
        elif packet_type == 0xFF:   # Manufacturer Specific Data
            if packet_payload[1] == 0x04 and packet_payload[0] == 0x9e:     # AND!XOR LLC
                badge_id = "%02X%02X" % (packet_payload[3],packet_payload[2])
                badge = True
            else:
                badge = False
                
    if badge and badge_name is not None and badge_year is not None:
        return { BADGE_ADDR : badge_address,
                  BADGE_ID   : badge_id,
                  BADGE_NAME : badge_name,
                  BADGE_YEAR : badge_year
                  }
    else:
        return None



def processAdvertisement(cept):
    timestamp,data = cept
    badge = badgeParse(data)
    if badge is not None:
        badge[BADGE_TIME] = timestamp
        live_display.intercept(badge)
        names_display.intercept(badge)
        badge_display.intercept(badge)
        log.intercept(cept)

btQueue = deque(maxlen=1000)
bt = BTAdapter(root, btQueue)
bt.start()

def signal_handler(signal, frame):
    bt.stop()
    log.closeout()
    root.quit()
signal.signal(signal.SIGINT, signal_handler)

def btPoller():
    while True:
        try:
            intercept = btQueue.pop()
            processAdvertisement(intercept)

        except IndexError:
            break;
            
    root.after(100, btPoller)
btPoller()


root.mainloop()
