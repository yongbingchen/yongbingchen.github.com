/*
 * Copyright (C) 2005 The Android Open Source Project
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

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <poll.h>
#include <sys/epoll.h>
#include <sys/inotify.h>
#include <pthread.h>
#include <unistd.h>
 
static const unsigned int EPOLL_ID_INOTIFY = 0x80000001;
#define MONITORING_DIR "/dev"
#define HIDRAW_DEV_FILE_PREFIX "hidraw"

static int init_epoll_fd()
{
	int epoll_fd = epoll_create(1);
	if (epoll_fd < 0){
		printf("epoll_crate failed %s",strerror(errno));
		return -1;
	}
	return epoll_fd;
}

static int init_inotify(char * dev_path, int epoll_fd)
{
	int inotify_fd = inotify_init();
	if(inotify_fd < 0){
		printf("inotify_init failed, errno %s",strerror(errno));
		return -1;
	}

	int result = inotify_add_watch(inotify_fd, dev_path, IN_DELETE | IN_CREATE);
	if(result < 0){
		printf("Could not register INotify for %s  errno %s", dev_path, strerror(errno));
		close(inotify_fd);
		return -1;
	}
	return inotify_fd;
}

static int add_inotify_to_epoll(int epoll_fd, int inotify_fd)
{
	struct epoll_event event_item;
	memset(&event_item, 0, sizeof(event_item));
	event_item.events = EPOLLIN;
	event_item.data.u32 = EPOLL_ID_INOTIFY;
	int result = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, inotify_fd, &event_item);
	if (result != 0){
		printf("Could not add INotify to epoll instance.  errno %s", strerror(errno));
		return -1;
	}
	return 0;
}

static void process_add_event(char *hidraw_file_name)
{
	printf("%s, %s\n", __func__, hidraw_file_name);
	//may add the file to epoll here.
}

static void process_remove_event(char *hidraw_file_name)
{
	printf("%s, %s\n", __func__, hidraw_file_name);
	//may remove the file from epoll here.
}

//Cite from Android EventHub.cpp
static int read_inotify(int inotify_fd) 
{
    int rc;
    char devname[128];
    char *filename;
    char event_buf[512];
    int event_size;
    int event_pos = 0;
    struct inotify_event *event;

    printf("read_inotify fd: %d\n", inotify_fd);
    rc = TEMP_FAILURE_RETRY(read(inotify_fd, event_buf, sizeof(event_buf)));
    if(rc < (int)sizeof(*event)) {
        printf("could not get event, %s\n", strerror(errno));
        return -1;
    }

    strcpy(devname, MONITORING_DIR);
    filename = devname + strlen(devname);
    *filename++ = '/';

    while(rc >= (int)sizeof(*event)) {
        event = (struct inotify_event *)(event_buf + event_pos);
        printf("%d: %08x \"%s\"\n", event->wd, event->mask, event->len ? event->name : "");
        if(event->len && !strncmp(event->name, HIDRAW_DEV_FILE_PREFIX, strlen(HIDRAW_DEV_FILE_PREFIX))) {
            strcpy(filename, event->name);
            if(event->mask & IN_CREATE) {
                process_add_event(devname);
            } else {
                process_remove_event(devname);
            }
        }
        event_size = sizeof(*event) + event->len;
        rc -= event_size;
        event_pos += event_size;
    }
    return 0;
}

static void process_inode_event(struct epoll_event *events, 
		int event_counts, int inotify_fd)
{
	int i;
	for (i = 0; i < event_counts; i++){
		if (events->data.u32 == EPOLL_ID_INOTIFY){
			if (events->events & EPOLLIN) {
				read_inotify(inotify_fd);
			} else {
				printf("received unexpected epoll event 0x%08x for inotify.", events->events);
			}
		}
		events++;
	}
}

int main(int argc, char **argv)
{
	int epoll_fd = init_epoll_fd();
	if (epoll_fd < 0)
		return EXIT_FAILURE;

	int inotify_fd = init_inotify(MONITORING_DIR, epoll_fd);
	if (inotify_fd < 0){
		close(epoll_fd);
		return EXIT_FAILURE;
	}

	int rc = add_inotify_to_epoll(epoll_fd, inotify_fd);
	if (rc < 0){
		close(epoll_fd);close(inotify_fd);
		return EXIT_FAILURE;
	}

	static const int EPOLL_MAX_EVENTS = 16;
	struct epoll_event epoll_events[EPOLL_MAX_EVENTS];
	while(1){
		int rc = TEMP_FAILURE_RETRY(epoll_wait(epoll_fd, epoll_events, EPOLL_MAX_EVENTS, -1));
		if (rc <= 0) {
			printf("poll failed errno=%s\n", strerror(errno));
			return 1;
		} else {
			process_inode_event(epoll_events, rc, inotify_fd);
		}
	}
	close(inotify_fd);
	close(epoll_fd);
 
 	return EXIT_SUCCESS;
 }
 
#ifdef __RUNNING_LOG
 130|shell@android:/data/app # ./test_inotify
[ 4998.540741] usb 1-1: USB disconnect, device number 3
read_inotify fd: 4
1: 00000200 "hidraw0"
Removing device '/dev/hidraw0' due to inotify event
process_remove_event, /dev/hidraw0
read_inotify fd: 4
1: 00000200 "hidraw1"
Removing device '/dev/hidraw1' due to inotify event
process_remove_event, /dev/hidraw1
[ 5002.247269] usb 1-1: new full-speed USB device number 4 using berlin-ehci
[ 5002.406589] usb 1-1: New USB device found, idVendor=046d, idProduct=c52b
[ 5002.413862] usb 1-1: New USB device strings: Mfr=1, Product=2, SerialNumber=0
[ 5002.421457] usb 1-1: Product: USB Receiver
[ 5002.425927] usb 1-1: Manufacturer: Logitech
read_inotify fd:[ 5002.443452] logitech-djreceiver 0003:046D:C52B.000B: hiddev0,hidraw0: USB HID v1.11 Device [Logitech USB Receiver] on usb-f7ed0000.usb-1/input2
 4
1: 00000100 "hidraw0"
process_add_event, /dev/hidraw0
[ 5002.464392] input: Logitech Unifying Device. Wireless PID:2012 as /devices/soc.0/f7ed0000.usb/usb1/1-1/1-1:1.2/0003:046D:C52B.000B/input/input4
read_inotify fd:[ 5002.479144] logitech-djdevice 0003:046D:C52B.000C: input,hidraw1: USB HID v1.11 Keyboard [Logitech Unifying Device. Wireless PID:2012] on usb-f7ed0000.usb-1:1
 4
1: 00000100 "hidraw1"
process_add_event, /dev/hidraw1
#endif

