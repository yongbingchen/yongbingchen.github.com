---
layout: post
title: "Dissect Bluedroid from A2DP  Part I: use case"
date: 2013-05-03 22:05
comments: true
categories: [Android, Bluetooth] 
---

Android framework defined two hardware interfaces for operating audio output devices, they are audio_hw_device_t and audio_stream_out_t, AudioFlinger is the only user of these interfaces. Bluedroid implemented these two interface in external/bluetooth/bluedroid/audio_a2dp_hw/audio_a2dp_hw.c, AudioFlinger can output audio sample to a connected A2DP sink device if this implementation has been registered to Android.
{% img http://yongbingchen.github.com/images/bluedroid/A2dp_use_case.jpg  %}

In Bluedroid's A2DP hardware implementation, it will use two sockets to communicate with A2DP server.
{% img center http://yongbingchen.github.com/images/bluedroid/audio_a2dp_hw_bluedroid.jpg  %}

Reference:
{% blockquote Source Code http://androidxref.com/4.2.2_r1 %}
{% endblockquote %}
