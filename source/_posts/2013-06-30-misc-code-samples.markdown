---
layout: post
title: "misc code samples"
date: 2013-06-30 22:49
comments: true
categories: [linux]
---

1 inotify: monitoring hidraw device file add/remove events:
{% include_code inotify-example.c lang:c %}
2 pthread timer_create: periodically generate an event, as a heartbeat:
{% include_code pthread_timer-example.c lang:c %}
3 netlink NETLINK_KOBJECT_UEVENT: same purpose as #1:
{% include_code ueventd-example.c lang:c %}

