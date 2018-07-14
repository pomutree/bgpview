BGP-4 Protocol Viewer
  Beta Version 0.43

   Kuniaki Kondo
Copyright (C) 1998 - 2002 Internet Initiative Japan Inc.
Copyright (C) 2003-2005 Intec NetCore, Inc.
Copyright (C) 2006-2015 Mahoroba Kobo, Inc.

     20 Apri 2007

-------------------------------------------------------------------------
1.    概要
2.    プラットフォーム
3.    インストール
3.1   コンパイル
3.2   設定
3.2.1 基本設定
3.2.2 Peer毎の設定
4.    起動方法
5.    利用方法
5.1   ログファイルのリスタート
5.2   Prefix Logファイルのリスタート
5.3   ダミー経路のアナウンス
5.4   PLAYBACK機能で利用するダンプファイルについて
6.    注意事項
7.    Distribution Policy
8.    作者連絡先
-------------------------------------------------------------------------

1. 概要

BGP-4 Protocol Viewer(以降、bgpview)は、BGP-4プロトコルで指定のルータ
等とコネクションを張り、そこから受信されるデータを詳細にモニタし、また、
受信したデータを基にした統計情報を取得することを目的につくられたソフト
ウエアです。


2. プラットフォーム

現在、動作が確認されているフラットフォームは以下の通りです。

-FreeBSD 4.10-RELEASE


3. インストール

3.1 コンパイル

本パッケージを適当なディレクトリに展開して下さい。

次に、そのディレクトリに移動し、

make

を実行して下さい。なお SunOS の場合は、'bind' のインストールと
Makefile の一部編集("LIBS=" の指定)が必要です。

※BGPView Shellの'show process'コマンドをサポートするには、topコマンド
が必要になります。topコマンドは、/usr/bin/ ディレクトリにあることが前
提としています。もしtopコマンドがない場合は、MakefileのCFLAGSから、
-DHAVE_TOPを削除してください。

makeが完了したら、suコマンドでroot権限になり、

make install

によって、BGPViewをインストールしてください。BGPView本体は、
/usr/local/binディレクトリに格納され、bgpview.cfg.sampleは、
/use/local/etcディレクトリにインストールされます。


3.2 設定

BGPViewの設定ファイルはdefaultで/usr/local/etc/bgpview.cfgを読み込み
ます。変更の必要がある場合は、BGPViewの起動オプション'-f'で指定してくだ
さい。

BGPViewの設定ファイルは、基本的に１行毎に１つの設定を行います。また、
行頭が"#"で始まるものは、コメント行として無視し、無効な設定は無視します。

各行は、

<COMMAND>	<Parameter>

の対になっています。


3.2.1 基本設定

LOCAL_AS        : ローカル(bgpview)のAS番号を指定します。
IDENTIFIER      : ルータIDを指定します。
基本的に、起動するマシンのインターフェースに指定され
ているIP Addressで問題ありません。
SYSTEM_LOG      : BGPViewが共通で出力するログファイルをFull Pathで指定
します。
また、ログファイルをsyslogにしたい場合は、@マーク付き
でファシリティ名を記述するとsyslogに出力されます。
(例: @LOCAL7)
尚、syslogのfacilityは、local0～local7に限られます。
DUMMY_ROUTE_CFG : BGPViewからアナウンスするダミーの経路に関する設定を記
述するファイルをFull Pathで指定します。指定が無い場合
は、デフォルトで、"/usr/local/etc/dummyroute.cfg"とな
ります。
(記述の詳細は後述します。)
SHELL_PORT      : BGPView Shellの接続ポート番号を指定します。
デフォルトは3000
SHELL_MAXCON    : BGPView Shellに接続できる最大数を指定します。
SHELL_PASSWD    : BGPView Shellに接続する際のログインパスワードを指定し
ます。
パスワードは、付属のbvspasswdによって変換された、暗号
化されたパスワードを指定してください。
SENDMAIL_CMD    : BGPViewでメールを送付する際に利用するsendmailコマンド
を指定します。
ADMIN_ADDR      : BGPViewから送付するメールのFromアドレスを指定します。
NOTIFY_ADDR     : BGPViewから送付するメールの送付先のメールアドレスを指
定します。
SPOOL_DIR       : BVSコマンドなどで指定される出力ファイルのデフォルトの
ディレクトリを指定します。
RTHISTORYCLR    : 経路情報更新履歴をクリアする間隔を指定する。
0が指定されている場合には動作しない。
V6RTHISTORYCLR  : IPv6経路情報更新履歴をクリアする間隔を指定する。
0が指定されている場合には動作しない。
CRON            : BGPViewのCron定義ファイルのファイル名を指定します。
SYSNAME         : 起動するBGPViewのシステム名を指定します。
BGPView Shellのプロンプトなどに利用されます。
IRR_CHECK       : IRRサーバへのチェックを有効にします。
IRR_HOST        : IRRサーバのホストを指定します。
IRR_PORT        : IRRサーバの問い合わせポートを指定します。

3.2.2 Peer毎の設定

