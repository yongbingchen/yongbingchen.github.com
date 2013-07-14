---
layout: post
title: "Dissect Bluedroid from A2DP: Part II: Init Bluetooth Adapter"
date: 2013-05-04 02:37
comments: true
categories: [Android, Bluetooth]
---

Android defined one interface bt_interface_t to control a Bluetooth Adapter, Bluedroid implemented this interface as "static const bt_interface_t bluetoothInterface" in external/bluetooth/bluedroid/btif/src/bluetooth.c.

{% img center http://yongbingchen.github.com/images/bluedroid/init_bt_adapter.jpg  %}

{% blockquote logcat:A2DP http://yongbingchen.github.com/txt/bluedroid/a2dp-init-logcat.txt %}
{% endblockquote %}

