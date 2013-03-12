---
layout: post
title: "Loose coopling hierarchical gerrit workflow"
date: 2013-03-11 22:58
comments: true
categories: [gerrit, git]
---

As we are working on an Android project, we are facing the very large scale source tree (10GB level), including works from BSP team (bootloader, linux kernel), midware team (A/V codec, media framework),  to Android team (Android HAL, vendor specific Apk). We need to do frequent relaese (weekly) for several products from single source tree, without any quality degradation. 

A work flow serves these requirement need to:

1. have least cross team dependancy.
2. provide tight collaboration inside team.
3. easily locate bad node when doing release.

To achive these requirements, I defined below branches:

1. There's one trunk branch for releasing. Every node is a stable node (passed full test).
2. Every team has independent developing branch, based on last release version, closed after current release is done.
3. Integration branch is based on last release version, will integrate above developing branches. Perform full test on this branch, cull out bad node when failed.

And a workflow based on these branches:

1. Every developer submits to his team's developing branch, gerrit review servers as first gate keeper in this stage. Add auto build, auto test in gerrit merge hook, let every commit has a build image, and this image will contain all its preceding commits, this makes possible of using binary search to cull out bad node in QA stage.
2. After submit window closed, integrators merge these developing branches into one integration branch, perform full test on the result, if failed, apply the failed case on all final nodes of developing branches, this step picks out the failed developing branch. Then use binary search method on this developing branch to cull out the bad node, and update the developing branch. Iterate untill pass full test.
3. Push the passed result into release branch, open submit window, start next cycle.

*Braches:*
{% img http://yongbingchen.github.com/images/gerrit/branches.png  %}