Peer毎の設定は、NEIGHBOR行で開始されます。

NEIGHBOR        : パラメータはありません。
この行を発見するとそれ以降は新しいPeerとしての設定を
記述します。
DESCRIPTION     : Peerの名称を登録します。ホワイトスペースは認識しませ
んので、"_"などで繋げて記述してください。
PEER_ADMIN      : Peerの管理者メールアドレスを指定します。
ADDRESS         : neighborアドレスを指定します。
REMOTE_AS       : リモート側のAS番号を指定します。
IND_IDENT       : Peer個別のBGP Identifierを設定します。
デフォルト値は、システム設定値IDENTIFIERとなります。
IND_AS          : Peer個別のローカルAS番号を設定します。
デフォルト値は、システム設定値LOCAL_ASとなります。
HOLD_TIMER      : OPEN時にリモート側に伝えるホールド時間を指定します。
NLRI            : 交換する経路のNLRIを指定する。サポートしているタイプ
は以下の通り。
IPv6_UNICAST	: IPv6経路
　　　　　　　　　　　IPv4_UNICAST	: IPv4経路
VIEW            : ログに残すメッセージの種類を指定します。
パラメータは以下の値をとることができます。
OPEN		 UPDATE		 UPDADDR
NOTIFICATION	 KEEPALIVE	 SYSTEM
UPDATEDUMP
ANNOUNCE        : パラメータで指定される挙動を認めた時に、なんらかの方
法で、その内容を伝えます。パラメータは以下の通りです。
ROUTE_DIFF    : (未サポート)
STATUS_CHANGE : Peerの状態がIDLEまたはESTABLISHEDに
         変化した時に設定されているメールアド
         レスに通知します。
LOGNAME         : Peer毎のログの出力ファイルをFull Pathで指定します。
INTERVAL_INFO   : BGPセッションで得た統計情報を出力する間隔を指定します。
(秒単位)
INFO            : BGPセッションで得た統計情報を出力するファイルをFull
Pathで指定します。
PREFIXOUT       : update,withdrawで受信したprefixのリストを指定のファイ
ルに出力します(Prefix Logファイル)。１つのupdateメッ
セージで受信した複数のprefixを１行に出力します。
PREFIXOUTSUM    : この設定のパラメータがONの場合、PREFIXOUTで指定する
Prefix Logファイルの先頭に、ファイルを生成した時点で
BGPViewのRADIX Treeに格納されている経路数、Prefix数
そしてRADIX Treeのノード数を出力する指定をおこなう。
ただし、このパラメータがOFFの場合は、出力しない。
　4OCTETS_AS      : ON/OFF
ON: 4Byte-AS対応をEnableにする。
OFF:Disableにする。
4OCTETS_ASをONにした場合は、NLRIオプションを付けなく
　　　　　　　　　　てはなりません。

4. 起動方法

BGPViewの起動には、いくつかのオプションが指定できます。

bgpview [-v] [-f filename] [-d]

-v          : BGPViewのバージョンを表示して終了します。
-f filename : BGPViewの起動設定ファイルを指定します。
指定しない場合は、/usr/local/etc/bgpview.cfgになります。
-d          : BGPViewをデーモンで起動します。


5. 利用方法

5.1 ログファイルのリスタート

ログファイルは、受信したメッセージの情報を細かく出力するため、瞬く間
に大きなサイズのファイルに成ってしまいます。この様な状況では、ファイル
システムがいっぱいになり問題がでます。

ログファイルが大きなサイズになってきた場合、bgpviewプロセスにKILLHUP
シグナルを送信することで、ログファイルが新しくなります。このとき、今ま
でのログは、指定のログファイル名に".bak"が付いたファイル名で保存されて
います。

BGPViewのプロセスIDは、/var/run/bgpview.pid(*)に記録されるようになっ
ていますので、

kill -HUP `cat /var/run/bgpview.pid`

などして、ログファイルを新しくして下さい。

(*)SunOS の場合は、/var/adm/bgpview.pid

この機能は、BGPView Shellの'do reflesh sytemlog'コマンドでも行えます。


5.2 Prefix Logファイルのリスタート

ログファイルと同様に、prefix logファイルも相当な大きさとなります。こ
のため、prefix logファイルもシグナルを与えることによりリスタートする事
が可能です。

prefix logファイルのリスタートは、SIGUSR1シグナルを与えることによって
行われますので、同じく

kill -USR1 `cat /var/run/bgpview.pid`

などして、prefix logファイルをリスタートして下さい。

この機能は、BGPView Shellの'do reflesh prefixlog all'コマンドでも行え
ます。


5.3 ダミー経路のアナウンス

BGPViewでは、ダミーの経路を作成してPeerにアナウンスすることができます。
アナウンスする経路は、bgpview.cfgの"DUMMY_ROUTE_CFG"の設定で記述される
ファイル、または、設定されていない場合は、デフォルトで
"/usr/local/etc/dummyroute.cfg"ファイルによって指定します。

