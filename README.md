SylNotify - 
=======================

this plugin demonstrates how to implement growl notification for Sylpheed

Requirement
--------------------------------------------------------------------------------

* Windows XP SP3/Windows 7 SP1

* Sylpheed 3.1.2 or later

* Growl 2.0.8 or later
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

you can choose Growl+growlnotify or Snarl+Snarl_CMD.exe.

Install
--------------------------------------------------------------------------------

1) copy sylnotify.dll to Sylpheed plugin directory.
2) copy sylnotify.mo to lib/locale/ja/LC_MESSAGES directory. (for Japanese users)
   
Usage
--------------------------------------------------------------------------------

1) click sylnotify statusbar icon to enable plugin.
2) setup sylnotify option [Tool]-[sylnotify]
   you need to modify proper growlnotify or Snarl_CMD file path.

Notice
--------------------------------------------------------------------------------

This plugin is not stable and 
generate notification when you receive new mail.
So, you received 100 mail, then sylnotify generate 100 growl/snarl notification!
you may dislike this behavior.

IMAP4 is not supported.

TODO
--------------------------------------------------------------------------------

may support following feature or not.

* Snarl(SNP/GNTP/HeySnarl)?
* Growl(GNTP)?

