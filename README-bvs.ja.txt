                      BGPView Shell $B%3%^%s%I%j%U%!%l%s%9(B
                          BGPView Beta Version 0.43

                                 Kuniaki Kondo
          Copyright (C) 2000 - 2002 Internet Initiative Japan Inc.
                 Copyright (C) 2003-2005 Intec NetCore, Inc.
                 Copyright (C) 2006-2015 Mahoroba Kobo.

                                 26 Jan 2007

1. BGPView Shell$B$X$N@\B3(B

    BGPView Shell($B0J9_!"(BBVS)$B$X$O(Btelnet$B%3%^%s%I$rMxMQ$7$F@\B3$G$-$^$9!#(B
  $B@\B3$K4X$9$k>pJs$O!"@_Dj%U%!%$%k(B(bgpview.cfg)$B$N(BSHELL_PORT,
  SHELL_MAXCON $B$*$h$S(B SHELL_PASSWD $B$K@_Dj$7$^$9!#(B

    $B@\B3$O!"@\B3%]!<%H$,(B3000(default$BCM(B)$B$N>l9g$O0J2<$N$h$&$K$7$F@\B3$G(B
  $B$-$^$9!#(B

    % telnet <hostname> 3000

    $B@\B3$5$l$l$P!"%Q%9%o!<%I$NF~NO$,5a$a$i$l$^$9$N$G!"(BSHELL_PASSWD$B$K(B
  $B@_Dj$5$l$F$$$k%Q%9%o!<%I$rF~NO$7%m%0%$%s$7$F2<$5$$!#%m%0%$%s8e$O!"(B
  $B0J2<$N$h$&$K(B"BGPView# "$B$H$$$&%W%m%s%W%H$,I=<($5$l!"%3%^%s%IF~NOBT$A(B
  $B$K$J$j$^$9!#(B

    % telnet <hostname> 3000
    Trying xxx.xxx.xxx.xxx...
    Connected to <hostname>.
    Escape character is '^]'.
    Password:*******
    BGPView# 

2. BVS$B$NMxMQ(B

     BVS$B$G$O!"%3%^%s%IF~NO;~$N%i%$%s%(%G%#%C%H$H$7$F4v$D$+$N5!G=$r%]!<(B
   $B%H$7$F$$$^$9!#(B

   1)$B%*!<%H%3%s%W%j!<%H$N%5%]!<%H(B
       $B%3%^%s%IF~NO;~$K!"%3%^%s%IJ8;zNs$rA4$FF~NO$7$J$/$F$b!"F~NO$7$?(B
     $BJ8;zNs$G%3%^%s%I$,3NDj$G$-$l$P!"<+F0E*$K$=$N%3%^%s%I$rJd40$7$^$9!#(B
       $BNc$($P!"(Bexit$B%3%^%s%I$N>l9g!"Bh0lC18l$N@hF,J8;zNs$,(B'e'$B$N%3%^%s%I(B
     $B$O!"(Bexit$B%3%^%s%I$7$+$J$$$?$a!"(Be<CR>$B$G(Bexit$B%3%^%s%I$HG'<1$7<B9T$7$^(B
     $B$9!#(B
       $B$^$?!"(B'e'$B$@$1$rF~NO$7$F%?%V%-!<$rF~NO$9$k$3$H$G!"(B'exit'$B$^$GJd40(B
     $B$7!"Jd40$7$?J8;zNs$rI=<($7$^$9!#(B

   2)$B%R%9%H%j$N%5%]!<%H(B
       $B2a5n$KF~NO$7$?%3%^%s%I$r:FEYF~NO$9$k:]$K(B<Ctrl-P>$B$rF~NO$9$k$3$H(B
     $B$G0l$DA0$KF~NO$7$?%3%^%s%I$r8F$S=P$9$3$H$,$G$-$^$9!#J];}$7$F$$$k(B
     $B%3%^%s%I$O!":GBg$G(B7$B$D$^$G$G$9!#$^$?!"(B<Ctrl-P>$B$rF~NO$7$9$.$?>l9g!"(B
     <Ctrl-N>$B$G0l$DLa$k$3$H$,2DG=$G$9!#(B

   3)$B9TJT=85!G=$N%5%]!<%H(B
       BGPView$B$G$O!"%3%^%s%I$NJT=8$N$?$a$K$$$/$D$+$N5!G=$rMxMQ$G$-$^$9!#(B
       $B0J2<$K$=$N5!G=$rNs5s$7$^$9!#(B

       Ctrl+U  : $B9T:o=|(B
       Ctrl+W  : $BC18l:o=|!"%+!<%=%k0LCV$h$jA0J}$N#1C18l$r:o=|$7$^$9!#(B
       Ctrl+D  : $B0LCVJ8;z:o=|!"%+!<%=%k0LCV$N#1J8;z$r:o=|$7$^$9!#(B
       Ctrl+A  : $B%+!<%=%k$r9T$N@hF,$K0\F0$5$;$^$9!#(B
       Ctrl+F  : $B%+!<%=%k$r#1J8;z:8$K0\F0$7$^$9!#(B
       Ctrl+B  : $B%+!<%=%k$r#1J8;z1&$K0\F0$7$^$9!#(B
       Ctrl+K  : $B%+!<%=%k0LCV0J9_$NJ8;z$r:o=|$7$^$9!#(B

       $B$3$NB>!"(BCtrl+L$B$K$h$C$F2hLL$N%/%j%"$,9T$($^$9!#(B


   4)$B%X%k%W$N%5%]!<%H(B
       $B%3%^%s%IF~NO;~!"%5%]!<%H$5$l$F$$$k%3%^%s%I$,$o$+$i$J$/$J$C$?>l9g!"(B
     $B$=$N;~E@$G(B'?'$B$rF~NO$9$k$3$H$G!"$=$N;~E@$G%5%]!<%H$5$l$F$$$k%3%^%s(B
     $B%I$N%j%9%H$,I=<($5$l$^$9!#(B

3. $B%3%^%s%I%j%U%!%l%s%9(B

3.1 exit/quit$B%3%^%s%I(B

      exit<CR>
      quit<CR>
	BVS$B$r%m%0%"%&%H$7$^$9!#(B

3.2 BGPView$B%W%m%;%9@)8f4XO"%3%^%s%I(B

      shutdown<CR>
	BGPView$B$r=*N;$5$;$^$9!#(B

      reboot<BR>
	BGPView$B$r:F5/F0$7$^$9!#(B

3.3 show$B%3%^%s%I(B

      show cron list<CR>
	$B8=:_@_Dj$5$l$F$$$k(BCron$B$N>uBV$rI=<($7$^$9!#(B

      show ip bgp [detail]<CR>
      show ip bgp [active|inactive] [detail]<CR>
	$BA4$F$N(BPeer$B$+$i<u?.$7$?A4$F$N7PO)$rI=<($7$^$9!#(B
        $B%Q%i%a!<%?$H$7$F(B'active'$B$r;XDj$7$?>l9g$O!"$=$N;~$KM-8z$H$J$C$F(B
        $B$$$k7PO)$N$_$rI=<($7!"(B'inactive'$B$r;XDj$7$?>l9g$O!"$=$N;~$KL58z(B
        $B$H$J$C$F$$$k7PO)$N$_$rI=<($7$^$9!#(B
        detail$B%Q%i%a!<%?$r;XDj$7$?>l9g$K$O!"7PO)$N99?7MzNr$b;2>H$G$-$k!#(B

      show ip bgp <address><CR>
        <address>$B$G;XDj$5$l$k7PO)$r8!:w$7$FI=<($7$^$9!#(B

      show ip bgp summary<CR>
	$B@_Dj$5$l$F$$$k$9$Y$F$N(BBGP Peer$B$N>uBV$r%j%9%H$GI=<($7$^$9!#(B

      show ip bgp neighbor <neighbor address><CR>
	$B;XDj$N(BNeighbor Address$B$N(BPeer$B$N>\:Y!"$*$h$S@_Dj$rI=<($7$^$9!#(B

      show ip bgp neighbor <neighbor address> routes<CR>
      show ip bgp neighbor <neighbor address> [active|inactive] routes<CR>
	$B;XDj$N(BNeighbor Address$B$N(BPeer$B$+$i<u?.$7$?7PO)$rA4$FI=<($7$^$9!#(B
        $B%Q%i%a!<%?$H$7$F(B'active'$B$r;XDj$7$?>l9g$O!"$=$N;~$KM-8z$H$J$C$F(B
        $B$$$k7PO)$N$_$rI=<($7!"(B'inactive'$B$r;XDj$7$?>l9g$O!"$=$N;~$KL58z(B
        $B$H$J$C$F$$$k7PO)$N$_$rI=<($7$^$9!#(B

      show ip bgp statics <neighbor address><CR>
	$B;XDj$N(BNeighbor Address$B$N<u?.%a%C%;!<%8?t$J$I$NDL?.>uBV$rI=<($7(B
        $B$^$9!#(B

      show ip bgp route time <Time>
      show ip bgp route time <Time> le
      show ip bgp route time <Time> ge
	Time$B$G;XDj$5$l$?;~9o!"$b$7$/$O0JA0(B/$B0J8e$N7PO)$N$_$rI=<($7$^$9!#(B
	le$BIU$-$O;XDj;~9o0J9_$K<u?.$7$?$b$N$r!"(Bge$BIU$-$O;XDj;~9o0JA0$K<u(B
        $B?.$7$?$b$r!"$=$7$F!";XDjL5$7$N>l9g$O;XDj;~9o$N$b$N$r$=$l$>$lI=(B
        $B<($7$^$9!#$^$?!"(BTime$B%U%#!<%k%I$K$O0J2<$N%U%)!<%^%C%H$,MxMQ$G$-(B
        $B$^$9!#(B
          - $BD>@\;~9o;XDj(B		hh:mm:ss
          - $BF|;~;XDj(B			w,d,h$B$,;XDj2DG=(B
		$BNc!'(B 1$B=54V$H(B2$BF|$r;XDj$9$k>l9g(B	1w2d
		     1$BF|$H(B2$B;~4V$r;XDj$9$k>l9g(B	1d2h

      show ip nexthop summary<CR>
      show ip nexthop summary neighbor <neighbor address><CR>
        $B<u?.$7$?7PO)$r(BNextHop$BKh$K2?7PO)$"$k$+$r=87W$7$FI=<($7$^$9!#(B
        <neighbor address>$B$r;XDj$7$?>l9g$O!";XDj$N(BNeighbor$B$NJ*$@$1$r=8(B
        $B7W$7$FI=<($7$^$9!#(B

      show ip route summary<CR>
        $B<u?.$7$?7PO)$NAm7PO)?t!"Am(BPrefix$B?t!"7PO)I=$N%N!<%I?t!"(BPrefix$BD9(B
        $BJL$N(BPrefix$B?t!"$*$h$SJ?6Q$N(BAS Path$BD9$rI=<($7$^$9!#(B

      show users<CR>
	$B8=:_!"(BBVS$B$K@\B3$5$l$F$$$k%f!<%6$N%j%9%H$rI=<($7$^$9!#(B

      show proccess<CR>
	BGPView$B$,5/F0$5$l$F$$$k%[%9%H$N%W%m%;%9!"%a%b%j$N>uBV$rI=<($7(B
        $B$^$9!#(B

      show version<CR>
	BGPView$B$N%P!<%8%g%s$H%7%9%F%`$N@_DjCM$rI=<($7$^$9!#(B

      show configuration<CR>
	BGPView$B$N5/F0@_Dj%U%!%$%k(B(default$B$O(Bbgpview.cfg)$B$NFbMF$rI=<($7(B
	$B$^$9!#(B

      show announce dummyroute<CR>
	$B2>A[7PO)9-Js$,<B9T$5$l$F$$$k>uBV$N$H$-!"$=$N2>A[7PO)$NAw?.>uBV(B
	$B$rI=<($7$^$9!#(B

      show v6 bgp [detail]<CR>
      show v6 bgp [active|inactive] [detail]<CR>
	$BA4$F$N(BPeer$B$+$i<u?.$7$?A4$F$N(BIPv6$B7PO)$rI=<($7$^$9!#(B
        $B%Q%i%a!<%?$H$7$F(B'active'$B$r;XDj$7$?>l9g$O!"$=$N;~$KM-8z$H$J$C$F(B
        $B$$$k7PO)$N$_$rI=<($7!"(B'inactive'$B$r;XDj$7$?>l9g$O!"$=$N;~$KL58z(B
        $B$H$J$C$F$$$k(BIPv6$B7PO)$N$_$rI=<($7$^$9!#(B
        detail$B%Q%i%a!<%?$r;XDj$7$?>l9g$K$O!"(BIPv6$B7PO)$N99?7MzNr$b;2>H$G$-$k!#(B

      show v6 bgp neighbor <neighbor address> routes<CR>
      show v6 bgp neighbor <neighbor address> [active|inactive] routes<CR>
	$B;XDj$N(BNeighbor Address$B$N(BPeer$B$+$i<u?.$7$?(BIPv6$B7PO)$rA4$FI=<($7$^$9!#(B
        $B%Q%i%a!<%?$H$7$F(B'active'$B$r;XDj$7$?>l9g$O!"$=$N;~$KM-8z$H$J$C$F(B
        $B$$$k7PO)$N$_$rI=<($7!"(B'inactive'$B$r;XDj$7$?>l9g$O!"$=$N;~$KL58z(B
        $B$H$J$C$F$$$k7PO)$N$_$rI=<($7$^$9!#(B

      show irr status
      show irr status active
        $B<u?.7PO)$r(BIRR$B$G3NG'$7!"$=$N3NG'>uBV$rI=<($7$^$9!#(B
        'active'$B$r;XDj$7$?>l9g$O!"(Bwithdrawn$B$N7PO)$OI=<($7$^$;$s!#(B

      show irr status summary
      show irr status summary neighbor <neighbor address>
        $B<u?.7PO)$r(BIRR$B$G3NG'$7!"$=$N%5%^%j$rI=<($7$^$9!#(B
        'neighbor address'$B$r;XDj$9$k$H!";XDj$7$?FCDj$N(Bpeer$B$N%5%^%j$r(B
        $BI=<($7$^$9!#(B

3.4 clear$B%3%^%s%I(B

      clear ip bgp <neighbor address><CR>
	$B;XDj$N(BNeighbor Address$B$N(BBGP Peer$B$r@ZCG$7$^$9!#@ZCG8e$O!"$7$P$i(B
	$B$/$7$F!":F@\B3$r;n$_$^$9!#(B

      clear ip bgp statics <all | neighbor address><CR>
	$B;XDj$N(BNeighbor Address$B$NDL?.>uBV$N>pJs$r%/%j%"$7$^$9!#(Ball$B$,;X(B
	$BDj$5$l$?>l9g$K$O!"A4$F$N>pJs$r%/%j%"$7$^$9!#(B

      clear route history<CR>
        $B7PO)>pJs99?7MzNr$r%/%j%"$7$^$9!#(B

      clear v6 route history<CR>
        IPv6$B7PO)>pJs99?7MzNr$r%/%j%"$7$^$9!#(B

3.5 do$B%3%^%s%I(B

      do announce dummyroute<CR>
	$B2>A[7PO)9-Js$r5/F0@_Dj%U%!%$%k$G;XDj$5$l$?2>A[7PO)9-Js$N@_Dj(B
        $B$K$7$?$,$C$F9-Js$r3+;O$7$^$9!#(B(SIGUSR2$B$r%W%m%;%9$KAw$C$?>l9g$H(B
	$BF1Ey$NF0:n$r9T$$$^$9!#(B)

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
        <Filename>$B$K;XDj$5$l$k%U%!%$%k$K!"(BBGPView$B$G<u$1$?7PO)$rA4$F=P(B
        $BNO$7$^$9!#(B
        'detail'$B$,;XDj$5$l$F$$$k>l9g$K$O!"I=<(7PO)$N99?7MzNr$bI=<($7$^(B
        $B$9!#$^$?!"%Q%i%a!<%?$H$7$F(B'active'$B$r;XDj$7$?>l9g$O!"$=$N;~$KM-(B
        $B8z$H$J$C$F$$$k7PO)$N$_$rI=<($7!"(B'inactive'$B$r;XDj$7$?>l9g$O!"$=(B
        $B$N;~$KL58z$H$J$C$F$$$k7PO)$N$_$rI=<($7$^$9!#(B
        'neighbor'$B%Q%i%a!<%?$,;XDj$5$l$?>l9g$K$O!";XDj$N(Bneighbor$B$N7PO)(B
        $B$N$_$r=PNO$7$^$9!#(B

      do read cron file<CR>
	bgpview.cfg$B%U%!%$%k$K@_Dj$5$l$F$$$k(BCron$B%U%!%$%k$rFI$_9~$_!"(B
        Cron$B%?%$%^$r:F@_Dj$7$^$9!#(B

      do reflesh sytemlog<CR>
	BGPView$B$N%m%0%U%!%$%k$r%j%U%l%C%7%e$7$^$9!#(B(SIGHUP$B$r%W%m%;%9$K(B
	$BAw$C$?>l9g$HF1Ey$NF0:n$r9T$$$^$9!#(B)
	
      do reflesh prefixlog all<CR>
	BGPView$B$N(BPrefix Log$B$NA4$F$r%j%U%l%C%7%e$7$^$9!#(B(SIGUSR1$B$r%W%m(B
	$B%;%9$KAw$C$?>l9g$HF1Ey$NF0:n$r9T$$$^$9!#(B)

      do test route rtt neighbor <Neighbor Address> 
	          inject <Injection Prefix> timeout <Timeout Sec> <CR>
	$B;XDj$N(B<Neighbor Address>$B$N(BPeer$B$h$j(B<Injection Prefix>$B$G;XDj$5$l(B
	$B$k7PO)$rAw=P$7$^$9!#Aw=P$7$?(BPrefix$B$O4F;k$5$l(BBGPView$B$G@\B3$7$F(B
	$B$$$k$$$E$l$+$N(BPeer$B$GAw=P$7$?(BPrefix$B$HF1Ey$N(BPrefix$B$r<u?.$9$k$^$G(B
	$B$N;~4V4V3V$r(BUpdate$B$*$h$S(BWithdraw$B$=$l$>$l$G7WB,$7$^$9!#(B
	$B7WB,$N%?%$%`%"%&%H$O!"(B<Timeout Sec>$B$KICC10L$G;XDj$7$^$9!#$?$@(B
	$B$7!"%3%^%s%I$N(Btimeout$B0J9_$r>JN,$7$?>l9g$K$O!"%G%U%)%k%H$G(B60$BIC(B
	$B$,@_Dj$5$l$^$9!#(B

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
        <Filename>$B$K;XDj$5$l$k%U%!%$%k$K!"(BBGPView$B$G<u$1$?(BIPv6$B7PO)$rA4(B
        $B$F=PNO$7$^$9!#(B
        'detail'$B$,;XDj$5$l$F$$$k>l9g$K$O!"I=<(7PO)$N99?7MzNr$bI=<($7$^(B
        $B$9!#$^$?!"%Q%i%a!<%?$H$7$F(B'active'$B$r;XDj$7$?>l9g$O!"$=$N;~$KM-(B
        $B8z$H$J$C$F$$$k(BIPv6$B7PO)$N$_$rI=<($7!"(B'inactive'$B$r;XDj$7$?>l9g$O!"(B
        $B$=$N;~$KL58z$H$J$C$F$$$k(BIPv6$B7PO)$N$_$rI=<($7$^$9!#(B

      do output irrstats filename <File Name> all
      do output irrstats filename <File Name> active
        $B<u?.7PO)$r(BIRR$B$G%A%'%C%/$7$?7k2L$r;XDj$N%U%!%$%k$K=PNO$7$^$9!#(B
        'active'$B$r;XDj$7$?>l9g$O!"(Bwithdrawn$B$N7PO)$O=PNO$7$^$;$s!#(B

      do output aslist filename <File Name> neighbor <neighbor address>
        nprobe$B$KF~NO2DG=$J(Bas-list$B%U%!%$%k$r=PNO$7$^$9!#(B


3.6 set$B%3%^%s%I(B

      set route history clear time <Clear Time>
        $B<!$N7PO)>pJs99?7MzNr$N%/%j%"$^$G$N;~4V$rIC$G;XDj$7$^$9!#(B
        $B<!2s$N%/%j%"$,<B;\$5$l$?8e$O!"@_Dj%U%!%$%k$G;XDj$5$l$F$$$k%/%j(B
        $B%"4V3V$KLa$j$^$9!#(B

      set neighbor <Neighbor Address> announce status <ON|OFF>
        BGP$B$N(BFSM$B>uBVJQ99;~$KDLCN$9$k%a!<%k$NDLCN!&HsDLCN$r@_Dj$9$k!#(B

      set neighbor <Neighbor Address> infolog <ON|OFF>
        $B;XDj(BPeer$B$N(BInformation Log$B$N=PNO$N(BON/OFF$B$r@_Dj$7$^$9!#(B

      set neighbor <Neighbor Address> <enable|disable>
        $B;XDj(BPeer$B$NM-8z!?L58z$N@_Dj$r$7$^$9!#(B

      set v6 route history clear time <Clear Time>
        $B<!$N(BIPv6$B7PO)>pJs99?7MzNr$N%/%j%"$^$G$N;~4V$rIC$G;XDj$7$^$9!#(B
        $B<!2s$N%/%j%"$,<B;\$5$l$?8e$O!"@_Dj%U%!%$%k$G;XDj$5$l$F$$$k%/%j(B
        $B%"4V3V$KLa$j$^$9!#(B

								  $B0J>e(B
readme-bvs.txt,v 1.4 2005/10/21 02:24:50 kuniaki Exp
