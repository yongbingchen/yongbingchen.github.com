---
layout: page
title: "bug: keep on reset bt will crash android"
date: 2013-03-11 02:12
comments: true
sharing: true
footer: true
---
**Bug: Keep on resetting BT adapter cause Android crash**

Bug description: In GTV/Honeycomb's System Setting UI, keep on turn on/off Bluetooth for about 270 times, Android will crash, and UI restart from blackscreen.

My analyze:
1. From logcat, located where Android system crashed, there must be something related to zygote, then I found this:
{% codeblock %}
	03-08 20:15:00.026 I/Zygote  ( 3749): Exit zygote because system server (3780) has terminated
{% endcodeblock %}
2. Above that I found a fatal error message:
{% codeblock %}
 03-08 20:14:59.226 F/Looper  ( 3780): Could not create wake pipe.  errno=24 
{% endcodeblock %}
then all system service died like below:
{% codeblock %}
03-08 20:14:59.316 I/ServiceManager(  690): service 'usagestats' died
{% endcodeblock %}
3. Tracing down the fatal error message in Android Xref, it occurs when creating a new Looper instance, in Honeycomb/frameworks/base/libs/utils/Looper.cpp, where 3 new pips need to be created for one new Looper object.
4. So I guess the isssue is kind of file/socket/pipe handle leakage issue, to verify this, compare "lsof" result after turn on/off Bluetooth once, I can see there's 3 newly opened pipes in system_service everytime I performed a turn on/off Bluetooth. Then I found from website that someone reported similar issue on [Android Bug List](https://code.google.com/p/android/issues/detail?id=24414 "Android Bug List")
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

5. The symptom is quite clear now, but how to locate leakage position in Android?
a. The first clue is Looper object is used for Android Handler object, it's hard to dump call stack in Android native C++ service, but it's easy to do that in Java.
b. So I add a dump stack in Handler's construction method, caught below message when I turn on Bluetooth:
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
c. Doing some source code search, I reached the place in BluetoothService.java when problem occurs:
c.1 When turn off BT, BluetoothService will do:
{% codeblock lang:java %}
2146     void removeProfileState(String address) {
2147         mDeviceProfileState.remove(address); //Comment out this line so BluetoothService will not create a new BluetoothDeviceProfileState every time BT reset.
2148     }
{% endcodeblock %}
c.2 Then when turn on BT again, it will do:
{% codeblock lang:java %}
2136     BluetoothDeviceProfileState addProfileState(String address) {
2137         BluetoothDeviceProfileState state = mDeviceProfileState.get(address);
2138         if (state != null) return state;
2140         state = new BluetoothDeviceProfileState(mContext, address, this, mA2dpService);
2144     }
{% endcodeblock %}

c.3 Problem here is, we only need do a reset when turn off BT, then we can resue the BluetoothDeviceProfileState object after we turn it on again, but instead we removed it, so we need to create a new BluetoothDeviceProfileState object next time. If we did not remove the object every time we turn off BT, then the issue disappeared.

My comments:
1. For Java class who will claim system resource, we must declare corresponding interface to release them, GC can not do this job for us. 




