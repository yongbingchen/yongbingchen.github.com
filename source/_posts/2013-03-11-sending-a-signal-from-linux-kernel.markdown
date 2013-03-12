---
layout: post
title: "Sending a signal from Linux kernel"
date: 2013-03-11 06:17
comments: true
categories: [Linux]
---
**Send SIGUSR1 from kernel**

I want to use a signal to inform an asynchronous event from one kernel module to a user space application.

To achieve this, add below code in kernel driver:

{% codeblock patch on kernel module lang:c %}
+#include <linux/sched.h>
+#include <asm/siginfo.h>
+#include <linux/pid_namespace.h>
+#include <linux/pid.h>

typedef enum
 {
+       THIS_MODULE_IOCTL_SET_OWNER = 0x111,
 }MODULE_IOCTL_CMD;


+static int owner;
+static struct task_struct * current_task;

+static void send_signal(int sig_num)
+{
+       printk("%s,%d.sending to owner %d\n",__func__, __LINE__, owner);
+       struct siginfo info;
+       memset(&info, 0, sizeof(struct siginfo));
+       info.si_signo = sig_num;
+       info.si_code = 0;
+       info.si_int = 1234;
+       if (current_task == NULL){
+               rcu_read_lock();
+               current_task = pid_task(find_vpid(owner), PIDTYPE_PID);
+               rcu_read_unlock();
+       }
+       int ret = send_sig_info(sig_num, &info, current_task);
+       if (ret < 0) {
+               printk("error sending signal\n");
+       }
+}
+
 static int device_event_handler(struct snd_pcm_substream *substream, int cmd)
 {
		if (cmd == INTRESTED_EVENT) {
+               send_signal(SIGUSR1);



@@ -325,6 +352,13 @@ static long device_ioctl(struct file *file,
+       case THIS_MODULE_IOCTL_SET_OWNER:
+               printk("%s, owner pid %d\n",__func__, owner);
+               if(copy_from_user(&owner, (int *)arg, sizeof(int))){
+                       ret = -EFAULT;
+               }
+               current_task = NULL;
+               break;

+MODULE_LICENSE("GPL");
{% endcodeblock %}

In user space application, set its pid to kernel module, then listen on the signal.
{% codeblock patch on user application lang:c %}
+#include <pthread.h>
+#include <semaphore.h>
+#include <signal.h>
+static sem_t event_sem;
+static volatile sig_atomic_t intrested_event = 0;
+
+void sig_handler_event1(int sig)
+{
+       LOGD("%s,%d, received signale %d, intrested_event1.\n",
+                       __func__, __LINE__, sig);
+       intrested_event1 = 1;
+       sem_post(&event_sem);
+}

+static void * event_handler_thread_func()
+{
+        while(1){
+                sem_wait(&event_sem);
+                if (intrested_event){
+                        LOGD("%s,%d, received intrested_event signal.\n",__func__, __LINE__);
+                        intrested_event = 0;
+                }
+        }
+        pthread_exit(NULL);
+}

int main(int argc, char *argv[])
+               pthread_t event_thread;
+               if (pthread_create(&event_thread, NULL, event_handler_thread_func, NULL) != 0){
+               	perror("Thread create failed");
+               	exit(1);
+               }
+               sem_init(&event_sem, 0, 0);

+               struct sigaction usr_action;
+               sigset_t block_mask;
+               sigfillset (&block_mask);
+               usr_action.sa_handler = sig_handler_event1;
+               usr_action.sa_mask = block_mask;//do not receive any signal inside signal handler.
+               usr_action.sa_flags = 0;
+               int fd = open("/dev/target_device_name", O_RDWR);
+               ioctl(fd, 0x111, &my_pid);
+               close(fd);
{% endcodeblock %}

A signal could interrupt below primitive calls:
*close, fcntl (operation F_SETLK), open, read, recv, recvfrom, select, send, sendto, tcdrain, waitpid, wait, and write*,
POXIS and BSD will handle this situation differently. POXIS will fail these primitive call with EINTR, caller need to use macro TEMP_FAILURE_RETRY (expression) to retry.
Programmer need to take care of this issue.

Reference:
<http://www.cs.utah.edu/dept/old/texinfo/glibc-manual-0.02/library_21.html#SEC349>?
<http://www.makelinux.net/ldd3/chp-6-sect-2>?
