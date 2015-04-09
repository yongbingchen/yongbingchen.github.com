---
layout: post
title: "Verify the signature of a X.509 certificate"
date: 2015-04-09 12:08
comments: true
categories: [security]
---

<h2>Get the certificate</h2>
```sh
$ openssl s_client -showcerts -connect www.google.com:443 </dev/null > www.google.com.crt
```
then extract the top two certificates from www.google.com.crt: <br />
Google's (call it Google.pem):
{% codeblock %}
-----BEGIN CERTIFICATE-----
MIIEdjCCA16gAwIBAgIIOPbIrrvyFtIwDQYJKoZIhvcNAQEFBQAwSTELMAkGA1UE
BhMCVVMxEzARBgNVBAoTCkdvb2dsZSBJbmMxJTAjBgNVBAMTHEdvb2dsZSBJbnRl
cm5ldCBBdXRob3JpdHkgRzIwHhcNMTUwMzI1MTUyNTE0WhcNMTUwNjIzMDAwMDAw
WjBoMQswCQYDVQQGEwJVUzETMBEGA1UECAwKQ2FsaWZvcm5pYTEWMBQGA1UEBwwN
TW91bnRhaW4gVmlldzETMBEGA1UECgwKR29vZ2xlIEluYzEXMBUGA1UEAwwOd3d3
Lmdvb2dsZS5jb20wggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEKAoIBAQCSTWLd
zWsBLTP2zxKl/WKRtJCGerseT381b6i71CRADCg+57sU2o0SaCOYFKSQ99iOul6j
qe2N/ua2/4QUBalk1HsfdttvOaaZD9AkBgJI+oiVgCcdbdpqet6xU4QlJo+qjC2C
SGXaPzD3jUiIfUyQ2G8KipXwqcyVG3Vi/JjskdEywclBgYIEVlK/UQ01fxeSIBEm
3yMM12vKWLNnowCI5mBv57gCZVgr2aI182rGCipsJmd1l5tSUBNVxRx5J6ul3Q/6
+jnfokCz8hgmGIqYGLXpdh044VrKmA7xK6FL7gDKPYx/ZhErVs7c4sqL5n3YrKi1
lNkGyudSIX9nQZ5ZAgMBAAGjggFBMIIBPTAdBgNVHSUEFjAUBggrBgEFBQcDAQYI
KwYBBQUHAwIwGQYDVR0RBBIwEIIOd3d3Lmdvb2dsZS5jb20waAYIKwYBBQUHAQEE
XDBaMCsGCCsGAQUFBzAChh9odHRwOi8vcGtpLmdvb2dsZS5jb20vR0lBRzIuY3J0
MCsGCCsGAQUFBzABhh9odHRwOi8vY2xpZW50czEuZ29vZ2xlLmNvbS9vY3NwMB0G
A1UdDgQWBBRRAvQ0zUd+a1XkMqsEqYzLoPJ6FDAMBgNVHRMBAf8EAjAAMB8GA1Ud
IwQYMBaAFErdBhYbvPZotXb1gba7Yhq6WoEvMBcGA1UdIAQQMA4wDAYKKwYBBAHW
eQIFATAwBgNVHR8EKTAnMCWgI6Ahhh9odHRwOi8vcGtpLmdvb2dsZS5jb20vR0lB
RzIuY3JsMA0GCSqGSIb3DQEBBQUAA4IBAQB+xWJQGlk2iKfjkj694iLpUPYqDW9z
fg/BAcfZa8gpwL7TmBCFxAxxxW84+22gQM8sH9pqeeHpNCTmTLbtx6XREH+UKeHn
u1zFhYFTrc0WDGXOR9RbZGEiXESCEwmZKL1/3/s9+rFfX3AV1BS7ickw3QzNHqzW
d8xvLuSdkCGui0/q+UXX2sGkJET9ZTT9fuy6UGA7zOOCMaAuGtXQaiZMCwo5IRmO
godUOFa70CaggZm9s4Cci0+JzwIBl7T7459NgtZaIvoRj/DVByboBaBFNkfcJmzb
cIDZKTG45MHEWZJNzacNjUg+KPCqf1bTMYKjTxSs1bkYy4cTYrg1NvHv
-----END CERTIFICATE-----
{% endcodeblock %}

