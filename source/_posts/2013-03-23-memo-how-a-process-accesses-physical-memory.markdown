---
layout: post
title: "Memo: how a process accesses physical memory"
date: 2013-03-23 21:39
comments: true
categories: 
---
Scenario: a process acquired a new block of memory, then try to access part of this block:
{% codeblock lang:c %}
        unsigned char * buf = malloc(1024);
        *(buf + 3) = 0xfe;
{% endcodeblock %}
What happens in malloc():

1. stdlib will try to handle it internally, if can't satisfy this request, thne,
2. Call brk() to let kernel enlarge current process's heap space.
    * VMA of heap will be enlarged to contains new space.

What would happen when try to access the memory:

1. CPU use the virtual address to look up the TLB, not found.
2. Then CPU switch to look this address in process's Page Table, try to do the virtual-to-physical address translation. 
3. Step 2 will failed, a Page Fault exception happens.
   * In Page Fault exception handler, check if current process has right permission to this address, that's done by check process's VMA list.
   * Allocate a physical page, update process's Page Table for this page.
4. After the exception handler returned, and the process get scheduled to execute again, it will resume from step 2, get correct physical address pointed to the new page. Update TLB for this new map.
5. Create new entry in Cache for this physical address, write the new value to Cache, then hardware will write back this new value from Cache to real memory at proper time. 
