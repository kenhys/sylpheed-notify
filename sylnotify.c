/*
 * SylNotify notifier Plug-in
 *  -- generate notification when new mail arrive by using external notifier.
 * Copyright (C) 2011-2012 HAYASHI Kentaro <kenhys@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "defs.h"

#include <glib.h>
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
#include "growl.xpm"
#include "growlx.xpm"
#include "sylnotify.h"

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

static SylPluginInfo info = {
	N_(PLUGIN_NAME),
	"0.1.0",
	"HAYASHI Kentaro",
	N_(PLUGIN_DESC)
};

static gchar* g_copyright = N_("SylNotify is distributed under GPL license.\n"
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

static SylNotifyOption g_opt;

void plugin_load(void)
{
  syl_init_gettext(SYLNOTIFY, "lib/locale");
  
  debug_print(gettext("SylNotify notify support Plug-in"));
  debug_print(dgettext("SylNotify", "Notify support Plug-in"));

  syl_plugin_add_menuitem("/Tools", NULL, NULL, NULL);
  syl_plugin_add_menuitem("/Tools", _("SylNotify Settings [SylNotify]"), exec_sylnotify_menu_cb, NULL);

  g_signal_connect(syl_app_get(), "add-msg", G_CALLBACK(exec_sylnotify_cb), NULL);
  g_signal_connect(syl_app_get(), "init-done", G_CALLBACK(init_done_cb), NULL);
  g_signal_connect(syl_app_get(), "app-exit", G_CALLBACK(app_exit_cb), NULL);
  g_signal_connect(syl_app_get(), "app-force-exit", G_CALLBACK(app_force_exit_cb), NULL);


  GtkWidget *mainwin = syl_plugin_main_window_get();
  GtkWidget *statusbar = syl_plugin_main_window_get_statusbar();
  GtkWidget *plugin_box = gtk_hbox_new(FALSE, 0);

  GdkPixbuf* on_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)growl);
  g_plugin_on=gtk_image_new_from_pixbuf(on_pixbuf);
    
  GdkPixbuf* off_pixbuf = gdk_pixbuf_new_from_xpm_data((const char**)growlx);
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

  g_opt.rcpath = g_strconcat(get_rc_dir(), G_DIR_SEPARATOR_S, SYLNOTIFYRC, NULL);
  g_opt.rcfile = g_key_file_new();

  if (g_key_file_load_from_file(g_opt.rcfile, g_opt.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
    gboolean startup=GET_RC_BOOLEAN(SYLNOTIFY, "startup");
    debug_print("startup:%s", startup ? "true" : "false");

    if (startup){
      g_enable=TRUE;
      gtk_widget_hide(g_plugin_off);
      gtk_widget_show(g_plugin_on);
      gtk_tooltips_set_tip(g_tooltip, g_onoff_switch,
                           _("SylNotify is enabled. Click the icon to disable plugin."),
                           NULL);
    } else {
    }
      
  } else {
      /**/
      g_opt.startup_flg = FALSE;
      g_opt.snarl_flg = FALSE;
      g_opt.growl_flg = TRUE;
  }
}

void plugin_unload(void)
{
  g_free(g_opt.rcpath);
}

SylPluginInfo *plugin_info(void)
{
	return &info;
}

gint plugin_interface_version(void)
{
    /* sylpheed 3.1.0 or later */
    return 0x0107;
}

