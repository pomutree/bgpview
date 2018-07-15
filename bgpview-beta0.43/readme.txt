                           BGP-4 Protocol Viewer
                             Beta Version 0.43

                              Kuniaki Kondo
         Copyright (C) 1998 - 2002 Internet Initiative Japan Inc.
                Copyright (C) 2003-2005 Intec NetCore, Inc.
		Copyright (C) 2006-2015 Mahoroba Kobo, Inc.

                                20 Apri 2007

-------------------------------------------------------------------------
1.    $B35MW(B
2.    $B%W%i%C%H%U%)!<%`(B
3.    $B%$%s%9%H!<%k(B
3.1   $B%3%s%Q%$%k(B
3.2   $B@_Dj(B
3.2.1 $B4pK\@_Dj(B
3.2.2 Peer$BKh$N@_Dj(B
4.    $B5/F0J}K!(B
5.    $BMxMQJ}K!(B
5.1   $B%m%0%U%!%$%k$N%j%9%?!<%H(B
5.2   Prefix Log$B%U%!%$%k$N%j%9%?!<%H(B
5.3   $B%@%_!<7PO)$N%"%J%&%s%9(B
5.4   PLAYBACK$B5!G=$GMxMQ$9$k%@%s%W%U%!%$%k$K$D$$$F(B
6.    $BCm0U;v9`(B
7.    Distribution Policy
8.    $B:n<TO"Mm@h(B
-------------------------------------------------------------------------

1. $B35MW(B

    BGP-4 Protocol Viewer($B0J9_!"(Bbgpview)$B$O!"(BBGP-4$B%W%m%H%3%k$G;XDj$N%k!<%?(B
  $BEy$H%3%M%/%7%g%s$rD%$j!"$=$3$+$i<u?.$5$l$k%G!<%?$r>\:Y$K%b%K%?$7!"$^$?!"(B
  $B<u?.$7$?%G!<%?$r4p$K$7$?E}7W>pJs$r<hF@$9$k$3$H$rL\E*$K$D$/$i$l$?%=%U%H(B
  $B%&%(%"$G$9!#(B


2. $B%W%i%C%H%U%)!<%`(B

    $B8=:_!"F0:n$,3NG'$5$l$F$$$k%U%i%C%H%U%)!<%`$O0J2<$NDL$j$G$9!#(B

      -FreeBSD 4.10-RELEASE


3. $B%$%s%9%H!<%k(B

3.1 $B%3%s%Q%$%k(B

    $BK\%Q%C%1!<%8$rE,Ev$J%G%#%l%/%H%j$KE83+$7$F2<$5$$!#(B

    $B<!$K!"$=$N%G%#%l%/%H%j$K0\F0$7!"(B

      make

    $B$r<B9T$7$F2<$5$$!#$J$*(B SunOS $B$N>l9g$O!"(B'bind' $B$N%$%s%9%H!<%k$H(B 
  Makefile $B$N0lItJT=8(B("LIBS=" $B$N;XDj(B)$B$,I,MW$G$9!#(B

  $B"((BBGPView Shell$B$N(B'show process'$B%3%^%s%I$r%5%]!<%H$9$k$K$O!"(Btop$B%3%^%s%I(B
    $B$,I,MW$K$J$j$^$9!#(Btop$B%3%^%s%I$O!"(B/usr/bin/ $B%G%#%l%/%H%j$K$"$k$3$H$,A0(B
    $BDs$H$7$F$$$^$9!#$b$7(Btop$B%3%^%s%I$,$J$$>l9g$O!"(BMakefile$B$N(BCFLAGS$B$+$i!"(B
    -DHAVE_TOP$B$r:o=|$7$F$/$@$5$$!#(B

    make$B$,40N;$7$?$i!"(Bsu$B%3%^%s%I$G(Broot$B8"8B$K$J$j!"(B

      make install

    $B$K$h$C$F!"(BBGPView$B$r%$%s%9%H!<%k$7$F$/$@$5$$!#(BBGPView$BK\BN$O!"(B
  /usr/local/bin$B%G%#%l%/%H%j$K3JG<$5$l!"(Bbgpview.cfg.sample$B$O!"(B
  /use/local/etc$B%G%#%l%/%H%j$K%$%s%9%H!<%k$5$l$^$9!#(B


3.2 $B@_Dj(B

    BGPView$B$N@_Dj%U%!%$%k$O(Bdefault$B$G(B/usr/local/etc/bgpview.cfg$B$rFI$_9~$_(B
  $B$^$9!#JQ99$NI,MW$,$"$k>l9g$O!"(BBGPView$B$N5/F0%*%W%7%g%s(B'-f'$B$G;XDj$7$F$/$@(B
  $B$5$$!#(B

    BGPView$B$N@_Dj%U%!%$%k$O!"4pK\E*$K#19TKh$K#1$D$N@_Dj$r9T$$$^$9!#$^$?!"(B
  $B9TF,$,(B"#"$B$G;O$^$k$b$N$O!"%3%a%s%H9T$H$7$FL5;k$7!"L58z$J@_Dj$OL5;k$7$^$9!#(B

    $B3F9T$O!"(B

      <COMMAND>	<Parameter>

  $B$NBP$K$J$C$F$$$^$9!#(B


3.2.1 $B4pK\@_Dj(B

  LOCAL_AS        : $B%m!<%+%k(B(bgpview)$B$N(BAS$BHV9f$r;XDj$7$^$9!#(B
  IDENTIFIER      : $B%k!<%?(BID$B$r;XDj$7$^$9!#(B
                    $B4pK\E*$K!"5/F0$9$k%^%7%s$N%$%s%?!<%U%'!<%9$K;XDj$5$l(B
                    $B$F$$$k(BIP Address$B$GLdBj$"$j$^$;$s!#(B
  SYSTEM_LOG      : BGPView$B$,6&DL$G=PNO$9$k%m%0%U%!%$%k$r(BFull Path$B$G;XDj(B
                    $B$7$^$9!#(B
                    $B$^$?!"%m%0%U%!%$%k$r(Bsyslog$B$K$7$?$$>l9g$O!"(B@$B%^!<%/IU$-(B
                    $B$G%U%!%7%j%F%#L>$r5-=R$9$k$H(Bsyslog$B$K=PNO$5$l$^$9!#(B
                    ($BNc(B: @LOCAL7)
                    $B>0!"(Bsyslog$B$N(Bfacility$B$O!"(Blocal0$B!A(Blocal7$B$K8B$i$l$^$9!#(B
  DUMMY_ROUTE_CFG : BGPView$B$+$i%"%J%&%s%9$9$k%@%_!<$N7PO)$K4X$9$k@_Dj$r5-(B
                    $B=R$9$k%U%!%$%k$r(BFull Path$B$G;XDj$7$^$9!#;XDj$,L5$$>l9g(B
                    $B$O!"%G%U%)%k%H$G!"(B"/usr/local/etc/dummyroute.cfg"$B$H$J(B
                    $B$j$^$9!#(B
                    ($B5-=R$N>\:Y$O8e=R$7$^$9!#(B)
  SHELL_PORT      : BGPView Shell$B$N@\B3%]!<%HHV9f$r;XDj$7$^$9!#(B
		    $B%G%U%)%k%H$O(B3000
  SHELL_MAXCON    : BGPView Shell$B$K@\B3$G$-$k:GBg?t$r;XDj$7$^$9!#(B
  SHELL_PASSWD    : BGPView Shell$B$K@\B3$9$k:]$N%m%0%$%s%Q%9%o!<%I$r;XDj$7(B
                    $B$^$9!#(B
		    $B%Q%9%o!<%I$O!"IUB0$N(Bbvspasswd$B$K$h$C$FJQ49$5$l$?!"0E9f(B
                    $B2=$5$l$?%Q%9%o!<%I$r;XDj$7$F$/$@$5$$!#(B
  SENDMAIL_CMD    : BGPView$B$G%a!<%k$rAwIU$9$k:]$KMxMQ$9$k(Bsendmail$B%3%^%s%I(B
                    $B$r;XDj$7$^$9!#(B
  ADMIN_ADDR      : BGPView$B$+$iAwIU$9$k%a!<%k$N(BFrom$B%"%I%l%9$r;XDj$7$^$9!#(B
  NOTIFY_ADDR     : BGPView$B$+$iAwIU$9$k%a!<%k$NAwIU@h$N%a!<%k%"%I%l%9$r;X(B
                    $BDj$7$^$9!#(B
  SPOOL_DIR       : BVS$B%3%^%s%I$J$I$G;XDj$5$l$k=PNO%U%!%$%k$N%G%U%)%k%H$N(B
                    $B%G%#%l%/%H%j$r;XDj$7$^$9!#(B
  RTHISTORYCLR    : $B7PO)>pJs99?7MzNr$r%/%j%"$9$k4V3V$r;XDj$9$k!#(B
                    0$B$,;XDj$5$l$F$$$k>l9g$K$OF0:n$7$J$$!#(B
  V6RTHISTORYCLR  : IPv6$B7PO)>pJs99?7MzNr$r%/%j%"$9$k4V3V$r;XDj$9$k!#(B
                    0$B$,;XDj$5$l$F$$$k>l9g$K$OF0:n$7$J$$!#(B
  CRON            : BGPView$B$N(BCron$BDj5A%U%!%$%k$N%U%!%$%kL>$r;XDj$7$^$9!#(B
  SYSNAME         : $B5/F0$9$k(BBGPView$B$N%7%9%F%`L>$r;XDj$7$^$9!#(B
                    BGPView Shell$B$N%W%m%s%W%H$J$I$KMxMQ$5$l$^$9!#(B
  IRR_CHECK       : IRR$B%5!<%P$X$N%A%'%C%/$rM-8z$K$7$^$9!#(B
  IRR_HOST        : IRR$B%5!<%P$N%[%9%H$r;XDj$7$^$9!#(B
  IRR_PORT        : IRR$B%5!<%P$NLd$$9g$o$;%]!<%H$r;XDj$7$^$9!#(B

3.2.2 Peer$BKh$N@_Dj(B

  Peer$BKh$N@_Dj$O!"(BNEIGHBOR$B9T$G3+;O$5$l$^$9!#(B

  NEIGHBOR        : $B%Q%i%a!<%?$O$"$j$^$;$s!#(B
                    $B$3$N9T$rH/8+$9$k$H$=$l0J9_$O?7$7$$(BPeer$B$H$7$F$N@_Dj$r(B
                    $B5-=R$7$^$9!#(B
  DESCRIPTION     : Peer$B$NL>>N$rEPO?$7$^$9!#%[%o%$%H%9%Z!<%9$OG'<1$7$^$;(B
                    $B$s$N$G!"(B"_"$B$J$I$G7R$2$F5-=R$7$F$/$@$5$$!#(B
  PEER_ADMIN      : Peer$B$N4IM}<T%a!<%k%"%I%l%9$r;XDj$7$^$9!#(B
  ADDRESS         : neighbor$B%"%I%l%9$r;XDj$7$^$9!#(B
  REMOTE_AS       : $B%j%b!<%HB&$N(BAS$BHV9f$r;XDj$7$^$9!#(B
  IND_IDENT       : Peer$B8DJL$N(BBGP Identifier$B$r@_Dj$7$^$9!#(B
		    $B%G%U%)%k%HCM$O!"%7%9%F%`@_DjCM(BIDENTIFIER$B$H$J$j$^$9!#(B
  IND_AS          : Peer$B8DJL$N%m!<%+%k(BAS$BHV9f$r@_Dj$7$^$9!#(B
                    $B%G%U%)%k%HCM$O!"%7%9%F%`@_DjCM(BLOCAL_AS$B$H$J$j$^$9!#(B
  HOLD_TIMER      : OPEN$B;~$K%j%b!<%HB&$KEA$($k%[!<%k%I;~4V$r;XDj$7$^$9!#(B
  NLRI            : $B8r49$9$k7PO)$N(BNLRI$B$r;XDj$9$k!#%5%]!<%H$7$F$$$k%?%$%W(B
                    $B$O0J2<$NDL$j!#(B
                      IPv6_UNICAST	: IPv6$B7PO)(B
$B!!!!!!!!!!!!!!!!!!!!!!(BIPv4_UNICAST	: IPv4$B7PO)(B
  VIEW            : $B%m%0$K;D$9%a%C%;!<%8$N<oN`$r;XDj$7$^$9!#(B
		    $B%Q%i%a!<%?$O0J2<$NCM$r$H$k$3$H$,$G$-$^$9!#(B
		    OPEN		 UPDATE		 UPDADDR
		    NOTIFICATION	 KEEPALIVE	 SYSTEM
		    UPDATEDUMP
  ANNOUNCE        : $B%Q%i%a!<%?$G;XDj$5$l$k5sF0$rG'$a$?;~$K!"$J$s$i$+$NJ}(B
                    $BK!$G!"$=$NFbMF$rEA$($^$9!#%Q%i%a!<%?$O0J2<$NDL$j$G$9!#(B
		    ROUTE_DIFF    : ($BL$%5%]!<%H(B)
                    STATUS_CHANGE : Peer$B$N>uBV$,(BIDLE$B$^$?$O(BESTABLISHED$B$K(B
                                    $BJQ2=$7$?;~$K@_Dj$5$l$F$$$k%a!<%k%"%I(B
                                    $B%l%9$KDLCN$7$^$9!#(B
  LOGNAME         : Peer$BKh$N%m%0$N=PNO%U%!%$%k$r(BFull Path$B$G;XDj$7$^$9!#(B
  INTERVAL_INFO   : BGP$B%;%C%7%g%s$GF@$?E}7W>pJs$r=PNO$9$k4V3V$r;XDj$7$^$9!#(B
                    ($BICC10L(B)
  INFO            : BGP$B%;%C%7%g%s$GF@$?E}7W>pJs$r=PNO$9$k%U%!%$%k$r(BFull
                    Path$B$G;XDj$7$^$9!#(B
  PREFIXOUT       : update,withdraw$B$G<u?.$7$?(Bprefix$B$N%j%9%H$r;XDj$N%U%!%$(B
                    $B%k$K=PNO$7$^$9(B(Prefix Log$B%U%!%$%k(B)$B!##1$D$N(Bupdate$B%a%C(B
                    $B%;!<%8$G<u?.$7$?J#?t$N(Bprefix$B$r#19T$K=PNO$7$^$9!#(B
  PREFIXOUTSUM    : $B$3$N@_Dj$N%Q%i%a!<%?$,(BON$B$N>l9g!"(BPREFIXOUT$B$G;XDj$9$k(B
                    Prefix Log$B%U%!%$%k$N@hF,$K!"%U%!%$%k$r@8@.$7$?;~E@$G(B
                    BGPView$B$N(BRADIX Tree$B$K3JG<$5$l$F$$$k7PO)?t!"(BPrefix$B?t(B
                    $B$=$7$F(BRADIX Tree$B$N%N!<%I?t$r=PNO$9$k;XDj$r$*$3$J$&!#(B
                    $B$?$@$7!"$3$N%Q%i%a!<%?$,(BOFF$B$N>l9g$O!"=PNO$7$J$$!#(B
$B!!(B4OCTETS_AS      : ON/OFF
                    ON: 4Byte-AS$BBP1~$r(BEnable$B$K$9$k!#(B
                    OFF:Disable$B$K$9$k!#(B
                    4OCTETS_AS$B$r(BON$B$K$7$?>l9g$O!"(BNLRI$B%*%W%7%g%s$rIU$1$J$/(B
$B!!!!!!!!!!!!!!!!!!!!$F$O$J$j$^$;$s!#(B

4. $B5/F0J}K!(B

    BGPView$B$N5/F0$K$O!"$$$/$D$+$N%*%W%7%g%s$,;XDj$G$-$^$9!#(B

    bgpview [-v] [-f filename] [-d]

    -v          : BGPView$B$N%P!<%8%g%s$rI=<($7$F=*N;$7$^$9!#(B
    -f filename : BGPView$B$N5/F0@_Dj%U%!%$%k$r;XDj$7$^$9!#(B
                  $B;XDj$7$J$$>l9g$O!"(B/usr/local/etc/bgpview.cfg$B$K$J$j$^$9!#(B
    -d          : BGPView$B$r%G!<%b%s$G5/F0$7$^$9!#(B


5. $BMxMQJ}K!(B

5.1 $B%m%0%U%!%$%k$N%j%9%?!<%H(B

    $B%m%0%U%!%$%k$O!"<u?.$7$?%a%C%;!<%8$N>pJs$r:Y$+$/=PNO$9$k$?$a!"=V$/4V(B
  $B$KBg$-$J%5%$%:$N%U%!%$%k$K@.$C$F$7$^$$$^$9!#$3$NMM$J>u67$G$O!"%U%!%$%k(B
  $B%7%9%F%`$,$$$C$Q$$$K$J$jLdBj$,$G$^$9!#(B

    $B%m%0%U%!%$%k$,Bg$-$J%5%$%:$K$J$C$F$-$?>l9g!"(Bbgpview$B%W%m%;%9$K(BKILLHUP
  $B%7%0%J%k$rAw?.$9$k$3$H$G!"%m%0%U%!%$%k$,?7$7$/$J$j$^$9!#$3$N$H$-!":#$^(B
  $B$G$N%m%0$O!";XDj$N%m%0%U%!%$%kL>$K(B".bak"$B$,IU$$$?%U%!%$%kL>$GJ]B8$5$l$F(B
  $B$$$^$9!#(B

    BGPView$B$N%W%m%;%9(BID$B$O!"(B/var/run/bgpview.pid(*)$B$K5-O?$5$l$k$h$&$K$J$C(B
  $B$F$$$^$9$N$G!"(B

    kill -HUP `cat /var/run/bgpview.pid`

  $B$J$I$7$F!"%m%0%U%!%$%k$r?7$7$/$7$F2<$5$$!#(B

  (*)SunOS $B$N>l9g$O!"(B/var/adm/bgpview.pid

  $B$3$N5!G=$O!"(BBGPView Shell$B$N(B'do reflesh sytemlog'$B%3%^%s%I$G$b9T$($^$9!#(B


5.2 Prefix Log$B%U%!%$%k$N%j%9%?!<%H(B

    $B%m%0%U%!%$%k$HF1MM$K!"(Bprefix log$B%U%!%$%k$bAjEv$JBg$-$5$H$J$j$^$9!#$3(B
  $B$N$?$a!"(Bprefix log$B%U%!%$%k$b%7%0%J%k$rM?$($k$3$H$K$h$j%j%9%?!<%H$9$k;v(B
  $B$,2DG=$G$9!#(B

    prefix log$B%U%!%$%k$N%j%9%?!<%H$O!"(BSIGUSR1$B%7%0%J%k$rM?$($k$3$H$K$h$C$F(B
  $B9T$o$l$^$9$N$G!"F1$8$/(B

    kill -USR1 `cat /var/run/bgpview.pid`

  $B$J$I$7$F!"(Bprefix log$B%U%!%$%k$r%j%9%?!<%H$7$F2<$5$$!#(B

    $B$3$N5!G=$O!"(BBGPView Shell$B$N(B'do reflesh prefixlog all'$B%3%^%s%I$G$b9T$((B
  $B$^$9!#(B


5.3 $B%@%_!<7PO)$N%"%J%&%s%9(B

    BGPView$B$G$O!"%@%_!<$N7PO)$r:n@.$7$F(BPeer$B$K%"%J%&%s%9$9$k$3$H$,$G$-$^$9!#(B
  $B%"%J%&%s%9$9$k7PO)$O!"(Bbgpview.cfg$B$N(B"DUMMY_ROUTE_CFG"$B$N@_Dj$G5-=R$5$l$k(B
  $B%U%!%$%k!"$^$?$O!"@_Dj$5$l$F$$$J$$>l9g$O!"%G%U%)%k%H$G(B
  "/usr/local/etc/dummyroute.cfg"$B%U%!%$%k$K$h$C$F;XDj$7$^$9!#(B

  $B0J2<$K(Bdummyrote.cfg$B%U%!%$%k$N@_DjJ}K!$r5-=R$7$^$9!#(B
  $B5-=R7A<0$O!"(BBGPView$B$N@_Dj%U%!%$%k$HF1MM$G%3%^%s%I$,I,MW$G$9!#(B

    NEIGHBOR	: $B%@%_!<$N7PO)$rAw=P$9$k(BPeer$B$N(Bneighbor address$B$r;XDj$7$^(B
                  $B$9!#(B
    STARTPREFIX	: $BAw=P$9$k7PO)$N@hF,(BPrefix$B$r;XDj$7$^$9!#(B
    NUMBEROF	: STARTPREFIX$B$r@hF,$N(BPrefix$B$H$7$F!"4v$D$N7PO)$rAw=P$9$k$+(B
                  $B$r5-=R$7$^$9!#(B
    PACK	: 1$B$D$N(BBGP Update$B%a%C%;!<%8$K4v$D$N(BPrefix$B$r%Q%C%/$7$FAw=P(B
                  $B$9$k$+$r;XDj$7$^$9!#(B
    ASPATH	: $BAw=P$9$k:]$N(BAS Path$B$r;XDj$7$^$9!#(BAS Path$B$N;XDj$O!"J#?t(B
                  $B$N(BAS$BHV9f$r(B'_'($B%"%s%@!<%9%3%"(B)$B$G6h@Z$C$?7A<0$G5-=R$7$^$9!#(B
                  $B$^$?!"(BAtomic Aggregate$BEy$N;XDj$O$G$-$^$;$s!#(B
                  $BNc(B: AS_SEQUENCE$B$,(B"10 20 30"$B$N>l9g$O!"(B"10_20_30"$B$H$J$j$^(B
                  $B$9!#(B
    TYPE	: $BAw=P(BUPDATE$B%a%C%;!<%8$N<oJL$r;XDj$7$^$9!#(B
                  UPDATE   : Prefix$B$N(BUPDATE$B$rAw=P$7$^$9!#(B
		  WITHDRAW : Prefix$B$N(BWITHDRAW$B$rAw=P$7$^$9!#(B
			     WITHDRAW$B$N>l9g!"(BASPATH$B$OL5;k$5$l$^$9!#(B
                  PLAYBACK : BGPView$B$N%m%0%U%!%$%k7A<0$K4^$^$l$k(BUpdate$B%a(B
                             $B%C%;!<%8%@%s%W$r$b$H$K!"<u$1$?(BUpdate$B%a%C%;!<(B
                             $B%8$HF1$8%Q%1%C%H$r;XDj$N(BPeer$B$KAw=P$7$^$9!#(B
    ASTYPE	: UPDATE$B%b!<%I;~$N(BAS Path$B$NJQ2=7ABV$r;XDj$7$^$9!#(B
		  NORMAL      : $BAw=P$9$k(BUpdate$B%a%C%;!<%8A4$F$K(BDummyroute.cfg
                                $B$G;XDj$5$l$k(BASPATH$B$r$=$N$^$^IuF~$7$^$9!#(B
                  INCREMENTAL : $BAw=P$9$k(BUpdate$B%a%C%;!<%8$KIuF~$9$k(BAS Path
                                $B$O!"(BDummyroute.cfg$B$N(BASPATH$B$K;XDj$5$l$k$b$N(B
                                $B$K2C$(!"#1%a%C%;!<%8$4$H$K(B "1_1"$B$r5/E@$H$7(B
				$B$F=g$K0[$J$k(BAS Path$B$rAwIU$9$k$h$&$K$7$^$9!#(B
				$BNc$($P!"(BASPATH$B$,(B "100_200"$B$H$J$C$F$$$?>l9g(B
				$B0lHV:G=i$N(BUpdate$B%a%C%;!<%8$G$O!"(B
				"100_200_1_1"$B$N(BAS Path$B$rAwIU$7$^$9!#$=$7$F!"(B
				10$BHVL\$KAwIU$9$k(BAS Path$B$O!"(B"100_200_10_1"
				$B$H$J$j$^$9!#(B
    DUMPFILE	: PLAYBACK$B%b!<%I;~$KFI$_9~$`%@%s%W%U%!%$%k$r;XDj$7$^$9!#(B
		  UPDATE/WITHDRAW$B%b!<%I;~$OL5;k$5$l$^$9!#(B
 
    $B$3$N5!G=$K$h$C$FAw=P$5$l$k(BPrefix$B$O!"0J2<$NB0@-$r8GDjE*$K$b$A!"$3$l0J(B
  $B30$NB0@-$OIU2C$5$l$^$;$s!#(B
 
    Metric       : 0
    Next_Hop     : BGPView$B$N@_Dj%U%!%$%k$G;XDj$5$l$k(BRouter ID
    Orign        : IGP
    AS Path $BB0@-(B : AS_SEQUENCE$B$N$_(B(Prefix Update$B$N$_(B)
  
  $B$^$?!"$3$N5!G=$K$h$C$FAw=P$5$l$k(BPrefix Length$B$O(B /24 $B$G8GDj$5$l$^$9!#(B

    $BAw=P$G$-$k(BPrefix$B?t$N8B3&$O!"4pK\E*$K(B256^3(16777216$B8D(B)$B$G$9$,!"(B
  STARTPREFIX$B$K$h$j$=$N?t$OJQF0$7$^$9!#(B

  $B"(Cm0U(B1 : $B$?$H$((BBGPView$B$,%@%_!<7PO)$r(B20$BK|7PO)Aw=P$7$F$b!"<u$1B&$N%k!<%?(B
            $B$N%a%b%jEy$K$h$j!"Aw=P$7$-$l$J$$>l9g$,$"$j$^$9!#(B
            $B7P83E*$K!"<u$1B&%k!<%?B&$,%a%b%jITB-Ey$K$h$j7PO)$,<u$1@Z$l$J(B
            $B$/$J$C$?>l9g!"%k!<%?B&$O(BNOTIFICATION$B%a%C%;!<%8$J$7$K(BPeer$B$r@Z(B
            $BCG$9$k$3$H$,$"$j$^$9!#(B
  $B"(Cm0U(B2 : PLAYBACK$B%b!<%I;~$O!"%@%s%W%U%!%$%k$+$iFI$_9~$s$@>pJs$r$=$N$^(B
            $B$^Aw=P$9$k$?$a!"$3$l$i$NB0@-$O$D$-$^$;$s!#(BAS Path$B$K$D$$$F$O!"(B
            $B;XDj$7$F$bL5;k$5$l$^$9!#(B

    dummyroute.cfg$B$G;XDj$7$?%@%_!<7PO)$N>pJs$O!"(BBGPView$B$K(BSIGUSR2$B%7%0%J%k(B
  $B$rAw?.$9$k$3$H$G!"(BBGPView$B$O%@%_!<7PO)$rAw=P$7$O$8$a$^$9!#0J2<$N%3%^%s%I(B
  $B$r;29M$K$7$F2<$5$$!#$J$*!"7PO)$NAw=P3+;O$N%?%$%_%s%0$O!"(BBGPView Shell$B$N(B
  'do announce dummyroute'$B$K$h$C$F$b9T$($^$9!#(B

    kill -USR2 `cat /var/run/bgpview.pid`

  $B$^$?!"%@%_!<7PO)Aw=PCf$O!"B>$N(BPeer$B$KBP$9$k%@%_!<7PO)$NAw=P$O9T$($^$;$s!#(B

$B!!"((B4Byte-AS$B$N%@%_!<7PO)Aw=P$K$bBP1~$7$F$$$^$9!#(B


5.4 PLAYBACK$B5!G=$GMxMQ$9$k%@%s%W%U%!%$%k$K$D$$$F(B

    $B%@%_!<7PO)Aw=P;~$G$N(BPLAYBACK$B%b!<%I$O!"2a5n$K(BBGPView$B$G<hF@$7$?%m%0$r$b(B
  $B$H$K$=$N;~$K<u$1$?(BUpdate$B%a%C%;!<%8$r:F8=$9$k$3$H$,$G$-$^$9!#(B
    $B$3$N$H$-!"(BBGPView$B$O!"%@%_!<7PO)Aw=P$N$b$H$H$J$k>pJs$r(BBGPView$B$N%m%0$N(B
  $BCf$N(BUPDATEDUMP$BB0@-$G=PNO$5$l$kItJ,$N$_$rMxMQ$7$^$9!#$D$^$j!"(BBGPView$B$N%m(B
  $B%0$r5-O?$9$k:]$K(BUPDATEDUMP$BB0@-$,M-8z$K$J$C$F$$$J$$%m%0$G$O!"(BPLAYBACK$B5!(B
  $BG=$rMxMQ$G$-$^$;$s!#(B


6. $BCm0U;v9`(B

  $B!{K\%P!<%8%g%s$G$O!"<u?.$7$?7PO)$rJ];}$7$^$;$s$N$G!"<u$1$?7PO)$N>\:Y$r(B
    $B8+$k$3$H$O$G$-$^$;$s!#(B

  $B!{K\%P!<%8%g%s$G$O!"(BRFC1771$B$G;XDj$5$l$k$h$&$J%(%i!<%O%s%I%j%s%0$O!"KX$s(B
    $B$I9T$C$F$*$j$^$;$s!#(B

  $B!{K\%=%U%H%&%(%"$rMxMQ$7$?:]$K@8$8$kG!2?$J$kB;32Ey$K4X$7$F!":n<T$O0l@Z(B
    $B$N@UG$$rIi$$$^$;$s!#(B

  $B!{K\%=%U%H%&%(%"$O!":FG[I[$r6X;_$7$^$9!#:FG[I[$r9T$$$?$$>l9g$O:n<T$KO"(B
    $BMm$7$F$/$@$5$$!#(B

  $B!{K\%=%U%H%&%(%"$K4X$9$k8f0U8+!&8fMWK>Ey$"$j$^$7$?$i!":n<T$^$G8fO"Mm2<(B
    $B$5$$!#(B


7. Distribution Policy

        All files in the package includes folowing description.

        /*
         * Copyright (c) 1998-2000 Internet Initiative Japan Inc.
         * All rights reserved.
         *
         * Redistribution and use in source and binary forms, with or without
         * modification, are permitted provided that the following conditions
         * are met:
         * 1. Redistributions of source code must retain the above copyright
         *    notice, this list of conditions and the following disclaimer.
         * 2. Redistribution with functional modification must include
         *    prominent notice stating how and when and by whom it is
         *    modified.
         * 3. Redistributions in binary form have to be along with the source
         *    code or documentation which include above copyright notice, this
         *    list of conditions and the following disclaimer.
         * 4. All commercial advertising materials mentioning features or use
         *    of this software must display the following acknowledgement:
         *      This product includes software developed by Internet
         *      Initiative Japan Inc.
         *
         * THIS SOFTWARE IS PROVIDED BY ``AS IS'' AND ANY EXPRESS OR IMPLIED
         * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
         * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
         * DISCLAIMED.
         */

        As described in the notice, redistribution is allowed for any
        purpose including commercial product as long as satisfying
        above conditions.  Redistribution with modification is not
        prohibited but we encourage everybody to share the improvement
        for public.  We are happy to integrate contributions in
        original release.  If you still want to distribute your own
        modified package, please make it clear that how it is changed
        and use different distribution name to avoid the conflict.


8. $B:n<TO"Mm@h(B

  $B3t<02q<R$^$[$m$P9)K<(B
  $B6aF#(B $BK.><(B 

  E-mail: kuniaki@ate-mahoroba.jp

                                                                     $B0J>e(B
---------+---------+---------+---------+---------+---------+---------+--+
readme.txt,v 1.5 2005/10/21 02:24:50 kuniaki Exp
