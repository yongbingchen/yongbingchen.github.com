---
layout: post
title: "atomic file writing"
date: 2013-07-24 05:16
comments: true
categories: [linux]
---

QA reported a bug, sometimes when power cycle target board right after pairing a bluetooth device, the whole bluetooth paring info lost.
I found out the root cause is that the configure file of bluedroid lost all content in this case.
Turned out the bluedroid configure file writing operation is not atomic.
I came up with a revised file writing process, to guarantee it's atomic:

1. copy the configure file to a temporary file.
2. write the update content to the temporary file.
3. fsync the temporary file. (the step that bluedroid missed)
4. rename the temporary file to configure file. 

Because rename is atomic, and all steps before it are revertable (will not affect the configure file), so the whole process is atomic.

{% include_code atomic_write.c lang:c %}

Reference:
{% blockquote atomic unix methods http://rcrowley.org/2010/01/06/things-unix-can-do-atomically.html%}{% endblockquote %}
{% blockquote man fsync http://linux.die.net/man/3/fsync%}{% endblockquote %}