static void init_done_cb(GObject *obj, gpointer data)
{
  debug_print("[DEBUG init_done_cb");
  gchar *cmdline = g_strdup_printf("\"%s\" \"register?app-sig=app/Sylpheed&title=%s&icon=%s\"",
                                   "C:\\Program Files (x86)\\full phat\\Snarl\\tools\\heysnarl.exe",
                                   "Sylpheed",
                                   "http://sylpheed.sraoss.jp/images/sylpheed.png"
                                   );
  gint ret = execute_command_line(cmdline, FALSE);
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
    g_key_file_load_from_file(g_opt.rcfile, g_opt.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);

    gboolean flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.startup));
    SET_RC_BOOLEAN(SYLNOTIFY, "startup", flg);
    debug_print("startup:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.snarl));
    SET_RC_BOOLEAN(SYLNOTIFY, "snarl", flg);
    debug_print("use snarl:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.growl));
    SET_RC_BOOLEAN(SYLNOTIFY, "growl", flg);
    debug_print("use growl:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.snarl_ntp));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "ntp", flg);
    debug_print("use snarl ntp:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.snarl_gntp));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "gntp", flg);
    debug_print("use snarl gntp:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.snarl_heysnarl));
    SET_RC_BOOLEAN(SYLNOTIFY_SNARL, "heysnarl", flg);
    debug_print("use heysnarl:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.growl_gntp));
    SET_RC_BOOLEAN(SYLNOTIFY_GROWL, "gntp", flg);
    debug_print("use growl gntp:%s\n", flg ? "true" : "false");

    flg = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(g_opt.growl_growlnotify));
    SET_RC_BOOLEAN(SYLNOTIFY_GROWL, "growlnotify", flg);
    debug_print("use growlnotify:%s\n", flg ? "true" : "false");

    gchar *buf = gtk_entry_get_text(GTK_ENTRY(g_opt.growl_growlnotify_path));
    g_key_file_set_string (g_opt.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", buf);
    debug_print("use growlnotify path:%s\n", buf);

    gsize sz;
    buf=g_key_file_to_data(g_opt.rcfile, &sz, NULL);
    g_file_set_contents(g_opt.rcpath, buf, sz, NULL);
    
    gtk_widget_destroy(GTK_WIDGET(data));
}
static void prefs_cancel_cb(GtkWidget *widget, gpointer data)
{
    gtk_widget_destroy(GTK_WIDGET(data));
}
static void prefs_test_cb(GtkWidget *widget, gpointer data)
{
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
    g_opt.growl_growlnotify_flg = TRUE;
    gint ret;
    if (g_opt.growl_growlnotify_flg != FALSE) {
      gchar *cmdline = g_strdup_printf("\"%s\" /a:%s /ai:%s /r:\"%s\" \"%s\"",
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
      gint ret = execute_command_line(cmdline, FALSE);
    }
    if (g_opt.snarl_heysnarl_flg != FALSE) {
      gchar *cmdline = g_strdup_printf("\"%s\" \"register?app-sig=app/Sylpheed&title=%s&icon=%s\"",
                                       "C:\\Program Files (x86)\\full phat\\Snarl\\tools\\heysnarl.exe",
                                       "Sylpheed",
                                       "http://sylpheed.sraoss.jp/images/sylpheed.png"
                                       );
      gint ret = execute_command_line(cmdline, FALSE);
      if (ret >= 0) {
        gchar *cmdline = g_strdup_printf("\"%s\" \"notify?app-sig=app/Sylpheed&title=%s&text=%s\"",
                                         "C:\\Program Files (x86)\\full phat\\Snarl\\tools\\heysnarl.exe",
                                         "subject",
                                         /*"http://sylpheed.sraoss.jp/images/sylpheed.png",*/
                                         "bodynextnext2hoge");
        execute_command_line(cmdline, FALSE);
      }
    }
#endif
}

static void ntp_mail_cb( GtkButton *widget,
                     gpointer   data )
{
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.snarl_gntp), FALSE);
}

static void gntp_mail_cb( GtkButton *widget,
                          gpointer   data )
{
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.snarl_ntp), FALSE);
}

static void exec_sylnotify_menu_cb(void)
{
    /* show modal dialog */
    GtkWidget *window;
    GtkWidget *vbox;
    GtkWidget *confirm_area;
    GtkWidget *ok_btn;
    GtkWidget *cancel_btn;
#if DEBUG
    GtkWidget *test_btn;
#endif
    
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
    GtkWidget *notebook = gtk_notebook_new();
    /* main tab */
    create_config_main_page(notebook, g_opt.rcfile);
    /* Growl option tab */
    create_config_growl_page(notebook, g_opt.rcfile);
    /* Snarl option tab */
    create_config_snarl_page(notebook, g_opt.rcfile);
    /* about, copyright tab */
    create_config_about_page(notebook, g_opt.rcfile);

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
    if (g_key_file_load_from_file(g_opt.rcfile, g_opt.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL)){
      g_opt.startup_flg = GET_RC_BOOLEAN(SYLNOTIFY, "startup");
      debug_print("startup:%s\n", g_opt.startup_flg ? "true" : "false");
      if (g_opt.startup_flg){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.startup), TRUE);
      }

      g_opt.snarl_flg = GET_RC_BOOLEAN(SYLNOTIFY, "snarl");
      debug_print("use snarl:%s\n", g_opt.snarl_flg ? "true" : "false");
      if (g_opt.snarl_flg != FALSE){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl), TRUE);
      }

      g_opt.growl_flg = GET_RC_BOOLEAN(SYLNOTIFY, "growl");
      debug_print("use growl:%s\n", g_opt.growl_flg ? "true" : "false");
      if (g_opt.growl_flg != FALSE){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.growl), TRUE);
      }

      g_opt.snarl_ntp_flg = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "ntp");
      debug_print("use ntp:%s\n", g_opt.snarl_ntp_flg ? "true" : "false");
      if (g_opt.snarl_ntp_flg){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl_ntp), TRUE);
      }

      g_opt.snarl_gntp_flg = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "gntp");
      debug_print("use gntp:%s\n", g_opt.snarl_gntp_flg ? "true" : "false");
      if (g_opt.snarl_gntp_flg){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl_gntp), TRUE);
      }

      g_opt.snarl_heysnarl_flg = GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "heysnarl");
      debug_print("use heysnarl:%s\n", g_opt.snarl_heysnarl_flg ? "true" : "false");
      if (g_opt.snarl_heysnarl_flg != FALSE){
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl_heysnarl), TRUE);
      }

      g_opt.growl_gntp_flg = GET_RC_BOOLEAN(SYLNOTIFY_GROWL, "gntp");
      debug_print("use growl gntp:%s\n", g_opt.growl_gntp_flg ? "true" : "false");
      if (g_opt.growl_gntp_flg != FALSE){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.growl_gntp), TRUE);
      }

      g_opt.growl_growlnotify_flg = GET_RC_BOOLEAN(SYLNOTIFY_GROWL, "growlnotify");
      debug_print("use growlnotify:%s\n", g_opt.growl_growlnotify_flg ? "true" : "false");
      if (g_opt.growl_growlnotify_flg != FALSE){
          gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.growl_growlnotify), TRUE);
      }

      gchar *buf = g_key_file_get_string(g_opt.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", NULL);
      if (g_opt.growl_growlnotify_flg != FALSE && buf != NULL){
          debug_print("use growlnotify path:%s\n", buf);
          gtk_entry_set_text(GTK_ENTRY(g_opt.growl_growlnotify_path), buf);
      }
    }else{
      /* default settings */
      g_opt.startup_flg = FALSE;
    }
 
    gtk_widget_show(window);
}

