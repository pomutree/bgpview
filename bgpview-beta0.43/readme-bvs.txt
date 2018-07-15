                      BGPView Shell コマンドリファレンス
                          BGPView Beta Version 0.43

                                 Kuniaki Kondo
          Copyright (C) 2000 - 2002 Internet Initiative Japan Inc.
                 Copyright (C) 2003-2005 Intec NetCore, Inc.
                 Copyright (C) 2006-2015 Mahoroba Kobo.

                                 26 Jan 2007

1. BGPView Shellへの接続

    BGPView Shell(以降、BVS)へはtelnetコマンドを利用して接続できます。
  接続に関する情報は、設定ファイル(bgpview.cfg)のSHELL_PORT,
  SHELL_MAXCON および SHELL_PASSWD に設定します。

    接続は、接続ポートが3000(default値)の場合は以下のようにして接続で
  きます。

    % telnet <hostname> 3000

    接続されれば、パスワードの入力が求められますので、SHELL_PASSWDに
  設定されているパスワードを入力しログインして下さい。ログイン後は、
  以下のように"BGPView# "というプロンプトが表示され、コマンド入力待ち
  になります。

    % telnet <hostname> 3000
    Trying xxx.xxx.xxx.xxx...
    Connected to <hostname>.
    Escape character is '^]'.
    Password:*******
    BGPView# 

2. BVSの利用

     BVSでは、コマンド入力時のラインエディットとして幾つかの機能をポー
   トしています。

   1)オートコンプリートのサポート
       コマンド入力時に、コマンド文字列を全て入力しなくても、入力した
     文字列でコマンドが確定できれば、自動的にそのコマンドを補完します。
       例えば、exitコマンドの場合、第一単語の先頭文字列が'e'のコマンド
     は、exitコマンドしかないため、e<CR>でexitコマンドと認識し実行しま
     す。
       また、'e'だけを入力してタブキーを入力することで、'exit'まで補完
     し、補完した文字列を表示します。

   2)ヒストリのサポート
       過去に入力したコマンドを再度入力する際に<Ctrl-P>を入力すること
     で一つ前に入力したコマンドを呼び出すことができます。保持している
     コマンドは、最大で7つまでです。また、<Ctrl-P>を入力しすぎた場合、
     <Ctrl-N>で一つ戻ることが可能です。

   3)行編集機能のサポート
       BGPViewでは、コマンドの編集のためにいくつかの機能を利用できます。
       以下にその機能を列挙します。

       Ctrl+U  : 行削除
       Ctrl+W  : 単語削除、カーソル位置より前方の１単語を削除します。
       Ctrl+D  : 位置文字削除、カーソル位置の１文字を削除します。
       Ctrl+A  : カーソルを行の先頭に移動させます。
       Ctrl+F  : カーソルを１文字左に移動します。
       Ctrl+B  : カーソルを１文字右に移動します。
       Ctrl+K  : カーソル位置以降の文字を削除します。

       この他、Ctrl+Lによって画面のクリアが行えます。


   4)ヘルプのサポート
       コマンド入力時、サポートされているコマンドがわからなくなった場合、
     その時点で'?'を入力することで、その時点でサポートされているコマン
     ドのリストが表示されます。

3. コマンドリファレンス

3.1 exit/quitコマンド

      exit<CR>
      quit<CR>
	BVSをログアウトします。

3.2 BGPViewプロセス制御関連コマンド

      shutdown<CR>
	BGPViewを終了させます。

      reboot<BR>
	BGPViewを再起動します。

