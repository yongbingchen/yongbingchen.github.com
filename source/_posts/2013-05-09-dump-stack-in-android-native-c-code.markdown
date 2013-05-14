---
layout: post
title: "dump_stack in Android native C code"
date: 2013-05-09 21:34
comments: true
categories: [Android]
---
1 Wrapper Android C++ method into a C function:
{% codeblock lang:cpp %}
external/bluetooth/bluedroid$ cat bta/sys/dump_stack.cpp
#include <utils/CallStack.h>
using namespace android;
extern "C" void dump_stack_android(void)
{
        CallStack stack;
        stack.update();
        stack.dump();
}
{% endcodeblock %}

2 Call this C function from target place:
{% codeblock lang:cpp %}
diff --git a/bta/dm/bta_dm_act.c b/bta/dm/bta_dm_act.c
+extern void dump_stack_android(void);
 static void bta_dm_adjust_roles(BOOLEAN delay_role_switch)
 {
+    dump_stack_android();
     if(bta_dm_cb.device_list.count)
     {
{% endcodeblock %}
	 
3 Add library libutils as dependency in LOCAL_SHARED_LIBRARIES, 
{% codeblock lang:cpp %}
diff --git a/main/Android.mk b/main/Android.mk
 LOCAL_SRC_FILES+= \
 +       ../bta/sys/dump_stack.cpp \
         ../udrv/ulinux/uipc.c

 LOCAL_SHARED_LIBRARIES := \
     libcutils \
+    libutils \
{% endcodeblock %}

I met a ld error as:
{% codeblock lang:cpp %}
error: undefined reference to 'android::CallStack::CallStack()'
{% endcodeblock %}
It turned out that I added the libutils dependency into a static library libbt-brcm_bta, which can not solve this dynamic symbol at link time 
{% blockquote refer to this wiki https://en.wikipedia.org/wiki/Static_library %}{% endblockquote %}
Move the dependency to dynamic library bluetooth.default.so solved the problem.

The final result on running board is:
{% codeblock lang:cpp %}
root@android:/ # logcat -v time |grep CallStack&
05-09 21:01:57.666 D/CallStack( 2133): (null)#00  pc 0005c444  /system/lib/hw/bluetooth.default.so (dump_stack_android+15)
05-09 21:01:57.666 D/CallStack( 2133): (null)#01  pc 0004ddb2  /system/lib/hw/bluetooth.default.so
05-09 21:01:57.666 D/CallStack( 2133): (null)#02  pc 0004c310  /system/lib/hw/bluetooth.default.so (bta_sys_conn_close+27)
05-09 21:01:57.666 D/CallStack( 2133): (null)#03  pc 000577d4  /system/lib/hw/bluetooth.default.so (bta_av_str_closed+115)
05-09 21:01:57.666 D/CallStack( 2133): (null)#04  pc 0004705e  /system/lib/hw/bluetooth.default.so (bta_av_ssm_execute+269)
05-09 21:01:57.666 D/CallStack( 2133): (null)#05  pc 00046f1c  /system/lib/hw/bluetooth.default.so (bta_av_hdl_event+159)
05-09 21:01:57.666 D/CallStack( 2133): (null)#06  pc 0004bf02  /system/lib/hw/bluetooth.default.so (bta_sys_event+49)
05-09 21:01:57.666 D/CallStack( 2133): (null)#07  pc 00074b70  /system/lib/hw/bluetooth.default.so (btu_task+559)
05-09 21:01:57.666 D/CallStack( 2133): (null)#08  pc 00042784  /system/lib/hw/bluetooth.default.so (gki_task_entry+91)
05-09 21:01:57.666 D/CallStack( 2133): (null)#09  pc 0000e3d8  /system/lib/libc.so (__thread_entry+72)
05-09 21:01:57.666 D/CallStack( 2133): (null)#10  pc 0000dac4  /system/lib/libc.so (pthread_create+160)
{% endcodeblock %}

This result can be verified as the same as addr2line:
{% codeblock lang:cpp %}
$ arm-eabi-addr2line -e ../../../out/target/product/bg2ct_dmp_emmc/symbols/system/lib/hw/bluetooth.default.so 0004c310
external/bluetooth/bluedroid/bta/./sys/bta_sys_conn.c:236
{% endcodeblock %}
