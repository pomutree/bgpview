#!/bin/sh
#
# Cron File Reflesh
#

#
# Copyright (c) 2001 Internet Initiative Japan Inc.
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistribution with functional modification must include
#    prominent notice stating how and when and by whom it is
#    modified.
# 3. Redistributions in binary form have to be along with the source
#    code or documentation which include above copyright notice, this
#    list of conditions and the following disclaimer.
# 4. All commercial advertising materials mentioning features or use
#    of this software must display the following acknowledgement:
#      This product includes software developed by Internet
#      Initiative Japan Inc.
#
# THIS SOFTWARE IS PROVIDED BY ``AS IS'' AND ANY EXPRESS OR IMPLIED
# WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED.
#
# cronrefresh.sh,v 1.2 2003/12/25 01:47:02 kuniaki Exp
#

#
# Copyright (c) 2003 Intec NetCore, Inc.
# All rights reserved.
#

CRONFILE='/usr/local/etc/bgpview.cron'
NEWFNAME='route'`date '+%Y%m%d'`'.txt'
SUMFNAME='sum'`date '+%Y%m%d'`'.txt'
ROUTEDIR='/var/spool/bgpviewlog/fullroute/'
CMDDO='do output route filename '$ROUTEDIR$NEWFNAME' detail'
FLAPSUM='/var/spool/bgpview-tools/flapsum.pl'
FLAPSUMSHELL='/var/spool/bgpview-tools/historycheck/execflapsum.sh'
TEMPFILE='/var/spool/bgpview-tools/historycheck/temp.txt'
SENDMAIL='/usr/sbin/sendmail'
SENDADDR='test@hogehoge.com'

echo '1 0 * * * '$CMDDO > $CRONFILE
echo '2 0 * * * clear route history' >> $CRONFILE
echo '30 0 * * * do read cron file' >> $CRONFILE

echo 'Subject: BGPView Route History Summary' > $TEMPFILE
echo 'To: '$SENDADDR >> $TEMPFILE
echo 'From: BGPView <kuniaki@inetcore.com>' >> $TEMPFILE
echo '' >> $TEMPFILE
echo 'Graph Data is Available at http://hogehoge.URL.Messages/' >> $TEMPFILE
echo '' >> $TEMPFILE


echo '#!/bin/sh' > $FLAPSUMSHELL
echo $FLAPSUM $ROUTEDIR$NEWFNAME `date '+%Y/%m/%d'` '>' $ROUTEDIR$SUMFNAME >> $FLAPSUMSHELL
echo 'cat ' $TEMPFILE $ROUTEDIR$SUMFNAME '|' $SENDMAIL $SENDADDR >> $FLAPSUMSHELL

chmod +x $FLAPSUMSHELL