3.3 showコマンド

      show cron list<CR>
	現在設定されているCronの状態を表示します。

      show ip bgp [detail]<CR>
      show ip bgp [active|inactive] [detail]<CR>
	全てのPeerから受信した全ての経路を表示します。
        パラメータとして'active'を指定した場合は、その時に有効となって
        いる経路のみを表示し、'inactive'を指定した場合は、その時に無効
        となっている経路のみを表示します。
        detailパラメータを指定した場合には、経路の更新履歴も参照できる。

      show ip bgp <address><CR>
        <address>で指定される経路を検索して表示します。

      show ip bgp summary<CR>
	設定されているすべてのBGP Peerの状態をリストで表示します。

      show ip bgp neighbor <neighbor address><CR>
	指定のNeighbor AddressのPeerの詳細、および設定を表示します。

      show ip bgp neighbor <neighbor address> routes<CR>
      show ip bgp neighbor <neighbor address> [active|inactive] routes<CR>
	指定のNeighbor AddressのPeerから受信した経路を全て表示します。
        パラメータとして'active'を指定した場合は、その時に有効となって
        いる経路のみを表示し、'inactive'を指定した場合は、その時に無効
        となっている経路のみを表示します。

      show ip bgp statics <neighbor address><CR>
	指定のNeighbor Addressの受信メッセージ数などの通信状態を表示し
        ます。

      show ip bgp route time <Time>
      show ip bgp route time <Time> le
      show ip bgp route time <Time> ge
	Timeで指定された時刻、もしくは以前/以後の経路のみを表示します。
	le付きは指定時刻以降に受信したものを、ge付きは指定時刻以前に受
        信したもを、そして、指定無しの場合は指定時刻のものをそれぞれ表
        示します。また、Timeフィールドには以下のフォーマットが利用でき
        ます。
          - 直接時刻指定		hh:mm:ss
          - 日時指定			w,d,hが指定可能
		例： 1週間と2日を指定する場合	1w2d
		     1日と2時間を指定する場合	1d2h

      show ip nexthop summary<CR>
      show ip nexthop summary neighbor <neighbor address><CR>
        受信した経路をNextHop毎に何経路あるかを集計して表示します。
        <neighbor address>を指定した場合は、指定のNeighborの物だけを集
        計して表示します。

      show ip route summary<CR>
        受信した経路の総経路数、総Prefix数、経路表のノード数、Prefix長
        別のPrefix数、および平均のAS Path長を表示します。

      show users<CR>
	現在、BVSに接続されているユーザのリストを表示します。

      show proccess<CR>
	BGPViewが起動されているホストのプロセス、メモリの状態を表示し
        ます。

      show version<CR>
	BGPViewのバージョンとシステムの設定値を表示します。

      show configuration<CR>
	BGPViewの起動設定ファイル(defaultはbgpview.cfg)の内容を表示し
	ます。

      show announce dummyroute<CR>
	仮想経路広報が実行されている状態のとき、その仮想経路の送信状態
	を表示します。

      show v6 bgp [detail]<CR>
      show v6 bgp [active|inactive] [detail]<CR>
	全てのPeerから受信した全てのIPv6経路を表示します。
        パラメータとして'active'を指定した場合は、その時に有効となって
        いる経路のみを表示し、'inactive'を指定した場合は、その時に無効
        となっているIPv6経路のみを表示します。
        detailパラメータを指定した場合には、IPv6経路の更新履歴も参照できる。

      show v6 bgp neighbor <neighbor address> routes<CR>
      show v6 bgp neighbor <neighbor address> [active|inactive] routes<CR>
	指定のNeighbor AddressのPeerから受信したIPv6経路を全て表示します。
        パラメータとして'active'を指定した場合は、その時に有効となって
        いる経路のみを表示し、'inactive'を指定した場合は、その時に無効
        となっている経路のみを表示します。

      show irr status
      show irr status active
        受信経路をIRRで確認し、その確認状態を表示します。
        'active'を指定した場合は、withdrawnの経路は表示しません。

      show irr status summary
      show irr status summary neighbor <neighbor address>
        受信経路をIRRで確認し、そのサマリを表示します。
        'neighbor address'を指定すると、指定した特定のpeerのサマリを
        表示します。

