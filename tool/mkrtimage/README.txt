
This tool is for The full route visualiser software, which was
released by Shuichi Okubo on http://micho.mimora.com/routegraph/ .

When you use this, please download the tool from the web.

 coord.pl
 mkimake.pl

mkimage.pl use GD library. If you use this, then you must install
GD perl module (p5-GD in FreeBSD ports) and GD library.


Next,
you should rib file.
BGPView tool package attached the convert script, and
easy make image script.

Convert script is  makerib.pl.
easy make image script is  routetoimage.sh.

'makerib.pl' use route file which is made by BGPView
'do output route filename...' command.

you can make route image in following process.


1. make route file by BGPView (rt.txt)
2. Use easy make image script (routetoimage.sh rt)
     which proccess... rt.txt -> rib.txt -> rt.png

Enjoy.

Copyrights (C) 2006 Kuniaki Kondo, Bugest-Network
All rights Reserved.
kuniaki@bgpview.org
README.txt,v 1.1 2006/02/21 01:59:10 kuniaki Exp
