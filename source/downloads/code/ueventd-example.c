/*
 * Copyright (C) 2013 Yongbing Chen <yongbing.chen.wh@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/types.h>
#include <unistd.h>
 
static int32_t init_event_sock(void)
{
    struct sockaddr_nl netlink_addr;
    const int32_t buffersize = 128 * 1024;
    int32_t ret = 0;

    memset(&netlink_addr, 0x00, sizeof(struct sockaddr_nl));
    netlink_addr.nl_family = AF_NETLINK;
    netlink_addr.nl_pid = getpid();
    netlink_addr.nl_groups = 1;

    int32_t event_socket = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_KOBJECT_UEVENT);
    if (event_socket == -1) {
        int32_t err = errno;
        printf("error getting socket: %s", strerror(errno));
        return -err;
    }

    /* set receive buffersize */
    setsockopt(event_socket, SOL_SOCKET, SO_RCVBUFFORCE, &buffersize, sizeof(buffersize));

    ret = bind(event_socket, (struct sockaddr *) &netlink_addr, sizeof(struct sockaddr_nl));
    if (ret < 0) {
        int32_t err = errno;
        printf("bind failed: %s", strerror(errno));
        close(event_socket);
        return -err;
    }

    return event_socket;
}




int32_t main(int32_t argc, char **argv)
{
	char event_sock_buf[1024] = {0};

	int32_t event_socket = init_event_sock();
	if (event_socket < 0){
		return EXIT_FAILURE;
	}

	while (1){
		int32_t len = TEMP_FAILURE_RETRY(recv(event_socket, &event_sock_buf, sizeof(event_sock_buf), 0));
		if (len < 0){
			printf("recv failed: %s", strerror(errno));
			return EXIT_FAILURE;
		}
		int32_t i = 0;
		while (i < len) {
			printf("next event in buf: %s\n",event_sock_buf+i);
			i += strlen(event_sock_buf+i)+1;
		}
	}
	close(event_socket);
	return EXIT_SUCCESS;
}


#ifdef __RUNNING_LOG
shell@android:/data/app # ./test_uevent
[ 4550.363457] usb 1-1: USB disconnect, device number 2
...
next event in buf: remove@/devices/soc.0/f7ed0000.usb/usb1/1-1/1-1:1.2/0003:046D:C52B.0003/hidraw/hidraw0
next event in buf: ACTION=remove
next event in buf: DEVPATH=/devices/soc.0/f7ed0000.usb/usb1/1-1/1-1:1.2/0003:046D:C52B.0003/hidraw/hidraw0
next event in buf: SUBSYSTEM=hidraw
next event in buf: MAJOR=249
next event in buf: MINOR=0
next event in buf: DEVNAME=hidraw0
next event in buf: SEQNUM=878
...

[ 4554.467274] usb 1-1: new full-speed USB device number 3 using berlin-ehci
[ 4554.626594] usb 1-1: New USB device found, idVendor=046d, idProduct=c52b
[ 4554.633604] usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[ 4554.641231] usb 1-1: Product: USB Receiver
[ 4554.645650] usb 1-1: Manufacturer: Logitech
[ 4554.664080] logitech-djreceiver 0003:046D:C52B.0007: hiddev0,hidraw0: USB HID v1.11 Device [Logitech USB Receiver] on usb-f7ed0000.usb-1/input2
[ 4554.686310] input: Logitech Unifying Device. Wireless PID:2012 as /devices/soc.0/f7ed0000.usb/usb1/1-1/1-1:1.2/0003:046D:C52B.0007/input/input3
...
next event in buf: add@/devices/soc.0/f7ed0000.usb/usb1/1-1/1-1:1.2/0003:046D:C52B.0007/hidraw/hidraw0
next event in buf: ACTION=add
next event in buf: DEVPATH=/devices/soc.0/f7ed0000.usb/usb1/1-1/1-1:1.2/0003:046D:C52B.0007/hidraw/hidraw0
next event in buf: SUBSYSTEM=hidraw
next event in buf: MAJOR=249
next event in buf: MINOR=0
next event in buf: DEVNAME=hidraw0
next event in buf: SEQNUM=896
#endif
