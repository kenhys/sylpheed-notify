/*
 * SylNotify notifier Plug-in
 *  -- generate notification when new mail arrive by using external notifier.
 * Copyright (c) 2012, HAYASHI Kentaro <kenhys@gmail.com> 
 * All rights reserved. 
 *
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met: 
 *
 * 1. Redistributions of source code must retain the above copyright notice, 
 *    this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright notice, 
 *    this list of conditions and the following disclaimer in the documentation 
 *    and/or other materials provided with the distribution. 
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE 
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF 
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. 
 */

#include "defs.h"

#include <glib.h>
#include <glib/gprintf.h>
#include <gtk/gtk.h>

#include <stdio.h>
#include <sys/stat.h>

#include "sylmain.h"
#include "plugin.h"
#include "procmsg.h"
#include "procmime.h"
#include "utils.h"
#include "alertpanel.h"
#include "prefs_common.h"
#include "foldersel.h"
#include "../res/growl.xpm"
#include "../res/growlx.xpm"
#include "sylnotify.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

#include "sylpf_utility.h"

static SylPluginInfo info = {
  N_(PLUGIN_NAME),
  "0.3.0",
  "HAYASHI Kentaro",
  N_(PLUGIN_DESC)
};

static gchar* g_copyright = N_("SylNotify is distributed under 2-clause BSD license.\n"
"\n"
"Copyright (C) 2011-2012 HAYASHI Kentaro <kenhys@gmail.com>"
"\n"
"sylnotify contains following resource.\n"
"\n"
"growl.xpm,growlx.xpm: converted from growl.ico.\n"
"Licensed under New BSD.\n"
"http://code.google.com/p/growl-for-windows/\n"
                               "\n");

static gboolean g_enable = FALSE;


static GtkWidget *g_plugin_on = NULL;
static GtkWidget *g_plugin_off = NULL;
static GtkWidget *g_onoff_switch = NULL;
static GtkTooltips *g_tooltip = NULL;

static SylNotifyOption SYLPF_OPTION;

void plugin_load(void)
{
  GtkWidget *mainwin, *statusbar, *plugin_box;
  GtkWidget *on_pixbuf, *off_pixbuf;
  gchar *pattern;

#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  syl_init_gettext(SYLNOTIFY, "lib/locale");
  
  debug_print(gettext("SylNotify notify support Plug-in"));
  debug_print(dgettext("SylNotify", "Notify support Plug-in"));

  syl_plugin_add_menuitem("/Tools", NULL, NULL, NULL);
  syl_plugin_add_menuitem("/Tools", _("SylNotify Settings [SylNotify]"), exec_sylnotify_menu_cb, NULL);

  g_signal_connect(syl_app_get(), "add-msg", G_CALLBACK(exec_sylnotify_cb), NULL);
  g_signal_connect(syl_app_get(), "init-done", G_CALLBACK(init_done_cb), NULL);
  g_signal_connect(syl_app_get(), "app-exit", G_CALLBACK(app_exit_cb), NULL);
  g_signal_connect(syl_app_get(), "app-force-exit", G_CALLBACK(app_force_exit_cb), NULL);

  syl_plugin_signal_connect("inc-mail-start", G_CALLBACK(inc_start_cb), NULL);
  syl_plugin_signal_connect("inc-mail-finished", G_CALLBACK(inc_finished_cb), NULL);

  mainwin = syl_plugin_main_window_get();
  statusbar = syl_plugin_main_window_get_statusbar();
  plugin_box = gtk_hbox_new(FALSE, 0);

  on_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)growl);
  g_plugin_on=gtk_image_new_from_pixbuf(on_pixbuf);
    
  off_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)growlx);
  g_plugin_off=gtk_image_new_from_pixbuf(off_pixbuf);

  gtk_box_pack_start(GTK_BOX(plugin_box), g_plugin_on, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(plugin_box), g_plugin_off, FALSE, FALSE, 0);
    
  g_tooltip = gtk_tooltips_new();
    
  g_onoff_switch = gtk_button_new();
  gtk_button_set_relief(GTK_BUTTON(g_onoff_switch), GTK_RELIEF_NONE);
  GTK_WIDGET_UNSET_FLAGS(g_onoff_switch, GTK_CAN_FOCUS);
  gtk_widget_set_size_request(g_onoff_switch, 20, 20);

  gtk_container_add(GTK_CONTAINER(g_onoff_switch), plugin_box);
  g_signal_connect(G_OBJECT(g_onoff_switch), "clicked",
                   G_CALLBACK(exec_sylnotify_onoff_cb), mainwin);
  gtk_box_pack_start(GTK_BOX(statusbar), g_onoff_switch, FALSE, FALSE, 0);

  gtk_widget_show_all(g_onoff_switch);
  gtk_widget_hide(g_plugin_on);

  info.name = g_strdup(_(PLUGIN_NAME));
  info.description = g_strdup(_(PLUGIN_DESC));

  SYLPF_OPTION.rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, SYLNOTIFYRC, NULL);
  SYLPF_OPTION.rcfile = g_key_file_new();

  if (g_key_file_load_from_file(SYLPF_OPTION.rcfile, SYLPF_OPTION.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    SYLPF_OPTION.startup_flag = GET_RC_BOOLEAN(SYLNOTIFY, "startup");
    debug_print("startup:%s", SYLPF_OPTION.startup_flag ? "true" : "false");

    if (SYLPF_OPTION.startup_flag != FALSE){
      g_enable=TRUE;
      gtk_widget_hide(g_plugin_off);
      gtk_widget_show(g_plugin_on);
      gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                           _("SylNotify is enabled. Click the icon to disable plugin."),
                           NULL);
    } else {
      g_enable=FALSE;
      gtk_widget_hide(g_plugin_on);
      gtk_widget_show(g_plugin_off);
      gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                           _("SylNotify is disalbed. Click the icon to enable plugin."),
                           NULL);
    }

    SYLPF_OPTION.growl_flag=GET_RC_BOOLEAN(SYLNOTIFY, "growl");
    debug_print("use growl:%s\n", SYLPF_OPTION.growl_flag ? "true" : "false");
    SYLPF_OPTION.snarl_flag=GET_RC_BOOLEAN(SYLNOTIFY, "snarl");
    debug_print("use snarl:%s\n", SYLPF_OPTION.snarl_flag ? "true" : "false");
    
    pattern = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY, "pattern", NULL);
    if (pattern != NULL) {
      if (strcmp(pattern, "summary") == 0) {
        SYLPF_OPTION.pattern_summary_flag = TRUE;
        SYLPF_OPTION.pattern_all_flag = FALSE;
      } else if (strcmp(pattern, "all") == 0) {
        SYLPF_OPTION.pattern_summary_flag = FALSE;
        SYLPF_OPTION.pattern_all_flag = TRUE;
      } else {
        SYLPF_OPTION.pattern_summary_flag = TRUE;
        SYLPF_OPTION.pattern_all_flag = FALSE;
      }
    } else {
      SYLPF_OPTION.pattern_summary_flag = TRUE;
      SYLPF_OPTION.pattern_all_flag = FALSE;
    }
    
    /* Growl */
    SYLPF_OPTION.growl_gntp_flag=GET_RC_BOOLEAN(SYLNOTIFY_GROWL, "gntp");
    debug_print("use gntp:%s\n", SYLPF_OPTION.growl_gntp_flag ? "true" : "false");
    SYLPF_OPTION.growl_growlnotify_flag=GET_RC_BOOLEAN(SYLNOTIFY_GROWL, "growlnotify");
    debug_print("use growlnotify:%s\n", SYLPF_OPTION.growl_growlnotify_flag ? "true" : "false");
    
    /* Snarl */
    SYLPF_OPTION.snarl_snp_flag=GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "snp");
    debug_print("use snp:%s\n", SYLPF_OPTION.snarl_snp_flag ? "true" : "false");
    SYLPF_OPTION.snarl_gntp_flag=GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "gntp");
    debug_print("use gntp:%s\n", SYLPF_OPTION.snarl_gntp_flag ? "true" : "false");
    SYLPF_OPTION.snarl_heysnarl_flag=GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "heysnarl");
    debug_print("use heysnarl:%s\n", SYLPF_OPTION.snarl_heysnarl_flag ? "true" : "false");
    SYLPF_OPTION.snarl_snarlcmd_flag=GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "snarlcmd");
    debug_print("use snarlcmd:%s\n", SYLPF_OPTION.snarl_snarlcmd_flag ? "true" : "false");

  } else {
      /**/
      SYLPF_OPTION.startup_flag = FALSE;

      SYLPF_OPTION.growl_flag = TRUE;
      SYLPF_OPTION.snarl_flag = FALSE;

      SYLPF_OPTION.growl_growlnotify_flag = TRUE;
      SYLPF_OPTION.snarl_snarlcmd_flag = TRUE;
  }
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

