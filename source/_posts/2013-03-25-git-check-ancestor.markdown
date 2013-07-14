---
layout: post
title: "git: check ancestor"
date: 2013-03-25 22:59
comments: true
categories: [git] 
---
If you want to check your code base, it's wrong to rely on whether the output of <code>git log $CODE_BASE_TAG..HEAD</code> is empty.

Suppose we have a git version tree as below:

{% img http://yongbingchen.github.com/images/git/version-graph.jpg  %}

DevBranch0 is branched out from Trunk, merged <code>Trunk</code> node C and <code>DevBranch0</code> node b, current HEAD is <code>DevBranch0_c</code>.

The git history is as below:
{% codeblock %}
yongbing@ubuntu:~/work/git_ancestor$ git log --oneline --graph
* 368cfb1 DevBranch0_c
*   6fecd0a DevBranch0_Merge
|\
| * defff5c Trunk_C
| * 5037bca Trunk_B
* | 0f45ab0 DevBranch0_b
* | 38f9ec9 DevBranch0_a
|/
* fc18378 Trunk_A
* 0d3b0bd Init state
{% endcodeblock %}

Clearly version <code>Trunk_D</code> is not an ancestor of <code>DevBranch0_c</code>, but <code>git log $Trunk_D..HEAD</code> is not empty:
{% codeblock %}
yongbing@ubuntu:~/work/git_ancestor$ git tag Trunk_D b25477d
yongbing@ubuntu:~/work/git_ancestor$ git log Trunk_D..HEAD --oneline
368cfb1 DevBranch0_c
6fecd0a DevBranch0_Merge
0f45ab0 DevBranch0_b
38f9ec9 DevBranch0_a
{% endcodeblock %}

The correct way is to use <code>git rev-list</code> command, then grep the result to see if the code base in really in the ancestor list:
{% codeblock %}
yongbing@ubuntu:~/work/git_ancestor$ git log -g --oneline
b25477d HEAD@{1}: commit: Trunk_D
5037bca HEAD@{9}: commit: Trunk_B
yongbing@ubuntu:~/work/git_ancestor$ git rev-list -n 10 HEAD |grep 5037bca
5037bca575ecbf76d9c9d939e52f8858dbaadfe1
yongbing@ubuntu:~/work/git_ancestor$ git rev-list -n 10 HEAD |grep b25477d
//Empty output

{% endcodeblock %}
