---
layout: post
title: "Dissect Bluedroid from A2DP: Part III: Init A2DP Service"
date: 2013-05-07 04:17
comments: true
categories: [Android, Bluetooth]
---

1. Get Android defined A2DP interface btav_interface_t from bt_interface_t get_bluetooth_interface().
2. Init A2DP service by btav_interface_t->init().
    * Start a btif_media_task as main loop for A2DP service.
    * Open a socket to listen on client's connect request from control channel.

{% img center http://yongbingchen.github.com/images/bluedroid/init_a2dp_service.jpg  %}
