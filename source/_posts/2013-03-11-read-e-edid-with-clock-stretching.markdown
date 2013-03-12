---
layout: post
title: "Read e-EDID with clock stretching"
date: 2013-03-11 17:00
comments: true
categories: 
---

##Read e-EDID with master clock stretching##
In E-DDC, a special I²C addressing scheme was introduced, in which multiple 256-byte segments could be selected. To do this, a single 8-bit segment index is passed to the display via the I²C address 30h. (Because this access is always a write, the first I²C octet will always be 60h.).

In my MHL project, this first write will always get an I2C NO_ACK error(E-DDC need tolerate with NO_ACK error for this write, to be back-compatible with legacy DDC, so we are using GPIO to simulate I2C, instead of using standard I2C controller).

From below signal, we found the ACK actually came, but master did not acquire it at that time, that's exactly what clock stretching targeted at.

{% img images/edid/write_0x60_clock_stretch.jpg  %}

Add clock stretching logic in our GPIO simulation function, then can successfully read all e-EDID segments.