以下にdummyrote.cfgファイルの設定方法を記述します。
記述形式は、BGPViewの設定ファイルと同様でコマンドが必要です。

NEIGHBOR	: ダミーの経路を送出するPeerのneighbor addressを指定しま
す。
STARTPREFIX	: 送出する経路の先頭Prefixを指定します。
NUMBEROF	: STARTPREFIXを先頭のPrefixとして、幾つの経路を送出するか
を記述します。
PACK	: 1つのBGP Updateメッセージに幾つのPrefixをパックして送出
するかを指定します。
ASPATH	: 送出する際のAS Pathを指定します。AS Pathの指定は、複数
のAS番号を'_'(アンダースコア)で区切った形式で記述します。
また、Atomic Aggregate等の指定はできません。
例: AS_SEQUENCEが"10 20 30"の場合は、"10_20_30"となりま
す。
TYPE	: 送出UPDATEメッセージの種別を指定します。
UPDATE   : PrefixのUPDATEを送出します。
WITHDRAW : PrefixのWITHDRAWを送出します。
WITHDRAWの場合、ASPATHは無視されます。
PLAYBACK : BGPViewのログファイル形式に含まれるUpdateメ
  ッセージダンプをもとに、受けたUpdateメッセー
  ジと同じパケットを指定のPeerに送出します。
ASTYPE	: UPDATEモード時のAS Pathの変化形態を指定します。
NORMAL      : 送出するUpdateメッセージ全てにDummyroute.cfg
     で指定されるASPATHをそのまま封入します。
INCREMENTAL : 送出するUpdateメッセージに封入するAS Path
     は、Dummyroute.cfgのASPATHに指定されるもの
     に加え、１メッセージごとに "1_1"を起点とし
て順に異なるAS Pathを送付するようにします。
例えば、ASPATHが "100_200"となっていた場合
一番最初のUpdateメッセージでは、
"100_200_1_1"のAS Pathを送付します。そして、
10番目に送付するAS Pathは、"100_200_10_1"
となります。
DUMPFILE	: PLAYBACKモード時に読み込むダンプファイルを指定します。
UPDATE/WITHDRAWモード時は無視されます。

この機能によって送出されるPrefixは、以下の属性を固定的にもち、これ以
外の属性は付加されません。

Metric       : 0
Next_Hop     : BGPViewの設定ファイルで指定されるRouter ID
Orign        : IGP
AS Path 属性 : AS_SEQUENCEのみ(Prefix Updateのみ)

また、この機能によって送出されるPrefix Lengthは /24 で固定されます。

送出できるPrefix数の限界は、基本的に256^3(16777216個)ですが、
STARTPREFIXによりその数は変動します。

※注意1 : たとえBGPViewがダミー経路を20万経路送出しても、受け側のルータ
のメモリ等により、送出しきれない場合があります。
経験的に、受け側ルータ側がメモリ不足等により経路が受け切れな
くなった場合、ルータ側はNOTIFICATIONメッセージなしにPeerを切
断することがあります。
※注意2 : PLAYBACKモード時は、ダンプファイルから読み込んだ情報をそのま
ま送出するため、これらの属性はつきません。AS Pathについては、
指定しても無視されます。

dummyroute.cfgで指定したダミー経路の情報は、BGPViewにSIGUSR2シグナル
を送信することで、BGPViewはダミー経路を送出しはじめます。以下のコマンド
を参考にして下さい。なお、経路の送出開始のタイミングは、BGPView Shellの
'do announce dummyroute'によっても行えます。

kill -USR2 `cat /var/run/bgpview.pid`

また、ダミー経路送出中は、他のPeerに対するダミー経路の送出は行えません。

　※4Byte-ASのダミー経路送出にも対応しています。


5.4 PLAYBACK機能で利用するダンプファイルについて

ダミー経路送出時でのPLAYBACKモードは、過去にBGPViewで取得したログをも
とにその時に受けたUpdateメッセージを再現することができます。
このとき、BGPViewは、ダミー経路送出のもととなる情報をBGPViewのログの
中のUPDATEDUMP属性で出力される部分のみを利用します。つまり、BGPViewのロ
グを記録する際にUPDATEDUMP属性が有効になっていないログでは、PLAYBACK機
能を利用できません。


6. 注意事項

○本バージョンでは、受信した経路を保持しませんので、受けた経路の詳細を
見ることはできません。

○本バージョンでは、RFC1771で指定されるようなエラーハンドリングは、殆ん
ど行っておりません。

○本ソフトウエアを利用した際に生じる如何なる損害等に関して、作者は一切
の責任を負いません。

○本ソフトウエアは、再配布を禁止します。再配布を行いたい場合は作者に連
絡してください。

○本ソフトウエアに関する御意見・御要望等ありましたら、作者まで御連絡下
さい。


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


8. 作者連絡先

株式会社まほろば工房
近藤 邦昭

E-mail: kuniaki@ate-mahoroba.jp

                                          以上
---------+---------+---------+---------+---------+---------+---------+--+
readme.txt,v 1.5 2005/10/21 02:24:50 kuniaki Exp