static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey)
{
  debug_print("create_config_main_page\n");
  if (notebook == NULL){
    return NULL;
  }
  /* startup */
  if (pkey!=NULL){
  }
  GtkWidget *vbox = gtk_vbox_new(FALSE, 0);

  /**/
  GtkWidget *startup_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(startup_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *startup_frm = gtk_frame_new(_("Startup Option"));
  GtkWidget *startup_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(startup_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);


  g_opt.startup = gtk_check_button_new_with_label(_("Enable plugin on startup."));
  gtk_container_add(GTK_CONTAINER(startup_frm_align), g_opt.startup);
  gtk_container_add(GTK_CONTAINER(startup_frm), startup_frm_align);
  gtk_container_add(GTK_CONTAINER(startup_align), startup_frm);

  gtk_widget_show(g_opt.startup);

  /* Application */
  GtkWidget *app_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *app_frm = gtk_frame_new(_("Application"));
  GtkWidget *app_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(app_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  g_opt.growl = gtk_radio_button_new_with_label(NULL, _("Growl"));
  g_opt.snarl = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (g_opt.growl), _("Snarl"));

  GtkWidget *vbox_app = gtk_vbox_new(FALSE, BOX_SPACE);
  gtk_box_pack_start(GTK_BOX(vbox_app), g_opt.growl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_app), g_opt.snarl, FALSE, FALSE, 0);

  gtk_container_add(GTK_CONTAINER(app_frm_align), vbox_app);
  gtk_container_add(GTK_CONTAINER(app_frm), app_frm_align);
  gtk_container_add(GTK_CONTAINER(app_align), app_frm);

  gtk_box_pack_start(GTK_BOX(vbox), startup_align, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), app_align, FALSE, FALSE, 0);

  /* disable snarl */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.growl), TRUE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.snarl), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl), FALSE);

  GtkWidget *general_lbl = gtk_label_new(_("General"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);
  return NULL;

}