void plugin_unload(void)
{
#define SYLPF_FUNC_NAME "plugin_unload"
  SYLPF_END_FUNC;

  g_free(SYLPF_OPTION.rcpath);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

SylPluginInfo *plugin_info(void)
{
  return &info;
}

gint plugin_interface_version(void)
{
  /* sylpheed 3.2 or later since r3005 */
  return 0x0109;
}

static void init_done_cb(GObject *obj, gpointer data)
{
  gchar *cmdline;
  gint ret;

#define SYLPF_FUNC_NAME "plugin_unload"
  SYLPF_END_FUNC;

  debug_print("[DEBUG init_done_cb");
  if (SYLPF_OPTION.snarl_flag != FALSE) {
    if (SYLPF_OPTION.snarl_heysnarl_flag != FALSE) {
      cmdline = g_strdup_printf("\"%s\" \"register?app-sig=app/Sylpheed&title=%s&icon=%s\"",
                                       "C:\\Program Files (x86)\\full phat\\Snarl\\tools\\heysnarl.exe",
                                       "Sylpheed",
                                       "http://sylpheed.sraoss.jp/images/sylpheed.png"
                                       );
      ret = execute_command_line(cmdline, FALSE);
      debug_print("ret:%d", ret);
      switch (ret) {
      case SNARL_ERROR_TIMED_OUT:
        debug_print("[DEBUG] ret:%d timeout\n", ret);
        break;
      case SNARL_ERROR_FAILED:
        debug_print("[DEBUG] ret:%d failed\n", ret);
        break;
      case SNARL_ERROR_NOT_RUNNING:
        debug_print("[DEBUG] ret:%d not running\n", ret);
        break;
      default:
        break;
      }
    }
  }
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}
static void app_exit_cb(GObject *obj, gpointer data)
{
}
static void app_force_exit_cb(GObject *obj, gpointer data)
{
}

/**
 *
 */
static void prefs_ok_cb(GtkWidget *widget, gpointer data)
{
  gboolean flg;
  gchar *buf;
  gsize sz;

#define SYLPF_FUNC_NAME "plugin_unload"
  SYLPF_END_FUNC;
    g_key_file_load_from_file(SYLPF_OPTION.rcfile, SYLPF_OPTION.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.startup));
    SET_RC_BOOLEAN(SYLNOTIFY, "startup", flg);
    debug_print("startup:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl));
    SET_RC_BOOLEAN(SYLNOTIFY, "snarl", flg);
    debug_print("use snarl:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl));
    SET_RC_BOOLEAN(SYLNOTIFY, "growl", flg);
    debug_print("use growl:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.pattern_summary));
    if (flg != FALSE) {
      g_key_file_set_string (SYLPF_OPTION.rcfile, SYLNOTIFY, "pattern", "summary");
      debug_print("use pattern:summary\n");
    }

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.pattern_all));
    if (flg != FALSE) {
      g_key_file_set_string (SYLPF_OPTION.rcfile, SYLNOTIFY, "pattern", "all");
      debug_print("use pattern:all\n");
    }

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_snp));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "snp", flg);
    debug_print("use snarl snp:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_gntp));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "gntp", flg);
    debug_print("use snarl gntp:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_heysnarl));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "heysnarl", flg);
    debug_print("use heysnarl:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_snarlcmd));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "snarlcmd", flg);
    debug_print("use snarlcmd:%s\n", flg ? "true" : "false");

    buf = gtk_entry_get_text(GTK_ENTRY(SYLPF_OPTION.snarl_heysnarl_path));
    g_key_file_set_string (SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "heysnarl_path", buf);
    debug_print("use heysnarl path:%s\n", buf);

    buf = gtk_entry_get_text(GTK_ENTRY(SYLPF_OPTION.snarl_snarlcmd_path));
    g_key_file_set_string (SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "snarlcmd_path", buf);
    debug_print("use snarlcmd path:%s\n", buf);

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl_gntp));
    SET_RC_BOOLEAN(SYLNOTIFY_GROWL, "gntp", flg);
    debug_print("use growl gntp:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl_growlnotify));
    SET_RC_BOOLEAN(SYLNOTIFY_GROWL, "growlnotify", flg);
    debug_print("use growlnotify:%s\n", flg ? "true" : "false");

    buf = gtk_entry_get_text(GTK_ENTRY(SYLPF_OPTION.growl_growlnotify_path));
    g_key_file_set_string (SYLPF_OPTION.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", buf);
    debug_print("use growlnotify path:%s\n", buf);

    buf=g_key_file_to_data(SYLPF_OPTION.rcfile, &sz, NULL);
    g_file_set_contents(SYLPF_OPTION.rcpath, buf, sz, NULL);
    
    gtk_widget_destroy(GTK_WIDGET(data));
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}
static void prefs_cancel_cb(GtkWidget *widget, gpointer data)
{
#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

    gtk_widget_destroy(GTK_WIDGET(data));

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}
static void prefs_test_cb(GtkWidget *widget, gpointer data)
{
  gint ret;
  gchar *cmdline;

#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;
#if 0
    gint sock = fd_connect_inet(SYLSNARL_PORT);
    gchar *buf = conv_unmime_header("SNP/3.0\r\nregister?app-sig=app/Sylpheed&title=Sylpheed\r\nnotify?app-sig=app/Sylpheed&title=Fromあああ:&text=From:\nSubject:\ntext...\r\nEND\r\n", "UTF-8");
    debug_print("buf:%s", buf);
    fd_write_all(sock, buf, strlen(buf));
    fd_close(sock);
#endif
#if 0
    gint sock = fd_connect_inet(SYLGNTP_PORT);
    gchar *buf = "GNTP/1.0 REGISTER NONE\r\nApplication-Name:Sylpheed\r\nApplication-Icon:http://sylpheed.sraoss.jp/images/sylpheed.png\r\nNotifications-Count:1\r\n\r\nApplication-Name:Sylpheed\r\nNotification-Name:Sylpheed\r\nNotification-Title:title\r\nNotification-Text:text\r\n\r\n\r\n";
    debug_print("buf:%s", buf);
    fd_write_all(sock, buf, strlen(buf));
    fd_close(sock);
#endif

#if DEBUG
    SYLPF_OPTION.growl_growlnotify_flag = TRUE;
    if (SYLPF_OPTION.growl_growlnotify_flag != FALSE) {
      cmdline = g_strdup_printf("\"%s\" /a:%s /ai:%s /r:\"%s\" \"%s\"",
                                       "C:\\WinApp\\growlnotify\\growlnotify.exe",
                                       "Sylpheed",
                                       "http://sylpheed.sraoss.jp/images/sylpheed.png",
                                       "New Mail",
                                       "dummy"
                                       );
      ret = execute_command_line(cmdline, FALSE);
      cmdline = g_strdup_printf("\"%s\" /a:%s /n:\"%s\" /t:\"%s\" \"%s\"",
                                "C:\\WinApp\\growlnotify\\growlnotify.exe",
                                "Sylpheed",
                                "New Mail",
                                "件名",
                                "本文"
                                );
      ret = execute_command_line(cmdline, FALSE);
    }
    if (SYLPF_OPTION.snarl_heysnarl_flag != FALSE) {
      cmdline = g_strdup_printf("\"%s\" \"register?app-sig=app/Sylpheed&title=%s&icon=%s\"",
                                       "C:\\Program Files (x86)\\full phat\\Snarl\\tools\\heysnarl.exe",
                                       "Sylpheed",
                                       "http://sylpheed.sraoss.jp/images/sylpheed.png"
                                       );
      ret = execute_command_line(cmdline, FALSE);
      if (ret >= 0) {
        cmdline = g_strdup_printf("\"%s\" \"notify?app-sig=app/Sylpheed&title=%s&text=%s\"",
                                         "C:\\Program Files (x86)\\full phat\\Snarl\\tools\\heysnarl.exe",
                                         "subject",
                                         /*"http://sylpheed.sraoss.jp/images/sylpheed.png",*/
                                         "bodynextnext2hoge");
        execute_command_line(cmdline, FALSE);
      }
    }
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
#endif
}

static void snp_mail_cb( GtkButton *widget,
                     gpointer   data )
{
#define SYLPF_FUNC_NAME "plugin_unload"
  SYLPF_END_FUNC;

  gtk_widget_set_sensitive(GTK_WIDGET(SYLPF_OPTION.snarl_gntp), FALSE);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

static void gntp_mail_cb( GtkButton *widget,
                          gpointer   data )
{
#define SYLPF_FUNC_NAME "plugin_unload"
  SYLPF_END_FUNC;

  gtk_widget_set_sensitive(GTK_WIDGET(SYLPF_OPTION.snarl_snp), FALSE);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

static void exec_sylnotify_menu_cb(void)
{
  /* show modal dialog */
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *confirm_area;
  GtkWidget *ok_btn;
  GtkWidget *cancel_btn;
  GtkWidget *notebook;
  gchar *buf;

#if DEBUG
  GtkWidget *test_btn;
#endif
    
#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_container_set_border_width(GTK_CONTAINER(window), 8);
  gtk_widget_set_size_request(window, 400, 300);
  gtk_window_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_modal(GTK_WINDOW(window), TRUE);
  gtk_window_set_policy(GTK_WINDOW(window), FALSE, TRUE, FALSE);
  gtk_widget_realize(window);

  vbox = gtk_vbox_new(FALSE, 6);
  gtk_widget_show(vbox);
  gtk_container_add(GTK_CONTAINER(window), vbox);

  /* notebook */ 
  notebook = gtk_notebook_new();
  /* main tab */
  create_config_main_page(notebook, SYLPF_OPTION.rcfile);
#ifdef G_OS_WIN32
  /* Growl option tab */
  create_config_growl_page(notebook, SYLPF_OPTION.rcfile);
  /* Snarl option tab */
  create_config_snarl_page(notebook, SYLPF_OPTION.rcfile);
#endif
  /* about, copyright tab */
  create_config_about_page(notebook, SYLPF_OPTION.rcfile);

  gtk_widget_show(notebook);
  gtk_box_pack_start(GTK_BOX(vbox), notebook, TRUE, TRUE, 0);

  confirm_area = gtk_hbutton_box_new();
  gtk_button_box_set_layout(GTK_BUTTON_BOX(confirm_area), GTK_BUTTONBOX_END);
  gtk_box_set_spacing(GTK_BOX(confirm_area), 6);


  ok_btn = gtk_button_new_from_stock(GTK_STOCK_OK);
  GTK_WIDGET_SET_FLAGS(ok_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), ok_btn, FALSE, FALSE, 0);
  gtk_widget_show(ok_btn);

  cancel_btn = gtk_button_new_from_stock(GTK_STOCK_CANCEL);
  GTK_WIDGET_SET_FLAGS(cancel_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), cancel_btn, FALSE, FALSE, 0);
  gtk_widget_show(cancel_btn);

#if DEBUG
  test_btn = gtk_button_new_from_stock(GTK_STOCK_NETWORK);
  GTK_WIDGET_SET_FLAGS(cancel_btn, GTK_CAN_DEFAULT);
  gtk_box_pack_start(GTK_BOX(confirm_area), test_btn, FALSE, FALSE, 0);
  gtk_widget_show(test_btn);
#endif
    
  gtk_widget_show(confirm_area);
	
  gtk_box_pack_end(GTK_BOX(vbox), confirm_area, FALSE, FALSE, 0);
  gtk_widget_grab_default(ok_btn);

  gtk_window_set_title(GTK_WINDOW(window), _("SylNotify Settings [SylNotify]"));

  g_signal_connect(G_OBJECT(ok_btn), "clicked",
                   G_CALLBACK(prefs_ok_cb), window);
  g_signal_connect(G_OBJECT(cancel_btn), "clicked",
                   G_CALLBACK(prefs_cancel_cb), window);
#if DEBUG
  g_signal_connect(G_OBJECT(test_btn), "clicked",
                   G_CALLBACK(prefs_test_cb), window);
#endif
    
  /* load settings */
  if (g_key_file_load_from_file(SYLPF_OPTION.rcfile, SYLPF_OPTION.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    SYLPF_OPTION.startup_flag = GET_RC_BOOLEAN(SYLNOTIFY, "startup");
    debug_print("startup:%s\n", SYLPF_OPTION.startup_flag ? "true" : "false");
    if (SYLPF_OPTION.startup_flag){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.startup), TRUE);
    }

    SYLPF_OPTION.snarl_flag = GET_RC_BOOLEAN(SYLNOTIFY, "snarl");
    debug_print("use snarl:%s\n", SYLPF_OPTION.snarl_flag ? "true" : "false");
    if (SYLPF_OPTION.snarl_flag != FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl), TRUE);
    }

    SYLPF_OPTION.growl_flag = GET_RC_BOOLEAN(SYLNOTIFY, "growl");
    debug_print("use growl:%s\n", SYLPF_OPTION.growl_flag ? "true" : "false");
    if (SYLPF_OPTION.growl_flag != FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl), TRUE);
    }

    SYLPF_OPTION.snarl_snp_flag = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "snp");
    debug_print("use snp:%s\n", SYLPF_OPTION.snarl_snp_flag ? "true" : "false");
    if (SYLPF_OPTION.snarl_snp_flag){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_snp), TRUE);
    }

    SYLPF_OPTION.snarl_gntp_flag = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "gntp");
    debug_print("use gntp:%s\n", SYLPF_OPTION.snarl_gntp_flag ? "true" : "false");
    if (SYLPF_OPTION.snarl_gntp_flag){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_gntp), TRUE);
    }

    SYLPF_OPTION.snarl_heysnarl_flag = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "heysnarl");
    debug_print("use heysnarl:%s\n", SYLPF_OPTION.snarl_heysnarl_flag ? "true" : "false");
    if (SYLPF_OPTION.snarl_heysnarl_flag != FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_heysnarl), TRUE);
    }

    buf = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "heysnarl_path", NULL);
    if (SYLPF_OPTION.snarl_heysnarl_flag != FALSE && buf != NULL){
      debug_print("use heysnarl path:%s\n", buf);
      gtk_entry_set_text(GTK_ENTRY(SYLPF_OPTION.snarl_heysnarl_path), buf);
    }

    SYLPF_OPTION.snarl_snarlcmd_flag = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "snarlcmd");
    debug_print("use snarlcmd:%s\n", SYLPF_OPTION.snarl_snarlcmd_flag ? "true" : "false");
    if (SYLPF_OPTION.snarl_snarlcmd_flag != FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_snarlcmd), TRUE);
    }

    buf = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "snarlcmd_path", NULL);
    if (SYLPF_OPTION.snarl_snarlcmd_flag != FALSE && buf != NULL){
      debug_print("use snarl_cmd path:%s\n", buf);
      gtk_entry_set_text(GTK_ENTRY(SYLPF_OPTION.snarl_snarlcmd_path), buf);
    }

    SYLPF_OPTION.growl_gntp_flag = GET_RC_BOOLEAN(SYLNOTIFY_GROWL, "gntp");
    debug_print("use growl gntp:%s\n", SYLPF_OPTION.growl_gntp_flag ? "true" : "false");
    if (SYLPF_OPTION.growl_gntp_flag != FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl_gntp), TRUE);
    }

    SYLPF_OPTION.growl_growlnotify_flag = GET_RC_BOOLEAN(SYLNOTIFY_GROWL, "growlnotify");
    debug_print("use growlnotify:%s\n", SYLPF_OPTION.growl_growlnotify_flag ? "true" : "false");
    if (SYLPF_OPTION.growl_growlnotify_flag != FALSE){
      gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl_growlnotify), TRUE);
    }

    buf = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", NULL);
    if (SYLPF_OPTION.growl_growlnotify_flag != FALSE && buf != NULL){
      debug_print("use growlnotify path:%s\n", buf);
      gtk_entry_set_text(GTK_ENTRY(SYLPF_OPTION.growl_growlnotify_path), buf);
    }
  }else{
    /* default settings */
    SYLPF_OPTION.startup_flag = FALSE;
  }
 
  gtk_widget_show(window);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey)
{
  GtkWidget *vbox, *startup_align, *startup_frm, *startup_frm_align;

#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  debug_print("create_config_main_page\n");
  if (notebook == NULL){
    return NULL;
  }
  /* startup */
  if (pkey!=NULL){
  }
  vbox = gtk_vbox_new(FALSE, 0);

  /**/
  startup_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(startup_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  startup_frm = gtk_frame_new(_("Startup Option"));
  startup_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(startup_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);


  SYLPF_OPTION.startup = gtk_check_button_new_with_label(_("Enable plugin on startup."));
  gtk_container_add(GTK_CONTAINER(startup_frm_align), SYLPF_OPTION.startup);
  gtk_container_add(GTK_CONTAINER(startup_frm), startup_frm_align);
  gtk_container_add(GTK_CONTAINER(startup_align), startup_frm);

  gtk_widget_show(SYLPF_OPTION.startup);

  /* Application */
  GtkWidget *app_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *app_frm = gtk_frame_new(_("Application"));
  GtkWidget *app_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  SYLPF_OPTION.growl = gtk_radio_button_new_with_label(NULL, _("Growl"));
  SYLPF_OPTION.snarl = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (SYLPF_OPTION.growl), _("Snarl"));

  GtkWidget *vbox_app = gtk_vbox_new(FALSE, BOX_SPACE);
  gtk_box_pack_start(GTK_BOX(vbox_app), SYLPF_OPTION.growl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_app), SYLPF_OPTION.snarl, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(app_frm_align), vbox_app);
  gtk_container_add(GTK_CONTAINER(app_frm), app_frm_align);
  gtk_container_add(GTK_CONTAINER(app_align), app_frm);

  /* disable snarl */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl), SYLPF_OPTION.growl_flag);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl), SYLPF_OPTION.snarl_flag);

  /* Notification */
  GtkWidget *pattern_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(pattern_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);
  GtkWidget *pattern_frm = gtk_frame_new(_("Notification"));
  GtkWidget *pattern_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(pattern_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  SYLPF_OPTION.pattern_summary = gtk_radio_button_new_with_label(NULL, _("Summary only (when receiving mail and finished)"));
  SYLPF_OPTION.pattern_all = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (SYLPF_OPTION.pattern_summary),
                                                                  _("All (1 notification per new mail)"));
  GtkWidget *pattern_lbl = gtk_label_new(_("Note: IMAP4 is not supported yet."));

  GtkWidget *vbox_pattern = gtk_vbox_new(TRUE, BOX_SPACE);
  gtk_box_pack_start(GTK_BOX(vbox_pattern), SYLPF_OPTION.pattern_summary, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_pattern), SYLPF_OPTION.pattern_all, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_pattern), pattern_lbl, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(pattern_frm_align), vbox_pattern);
  gtk_container_add(GTK_CONTAINER(pattern_frm), pattern_frm_align);
  gtk_container_add(GTK_CONTAINER(pattern_align), pattern_frm);

  /**/
  gtk_box_pack_start(GTK_BOX(vbox), startup_align, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), app_align, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), pattern_align, TRUE, TRUE, 0);

  GtkWidget *general_lbl = gtk_label_new(_("General"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME

  return NULL;
}

#ifdef G_OS_WIN32
static GtkWidget *create_config_snarl_page(GtkWidget *notebook, GKeyFile *pkey)
{
#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  GtkWidget *proto_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *proto_frm = gtk_frame_new(_("Notification Method"));
  GtkWidget *proto_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  /* HeySnarl or NTP or GNTP or HeySnarl or */
  SYLPF_OPTION.snarl_snp = gtk_radio_button_new_with_label(NULL, _("SNP (Snarl Network Protocol)"));
  SYLPF_OPTION.snarl_gntp = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (SYLPF_OPTION.snarl_snp),
                                                                  _("GNTP (Growl Notification Transport Protocol)"));
  SYLPF_OPTION.snarl_heysnarl = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (SYLPF_OPTION.snarl_snp),
                                                         _("HeySnarl (Command line tool)"));
  SYLPF_OPTION.snarl_snarlcmd = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (SYLPF_OPTION.snarl_snp),
                                                         _("Snarl_CMD (Command line tool)"));

  /* HeySnarl command */
  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  GtkWidget *lbl = gtk_label_new(_("Command Path:"));
  SYLPF_OPTION.snarl_heysnarl_path = gtk_entry_new();
  GtkWidget *cmd_btn = gtk_button_new_from_stock(GTK_STOCK_OPEN);
  gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), SYLPF_OPTION.snarl_heysnarl_path, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), cmd_btn, FALSE, FALSE, 0);

  g_signal_connect(GTK_BUTTON(cmd_btn), "clicked",
                   G_CALLBACK(command_path_clicked), SYLPF_OPTION.snarl_heysnarl_path);

  /* Snarl_CMD.exe */
  GtkWidget *hbox2 = gtk_hbox_new(FALSE, 0);
  lbl = gtk_label_new(_("Command Path:"));
  SYLPF_OPTION.snarl_snarlcmd_path = gtk_entry_new();
  cmd_btn = gtk_button_new_from_stock(GTK_STOCK_OPEN);
  gtk_box_pack_start(GTK_BOX(hbox2), lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), SYLPF_OPTION.snarl_snarlcmd_path, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox2), cmd_btn, FALSE, FALSE, 0);

  g_signal_connect(GTK_BUTTON(cmd_btn), "clicked",
                   G_CALLBACK(command_path_clicked), SYLPF_OPTION.snarl_snarlcmd_path);

  GtkWidget *vbox_cond = gtk_vbox_new(FALSE, 0);
    
  /* enable or disable control */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_snp), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_gntp), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_heysnarl), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.snarl_snarlcmd), TRUE);

  /* currently does not support snarl */
  gtk_widget_set_sensitive(GTK_WIDGET(SYLPF_OPTION.snarl_snp), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(SYLPF_OPTION.snarl_gntp), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(SYLPF_OPTION.snarl_heysnarl), FALSE);

  gtk_box_pack_start(GTK_BOX(vbox_cond), SYLPF_OPTION.snarl_snp, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), SYLPF_OPTION.snarl_gntp, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), SYLPF_OPTION.snarl_heysnarl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), SYLPF_OPTION.snarl_snarlcmd, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), hbox2, FALSE, FALSE, 0);

  gtk_widget_show_all(vbox_cond);

  g_signal_connect(GTK_BUTTON(SYLPF_OPTION.snarl_snp), "clicked",
                   G_CALLBACK(snp_mail_cb), NULL);

  g_signal_connect(GTK_BUTTON(SYLPF_OPTION.snarl_gntp), "clicked",
                   G_CALLBACK(gntp_mail_cb), NULL);


  gtk_container_add(GTK_CONTAINER(proto_frm_align), vbox_cond);
  gtk_container_add(GTK_CONTAINER(proto_frm), proto_frm_align);
  gtk_container_add(GTK_CONTAINER(proto_align), proto_frm);

  gtk_box_pack_start(GTK_BOX(vbox), proto_align, FALSE, FALSE, 0);

  GtkWidget *general_lbl = gtk_label_new(_("Snarl"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME

  return NULL;
}

/**
 *
 */
static GtkWidget *create_config_growl_page(GtkWidget *notebook, GKeyFile *pkey)
{
#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  GtkWidget *proto_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *proto_frm = gtk_frame_new(_("Notification Method"));
  GtkWidget *proto_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);


  /* GNTP or growlnotify */
  SYLPF_OPTION.growl_gntp = gtk_radio_button_new_with_label(NULL, _("GNTP (Growl Notification Transport Protocol)"));
  SYLPF_OPTION.growl_growlnotify = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (SYLPF_OPTION.growl_gntp),
                                                                  _("growlnotify (Command line tool)"));

  /* disable gntp and enable growlnotify */
  gtk_widget_set_sensitive(GTK_WIDGET(SYLPF_OPTION.growl_gntp), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(SYLPF_OPTION.growl_growlnotify), TRUE);

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  GtkWidget *lbl = gtk_label_new(_("Command Path:"));
  SYLPF_OPTION.growl_growlnotify_path = gtk_entry_new();
  GtkWidget *cmd_btn = gtk_button_new_from_stock(GTK_STOCK_OPEN);
  gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), SYLPF_OPTION.growl_growlnotify_path, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), cmd_btn, FALSE, FALSE, 0);

  GtkWidget *vbox_cond = gtk_vbox_new(FALSE, 0);
    
  gtk_box_pack_start(GTK_BOX(vbox_cond), SYLPF_OPTION.growl_gntp, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), SYLPF_OPTION.growl_growlnotify, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), hbox, FALSE, FALSE, 0);

  g_signal_connect(GTK_BUTTON(SYLPF_OPTION.growl_gntp), "clicked",
                   G_CALLBACK(snp_mail_cb), NULL);

  g_signal_connect(GTK_BUTTON(SYLPF_OPTION.growl_growlnotify), "clicked",
                   G_CALLBACK(gntp_mail_cb), NULL);

  g_signal_connect(GTK_BUTTON(cmd_btn), "clicked",
                   G_CALLBACK(command_path_clicked), SYLPF_OPTION.growl_growlnotify_path);

  gtk_container_add(GTK_CONTAINER(proto_frm_align), vbox_cond);
  gtk_container_add(GTK_CONTAINER(proto_frm), proto_frm_align);
  gtk_container_add(GTK_CONTAINER(proto_align), proto_frm);

  gtk_box_pack_start(GTK_BOX(vbox), proto_align, FALSE, FALSE, 0);

  GtkWidget *general_lbl = gtk_label_new(_("Growl"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME

  return NULL;
}
#endif

/* about, copyright tab */
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey)
{
#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  debug_print("create_config_about_page\n");
  if (notebook == NULL){
    return NULL;
  }
  GtkWidget *hbox = gtk_hbox_new(TRUE, 6);
  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  GtkWidget *lbl = gtk_label_new(_("SylNotify"));
  GtkWidget *desc = gtk_label_new(PLUGIN_DESC);

  /* copyright */
  GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);

  gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, TRUE, 6);
  gtk_box_pack_start(GTK_BOX(vbox), desc, FALSE, TRUE, 6);
  gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 6);
  gtk_box_pack_start(GTK_BOX(hbox), vbox, TRUE, TRUE, 6);

  GtkTextBuffer *tbuffer = gtk_text_buffer_new(NULL);
  gtk_text_buffer_set_text(tbuffer, _(g_copyright), strlen(g_copyright));
  GtkWidget *tview = gtk_text_view_new_with_buffer(tbuffer);
  gtk_text_view_set_editable(GTK_TEXT_VIEW(tview), FALSE);
  gtk_container_add(GTK_CONTAINER(scrolled), tview);
    
  gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 6);
    
  /**/
  GtkWidget *general_lbl = gtk_label_new(_("About"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), hbox, general_lbl);
  gtk_widget_show_all(notebook);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME

  return NULL;
}

