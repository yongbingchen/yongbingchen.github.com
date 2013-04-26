---
layout: post
title: "Skeleton of a Bluetooth SDIO driver"
date: 2013-04-23 05:28
comments: true
categories: [linux, Bluetooth] 
---

A Bluetooth SDIO card driver talks with hardware through SDIO interface, providing R/W method for Bluetooth Adapter layer, here's the class diagram for this relationship:
{% img http://yongbingchen.github.com/images/bt_drv/bt_drv_class_diagram.jpg  %}

The outbound Bluetooth data path:

1. Upper layer use HCI interface send() to send data/command packet, implemented as btmrvl_send_frame() in this driver.
2. Put this packet in adapter's tx queue, wakeup the main data processing thread (like NAPI in a network driver, thread function is btmrvl_service_main_thread()).
3. In main data procssing thread, re-organize skb data payload for DMA transfer (in btmrvl_tx_pkt()).
4. Call sdio_writesb() to write data to hardware (in btmrvl_sdio_host_to_card()).

The incoming Bluetooth data path:

1. SDIO card received a data packet, triggered a interrupt to host.
2. The SDIO ISR triggred the main data processing thread.
3. In this thread, allocate a skb with DAM aligned, call sdio_readsb() to read the data from SDIO interface (in btmrvl_sdio_card_to_host()).
4. Call hci_recv_frame(skb) to send this data packet to upper layer Bluetooth stack.

Appendix: How to register a driver specific ISR to SDIO's ISR:
{% codeblock In driver module init, hook up a device ISR to SDIO's ISR. lang:c %}
    int __init btmrvl_sdio_init_module(void)
    |
    |
    -->sdio_register_driver(&bt_mrvl_sdio) != 0) 
    	|
    	|
    	-->btmrvl_sdio_probe(struct sdio_func *func, const struct sdio_device_id *id)
    		|
    		|
    		-->sdio_claim_irq(func, btmrvl_sdio_interrupt);{% endcodeblock %}
{% codeblock In this ISR, wake up main data pocessing thread to read data from card. lang:c %}
    void btmrvl_sdio_interrupt(struct sdio_func *func)
    |
    |
    -->btmrvl_interrupt(priv);
    	|
    	|
    	-->wake_up_interruptible(&priv->main_thread.wait_q);{% endcodeblock %}

Source Code:
{% blockquote @btmrvl_sdio.c http://lxr.linux.no/linux+v3.8.8/drivers/bluetooth/btmrvl_sdio.c %}
{% endblockquote %}
{% blockquote @btmrvl_main.c http://lxr.linux.no/linux+v3.8.8/drivers/bluetooth/btmrvl_main.c %}
{% endblockquote %}

##Note: How to load firmware for a SDIO device##
{% codeblock 1. Disable interrupt from this SDIO device. lang:c %}
sdio_claim_host(card->func);
btmrvl_sdio_disable_host_int(card);
	host_int_mask = sdio_readb(card->func, card->reg->host_int_mask, &ret);
	host_int_mask &= ~HIM_DISABLE;
	sdio_writeb(card->func, host_int_mask, card->reg->host_int_mask, &ret);
sdio_release_host(card->func);
{% endcodeblock %}

{% codeblock 2. Get and Write firmware. lang:c %}
sdio_claim_host(card->func);
	//Get firmware from user space.
	request_firmware(&fw_firmware, card->firmware,//name of firmware file, = "mrvl/sd8787_uapsta.bin",
			&card->func->dev);
	//Write firmware into SDIO device, check firmware status.
	tmpfwbufsz = ALIGN_SZ(BTM_UPLD_SIZE, BTSDIO_DMA_ALIGN);
	fwbuf = (u8 *) ALIGN_ADDR(tmpfwbuf, BTSDIO_DMA_ALIGN);
	memcpy(fwbuf, &firmware[offset], txlen);
	sdio_writesb(card->func, card->ioport, fwbuf,tx_blocks * blksz_dl);
	
	//Release firmware related resource in kernel.
	release_firmware(fw_firmware);
sdio_release_host(card->func);
{% endcodeblock %}
Alternatively, you can use request_firmware_nowait() if current thread is not allowed to sleep for a long time.


{% codeblock 3. Enable SDIO device interrupt. lang:c %}
btmrvl_sdio_enable_host_int(card);
{% endcodeblock %}


Reference:
{% blockquote @How request_firmware() works http://git.kernel.org/cgit/linux/kernel/git/torvalds/linux.git/tree/Documentation/firmware_class/README?id=HEAD %}
{% endblockquote %}
{% blockquote @Default firmware search path in Android http://stackoverflow.com/questions/6019915/kernel-module-cannot-find-firmware-file-where-should-it-be %}
{% endblockquote %}
{% codeblock $Jellybean/system/core/init/devices.c lang:c %}
#define SYSFS_PREFIX    "/sys"
#define FIRMWARE_DIR1   "/etc/firmware"
asprintf(&root, SYSFS_PREFIX"%s/", uevent->path);
asprintf(&file1, FIRMWARE_DIR1"/%s", uevent->firmware);
fw_fd = open(file1, O_RDONLY);
{% endcodeblock %}