static GtkWidget *create_config_snarl_page(GtkWidget *notebook, GKeyFile *pkey)
{
  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  GtkWidget *proto_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *proto_frm = gtk_frame_new(_("Notification Method"));
  GtkWidget *proto_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  /* HeySnarl or NTP or GNTP */
  g_opt.snarl_ntp = gtk_radio_button_new_with_label(NULL, _("NTP (Snarl Network Protocol)"));
  g_opt.snarl_gntp = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (g_opt.snarl_ntp),
                                                                  _("GNTP (Growl Notification Transport Protocol)"));
  g_opt.snarl_heysnarl = gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON (g_opt.snarl_ntp),
                                                         _("HeySnarl (command line tool)"));

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  GtkWidget *lbl = gtk_label_new(_("Command Path:"));
  g_opt.snarl_heysnarl_path = gtk_entry_new();
  GtkWidget *cmd_btn = gtk_button_new_from_stock(GTK_STOCK_OPEN);
  gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), g_opt.snarl_heysnarl_path, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), cmd_btn, FALSE, FALSE, 0);

  GtkWidget *vbox_cond = gtk_vbox_new(FALSE, 0);
    
  /* enable or disable control */
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl_ntp), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.snarl_gntp), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.snarl_heysnarl), TRUE);

  /* currently does not support snarl */
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.snarl_ntp), FALSE);
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.snarl_heysnarl), FALSE);

  gtk_box_pack_start(GTK_BOX(vbox_cond), g_opt.snarl_ntp, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), g_opt.snarl_gntp, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), g_opt.snarl_heysnarl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), hbox, FALSE, FALSE, 0);

  gtk_widget_show_all(vbox_cond);

  g_signal_connect(GTK_BUTTON(g_opt.snarl_ntp), "clicked",
                   G_CALLBACK(ntp_mail_cb), NULL);

  g_signal_connect(GTK_BUTTON(g_opt.snarl_gntp), "clicked",
                   G_CALLBACK(gntp_mail_cb), NULL);

  gtk_container_add(GTK_CONTAINER(proto_frm_align), vbox_cond);
  gtk_container_add(GTK_CONTAINER(proto_frm), proto_frm_align);
  gtk_container_add(GTK_CONTAINER(proto_align), proto_frm);

  gtk_box_pack_start(GTK_BOX(vbox), proto_align, FALSE, FALSE, 0);

  GtkWidget *general_lbl = gtk_label_new(_("Snarl"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);
  return NULL;

}

/**
 *
 */
