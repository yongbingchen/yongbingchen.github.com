---
layout: post
title: "Dissect Bluedroid from A2DP: Part III: Init A2DP Service"
date: 2013-05-07 04:17
comments: true
categories: [Android, Bluetooth]
---

{% img center http://yongbingchen.github.com/images/bluedroid/init_a2dp_service.jpg  %}
1 Get Android defined A2DP interface btav_interface_t from bt_interface_t get_bluetooth_interface().
{% codeblock lang:cpp %}
04-25 01:56:30.530 I/BluetoothA2dpServiceJni( 2093): classInitNative: succeeds
packages/apps/Bluetooth/jni/com_android_bluetooth_a2dp.cpp
137 static void initNative(JNIEnv *env, jobject object)
	GLOBAL const btav_interface_t *sBluetoothA2dpInterface = (btav_interface_t *)btInf->get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
	sBluetoothA2dpInterface->init(&sBluetoothA2dpCallbacks);
		external/bluetooth/bluedroid/btif/src/btif_av.c
		725 static bt_status_t init(btav_callbacks_t* callbacks )
			686 int btif_a2dp_start_media_task(void)
				GKI_create_task((TASKPTR)btif_media_task, A2DP_MEDIA_TASK,
			btif_enable_service(BTA_A2DP_SERVICE_ID);//Upon BT enable, BTIF core shall invoke the BTA APIs to enable the profiles
{% endcodeblock %}

2 Init A2DP service by btav_interface_t->init().
    * Start a btif_media_task as main loop for A2DP service.
    * Open a socket to listen on client's connect request from control channel.
{% codeblock lang:cpp %}
external/bluetooth/bluedroid/btif/src/btif_media_task.c
//Task for SBC encoder.  This task receives an event when the waveIn interface has a pcm data buffer ready.  On receiving the event, handle all ready pcm data buffers.  If stream is started, run the SBC encoder on each chunk of pcm samples and build an output packet consisting of one or more encoded SBC frames.
1066 int btif_media_task(void *p)
1073     btif_media_task_init();
1044     UIPC_Init(NULL);
			606 UDRV_API void UIPC_Init(void *p_data)
				569 int uipc_start_main_server_thread(void)
					pthread_create(&uipc_main.tid, (const pthread_attr_t *) NULL, (void*)uipc_read_task, NULL)
1047     UIPC_Open(UIPC_CH_ID_AV_CTRL , btif_a2dp_ctrl_cb);
			uipc_setup_server_locked(ch_id, A2DP_CTRL_PATH, p_cback);//This is the control socket that listen on A2DP client.
1079     while (1)//A2DP event loop 
1080     {
1085         if (event & BTIF_MEDIA_TASK_CMD)
1090                 btif_media_task_handle_cmd(p_msg);
1093
1094         if (event & BTIF_MEDIA_TASK_DATA)
1099                 btif_media_task_handle_media(p_msg);
1102
1103         if (event & BTIF_MEDIA_AA_TASK_TIMER)
1105             /* advance audio timer expiration */
1106             btif_media_task_aa_handle_timer();
{% endcodeblock %}


Reference:
{% blockquote logcat:A2DP http://yongbingchen.github.com/txt/bluedroid/a2dp-init-logcat.txt %}
{% endblockquote %}
{% blockquote air log: LMP http://yongbingchen.github.com/images/bluedroid/A2DP-connect-LMP.jpg %}
{% endblockquote %}
{% blockquote air log: L2CAP http://yongbingchen.github.com/images/bluedroid/A2DP-connect-L2CAP.jpg %}
{% endblockquote %}
{% blockquote source code reading note http://yongbingchen.github.com/txt/bluedroid/a2dp-source-code-reading-note.txt %}
{% endblockquote %}

