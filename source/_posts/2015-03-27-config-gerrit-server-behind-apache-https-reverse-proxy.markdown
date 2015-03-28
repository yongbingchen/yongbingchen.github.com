---
layout: post
title: "config gerrit server behind Apache https reverse-proxy"
date: 2015-03-27 16:20
comments: true
categories: [gerrit]
---

I want to setup a secure gerrit server for a small developer group within intranet, I choose Apache as its reverse-proxy server, and use HTTP as gerrit server's auth type, becasue I only want a few selected people to see the server, so no LDAP.

Here's the final web view from a registered developer:<br />
{% img http://yongbingchen.github.com/images/git/repo/sample-cl-webpage.jpg  %}

Here's the gerrit server config:
{% codeblock %}
[gerrit]
	basePath = git
	canonicalWebUrl = https://gerritreview.com/gerrit
[database]
	type = mysql
	hostname = localhost
	database = reviewdb
	username = gerrit2
	password = 12345678
[auth]
	type = HTTP
[sendemail]
	smtpServer = localhost
[container]
	user = gerrit2
	javaHome = /usr/lib/jvm/java-7-openjdk-amd64/jre
[sshd]
	listenAddress = *:29418
[httpd]
	listenUrl = proxy-https://localhost:8080/gerrit
[cache]
	directory = cache
[index]
	type = LUCENE
{% endcodeblock %}

And the changes I made upon default Apache HTTPS site config:
{% codeblock %}
--- /etc/apache2/sites-available/default-ssl.conf	2014-01-07 05:23:42.000000000 -0800
+++ /etc/apache2/sites-available/000-default.conf	2015-03-25 14:41:20.867255345 -0700
@@ -130,6 +130,71 @@
 		# MSIE 7 and newer should be able to use keepalive
 		BrowserMatch "MSIE [17-9]" ssl-unclean-shutdown
 
+               ServerName gerritreview.com
+               ProxyRequests Off
+               ProxyVia Off
+               ProxyPreserveHost On
+               <Proxy *>
+                       Order deny,allow
+                       Allow from all
+               </Proxy>
+ 
+               <Location />
+                       AuthType Basic
+                       AuthName "Gerrit Code Review"
+                       Require valid-user
+                       AuthBasicProvider file
+                       AuthUserFile /etc/apache2/passwords
+               </Location>
+
+               AllowEncodedSlashes On
+               SSLProxyEngine On
+               SSLProxyVerify none
+               SSLProxyCheckPeerCN off
+               SSLProxyCheckPeerName off
+ 
+               ProxyPass /gerrit/ http://localhost:8080/gerrit/ nocanon
+               ProxyPassReverse /gerrit/ http://localhost:8080/gerrit/
+               # is this necessary?	
+               Header edit Location "^http:(.*)$" "https:$1"
+
 	</VirtualHost>
 </IfModule>
{% endcodeblock %}

After setup, this gerrit server was deployed in a kvm guest machine, connected to its kvm host through an isolated virtual bridge.  Allowing bidirectional access to tcp port 29418 (gerrit ssh), 443 (HTTPS), 25 (sendmail), as below command:
```sh
#forward kvm host's incoming (from NIC eth0) tcp dst port 29418 to gerrit server vm.  
iptables -I FORWARD -i eth0 -p tcp -m state --state NEW,RELATED,ESTABLISHED -m tcp -d $VM_GUEST_IP/32 -dport 29418 -j ACCEPT 
#any incoming packets from interface eth0, protocol tcp, dst port 29418 will be applied DNAT function (replace the dst addr from kvm host to $VM_GUEST_IP)
iptables -t nat -i eth0 -I PREROUTING -p tcp  --dport 29418 -j DNAT --to $VM_GUEST_IP:29418 
#replace any outboud tcp/29418 packet from $VM_GUEST_IP with kvm host's addr, and push to host's NIC eth0
iptables -t nat -A POSTROUTING -p tcp -o eth0 -s $VM_GUEST_IP --sport 29418 -j MASQUERADE 
#forward outgoing tcp/29418 connect from $VM_GUEST_IP to host's NIC eth0
iptables -I FORWARD -o eth0 -p tcp -m state --state NEW,RELATED,ESTABLISHED -m tcp -s $VM_GUEST_IP --sport 29418 -j ACCEPT 
```

Also NAT rules to allow connection from the vm guest (gerrit server) to connect to a NTP server:
```sh
iptables -t nat -A POSTROUTING -p udp -s $VM_GUEST_IP/32 -d 174.137.132.100 -dport 123 -j MASQUERADE
iptables -I FORWARD -p udp -s $VM_GUEST_IP/32 -d 174.137.132.100/32 -dport 123 -j ACCEPT
iptables -I FORWARD -p udp -d $VM_GUEST_IP/32 -s 174.137.132.100/32 -dport 123 -j ACCEPT
```
##Notes:
{% blockquote Installation and config note http://yongbingchen.github.com/txt/gerrit/install-and-maintain.txt %} {% endblockquote %}
{% blockquote Full Apache HTTPS site config http://yongbingchen.github.com/txt/gerrit/000-default.conf %} {% endblockquote %}