static void exec_sylnotify_onoff_cb(void)
{
#define SYLPF_FUNC_NAME "plugin_unload"
  SYLPF_END_FUNC;

  if (g_enable != TRUE) {
    syl_plugin_alertpanel_message(_("SylNotify"),
                                  _("SylNotify plugin is enabled."),
                                  ALERT_NOTICE);
    g_enable=TRUE;
    gtk_widget_hide(g_plugin_off);
    gtk_widget_show(g_plugin_on);
    gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                         _("SylNotify is enabled. Click the icon to disable plugin."),
                         NULL);
  } else {
    syl_plugin_alertpanel_message(_("SylNotify"),
                                  _("SylNotify plugin is disabled."),
                                  ALERT_NOTICE);
    g_enable=FALSE;
    gtk_widget_hide(g_plugin_on);
    gtk_widget_show(g_plugin_off);
    gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                         _("SylNotify is disabled. Click the icon to enable plugin."),
                         NULL);
  }

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

void exec_sylnotify_cb(GObject *obj, FolderItem *item, const gchar *file, guint num)
{
#define SYLPF_FUNC_NAME "plugin_load"
  
  PrefsCommon *prefs_common;
  PrefsAccount *ac;
  MsgInfo *msginfo;
  gint ret, sock;
  gchar *buf, *path, *cmdline;

  SYLPF_START_FUNC;

  if (g_enable != TRUE) {
    debug_print("[DEBUG] disabled sylnotify plugin\n");
    return;
  }
  if (item->stype != F_NORMAL && item->stype != F_INBOX) {
    debug_print("[DEBUG] not F_NORMAL and F_INBOX %d\n", item->stype);
    if (item->folder) {
      if (item->folder->klass) {
        debug_print("[DEBUG] item->name:%s FolderType:%d %d\n", item->name, item->folder->klass->type);
      }
    }
    return;
  }

  prefs_common = prefs_common_get();
  if (prefs_common->online_mode != TRUE) {
    debug_print("[DEBUG] not online\n");
    return;
  }

  if (SYLPF_OPTION.pattern_all_flag != TRUE ||
      SYLPF_OPTION.pattern_summary_flag != FALSE) {
    debug_print("[DEBUG] notify summary only.\n");
    return;
  }
    
  ac = (PrefsAccount*)account_get_default();
  g_return_if_fail(ac != NULL);

  /* check item->path for filter */
  g_print("%s\n", item->name);
  g_print("%s\n", item->path);

  msginfo = folder_item_get_msginfo(item, num);
  debug_print("[DEBUG] flags:%08x UNREAD:%08x NEW:%08x MARKED:%08x ",
              msginfo->flags, MSG_UNREAD, MSG_NEW, MSG_MARKED);
  debug_print("[DEBUG] perm_flags:%08x \n", msginfo->flags.perm_flags);
  debug_print("[DEBUG] tmp_flags:%08x \n", msginfo->flags.tmp_flags);

  g_key_file_load_from_file(SYLPF_OPTION.rcfile,
                            SYLPF_OPTION.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);

#ifdef DEBUG
  debug_print("[DEBUG] item->path:%s\n", item->path);
#endif

  ret = -1;
  if (SYLPF_OPTION.snarl_flag != FALSE) {
    if (SYLPF_OPTION.snarl_snp_flag != FALSE) {
      debug_print("[DEBUG] snarl snp mode\n");
      sock = fd_connect_inet(SYLSNARL_PORT);
      debug_print("[DEBUG] sock:%d\n", sock);
      if (sock < 0) {
        debug_print("[DEBUG] sock error:%d\n", sock);
        return;
      }
    
      buf = g_strdup_printf("%s\r\n%s\r\n%s&title=%s&text=Date:%s\\nFrom:%s\\nTo:%s\\nSubject:%s\r\nEND\r\n",
                                   "SNP/3.0",
                                   "register?app-sig=app/Sylpheed&title=Sylpheed",
                                   "notify?app-sig=app/Sylpheed",
                                   conv_unmime_header(msginfo->subject, "UTF-8"),
                                   msginfo->date,
                                   msginfo->from,
                                   msginfo->to,
                                   conv_unmime_header(msginfo->subject, "UTF-8"));
      debug_print("[DEBUG] msg:%s\n", buf);
      fd_write_all(sock, buf, strlen(buf));
      g_free(buf);
      fd_close(sock);
    } else if (SYLPF_OPTION.snarl_gntp_flag != FALSE) {
      debug_print("[DEBUG] snarl gntp mode\n");
    } else if (SYLPF_OPTION.snarl_heysnarl_flag != FALSE) {
      debug_print("[DEBUG] snarl heysnarl mode\n");
      /**/
    } else if (SYLPF_OPTION.snarl_snarlcmd_flag != FALSE) {
      debug_print("[DEBUG] snarl snarlcmd mode\n");
      path = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "snarlcmd_path", NULL);
      if (path != NULL) {
        cmdline = g_strdup_printf("\"%s\" snShowMessage %d \"%s\" \"%s\" \"%s\"",
                                         path,
                                         5,
                                         msginfo->from,
                                         unmime_header(msginfo->subject),
                                         "http://sylpheed.sraoss.jp/images/sylpheed.png");
        ret = execute_command_line(cmdline, FALSE);
      }
    }
  } else if (SYLPF_OPTION.growl_flag != FALSE) {
    if (SYLPF_OPTION.growl_growlnotify_flag != FALSE) {
      path = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", NULL);
      if (path != NULL) {
        cmdline = g_strdup_printf("\"%s\" /a:%s /ai:%s /r:\"%s\" \"%s\"",
                                         path,
                                         "Sylpheed",
                                         "http://sylpheed.sraoss.jp/images/sylpheed.png",
                                         "New Mail",
                                         "dummy"
                                         );
        ret = execute_command_line(cmdline, FALSE);
        cmdline = g_strdup_printf("\"%s\" /a:%s /n:\"%s\" /t:\"%s\" \"%s\"",
                                  path,
                                  "Sylpheed",
                                  "New Mail",
                                  msginfo->from,
                                  msginfo->subject);
        ret = execute_command_line(cmdline, FALSE);
      }
    }
  } else {
    /* nop */
  }
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

