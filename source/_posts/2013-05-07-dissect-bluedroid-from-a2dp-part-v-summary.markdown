---
layout: post
title: "Dissect Bluedroid from A2DP: Part V: Key components"
date: 2013-05-07 20:57
comments: true
categories: [Android, Bluetooth]
---

###1 Interface and implementation

<font color = "green">bt_interface_t</font>: Android defined, Bluedroid implemented as bluetoothInterface in external/bluetooth/bluedroid/btif/src/bluetooth.c: System control BT adapter.

<font color = "green">btav_interface_t</font>: Android defined, Bluedroid implemented as bt_av_interface in external/bluetooth/bluedroid/btif/src/btif_av.c: System control A2DP service.

<font color = "green">audio_hw_device and audio_stream_out</font>: Android defined, Bluedroid implemented in external/bluetooth/bluedroid/audio_a2dp_hw/audio_a2dp_hw.c: AudioFlinger use A2DP client as audio output device.

<font color = "green">bt_vendor_interface_t</font>: Bluedroid defined, BRCM implemented as BLUETOOTH_VENDOR_LIB_INTERFACE in device/common/libbt/src/bt_vendor_brcm.c: Bluedroid talk to BT char device driver, internal usage only.

<font color = "green">tHCI_IF</font>: Bluedroid defined, Bluedroid implemented as hci_h4_func_table in external/bluetooth/bluedroid/hci/src/hci_h4.c: Bluedroid HCI interface (data/cmd/evt in/out), internal usage only.

<font color = "green">bt_hc_interface_t</font>: Bluedroid defined, Bluedroid implemented as bluetoothHCLibInterface in external/bluetooth/bluedroid/hci/src/bt_hci_bdroid.c: Wrapper of tHCI_IF, has bt_hc_worker_thread to serialize downcoming HCI commands and read upcoming data/evt from HCI device.

<font color = "green">L2CAP </font> layer API, in external/bluetooth/bluedroid/stack/include/l2c_api.h, internal usage only.

<font color = "green">LMP </font> API, in external/bluetooth/bluedroid/stack/include/btm_api.h, internal usage only.

###2 Tasks/Roles/Layers

<font color = "green">btif_task</font>, managing all messages being passed Android Bluetooth HAL and BTA.

<font color = "green">btu_task</font>, the main task of the Bluetooth Upper Layers unit, routing in/out BT cmd/event/data, processing timeout events.

<font color = "green">bt_hc_worker_thread</font>, HCI worker thread, all HCI traffic come through this thread.

<font color = "green">userial_read_thread</font>, monitoring incoming packets from BT char device driver, transfering these to bt_hc_worker_thread.

<font color = "green">btif_media_task</font>, task for A2DP SBC encoder.

<font color = "green">uipc_read_task</font>, A2DP server thread, receive audio input data from A2DP client, feed into btif_media_task.

<font color = "green">UIPC/A2DP_CTRL_PATH/A2DP_DATA_PATH</font>, socket based IPC, for A2DP client connect/control to A2DP server.

<font color = "green">AVDT_CHAN_SIG/AVDT_CHAN_MEDIA/AVDT_CHAN_REPORT</font>, A2DP channels, communicate with remote device.

Serial Finished.

Reference:
{% blockquote source code reading note http://yongbingchen.github.com/txt/bluedroid/a2dp-source-code-reading-note.txt %}
{% endblockquote %}