static GtkWidget *create_config_growl_page(GtkWidget *notebook, GKeyFile *pkey)
{
  GtkWidget *vbox = gtk_vbox_new(FALSE, 6);

  GtkWidget *proto_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);

  GtkWidget *proto_frm = gtk_frame_new(_("Notification Method"));
  GtkWidget *proto_frm_align = gtk_alignment_new(0, 0, 1, 1);
  gtk_alignment_set_padding(GTK_ALIGNMENT(proto_frm_align), ALIGN_TOP, ALIGN_BOTTOM, ALIGN_LEFT, ALIGN_RIGHT);


  /* GNTP or growlnotify */
  g_opt.growl_gntp = gtk_radio_button_new_with_label(NULL, _("GNTP (Growl Notification Transport Protocol)"));
  g_opt.growl_growlnotify = gtk_radio_button_new_with_label_from_widget (GTK_RADIO_BUTTON (g_opt.growl_gntp),
                                                                  _("growlnotify (Command line tool)"));

  /* disable gntp and enable growlnotify */
  gtk_widget_set_sensitive(GTK_WIDGET(g_opt.growl_gntp), FALSE);
  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(g_opt.growl_growlnotify), TRUE);

  GtkWidget *hbox = gtk_hbox_new(FALSE, 0);
  GtkWidget *lbl = gtk_label_new(_("Command Path:"));
  g_opt.growl_growlnotify_path = gtk_entry_new();
  GtkWidget *cmd_btn = gtk_button_new_from_stock(GTK_STOCK_OPEN);
  gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), g_opt.growl_growlnotify_path, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(hbox), cmd_btn, FALSE, FALSE, 0);

  GtkWidget *vbox_cond = gtk_vbox_new(FALSE, 0);
    
  gtk_box_pack_start(GTK_BOX(vbox_cond), g_opt.growl_gntp, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), g_opt.growl_growlnotify, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox_cond), hbox, FALSE, FALSE, 0);

  g_signal_connect(GTK_BUTTON(g_opt.growl_gntp), "clicked",
                   G_CALLBACK(ntp_mail_cb), NULL);

  g_signal_connect(GTK_BUTTON(g_opt.growl_growlnotify), "clicked",
                   G_CALLBACK(gntp_mail_cb), NULL);

  g_signal_connect(GTK_BUTTON(cmd_btn), "clicked",
                   G_CALLBACK(growl_growlnotify_path_clicked), g_opt.growl_growlnotify_path);

  gtk_container_add(GTK_CONTAINER(proto_frm_align), vbox_cond);
  gtk_container_add(GTK_CONTAINER(proto_frm), proto_frm_align);
  gtk_container_add(GTK_CONTAINER(proto_align), proto_frm);

  gtk_box_pack_start(GTK_BOX(vbox), proto_align, FALSE, FALSE, 0);

  GtkWidget *general_lbl = gtk_label_new(_("Growl"));
  gtk_notebook_append_page(GTK_NOTEBOOK(notebook), vbox, general_lbl);
  gtk_widget_show_all(notebook);
  return NULL;
}

/* about, copyright tab */
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey)
{
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
  return NULL;
}

static void exec_sylnotify_onoff_cb(void)
{

    if (g_enable != TRUE){
        syl_plugin_alertpanel_message(_("SylNotify"), _("SylNotify plugin is enabled."), ALERT_NOTICE);
        g_enable=TRUE;
        gtk_widget_hide(g_plugin_off);
        gtk_widget_show(g_plugin_on);
        gtk_tooltips_set_tip
			(g_tooltip, g_onoff_switch,
			 _("SylNotify is enabled. Click the icon to disable plugin."),
			 NULL);
    }else{
        syl_plugin_alertpanel_message(_("SylNotify"), _("SylNotify plugin is disabled."), ALERT_NOTICE);
        g_enable=FALSE;
        gtk_widget_hide(g_plugin_on);
        gtk_widget_show(g_plugin_off);
        gtk_tooltips_set_tip
			(g_tooltip, g_onoff_switch,
			 _("SylNotify is disabled. Click the icon to enable plugin."),
			 NULL);
    }
}

