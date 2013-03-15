---
layout: post
title: "Loose coopling hierarchical gerrit workflow"
date: 2013-03-11 22:58
comments: true
categories: [gerrit, git]
---

As we are working on an Android project, we are handling a very large scale source tree (10GB level), including works from BSP team (bootloader, linux kernel), midware team (A/V codec, media framework),  and Android team (Android HAL, vendor specific Apks). We need to do weekly relaese for several products from single source tree, ensuring no quality degradation in the mean time. 

A work flow serves these requirement needs to:

1. have least cross team dependancy.
2. provide tight collaboration in developers.
3. easily locate bad node when doing release.

To achive these requirements, I defined below branches:

1. One trunk branch for releasing. Every node is a stable node (passed full test).
2. Every team has one independent developing branch, covering this team's source code partition, based on last release version, closed after current release is done.
3. Integration branch is based on last release version, will integrate above developing branches. Perform full test on this branch, cull out bad node when failed.

And a workflow based on these branches:

1. Every developer submits to developing branch (cross team submit is permitted), gerrit review process servers as first gate keeper in this stage. Add auto build, auto test in gerrit merge hook, let every commit has a build image, and this image will contain all its preceding commits, this makes possible of using binary search to cull out bad node in QA stage.
2. After submit window closed, integrators merge these developing branches into one integration branch, perform full test on the result, if failed, apply the failed case on all tail nodes of developing branches, this step picks out the failed developing branch. Then use binary search method on this developing branch to cull out the bad node, and update the developing branch. Iterate untill integration result pass full test.
3. Push the passed result into release branch, open submit window, start next developing/release cycle.

*Branches:*
{% img http://yongbingchen.github.com/images/gerrit/branches.jpg  %}
