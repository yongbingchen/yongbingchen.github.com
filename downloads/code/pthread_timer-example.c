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
#include <sys/types.h>
#include <pthread.h>
#include <time.h>


typedef struct wrapper_class{
	int is_alarm_fired;
	timer_t timerid;
}wrapper_class, *wrapper_class_t;


static void something_is_on_fire(wrapper_class_t p)
{
	printf("%s, is_alarm_fired %d\n", __func__, p->is_alarm_fired);
}

static void defuse_the_fire_alarm(wrapper_class_t p)
{
	printf("%s, is_alarm_fired %d\n", __func__, p->is_alarm_fired);
}

static void trigger_alarm(union sigval sigev_val)
{
	wrapper_class_t p = (wrapper_class_t)sigev_val.sival_ptr;
	//TODO: may need mutex to protect object p.
	if (p->is_alarm_fired){
		something_is_on_fire(p);
	}
}

static void start_timer_for_alarm(wrapper_class_t p)
{
	struct sigevent sev;
	memset(&sev, 0, sizeof( struct sigevent));
	sev.sigev_notify = SIGEV_THREAD;
	sev.sigev_notify_function = trigger_alarm;
	sev.sigev_value.sival_ptr = p;
	if (timer_create(CLOCK_REALTIME, &sev, &p->timerid) == -1){
		return;
	}

	printf("timer ID is 0x%x\n", p->timerid);

	struct itimerspec its;
	its.it_value.tv_sec = 10;
	its.it_value.tv_nsec = 0;
	its.it_interval.tv_sec = 10;
	its.it_interval.tv_nsec = 0;
	if (timer_settime(p->timerid, 0, &its, NULL) == -1){
		printf("timer_settime failed, err %s\n", strerror(errno));
		timer_delete(p->timerid); p->timerid = 0;
		return;
	}
}

static void event_handler(wrapper_class_t p)
{
	if (p->is_alarm_fired){
 		something_is_on_fire(p);
		start_timer_for_alarm(p);
	}
	else{
		if (p->timerid != 0){
			timer_delete(p->timerid); p->timerid = 0;
		}
 		defuse_the_fire_alarm(p);
	}
}

//Do not forget to delete this timer in wrapper class's destroy method!
//for abnormal exit path.
static void wrapper_class_deconstructor(wrapper_class_t p)
{
	if (p->timerid != 0){
		timer_delete(p->timerid); p->timerid = 0;
	}
	free(p);
}

int32_t main(int32_t argc, char** argv)
{
	wrapper_class_t p = (wrapper_class_t)calloc(1, sizeof(wrapper_class));
	if (p == NULL)
		return EXIT_FAILURE;
	
	p->is_alarm_fired = 1;
	event_handler(p);
	sleep(55);
	
	p->is_alarm_fired = 0;
	event_handler(p);
	wrapper_class_deconstructor(p);

	return EXIT_SUCCESS;
}

#ifdef __RUNNING_LOG
shell@android:/data/app # logwrapper ./test_pthread_timer
07-01 03:01:37.248 I/test_pthread_timer( 2945): something_is_on_fire, is_alarm_fired 1
07-01 03:01:37.248 I/test_pthread_timer( 2945): timer ID is 0x80000000
07-01 03:01:47.248 I/test_pthread_timer( 2945): something_is_on_fire, is_alarm_fired 1
07-01 03:01:57.248 I/test_pthread_timer( 2945): something_is_on_fire, is_alarm_fired 1
07-01 03:02:07.248 I/test_pthread_timer( 2945): something_is_on_fire, is_alarm_fired 1
07-01 03:02:17.248 I/test_pthread_timer( 2945): something_is_on_fire, is_alarm_fired 1
07-01 03:02:27.248 I/test_pthread_timer( 2945): something_is_on_fire, is_alarm_fired 1
07-01 03:02:32.248 I/test_pthread_timer( 2945): defuse_the_fire_alarm, is_alarm_fired 0
#endif
