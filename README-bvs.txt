                       BGPView Shell Command Reference
                          BGPView Beta Version 0.43

                                Kuniaki Kondo
          Copyright (C) 2000 - 2002 Internet Initiative Japan Inc.
                  Copyright (C) 2003-2005 Intec NetCore, Inc.
                  Copyright (C) 2006-2015 Mahoroba Kobo.

                                 26 Jan 2007

1. Connect to BGPView Shell

    BGPView Shell (hereinafter referred to as "BVS") is connected by
  using as telnet command. Connecting method is configured by 
  SHELL_PORT, SHELL_MAXCON and SHELL_PASSWD in BGPView configuration
  file as bgpview.cfg.

    Connecting to BVS is used TCP port 3000 in default. Following is
  sample connecting command.

    % telnet <hostname> 3000

    Established connection to BVS, it require to input password. So,
  input password which is configured by SHELL_PASSWD. After logged in,
  it will print follwing prompt, and becoming waiting for inputing
  any commands.


2. Using BVS

     BVS support some line edit functions for inputing command.

   1) Auto Complete

       Inputting commands, when you don't enter complete command words,
     if BPGView recognize commands, then BGPView supplement to complete
     command automatically.
       As an example, a case of inputting 'exit' command, the command is
     only one that the first spell is 'e', BGPView will execute 'exit'
     command when BGPView is inputted e<CR>.
       And, If you input just 'e' and input TAB key, then BGPView
     supplment to 'exit'.

   2) History

       BVS remember seven previous commands which is inputed by use.
     These saved commands can call <Ctrl-P> and <Ctrl-N>. When
     <Ctrl-P> is inputed, BVS back to previous command. When <Ctrl-N>
     is inputed, BVS forward to next command.

   3) Command line edition

       BVS support following functions for command line edition.

       Ctrl+U  : Line delete
       Ctrl+W  : Word delete from cursor to left
       Ctrl+D  : One character delete at cursor
       Ctrl+A  : Moving cursor to top of line
       Ctrl+F  : Moving cursor one character to left
       Ctrl+B  : Moving cursor one character to right
       Ctrl+k  : Deletion characters from cursor to right

       Ctrl+L  : Clear screen

   4) Printing help

       When inputting command, if you don't know command which is supported
     BVS, then you can see help if you type '?'. BVS list supported commands.

3. Command Reference

3.1 exit/quit commands

      exit<CR>
      quit<CR>
	Logout BVS.

3.2 BGPView proccess managing commands

      shutdown<CR>
      	Shutdown BGPView process.

      reboot<BR>
        Reboot BGPView process.

3.3 show commands

      show cron list<CR>
        Showing current cron status

      show ip bgp [detail]<CR>
      show ip bgp [active|inactive] [detail]<CR>
        Showing all routes which is received from peer.
        When this command is specified 'active', it display only active
        routes. When this command is specified 'inactive', it display
        only inactive route.
        When this command is specfied 'detail' parameter, it desplay 
        route histories.

      show ip bgp <address><CR>
        Showing routes which is specified by <address>.

      show ip bgp summary<CR>
        Showing all peer list.

      show ip bgp neighbor <neighbor address> routes<CR>
      show ip bgp neighbor <neighbor address> [active|inactive] routes<CR>
        Showing all routes from specified neighbor address
        When this command is specified 'active', it display only active
        routes. When this command is specified 'inactive', it display
        only inactive route.

      show ip bgp statics <neighbor address><CR>
	Showing communication status such as a number of received
	messages from specified neighbor address. 

      show ip bgp neighbor <neighbor address><CR>
        Showing status and configuration for specified neighbor address.

      show ip bgp route time <Time>
      show ip bgp route time <Time> le
      show ip bgp route time <Time> ge
        This command display routes which is specified <Time>.
        When this command is specified 'le', it display routes which
        is received after <Time>, and specified 'ge', it display routes
        which received before <Time>.
        Following is format of the <Time>:
          - Time format		hh:mm:ss
          - Date/Hour format	it can use w(Week),d(Day),h(Hour)
		For example: 1week and 2day	1w2d
			     1day and 2hours	1d2h

      show ip nexthop summary<CR>
      show ip nexthop summary neighbor <neighbor address><CR>
        Showing number of routes for each nexthop.
        When you set <neighbor address>, showing only specified neighbor's
        routes

      show ip route summary<CR>
        Showing total number of prefixes, total number of routes, a 
        number of prefixes each prefix length and AS path length 
        avarage

      show users<CR>
        Showing connected user list

      show proccess<CR>
        Showing process, memory status and some host utilization

      show version<CR>
        Showing BGPView version and system configuration

      show configuration<CR>
        Showing configuration file

      show announce dummyroute<CR>
        Showing transmit status of virtual routes when dummy route
        announcement function has been executing.

      show v6 bgp [detail]<CR>
      show v6 bgp [active|inactive] [detail]<CR>
        Showing all IPv6 routes which is received from peer.
        When this command is specified 'active', it display only active
        IPv6 routes. When this command is specified 'inactive', it
        display only inactive route.
        When this command is specfied 'detail' parameter, it desplay 
        IPv6 route histories.

      show v6 bgp neighbor <neighbor address> routes<CR>
      show v6 bgp neighbor <neighbor address> [active|inactive] routes<CR>
        Showing all IPv6 routes from specified neighbor address
        When this command is specified 'active', it display only active
        routes. When this command is specified 'inactive', it display
        only inactive route.

      show irr status
      show irr status active
        Showing result of IRR Checking for received routes.
        if set 'active', showing only active routes.

      show irr status summary
      show irr status summary neighbor <neighbor address>
        Showing summary of IRR Chekcing for reveibed routs.