void exec_sylnotify_cb(GObject *obj, FolderItem *item, const gchar *file, guint num)
{
    if (g_enable!=TRUE){
        debug_print("[DEBUG] disabled sylnotify plugin\n");
        return;
    }
    if (item->stype != F_NORMAL && item->stype != F_INBOX){
        debug_print("[DEBUG] not F_NORMAL and F_INBOX\n");
        return;
    }

    PrefsCommon *prefs_common = prefs_common_get();
    if (prefs_common->online_mode != TRUE){
        debug_print("[DEBUG] not online\n");
        return;
    }
    
    PrefsAccount *ac = (PrefsAccount*)account_get_default();
    g_return_if_fail(ac != NULL);

    /* check item->path for filter */
    g_print("%s\n", item->name);
    g_print("%s\n", item->path);

    MsgInfo *msginfo = folder_item_get_msginfo(item, num);
    debug_print("[DEBUG] flags:%08x UNREAD:%08x NEW:%08x MARKED:%08x ", msginfo->flags, MSG_UNREAD, MSG_NEW, MSG_MARKED);
    debug_print("[DEBUG] perm_flags:%08x \n", msginfo->flags.perm_flags);
    debug_print("[DEBUG] tmp_flags:%08x \n", msginfo->flags.tmp_flags);

    g_key_file_load_from_file(g_opt.rcfile, g_opt.rcpath, G_KEY_FILE_KEEP_COMMENTS, NULL);
    gboolean ntp= GET_RC_BOOLEAN(SYLNOTIFY_SNARL, "ntp");

#ifdef DEBUG
    debug_print("[DEBUG] item->path:%s\n", item->path);
#endif

    gint ret = -1;
    if (g_opt.snarl_flg != FALSE) {
      if (g_opt.snarl_ntp_flg != FALSE) {
          debug_print("[DEBUG] snarl ntp mode\n");
        gint sock = fd_connect_inet(SYLSNARL_PORT);
        debug_print("[DEBUG] sock:%d\n", sock);
        if (sock < 0) {
          debug_print("[DEBUG] sock error:%d\n", sock);
          return;
        }
    
        gchar *buf = g_strdup_printf("%s\r\n%s\r\n%s&title=%s&text=Date:%s\\nFrom:%s\\nTo:%s\\nSubject:%s\r\nEND\r\n",
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
      } else if (g_opt.snarl_gntp_flg != FALSE) {
          debug_print("[DEBUG] snarl gntp mode\n");
      } else if (g_opt.snarl_heysnarl_flg != FALSE) {
          debug_print("[DEBUG] snarl heysnarl mode\n");
        /**/
      }
    } else if (g_opt.growl_flg != FALSE) {
        if (g_opt.growl_growlnotify_flg != FALSE) {
            gchar *path = g_key_file_get_string(g_opt.rcfile, SYLNOTIFY_GROWL, "growlnotify_path", NULL);
            if (path != NULL) {
                gchar *cmdline = g_strdup_printf("\"%s\" /a:%s /ai:%s /r:\"%s\" \"%s\"",
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
                                          msginfo->subject
                                          );
                ret = execute_command_line(cmdline, FALSE);
            }
        }
    } else {
      /* nop */
    }
}

static void growl_growlnotify_path_clicked(GtkWidget *widget, gpointer data)
{
  GtkWidget *dialog = gtk_file_chooser_dialog_new(NULL, NULL, GTK_FILE_CHOOSER_ACTION_OPEN,
                                                  GTK_STOCK_OPEN,GTK_RESPONSE_ACCEPT,
                                                  GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
                                                  NULL);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT){
    gchar *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER(dialog));

    gtk_entry_set_text(GTK_ENTRY(g_opt.growl_growlnotify_path), filename);
    g_free (filename);
  }
  gtk_widget_destroy (dialog);
}