and the issuer's (call it Issuer.pem):
{% codeblock %}
-----BEGIN CERTIFICATE-----
MIID8DCCAtigAwIBAgIDAjp2MA0GCSqGSIb3DQEBBQUAMEIxCzAJBgNVBAYTAlVT
MRYwFAYDVQQKEw1HZW9UcnVzdCBJbmMuMRswGQYDVQQDExJHZW9UcnVzdCBHbG9i
YWwgQ0EwHhcNMTMwNDA1MTUxNTU1WhcNMTYxMjMxMjM1OTU5WjBJMQswCQYDVQQG
EwJVUzETMBEGA1UEChMKR29vZ2xlIEluYzElMCMGA1UEAxMcR29vZ2xlIEludGVy
bmV0IEF1dGhvcml0eSBHMjCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEB
AJwqBHdc2FCROgajguDYUEi8iT/xGXAaiEZ+4I/F8YnOIe5a/mENtzJEiaB0C1NP
VaTOgmKV7utZX8bhBYASxF6UP7xbSDj0U/ck5vuR6RXEz/RTDfRK/J9U3n2+oGtv
h8DQUB8oMANA2ghzUWx//zo8pzcGjr1LEQTrfSTe5vn8MXH7lNVg8y5Kr0LSy+rE
ahqyzFPdFUuLH8gZYR/Nnag+YyuENWllhMgZxUYi+FOVvuOAShDGKuy6lyARxzmZ
EASg8GF6lSWMTlJ14rbtCMoU/M4iarNOz0YDl5cDfsCx3nuvRTPPuj5xt970JSXC
DTWJnZ37DhF5iR43xa+OcmkCAwEAAaOB5zCB5DAfBgNVHSMEGDAWgBTAephojYn7
qwVkDBF9qn1luMrMTjAdBgNVHQ4EFgQUSt0GFhu89mi1dvWBtrtiGrpagS8wEgYD
VR0TAQH/BAgwBgEB/wIBADAOBgNVHQ8BAf8EBAMCAQYwNQYDVR0fBC4wLDAqoCig
JoYkaHR0cDovL2cuc3ltY2IuY29tL2NybHMvZ3RnbG9iYWwuY3JsMC4GCCsGAQUF
BwEBBCIwIDAeBggrBgEFBQcwAYYSaHR0cDovL2cuc3ltY2QuY29tMBcGA1UdIAQQ
MA4wDAYKKwYBBAHWeQIFATANBgkqhkiG9w0BAQUFAAOCAQEAJ4zP6cc7vsBv6JaE
+5xcXZDkd9uLMmCbZdiFJrW6nx7eZE4fxsggWwmfq6ngCTRFomUlNz1/Wm8gzPn6
8R2PEAwCOsTJAXaWvpv5Fdg50cUDR3a4iowx1mDV5I/b+jzG1Zgo+ByPF5E0y8tS
etH7OiDk4Yax2BgPvtaHZI3FCiVCUe+yOLjgHdDh/Ob0r0a678C/xbQF9ZR1DP6i
vgK66oZb+TWzZvXFjYWhGiN3GhkXVBNgnwvhtJwoKvmuAjRtJZOcgqgXe/GFsNMP
WOH7sf6coaPo/ck/9Ndx3L2MpBngISMjVROPpBYCCX65r+7bU2S9cS+5Oc4wt7S8
VOBHBw==
-----END CERTIFICATE-----
{% endcodeblock %}

