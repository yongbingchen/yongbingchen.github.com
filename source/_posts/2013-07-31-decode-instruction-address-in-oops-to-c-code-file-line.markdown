---
layout: post
title: "decode instruction address in OOPS to C code file:line"
date: 2013-07-31 05:47
comments: true
categories: [linux]
---
The OOPS message:
{% codeblock lang:c %}
[ 2405.090047] Unable to handle kernel paging request at virtual address 008b2005
[ 2405.097586] pgd = 80004000
[ 2405.100427] [008b2005] *pgd=00000000
[ 2405.104187] Internal error: Oops: 5 [#1] PREEMPT SMP ARM
[ 2405.109673] Modules linked in: mbtusbchar usbfwdnld fusion(O) gal3d amp_core(O)
[ 2405.117310] CPU: 1    Tainted: G        W  O  (3.4.50+ #11)
[ 2405.123072] PC is at module_put+0x44/0x8c
[ 2405.127219] LR is at cdev_put+0x24/0x28
[ 2405.131182] pc : [<8007521c>]    lr : [<800c97d0>]    psr: 20000013
[ 2406.111129] Backtrace:
[ 2406.113680] [<800751d8>] (module_put+0x0/0x8c) from [<800c97d0>] (cdev_put+0x24/0x28)
[ 2406.121764]  r4:45a8e010 r3:ffffffff
[ 2406.125491] [<800c97ac>] (cdev_put+0x0/0x28) from [<800c7410>] (fput+0x21c/0x22c)
{% endcodeblock%}

Method A: Recompile vmlinux or failed module with debug info enabled if possible (CONFIG_DEBUG_INFO=y), then gdb can give out the C source code line of the failed instruction:
{% codeblock lang:c %}
$ make $MY_DEFAULT_CONFIG_FILE -C $LINUX_ROOT_DIR CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm
$ make $MY_UIMAGE_NAME -C $LINUX_ROOT_DIR CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm CONFIG_DEBUG_INFO=y -j32
{% endcodeblock%}
Recompile individual module if needed:
{% codeblock lang:c %}
$ make modules -C $LINUX_ROOT_DIR CROSS_COMPILE=arm-linux-gnueabi- ARCH=arm CONFIG_DEBUG_INFO=y -j32
{% endcodeblock%}

Use gdb to get C source file:line info from given instruction address:
{% codeblock lang:c %}
$ arm-linux-androideabi-gdb –f vmlinux
Reading symbols from linux/vmlinux...done.
(gdb) list *(module_put+0x44)
0x80073964 is in module_put (kernel/module.c:942).
937     void module_put(struct module *module)
938     {
939             if (module) {
940                     preempt_disable();
941                     smp_wmb(); /* see comment in module_refcount */
942                     __this_cpu_inc(module->refptr->decs);
943
944                     trace_module_put(module, _RET_IP_);
945                     /* Maybe they're waiting for us to drop reference? */
946                     if (unlikely(!module_is_live(module)))
(gdb) disas (module_put+0x44)
Dump of assembler code for function module_put:
   0x80073960 <+64>:    add     r3, r3, #4
   0x80073964 <+68>:    ldr     r1, [r3, r2]
{% endcodeblock%}

Method B: If can not recompile the vmlinux/module, but the problematic vmlinux/module file is available, use objdump to get the assembly line, but no C source line available.
{% codeblock lang:c %}
$ arm-linux-androideabi-objdump -dSl vmlinux >vmlinux.disasm
{% endcodeblock%}

module_put+0x44 = 0x800751d8 + 0x44 = 0x8007521c:
{% codeblock lang:c %}
800751d8 <module_put>:
module_put():
8007521c:       e7931002        ldr     r1, [r3, r2] 
{% endcodeblock%}
Compared with method A, method B get the same instruction line.



Appendix: only for comparison/reference: use objdump to decode an object file with debug info:
{% codeblock lang:c %}
$ arm-linux-androideabi-objdump -dSlr kernel/module.o >kernel/module.disasm
{% endcodeblock%}

Check if module_put+0x44 = 0xf28+0x44 = 0xf6c point to the same C source line and assembly line:
{% codeblock lang:c %}
 00000f28 <module_put>:
  module_put():
  linux/kernel/module.c:938
          return ret;
  }
  EXPORT_SYMBOL(try_module_get);
 
  void module_put(struct module *module)
  {
       f28:       e1a0c00d        mov     ip, sp
       f2c:       e92dd818        push    {r3, r4, fp, ip, lr, pc}
       f30:       e24cb004        sub     fp, ip, #4
  linux/kernel/module.c:939
          if (module) {
       f34:       e3500000        cmp     r0, #0
       f38:       089da818        ldmeq   sp, {r3, r4, fp, sp, pc}
  current_thread_info():
  linux/arch/arm/include/asm/thread_info.h:97
       f3c:       e1a0300d        mov     r3, sp
       f40:       e3c34d7f        bic     r4, r3, #8128   ; 0x1fc0
       f44:       e3c4403f        bic     r4, r4, #63     ; 0x3f
  module_put():
  linux/kernel/module.c:940
                  preempt_disable();
       f48:       e5943004        ldr     r3, [r4, #4]
       f4c:       e2833001        add     r3, r3, #1
       f50:       e5843004        str     r3, [r4, #4]
  linux/kernel/module.c:941
                  smp_wmb(); /* see comment in module_refcount */
       f54:       f57ff05f        dmb     sy
  linux/kernel/module.c:942
                  __this_cpu_inc(module->refptr->decs);
       f58:       e59f2050        ldr     r2, [pc, #80]   ; fb0 <module_put+0x88>
       f5c:       e5941014        ldr     r1, [r4, #20]
       f60:       e5903138        ldr     r3, [r0, #312]  ; 0x138
       f64:       e7922101        ldr     r2, [r2, r1, lsl #2]
       f68:       e2833004        add     r3, r3, #4
       f6c:       e7931002        ldr     r1, [r3, r2]
       f70:       e2811001        add     r1, r1, #1
       f74:       e7831002        str     r1, [r3, r2]
  linux/kernel/module.c:946
{% endcodeblock%}
