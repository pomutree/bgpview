# BGP-4 Protocol Viewer
BGP-4 Protocol Viewer(hereinafter referred to as "BGPView") make connection to the router by BGP-4 protocol. the purpose is monitoring of detail that this received BGP packets and collecting of statics based on received data.

## Table of content
1. [Plathome](#Plathome)
2. [Install](#Install)
  1. [Compile](#Compile)
  2. [Configuration](#Configuration)
    1. [Basic Configuration](#Basic Configuration)
    2. [Configuration of each peer](#Configuration of each peer)
3. [Execute](#Execute)
4. [How to use](#How to use)
  1. [Re-start of log file](#Re-start of log file)
  2. [Re-start of Prefix Log file](#Re-start of Prefix Log file)
  3. [Announcement of dummy routes](#Announcement of dummy routes)
  4. [About Dump file which is used PLAYBACK function](#About Dump file which is used PLAYBACK function)
5.    [Distribution Policy](#Distribution Policy)
6.    [Author](#Author)

## Plathome
BGPView has been tested on FreeBSD-4.10 up to now. And, I checked to run on following plathomes.
- FreeBSD 4.10-RELEASE

## Install

### Compile
1. Extract this package. And change directory to extracted directory. And compile this as using following command.

  `make`

  Note, if you use SunOS, then you should install 'bind' and modify
  a part of the Makefile as 'LIBS=' option.

  - If you want to surpport 'show process' commnad of BGPView Shell, then
    BGPView needs 'top' command. Default configuration is that this use
    '/usr/bin/top'. If you do not use this command, then please modify
    that delete '-DHAVE_TOP' from CFLAGS at Makefile.

2. Finished make, becoming root by 'su' command. And run following command for install BGPView.

  `make install`

  After installing, BGPView executable file is in '/usr/local/bin/' directory, and configuration sample file, 'bgpview.cfg.sample', is in '/usr/local/etc/' directory.


### Configuration
BGPView will read '/usr/local/etc/bgpview.cfg' in default. If you put confguration file at another directory, then you should execute bgpview with '-f' execute option.

Basicly, BGPView configuration file is written ONE parameter using ONE line. And, If top of line is written '#', then that line is identified comment.

So, each line is described as following.

`<COMMAND>	<Parameter>`

#### Basic Configuration (System Congifuration)
| Parameter | Description |
| :--- | :--- |
| LOCAL_AS        | Local AS Number.|
| IDENTIFIER      | Local BGP Identifier Basicly, describe IP Address to be assigned local interface.|
| SYSTEM_LOG      | Describe log file name by full path name which is logged commonly BGPView. And, if you want to output this log to syslog, then describe facility name with head of '@' character. (Exampel: @LOCAL7) Note: Facility name is allowd only 'local0' - 'local7'. |
| DUMMY_ROUTE_CFG | Describe configuration file name of announcement dummy routes by full path name. This file is configured '/usr/local/etc/dummyroute.cfg' in default. |
| SHELL_PORT      | Describe TCP port number of BGPView Shell. Default is 3000. |
| SHELL_MAXCON    | Maximum number of connection of BGPView Shell.
| SHELL_PASSWD    | Password of BGPView Shell. This password have to encripted by 'bvspasswd'. |
| SENDMAIL_CMD    | Describe sendmail command path which is used that BGPView send some administrative mail. |
| ADMIN_ADDR      | From address of e-mail which is sent by BGPView. |
| NOTIFY_ADDR     | Destination e-mail address which is sent by BGPView. |
| SPOOL_DIR       | Default spool directory for outputting file which is described by BVS command. |
| RTHISTORYCLR    | Route change history clearing timer interval. If it was set 0, then this function does not work. |
| RTHISTORYCLR    | IPv6 Route change history clearing timer interval. If it was set 0, then this function does not work. |
| CRON            | Set the filename of BGPView Cron configuration file. |
| SYSNAME         | Set the BGPView system name. It is used for BGPView Shell prompt. |
| IRR_CHECK       | if 'ON', enabling IRR check functions. |
| IRR_HOST        | Set IRR Host |
| IRR_PORT        | Set IRR Port |

#### Configuration of each peer
Configuration of each peer is begun by 'NEIGHBOR' line.

| Parameter | Description |
| :--- | :--- |
| NEIGHBOR        | No parameter. This line separate another peer configuration. |
| DESCRIPTION     | Describe Peering Name. This parameter field could not identify white space. |
| ADDRESS         | neighbor address. |
| PEER_ADMIN      | Describe peering administrator e-mail address. |
| REMOTE_AS       | Remote AS number. |
| IND_IDENT       | Local side BGP Identifier. Default is the value of 'IDENTIFIER' in Basic Configuration. |
| IND_AS          | Local side AS number. Default is the value of 'LOCAL_AS' in Basic Configuration. |
| HOLD_TIMER      | Hold time. |
| NLRI            | Describe NLRI. Supported NLRI type is below. <ul><li> IPv6_UNICAST	: IPv6 Routes </li><li> IPv4_UNICAST	: IPv4 Routes </li></ul> |
| VIEW            | describe a kind of log message. Following parameter are available. <ul><li> OPEN </li><li> UPDATE </li><li> UPDADDR </li><li> NOTIFICATION </li><li> KEEPALIVE </li><li> SYSTEM </li><li> UPDATEDUMP </li></ul> |
| ANNOUNCE        | Notify some movement of BGPView. Following parameter is available. <ul><li> ROUTE_DIFF  : (Not Surpported) </li><li> STATUS_CHANGE : Notify by e-mail to change BGP Status at IDLE and ESTABLISHED.</li></ul> |
| LOGNAME         | Describe log file name by full path name which is logged each peer. |
| INTERVAL_INFO   | Interval of output of BGP statics. |
| INFO            | Describe statics log file name by full path. |
| PREFIXOUT       | Describe prefix log file name by full path which is logged received prefixes on this peer by update and withdraw. |
| PREFIXOUTSUM    | When the parameter is 'ON', BGPView add a number of prefixes, a number of routes and a number of radix nodes to head of prefix log file. When the parameter is 'OFF', those information are not added. |
| 4OCTETS_AS      | If ON, BGPView supports 4Byte-AS Capability. This option require NLRI option. |

## Execute
There is some executing options.

```
bgpview [-v] [-f filename] [-d]

-v          : Describe BGPView Version Number。
-f filename : Describe BGPView configuration file name.
              Default is '/usr/local/etc/bgpview.cfg'
-d          : Executing daemon mode
```

## How to use

### Re-start of log file
Log file is recorded detail message of received packets. Therefore, its file size become big soon. In this situation, big size file probably makes some troubles sometime.

When log file size become big, BGPView re-make new log file to send 'KILLHIP' signal. In this time, current log file is re-named '.bak'.

Process ID of BGPView is stored at `/var/run/bgpview.pid`.[^1] So, you run following commnad.

``kill -HUP `cat /var/run/bgpview.pid` ``

[^1]: If you use SunOS, Process ID is stored at '/var/adm/bgpview.pid'. This function is same as 'do reflesh systemlog' on BGPView Shell.

### Re-starrt of Prefix Log file
At the same as log file, prefix log file size will become big. Therefore, BGPView re-make new prefix log file to send 'USR1' signal same as log file.

you run following command.

``kill -USR1 `cat /var/run/bgpview.pid` ``

This function is same as 'do reflesh prefixlog all' on BGPView Shell.

### Announcement of dummy routes
BGPView can announce many dummy route to specific peer. Announcing routes is configured by configuration file which is specified 'DYMMY_ROUTE_CFG' in 'bgpview.cfg' configuration file. If not be specified, then defalut configuration file is '/usr/local/etc/dummyroute.cfg'.

Following is how to modify 'dummyroute.cfg' file.Writing format is same as BGPView configuration file, 'bgpview.cfg'.

| Parameter | Description |
| :--- | :--- |
| NEIGHBOR	   | Specifying destination peer to send dummy routes. |
| STARTPREFIX  | Specifying starting prefix. |
| NUMBEROF	   | Specifying how many prefixes to send neighbor. |
| PACK	       | Specifying how many prefixes in a Update message. |
| ASPATH	     | Specifying AS Path which is separated by underscore '_' character. this specification is only AS SEQUENCE. <br> Example: If AS_SEQUENCE is '10 20 30', then specify '10_20_30'.
| TYPE         | Specifying a kind of Update message. Following is kind of it. <ul><li> UPDATE   : Update. </li><li> WITHDRAW : Withdeawn. If withdrawn, AS Path is ignored. </li><li> PLAYBACK : BGPView send many prefix based on dump file included log file which is made each peer. </li></ul> |
| ASTYPE       | Specifying AS Path auto modification type. <ul><li> NORMAL : BGPView send same AS path in all update messages which specified in "dummyroute.cfg". </li><li> INCREMENTAL : BGPView send different AS path in each update messages. Those AS path will be add automatically and incremental from "1_1" to as path which is specified in "dummyroute.cfg". <br> For example, when AS path is specified "100_200" in "dummyroute.cfg", firstly BGPView	will send "100_200_1_1", and in 10th update message, it will send "100_200_10_1".
| DUMPFILE	   | Specifying dump file at PLAYBACK mode. This line is ignored at UPDATE/WITHDRAW mode.

At UPDATE/WITHDRAW mode, BGPView send update packet with following attributes.
```
Metric       : 0
Next_Hop     : Local Router ID
Orign        : IGP
AS Path Attr.: Only AS_SEQUENCE
```

BGPView send that prefix length is /24.

Configuration file of dummy routes, as 'dummyroute.cfg', is read that BGPView is received SIGUSR2 signal. and bgpview send dummy routes as soon. So, you run following command. This function is same as 'do annouce dummyroute' on BGPView Shell.

``kill -USR2 `cat /var/run/bgpview.pid` ``

NOTE) Dummuy route announcement command support 4 byte AS routes.
z
### About Dump file which is used PLAYBACK function

PLAYBACK mode on announcement of dummy routes can use log file of BGPView getting past.

Therefore, If you want to use this mode, then enable attribute 'UPDATEDUMP' in bgpview configuration file. If this attribute is not enabled, then BGPView dose not store dump log which is used this mode.


## Distribution Policy
All files in the package includes folowing description.

Copyright (c) 1998-2000 Internet Initiative Japan Inc.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:
1. Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
2. Redistribution with functional modification must include
  prominent notice stating how and when and by whom it is
  modified.
3. Redistributions in binary form have to be along with the source
  code or documentation which include above copyright notice, this
  list of conditions and the following disclaimer.
4. All commercial advertising materials mentioning features or use
  of this software must display the following acknowledgement:
    This product includes software developed by Internet
    Initiative Japan Inc.

THIS SOFTWARE IS PROVIDED BY \`\`AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED.

As described in the notice, redistribution is allowed for any
purpose including commercial product as long as satisfying
above conditions.  Redistribution with modification is not
prohibited but we encourage everybody to share the improvement
for public.  We are happy to integrate contributions in
original release.  If you still want to distribute your own
modified package, please make it clear that how it is changed
and use different distribution name to avoid the conflict.

## Author
Kuniaki Kondo  
Copyright (C) 1998 - 2002 Internet Initiative Japan Inc.  
Copyright (C) 2003-2005 Intec NetCore, Inc.  
Copyright (C) 2006-2015 Mahoroba Kobo, Inc.  

Mahoroba Kobo, Inc.

E-mail: kuniaki@ate-mahoroba.jp

---------
2005/10/21 02:24:50 kuniaki Exp