then extract the signature from Google.pem, and the RSA public key (used to decode the signature) from Issuer.pem:
{% codeblock %}
$ openssl x509 -in Google.pem -noout -text
    Signature Algorithm: sha1WithRSAEncryption
         7e:c5:62:50:1a:59:36:88:a7:e3:92:3e:bd:e2:22:e9:50:f6:
         2a:0d:6f:73:7e:0f:c1:01:c7:d9:6b:c8:29:c0:be:d3:98:10:
         85:c4:0c:71:c5:6f:38:fb:6d:a0:40:cf:2c:1f:da:6a:79:e1:
         e9:34:24:e6:4c:b6:ed:c7:a5:d1:10:7f:94:29:e1:e7:bb:5c:
         c5:85:81:53:ad:cd:16:0c:65:ce:47:d4:5b:64:61:22:5c:44:
         82:13:09:99:28:bd:7f:df:fb:3d:fa:b1:5f:5f:70:15:d4:14:
         bb:89:c9:30:dd:0c:cd:1e:ac:d6:77:cc:6f:2e:e4:9d:90:21:
         ae:8b:4f:ea:f9:45:d7:da:c1:a4:24:44:fd:65:34:fd:7e:ec:
         ba:50:60:3b:cc:e3:82:31:a0:2e:1a:d5:d0:6a:26:4c:0b:0a:
         39:21:19:8e:82:87:54:38:56:bb:d0:26:a0:81:99:bd:b3:80:
         9c:8b:4f:89:cf:02:01:97:b4:fb:e3:9f:4d:82:d6:5a:22:fa:
         11:8f:f0:d5:07:26:e8:05:a0:45:36:47:dc:26:6c:db:70:80:
         d9:29:31:b8:e4:c1:c4:59:92:4d:cd:a7:0d:8d:48:3e:28:f0:
         aa:7f:56:d3:31:82:a3:4f:14:ac:d5:b9:18:cb:87:13:62:b8:
         35:36:f1:ef
{% endcodeblock %}

{% codeblock %}
$ openssl x509 -in Issuer.pem -noout -text
        Subject Public Key Info:
            Public Key Algorithm: rsaEncryption
                Public-Key: (2048 bit)
                Modulus:
                    00:9c:2a:04:77:5c:d8:50:91:3a:06:a3:82:e0:d8:
                    50:48:bc:89:3f:f1:19:70:1a:88:46:7e:e0:8f:c5:
                    f1:89:ce:21:ee:5a:fe:61:0d:b7:32:44:89:a0:74:
                    0b:53:4f:55:a4:ce:82:62:95:ee:eb:59:5f:c6:e1:
                    05:80:12:c4:5e:94:3f:bc:5b:48:38:f4:53:f7:24:
                    e6:fb:91:e9:15:c4:cf:f4:53:0d:f4:4a:fc:9f:54:
                    de:7d:be:a0:6b:6f:87:c0:d0:50:1f:28:30:03:40:
                    da:08:73:51:6c:7f:ff:3a:3c:a7:37:06:8e:bd:4b:
                    11:04:eb:7d:24:de:e6:f9:fc:31:71:fb:94:d5:60:
                    f3:2e:4a:af:42:d2:cb:ea:c4:6a:1a:b2:cc:53:dd:
                    15:4b:8b:1f:c8:19:61:1f:cd:9d:a8:3e:63:2b:84:
                    35:69:65:84:c8:19:c5:46:22:f8:53:95:be:e3:80:
                    4a:10:c6:2a:ec:ba:97:20:11:c7:39:99:10:04:a0:
                    f0:61:7a:95:25:8c:4e:52:75:e2:b6:ed:08:ca:14:
                    fc:ce:22:6a:b3:4e:cf:46:03:97:97:03:7e:c0:b1:
                    de:7b:af:45:33:cf:ba:3e:71:b7:de:f4:25:25:c2:
                    0d:35:89:9d:9d:fb:0e:11:79:89:1e:37:c5:af:8e:
                    72:69
                Exponent: 65537 (0x10001)
{% endcodeblock %}

