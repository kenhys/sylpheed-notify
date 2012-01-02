
SylNotify プラグイン
================================================================================

このプラグインはSylpheedの新着メール通知をGrowlもしくはSnarlに対応させるためのプラグインです。

動作環境
--------------------------------------------------------------------------------

* Windows XP SP3/Windows 7 SP1

* Sylpheed 3.1.2以降

* Growl 2.0.8以降
  http://www.growlforwindows.com/gfw/default.aspx

* growlnotify 
  http://www.growlforwindows.com/gfw/help/growlnotify.aspx
  http://www.growlforwindows.com/gfw/d.ashx?f=growlnotify.zip

* Snarl R2.5
  https://sites.google.com/site/snarlapp/home
  
* Snarl_CMD.exe 1.0
  http://tlhan-ghun.de/
  http://tlhan-ghun.de/Projects/SnarlCommandLineTools/Snarl_CMD/
  http://sourceforge.net/projects/mozillasnarls/files/Snarl_CMD/
  
Growl + growlnotifyもしくはSnarl + Snarl_CMD.exeで通知させることができます。

インストール方法
--------------------------------------------------------------------------------

1) sylnotify.dllをSylpheedのプラグインディレクトリへとコピーします。

2) sylnoitfy.moをロケールディレクトリへとコピーします。

注: Sylpheedをインスーラによりセットアップした場合

%APPDATA%/Sylpheed/plugins以下に配置します。
Windows XPの場合以下の場所にdllをコピーして下さい。
C:\Documents and Settings\(ユーザアカウント)\Application Data\Sylpheed\plugins

使い方
--------------------------------------------------------------------------------

ステータスバーのsylnotifyのアイコンをクリックして有効にします。
[ツール]-[sylnotify]からgrowlnotifyもしくはSnarl_CMD.exeのフルパスを設定します。

注意事項
--------------------------------------------------------------------------------

新着メール一通ごとに通知するため、大量のメールを受信している場合には
向きません。

ライセンス
--------------------------------------------------------------------------------

Sylpheed本体のライセンスに準じてGPLとなります。

TODO
--------------------------------------------------------------------------------

Snarlのサポート(SNP/GNTP/HeySnarl)
Growlのサポート(GNTP)