static void command_path_clicked(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog;
  gchar *filename;

#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

  dialog = gtk_file_chooser_dialog_new(NULL, NULL,
                                       GTK_FILE_CHOOSER_ACTION_OPEN,
                                       GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,
                                       GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                       NULL);
  if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
    filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
    if (filename) {
      gtk_entry_set_text(GTK_ENTRY(data), filename);
      g_free(filename);
    }
  }
  gtk_widget_destroy(dialog);

  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

static void inc_start_cb(GObject *obj, PrefsAccount *ac)
{
#define SYLPF_FUNC_NAME "plugin_load"
  SYLPF_START_FUNC;

#if 1
  if (ac)
    g_print("test: receive start: account: %s\n", ac->account_name);
  else
    g_print("test: receive start: all accounts\n");
#else
  gint ret;
  if (SYLPF_OPTION.snarl_flag != FALSE) {
    if (SYLPF_OPTION.snarl_snarlcmd_flag != FALSE) {
      debug_print("[DEBUG] snarl snarlcmd mode\n");
      gchar *path = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "snarlcmd_path", NULL);
      if (path != NULL) {
        gchar *cmdline = g_strdup_printf("\"%s\" snShowMessage %d \"%s\" \"%s\" \"%s\"",
                                         path,
                                         5,
                                         ac ? ac->account_name : "All",
                                         _("receive start"),
                                         "http://sylpheed.sraoss.jp/images/sylpheed.png");
        ret = execute_command_line(cmdline, FALSE);
      }
    } 
  } else if (SYLPF_OPTION.growl_flag != FALSE) {
    if (SYLPF_OPTION.growl_growlnotify_flag != FALSE) {
      gchar *path = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", NULL);
      if (path != NULL) {
        gchar *cmdline = g_strdup_printf("\"%s\" /a:%s /ai:%s /r:\"%s\" \"%s\"",
                                         path,
                                         "Sylpheed",
                                         "http://sylpheed.sraoss.jp/images/sylpheed.png",
                                         "New Mail",
                                         "dummy"
                                         );
        ret = execute_command_line(cmdline, FALSE);
        if (ret >= 0) {
          cmdline = g_strdup_printf("\"%s\" /a:%s /n:\"%s\" /t:\"%s\" \"%s\"",
                                    path,
                                    "Sylpheed",
                                    "New Mail",
                                    ac ? ac->account_name : "All",
                                    _("receive start")
                                    );
          ret = execute_command_line(cmdline, FALSE);
        }
      }
    }
  }
