#!/usr/bin/env python

import nfc
clf = nfc.ContactlessFrontend()
assert clf.open('tty:USB0:pn532') is True   # open /dev/ttyUSB0 with pn532 driver

from nfc.clf import RemoteTarget

target = clf.sense(RemoteTarget('106A'), RemoteTarget('106B'))
print(target)

tag = nfc.tag.activate(clf, target)
print(tag)

# Alternate tag acquisition method
#tag = clf.connect(rdwr={'on-connect': lambda tag: False})
#print(tag)


# there don't appear to be any ndef records on our tag
assert tag.ndef is not None
for record in tag.ndef.records:
    print(record)
#
#record = tag.ndef.records[0]
#print(record.type)
#print(record.uri)

clf.close()
