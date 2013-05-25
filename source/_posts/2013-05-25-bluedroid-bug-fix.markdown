---
layout: post
title: "bluedroid bug fixing"
date: 2013-05-25 07:13
comments: true
categories: [Bluetooth] 
---
I fixed thress tricky bugs in one week, that's pretty efficient, even out of my own expectation ^-^

##bug I: TiVo remote connected but not working.

QA reported a bug that TiVo remote control connected, but no key input captured by our host.

Compared with the normal case (some other HID device), I found below message missed for TiVo case after BT connect:
{% codeblock%}
05-16 22:13:34.386 D/bt-btif ( 2146): UHID_START from uhid-dev
05-16 22:13:34.386 D/bt-btif ( 2146): UHID_OPEN from uhid-dev
{% endcodeblock %}

That means the input device for this newly added HID device(TiVo remote) has never been opened by Android EventHub.cpp in failed case.

Then I checked the getevent result when TiVo remote connected, found out that there's no input device created for it at all.

Tracing from where Bluedroid created a uhid device for a newly added HID device in bta_hh_co_send_hid_info() down to hid kernel code, turned out that the Vendor_id/Product_id for TiVo remote already registered in hid kernel part (point to an existing kernel driver linux/drivers/hid/hid-tivo.c, which is based on BlueZ hidp), so this is an issue of Bluedroid, affecting a group of device which has registered id in kernel as HID_BLUETOOTH_DEVICE.

This kind of devices suppose to be servered by their own driver, so the generic probe method hid_device_probe() will not be triggered when trying to create a uhid device from Bluedroid, thus the input device file will not be created by input_register_device() within this probe.


##bug II: Lenovo BT mouse can not pair with our host.
The log shows below error when trying to pair Lenovo BT mouse:
{% codeblock%}
05-16 21:07:41.451 W/bt-sdp  ( 2146): result :36005A0900000A000100010200013503191200090004350D350619010009000135031900010902053503191002090009350835061912000901000902000901000902010917EF0902020960020902030902450902042801090205090002
05-16 21:07:41.451 W/bt-sdp  ( 2146): SDP - Bad type: 0x02 or len: 4 in attr_rsp
{% endcodeblock %}

Compared with air log, I found that one byte of the SDP message is different:
In air log, I found :0900000A00010001<font color = "red">02</font>0001,
while in dump message in the error message, it's 0900000A00010001<font color = "green">09</font>0001

Narrow down this change from driver level to Bluedroid SDP, finally found out it's a bug in btsnoop_capture(), a simple load/restore bug.

##bug III: Can not unpair a remote HID device.

QA found one HID device can not be unpaired: if the device is connected when doing unpair, the device remain connected after unpair, and can not be paired again after unpair.

From the log, I found below error message when trying to re-piar:
{% codeblock%}
05-20 21:09:19.567  2125  2158 W bt-btif : btif_hh_connect: Device  already added, attr_mask = 0x8005
05-20 21:09:19.567  2125  2158 E bt-btif : btif_hh_connect: Error, device  can only be reconnected from device side
{% endcodeblock %}

Compared with another HID device which can successfully do unpair/pair, I found that the success case can correctly response to host's BTA_HH_CTRL_VIRTUAL_CABLE_UNPLUG request as defined in HID spec 1.1, figure A.4.

In the failed case, the HID device will not initiate disconnect against our host after receiving the unplug request, and Bluedroid in our host failed to deal with this situation, as required by HID spec, to delete pair information after issue this requirement. Due to this remaining piece of pair information, next pair attempt will fail.
