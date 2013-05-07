---
layout: post
title: "Dissect Bluedroid from A2DP: Part IV: A2DP traffic"
date: 2013-05-07 04:50
comments: true
categories: [Android, Bluetooth]
---
Item A. Connect a remote A2DP device:
{% img center http://yongbingchen.github.com/images/bluedroid/a2dp_connect.jpg  %}
1. Android system will try to reconnect paired A2DP device automatically after BT enable.
{% codeblock lang:cpp %}
04-25 01:56:31.080 D/BluetoothAdapterService( 2093): Auto Connecting A2DP Profile with device 50:C9:71:0D:D2:D9
	packages/apps/Bluetooth/jni/com_android_bluetooth_a2dp.cpp	
	static jboolean connectA2dpNative(JNIEnv *env, jobject object, jbyteArray address)
		const bt_interface_t* btInf= getBluetoothInterface();
		const btav_interface_t *sBluetoothA2dpInterface = (btav_interface_t *)btInf->get_profile_interface(BT_PROFILE_ADVANCED_AUDIO_ID);
		status = sBluetoothA2dpInterface->connect((bt_bdaddr_t *)addr)
			external/bluetooth/bluedroid/btif/src/btif_av.c
			btif_queue_connect(UUID_SERVCLASS_AUDIO_SOURCE, bd_addr, connect_int);//This will trigger an event in btu_task, now the caller thread returned.
				GKI_send_msg(BTIF_TASK, BTU_BTIF_MBOX, p_msg);
{% endcodeblock %}
2. This will later trigger a A2DP server event API_CONNECT_REQ_EVT in state CCB_IDLE_ST:
{% codeblock lang:cpp %}
05-02 01:54:33.342 I/bt-avp  ( 2093): CCB ccb=0 event=API_CONNECT_REQ_EVT state=CCB_IDLE_ST
{% endcodeblock %}
3. A2DP server handle this event in bellow two actions:
{% codeblock lang:cpp %}
external/bluetooth/bluedroid/stack/avdt/avdt_ccb.c
129 const UINT8 avdt_ccb_st_idle[][AVDT_CCB_NUM_COLS] = {
130 /* Event                      Action 1                    Action 2                    Next state */
139 /* API_CONNECT_REQ_EVT */    {AVDT_CCB_SET_CONN,          AVDT_CCB_CHAN_OPEN,         AVDT_CCB_OPENING_ST},
//3.1 Set CCB variables associated with AVDT_ConnectReq().
996 void avdt_ccb_set_conn(tAVDT_CCB *p_ccb, tAVDT_CCB_EVT *p_data)
	BTM_SetSecurityLevel(TRUE, "", BTM_SEC_SERVICE_AVDTP, p_data->connect.sec_mask,AVDT_PSM, BTM_SEC_PROTO_AVDT, AVDT_CHAN_SIG);

//3.2 initiate a signaling channel connection.
87 void avdt_ccb_chan_open(tAVDT_CCB *p_ccb, tAVDT_CCB_EVT *p_data)
					BTM_SetOutService(p_ccb->peer_addr, BTM_SEC_SERVICE_AVDTP, AVDT_CHAN_SIG);
					avdt_ad_open_req(AVDT_CHAN_SIG, p_ccb, NULL, AVDT_INT);

{% endcodeblock %}
4 How L2CAP handle this channel connection:
{% codeblock lang:cpp %}
external/bluetooth/bluedroid/stack/l2cap/l2c_api.c						
229 UINT16 L2CA_ErtmConnectReq (UINT16 psm, BD_ADDR p_bd_addr, tL2CAP_ERTM_INFO *p_ertm_info)						
	p_lcb = l2cu_allocate_lcb (p_bd_addr, FALSE);
	l2cu_create_conn(p_lcb);//This function initiates an acl connection via HCI
		2180 BOOLEAN l2cu_create_conn_after_switch (tL2C_LCB *p_lcb)
			//external/bluetooth/bluedroid/stack/hcic/hcicmds.c
			btsnd_hcic_create_conn (p_lcb->remote_bd_addr,HCI_PKT_TYPES_MASK_DM1 + HCI_PKT_TYPES_MASK_DH1,page_scan_rep_mode,page_scan_mode,clock_offset,allow_switch));
				HCI_CMD_TO_LOWER(p_buf);
					//external/bluetooth/bluedroid/main/bte_main.c
					bt_hc_if->transmit_buf((TRANSAC)p_msg, \ (char *) (p_msg + 1), \p_msg->len);
						utils_enqueue(&tx_q, (void *) transac);
						 bthc_signal_event(HC_EVENT_TX);
			btu_start_timer (&p_lcb->timer_entry, BTU_TTYPE_L2CAP_LINK,L2CAP_LINK_CONNECT_TOUT);
	if (p_lcb->link_state == LST_CONNECTED)
		l2c_csm_execute (p_ccb, L2CEVT_L2CA_CONNECT_REQ, NULL);
{% endcodeblock %}


Item B. a2dp_write data path:
{% img center http://yongbingchen.github.com/images/bluedroid/a2dp_write.jpg  %}
1. A2DP client write to A2DP data socket will trigger API_WRITE_REQ_EVT in SCB_STREAM_ST state:
{% codeblock lang:cpp %}
05-02 01:14:03.134 I/bt-avp  ( 2139): SCB hdl=1 event=1/API_WRITE_REQ_EVT state=SCB_STREAM_ST
394 /* state table for streaming state */
395 const UINT8 avdt_scb_st_stream[][AVDT_SCB_NUM_COLS] = {
396 /* Event                     Action 1                       Action 2                    Next state */
398 /* API_WRITE_REQ_EVT */     {AVDT_SCB_HDL_WRITE_REQ,        AVDT_SCB_CHK_SND_PKT,       AVDT_SCB_STREAM_ST},
{% endcodeblock %}
2. A2DP server handle this in bellow two action:
{% codeblock lang:cpp %}
//2.1 builds a new media packet and stores it in the SCB.
external/bluetooth/bluedroid/stack/avdt/avdt_scb_act.c
1320 void avdt_scb_hdl_write_req(tAVDT_SCB *p_scb, tAVDT_SCB_EVT *p_data) 

//2.2 sends this stored media packet to L2CAP layer.
1921 void avdt_scb_chk_snd_pkt(tAVDT_SCB *p_scb, tAVDT_SCB_EVT *p_data)
	avdt_ad_write_req(AVDT_CHAN_MEDIA, p_scb->p_ccb, p_scb, p_pkt);
		L2CA_DataWrite(avdt_cb.ad.rt_tbl[avdt_ccb_to_idx(p_ccb)][tcid].lcid, p_buf);
{% endcodeblock %}
3. L2CAP to HCI layer
{% codeblock lang:cpp %}
	bluedroid/stack/l2cap/l2c_api.c
1633 UINT8 L2CA_DataWrite (UINT16 cid, BT_HDR *p_data)
1634 {
1636     return l2c_data_write (cid, p_data, L2CAP_FLUSHABLE_CH_BASED);
					p_ccb = l2cu_find_ccb_by_cid (NULL, cid);
					l2c_csm_execute (p_ccb, L2CEVT_L2CA_DATA_WRITE, p_data);
						935 static void l2c_csm_open (tL2C_CCB *p_ccb, UINT16 event, void *p_data)//Just consider channel connected state
						1050     case L2CEVT_L2CA_DATA_WRITE:                    /* Upper layer data to send */
						1051         l2c_enqueue_peer_data (p_ccb, (BT_HDR *)p_data);
						1052         l2c_link_check_send_pkts (p_ccb->p_lcb, NULL, NULL);
						1053         break;
								l2c_link_check_send_pkts (p_lcb, NULL, NULL);
									l2c_link_send_to_lower (p_lcb, p_buf);
									1341 #if BLE_INCLUDED == TRUE
									1342         if (p_lcb->is_ble_link)
									1344             L2C_LINK_SEND_BLE_ACL_DATA(p_buf);
									1346         else
									1349             L2C_LINK_SEND_ACL_DATA (p_buf);
														bte_main_hci_send((BT_HDR *)(p), BT_EVT_TO_LM_HCI_ACL);
															bt_hc_if->transmit_buf((TRANSAC)p_msg, \(char *) (p_msg + 1),p_msg->len);
																bluedroid/hci/src/bt_hci_bdroid.c:249 static int transmit_buf(TRANSAC transac, char *p_buf, int len)
																	utils_enqueue(&tx_q, (void *) transac);
																	bthc_signal_event(HC_EVENT_TX);
{% endcodeblock %}
4 HCI content write to hardware device driver 
{% codeblock lang:cpp %}
339 static void *bt_hc_worker_thread(void *arg)
		if (events & HC_EVENT_TX)
			p_hci_if->send(sending_msg_que[i]);
			593 void hci_h4_send_msg(HC_BT_HDR *p_msg)
				bytes_sent = userial_write(event,(uint8_t *) p,bytes_to_send);
					 ret = write(userial_cb.fd, p_data+total, len);
{% endcodeblock %}