3.4 clearコマンド

      clear ip bgp <neighbor address><CR>
	指定のNeighbor AddressのBGP Peerを切断します。切断後は、しばら
	くして、再接続を試みます。

      clear ip bgp statics <all | neighbor address><CR>
	指定のNeighbor Addressの通信状態の情報をクリアします。allが指
	定された場合には、全ての情報をクリアします。

      clear route history<CR>
        経路情報更新履歴をクリアします。

      clear v6 route history<CR>
        IPv6経路情報更新履歴をクリアします。

3.5 doコマンド

      do announce dummyroute<CR>
	仮想経路広報を起動設定ファイルで指定された仮想経路広報の設定
        にしたがって広報を開始します。(SIGUSR2をプロセスに送った場合と
	同等の動作を行います。)

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
        <Filename>に指定されるファイルに、BGPViewで受けた経路を全て出
        力します。
        'detail'が指定されている場合には、表示経路の更新履歴も表示しま
        す。また、パラメータとして'active'を指定した場合は、その時に有
        効となっている経路のみを表示し、'inactive'を指定した場合は、そ
        の時に無効となっている経路のみを表示します。
        'neighbor'パラメータが指定された場合には、指定のneighborの経路
        のみを出力します。

      do read cron file<CR>
	bgpview.cfgファイルに設定されているCronファイルを読み込み、
        Cronタイマを再設定します。

      do reflesh sytemlog<CR>
	BGPViewのログファイルをリフレッシュします。(SIGHUPをプロセスに
	送った場合と同等の動作を行います。)
	
      do reflesh prefixlog all<CR>
	BGPViewのPrefix Logの全てをリフレッシュします。(SIGUSR1をプロ
	セスに送った場合と同等の動作を行います。)

      do test route rtt neighbor <Neighbor Address> 
	          inject <Injection Prefix> timeout <Timeout Sec> <CR>
	指定の<Neighbor Address>のPeerより<Injection Prefix>で指定され
	る経路を送出します。送出したPrefixは監視されBGPViewで接続して
	いるいづれかのPeerで送出したPrefixと同等のPrefixを受信するまで
	の時間間隔をUpdateおよびWithdrawそれぞれで計測します。
	計測のタイムアウトは、<Timeout Sec>に秒単位で指定します。ただ
	し、コマンドのtimeout以降を省略した場合には、デフォルトで60秒
	が設定されます。

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
        <Filename>に指定されるファイルに、BGPViewで受けたIPv6経路を全
        て出力します。
        'detail'が指定されている場合には、表示経路の更新履歴も表示しま
        す。また、パラメータとして'active'を指定した場合は、その時に有
        効となっているIPv6経路のみを表示し、'inactive'を指定した場合は、
        その時に無効となっているIPv6経路のみを表示します。

      do output irrstats filename <File Name> all
      do output irrstats filename <File Name> active
        受信経路をIRRでチェックした結果を指定のファイルに出力します。
        'active'を指定した場合は、withdrawnの経路は出力しません。

      do output aslist filename <File Name> neighbor <neighbor address>
        nprobeに入力可能なas-listファイルを出力します。


3.6 setコマンド

      set route history clear time <Clear Time>
        次の経路情報更新履歴のクリアまでの時間を秒で指定します。
        次回のクリアが実施された後は、設定ファイルで指定されているクリ
        ア間隔に戻ります。

      set neighbor <Neighbor Address> announce status <ON|OFF>
        BGPのFSM状態変更時に通知するメールの通知・非通知を設定する。

      set neighbor <Neighbor Address> infolog <ON|OFF>
        指定PeerのInformation Logの出力のON/OFFを設定します。

      set neighbor <Neighbor Address> <enable|disable>
        指定Peerの有効／無効の設定をします。

      set v6 route history clear time <Clear Time>
        次のIPv6経路情報更新履歴のクリアまでの時間を秒で指定します。
        次回のクリアが実施された後は、設定ファイルで指定されているクリ
        ア間隔に戻ります。

								  以上
readme-bvs.txt,v 1.4 2005/10/21 02:24:50 kuniaki Exp
