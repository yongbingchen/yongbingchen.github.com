---
layout: page
title: "bug: keep on reset bt will crash android"
date: 2013-03-11 02:12
comments: true
sharing: true
categories: [Android, Bluetooth]
footer: true
---
**Bug: Keep on resetting BT adapter cause Android crash**

Bug description: In GTV/Honeycomb's System Setting UI, keep on turning on/off Bluetooth for about 270 times, Android will crash, and UI restart from blackscreen.

My analyze:
First thing is to located where Android system crashed from logcat, there must be something related to zygote, then I found this:
{% codeblock %}
	03-08 20:15:00.026 I/Zygote  ( 3749): Exit zygote because system server (3780) has terminated
{% endcodeblock %}
Above that I found a fatal error message:
{% codeblock %}
 03-08 20:14:59.226 F/Looper  ( 3780): Could not create wake pipe.  errno=24 
{% endcodeblock %}
then all system service died like below:
{% codeblock %}
03-08 20:14:59.316 I/ServiceManager(  690): service 'usagestats' died
{% endcodeblock %}
Tracing down the fatal error message in Android XRef, the error occurs when creating a new Looper instance, in Honeycomb/frameworks/base/libs/utils/Looper.cpp, where 3 new pips need to be created for one new Looper object.
So I guess the isssue is kind of file/socket/pipe handle leakage issue, to verify this, compare "lsof" result after turn on/off Bluetooth once, I can see there's 3 newly opened pipes in system_service everytime I performed a turn on/off Bluetooth. Then I found from website that someone reported similar issue on [Android Bug List](https://code.google.com/p/android/issues/detail?id=24414 "Android Bug List")
{% codeblock %}
/ # lsof |grep system_se >/data/debug/after_turn_off_bt.txt
/ # hciconfig hci0 up; sleep 5; hciconfig hci0 down
/ # lsof |grep system_se >/data/debug/after_turn_off_bt_again.txt
/ # busybox diff  /data/debug/after_turn_off_bt.txt /data/debug/after_turn_off_bt_again.txt
 system_se   887        ???  224       ???                ???       ???        ??? /dev/null
+system_se   887        ???  225       ???                ???       ???        ??? pipe:[7641]
+system_se   887        ???  227       ???                ???       ???        ??? pipe:[7641]
+system_se   887        ???  228       ???                ???       ???        ??? anon_inode:[eventpoll]
{% endcodeblock %}

The symptom is quite clear now, but how to locate leakage position in Android?

The first clue is that Looper object is used for Android Handler object, it's hard to dump call stack in Android native C++ service, but it's easy to do that in Java.
So I added a dump stack in Handler's construction method, caught below message when I turn on Bluetooth:
{% codeblock %}
03-11 05:14:07.806 D/Handler (  898):android.os.Handler
03-11 05:14:07.806 D/Handler (  898):com.android.internal.util.HierarchicalStateMachine$HsmHandler
03-11 05:14:07.806 D/Handler (  898):com.android.internal.util.HierarchicalStateMachine$HsmHandler
03-11 05:14:07.806 D/Handler (  898):com.android.internal.util.HierarchicalStateMachine
03-11 05:14:07.806 D/Handler (  898):com.android.internal.util.HierarchicalStateMachine
03-11 05:14:07.806 D/Handler (  898):android.bluetooth.BluetoothDeviceProfileState
03-11 05:14:07.806 D/Handler (  898):android.server.BluetoothService
03-11 05:14:07.806 D/Handler (  898):android.server.BluetoothService
03-11 05:14:07.806 D/Handler (  898):android.server.BluetoothService
03-11 05:14:07.806 D/Handler (  898):android.server.BluetoothService$EnableThread
{% endcodeblock %}
After doing some source code search, I reached the place in BluetoothService.java when problem occurs:
When turn off BT, BluetoothService will do:
{% codeblock lang:java %}
2146     void removeProfileState(String address) {
2147         mDeviceProfileState.remove(address); //Comment out this line so BluetoothService will not create a new BluetoothDeviceProfileState every time BT reset.
2148     }
{% endcodeblock %}

Then when turn on BT again, it will do:
{% codeblock lang:java %}
2136     BluetoothDeviceProfileState addProfileState(String address) {
2137         BluetoothDeviceProfileState state = mDeviceProfileState.get(address);
2138         if (state != null) return state;
2140         state = new BluetoothDeviceProfileState(mContext, address, this, mA2dpService);
2144     }
{% endcodeblock %}

Problem here is, we only need do a reset when turn off BT, then we can reuse the BluetoothDeviceProfileState object after we turn it on again, but Android removed it instead, then need to create a new BluetoothDeviceProfileState object next time. If we did not remove the object every time we turn off BT, then the issue disappeared.

*My comments:*
For Java class who claims system resource in its construction method, we must declare corresponding interface to release them, GC can not do this job for us. 




