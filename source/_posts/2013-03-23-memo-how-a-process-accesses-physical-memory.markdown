---
layout: post
title: "Refresh memo: how a process accesses physical memory"
date: 2013-03-23 21:39
comments: true
categories: [linux]
---
Scenario: a process acquired a new block of memory, then try to access part of this block:
{% codeblock lang:c %}
        unsigned char * buf = (unsigned char *)malloc(1024);
        if (buf != NULL)
                *(buf + 3) = 0xfe;
{% endcodeblock %}
What happens in malloc():

1. stdlib will try to handle it internally, if can't satisfy this request, then,
2. Call sbrk() to let kernel enlarge current process's heap space (by adjust process's VMAs).

What happens next when try to access the memory:

1. CPU's MMU use this virtual address to look up in current CPU's TLB, not found.
2. Then MMU switch to look up this address in process's Page Table, try to do the virtual-to-physical address translation. 
3. Step 2 will fail, a Page Fault exception happens.
   * In Page Fault exception handler, check if current process has write permission to this address, that's done by check process's VMA list.
   * Allocate a physical page, update process's Page Table for this page.
4. After the exception handler returned, and the process get scheduled to execute again, it will retry the instruction that caused the Page Fault, this time will get correct physical address pointed to the new page. Update TLB entry for this new map.
5. Select proper line in Cache for this physical address, write the new value to Cache, then hardware will write back this new value from Cache to real memory at proper time. 