#endif
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
}

static void inc_finished_cb(GObject *obj, gint new_messages)
{
#define SYLPF_FUNC_NAME "inc_finished_cb"
  SYLPF_START_FUNC;

  g_print("test: received %d new messages\n", new_messages);
#if 0
  gint ret = 0;
  if (SYLPF_OPTION.snarl_flag != FALSE) {
    if (SYLPF_OPTION.snarl_snarlcmd_flag != FALSE) {
      debug_print("[DEBUG] snarl snarlcmd mode\n");
      gchar *path = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_SNARL, "snarlcmd_path", NULL);
      if (path != NULL) {
        gchar *cmdline = g_strdup_printf("\"%s\" snShowMessage %d \"%s\" \"%s\" \"%s\"",
                                         path,
                                         5,
                                         _("receive finished"),
                                         g_sprintf("%d new messages", new_messages),
                                         "http://sylpheed.sraoss.jp/images/sylpheed.png");
        ret = execute_command_line(cmdline, FALSE);
        g_free(cmdline);
      }
    } 
  } else if (SYLPF_OPTION.growl_flag != FALSE) {
    if (SYLPF_OPTION.growl_growlnotify_flag != FALSE) {
      gchar *path = g_key_file_get_string(SYLPF_OPTION.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", NULL);
      if (path != NULL) {
        gchar *cmdline = g_strdup_printf("\"%s\" /a:%s /ai:%s /r:\"%s\" \"%s\"",
                                         path,
                                         "Sylpheed",
                                         "http://sylpheed.sraoss.jp/images/sylpheed.png",
                                         "New Mail",
                                         "dummy"
                                         );
        ret = execute_command_line(cmdline, FALSE);
        g_free(cmdline);
        if (ret >= 0) {
          cmdline = g_strdup_printf("\"%s\" /a:%s /n:\"%s\" /t:\"%s\" \"%s\"",
                                    path,
                                    "Sylpheed",
                                    "New Mail",
                                    _("receive finished"),
                                    g_sprintf("%d new messages", new_messages)
                                    );
          ret = execute_command_line(cmdline, FALSE);
          g_free(cmdline);
        }
      }
    }
  }
  SYLPF_END_FUNC;
#undef SYLPF_FUNC_NAME
#endif
}