<h2>Decode certificate's signature with issuer's RSA public key</h2>
{% codeblock %}
$ python
>>> signature = 0x7ec562501a593688a7e3923ebde222e950f62a0d6f737e0fc101c7d96bc829c0bed3981085c40c71c56f38fb6da040cf2c1fda6a79e1e93424e64cb6edc7a5d1107f9429e1e7bb5cc5858153adcd160c65ce47d45b6461225c448213099928bd7fdffb3dfab15f5f7015d414bb89c930dd0ccd1eacd677cc6f2ee49d9021ae8b4feaf945d7dac1a42444fd6534fd7eecba50603bcce38231a02e1ad5d06a264c0b0a3921198e8287543856bbd026a08199bdb3809c8b4f89cf020197b4fbe39f4d82d65a22fa118ff0d50726e805a0453647dc266cdb7080d92931b8e4c1c459924dcda70d8d483e28f0aa7f56d33182a34f14acd5b918cb871362b83536f1ef
>>> modulus = 0x009c2a04775cd850913a06a382e0d85048bc893ff119701a88467ee08fc5f189ce21ee5afe610db7324489a0740b534f55a4ce826295eeeb595fc6e1058012c45e943fbc5b4838f453f724e6fb91e915c4cff4530df44afc9f54de7dbea06b6f87c0d0501f28300340da0873516c7fff3a3ca737068ebd4b1104eb7d24dee6f9fc3171fb94d560f32e4aaf42d2cbeac46a1ab2cc53dd154b8b1fc819611fcd9da83e632b8435696584c819c54622f85395bee3804a10c62aecba972011c739991004a0f0617a95258c4e5275e2b6ed08ca14fcce226ab34ecf46039797037ec0b1de7baf4533cfba3e71b7def42525c20d35899d9dfb0e1179891e37c5af8e7269
>>> print "%x" % pow( signature, 65537, modulus )
1ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff003021300906052b0e03021a05000414f97eff4a56c70bf2a2cdf0546b1efedcb2d8c870
{% endcodeblock %}
The leading "1fff..." of the result is the standard RSA algorithm padding, following "3021300906052b0e03021a05000414" is the "ASN.1 BER SHA1 algorithm designator prefix", so the actual decoded SHA1 hash value is the remaining "f97eff4a56c70bf2a2cdf0546b1efedcb2d8c870".

<h2>Compare the decoded signature value with certificate's hash</h2>
As defined in RFC 3280, X.509 certificate's ANS.1 format is 
{% codeblock %}
Certificate  ::=  SEQUENCE  {
     tbsCertificate       TBSCertificate,
     signatureAlgorithm   AlgorithmIdentifier,
     signatureValue       BIT STRING  } 
{% endcodeblock %}
The decoded SHA1 hash value is tbsCertificate's hash value, not the whols certificate's hash value (the output of "openssl x509 -noout -in Google.pem -fingerprint -sha1").

To extract tbsCertificate from the certificate, we need to convert it from PEM format to DER format (the binary format) first:
{% codeblock %}
$ openssl x509 -in Google.pem -inform PEM -out Google.crt -outform DER
{% endcodeblock %}
Then use "openssl asn1parse" to find the offset and size of tbsCertificate in the certificate:
{% codeblock %}
$ openssl asn1parse -inform der -in Google.crt
    0:d=0  hl=4 l=1142 cons: SEQUENCE
    4:d=1  hl=4 l= 862 cons: SEQUENCE
    8:d=2  hl=2 l=   3 cons: cont [ 0 ]
   10:d=3  hl=2 l=   1 prim: INTEGER           :02
   13:d=2  hl=2 l=   8 prim: INTEGER           :38F6C8AEBBF216D2
   23:d=2  hl=2 l=  13 cons: SEQUENCE
   25:d=3  hl=2 l=   9 prim: OBJECT            :sha1WithRSAEncryption
...
{% endcodeblock %}
Based on certificate's ANS.1 definition, the second line represents tbsCertificate's offset (in the binary format DER) 4, size 866( head length 4, body length 862).
So it can be extracted by:
```sh
$ dd if=Google.crt of=Google.tbsCertificate skip=4 bs=1 count=866
```

And its SHA1 hash matches the decoded value:
```sh
$ sha1sum Google.tbsCertificate
f97eff4a56c70bf2a2cdf0546b1efedcb2d8c870  Google.tbsCertificate
```