3.4 clear commands

      clear ip bgp <neighbor address><CR>
	Clearing specified bgp peer

      clear ip bgp statics <all | neighbor address><CR>
        Clearing communication status of specified neighbor address.
        If specified neighbor address is 'all', then it will clear
        all status.

      clear route history<CR>
        Clearing a information of route change history.

      clear v6 route history<CR>
        Clearing a information of IPv6 route change history.

3.5 do command

      do announce dummyroute<CR>
	Start dummy route announcement depend on dummyroute configuration
        file which is specified in bgpview configuration file.
        This action is same as 'kill -USR2'.

      do output route filename <Filename><CR>
      do output route filename <Filename> neighbor <neighbor address><CR>
      do output route filename <Filename> [active|inactive]<CR>
      do output route filename <Filename> neighbor <neighbor address>
                                                    [active|inactive]<CR>
      do output route filename <Filename> detail<CR>
      do output route filename <Filename> neighbor <neighbor address>
                                                               detail<CR>
      do output route filename <Filename> neighbor <neighbor address>
                                             [active|inactive] detail<CR>
        Received all routes output to the file.
        When this command is specified 'detail', it also output a 
        history of route update.
        When this command is specified 'active', it display only active
        routes. When this command is specified 'inactive', it display
        only inactive routes.
        When this command is specified 'neighbor' parameter, it display
        only specified neighbor's routes.

      do read cron file<CR>
	Re-read cron configuration file which is set in 'bgpview.cfg'.
        And, BGPView cron timer is re-started.

      do reflesh sytemlog<CR>
        BGPView logfiles reflesh. (This action is same as 'kill -HUP')
	
      do reflesh prefixlog all<CR>
        All prefix logfiles refresh. (This action is same as 
        'kill -USR1')

      do test route rtt neighbor <Neighbor Address> 
	          inject <Injection Prefix> timeout <Timeout Sec> <CR>
        A route which is specified <Injection Prefix> is announced to
        a neighbor router from a peer of <Neighbor Address>.
        Announced route is detedced all peer on the BGPView, and
        if BGPView detects the route by update or withdraw, then
        BGPView displaies a rtt time.
        Default timeout is 60 seconds. However, when <Timeout Sec>
        is specified, default timeout is <Timeout Sec> seconds. 

      do output v6 route filename <Filename><CR>
      do output v6 route filename <Filename> neighbor <neighbor address><CR>
      do output v6 route filename <Filename> [active|inactive]<CR>
      do output v6 route filename <Filename> neighbor <neighbor address>
                                                    [active|inactive]<CR>
      do output v6 route filename <Filename> detail<CR>
      do output v6 route filename <Filename> neighbor <neighbor address>
                                                               detail<CR>
      do output v6 route filename <Filename> neighbor <neighbor address>
                                             [active|inactive] detail<CR>
        Received all IPv6 routes output to the file.
        When this command is specified 'detail', it also output a 
        history of IPv6 route update.
        When this command is specified 'active', it display only active
        IPv6 routes. When this command is specified 'inactive', it 
        display only inactive IPv6 routes.

      do output irrstats filename <File Name> all
      do output irrstats filename <File Name> active
        results of IRR checking for received routes output to the file.
        when this command is specified 'active', it output only active
        routes.

      do output aslist filename <File Name> neighbor <neighbor address>
        this commnad output as-list file, which can use 'nprobe' command.
 

3.6 set command

      set route history clear time <Clear Time>
        This command set a next clear time for route change history.
        After this clear, clear interval time is set specified time in
        bgpview configuration file.

      set neighbor <Neighbor Address> announce status <ON|OFF>
        This command set notification switch for peering status.

      set neighbor <Neighbor Address> infolog <ON|OFF>
	This command set outputting status of information log.

      set neighbor <Neighbor Address> <enable|disable>
        This command set enable or disable for peer.

      set v6 route history clear time <Clear Time>
        This command set a next clear time for IPv6 route change history.
        After this clear, clear interval time is set specified time in
        bgpview configuration file.

readme-bvs-e.txt,v 1.4 2005/10/21 02:24:50 kuniaki Exp
