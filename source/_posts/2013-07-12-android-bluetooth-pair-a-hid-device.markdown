---
layout: post
title: "Android Bluetooth: pairing a HID device"
date: 2013-07-12 23:06
comments: true
categories: [Android, Bluetooth] 
---

{% img http://yongbingchen.github.com/images/android_bt/android-pair-hid-device.jpg  %}

1. User started scanning from Bluetooth Settings UI, Android Bluetooth service responded to this request, calling bluedroid to start discovery.
2. Bluedroid found nearby devices in discovery mode, reported them through device found callback.
3. User selected one device from found devices, started to pairing it, Android Bluetooth service called bluedroid to creat bond with it.
4. Bluedroid requested PIN code from end user (simple secure pairing mode has different procedure).
5. Bluedroid started SDP process to find remote device's UUID.
6. After SDP finished, bluedroid called remote device property changed on UUID changed to notify Android.
7. Android received the event, then HID profile service started to connect it. Without this SDP event, the pair will fail due to no further action over L2CAP anymore, see <a href ="http://yongbingchen.github.com/logs/andoird-hid/SDP-fail-cause-HID-pair-fail.txt ">pairing failed by SDP</a> for example.
8. In HID profile connection process, bluedroid conducted another round of SDP, fetched HID descriptor from remote device, created hidraw and input device file for the new remote device, through uhid interface, the remote device was ready to use from that point.

Logs and call stack:
{% blockquote scan to found device cb http://yongbingchen.github.com/logs/andoird-hid/start-discovery-to-found-device-callback.txt %} {% endblockquote %} 
{% blockquote pair to bonded cb http://yongbingchen.github.com/logs/andoird-hid/start-pairing-to-bonded.txt %} {% endblockquote %} 
{% blockquote on UUID changed to HID connect http://yongbingchen.github.com/logs/andoird-hid/onUuidChanged-to-HID-connect.txt %} {% endblockquote %} 

Full logs:
{% blockquote scan http://yongbingchen.github.com/logs/andoird-hid/logs/scan.txt %} {% endblockquote %} 
{% blockquote pair http://yongbingchen.github.com/logs/andoird-hid/logs/pair.txt %} {% endblockquote %} 
{% blockquote hcidump http://yongbingchen.github.com/logs/andoird-hid/logs/hcidump.txt %} {% endblockquote %} 
{% blockquote air log http://yongbingchen.github.com/logs/andoird-hid/logs/scan-pair.cfa %} {% endblockquote %} 
