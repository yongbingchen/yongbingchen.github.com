---
layout: post
title: "create my own repo to manage bundle of git repositories"
date: 2015-03-27 17:35
comments: true
categories: [gerrit]
---
##Create repo manifest:

1. As gerrit server administrator, create below pojects in gerrit server
    1. container //for global access control, choose "Only serve as parent for other projects", unselect "Create initial empty commit"
    2. container/repo_1/manifest //repo manifest, access right inherit from "container", unselect "Only serve as parent for other projects", select "Create initial empty commit"
    3. container/repo_1/git_one //child project inside the repo, access right inherit from "container", unselect "Only serve as parent for other projects", select "Create initial empty commit".
    4. container/repo_1/git_two 
2. Create my own repo config:
    1. on a client working machine, checkout the manifest git first: 
```sh
~$ git clone ssh://gerritreview.com:29418/container/repo_1/manifest
```
<p/>
    b. add a new file manifest.xml to the git as below, and merge it to master branch:

```xml
	<?xml version="1.0" encoding="UTF-8"?>
	<manifest>
	        <remote  name="my_repo"
	                fetch="ssh://gerritreview.com:29418/"
	                review="http://gerritreview.com/gerrit/"
	                />
	        <default revision="my_working_branch"
	                remote="my_repo"
	                sync-j="4"
	                />
	
	        <project path="container/repo_1/git_one" name="container/repo_1/git_one" />
	        <project path="container/repo_1/git_two" name="container/repo_1/git_two" />
	</manifest>
```
<p/>
3. On gerrit server, create a working "Branch Name" "my_working_branch" on these three projects, with "Initial Revision" set to "master" in https://gerritreview.com/gerrit/#/admin/projects/container/repo_1/${GIT_NAME},branches
<p/>
4. On a client's working machine, checkout the repo with below command:

```sh
~$ repo init -u ssh://gerritreview.com:29418/container/repo_1/manifest -b my_working_branch -m manifest.xml --repo-url https://chromium.googlesource.com/external/repo.git --no-repo-verify
~$ repo sync -j 32
```

<h2> Setup the access control rules for this repo:</h2>

1. define thress group and their access right: <br />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; developers  :can submit CL, checkout code, review +/_ 1  <br />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; reviewers   :can review +/- 2 <br />
  &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; submitters  :can merge CL to working branch <br />
2. implement these rules in gerrit server: <br />
First delete all default access permission granted for "Registered Users" from https://gerritreview.com/gerrit/#/admin/projects/All-Projects,access <br />
Then create below access rules for the projects under container: {% img http://yongbingchen.github.com/images/git/repo/gerrit_server_access_control.jpeg  %}
