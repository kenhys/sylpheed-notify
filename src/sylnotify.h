/*
 * SylNotify -- notifier plug-in for Sylpheed
 * Copyright (C) 2011-2012 HAYASHI Kentaro
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

#ifndef __SYLNOTIFY_H__
#define __SYLNOTIFY_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

#define SYLPF_ID "sylnotify"
#define SYLPF_OPTION sylnotify_option

#define SYLPF_FUNC(arg) sylnotify ## _ ## arg

#define SYLNOTIFY "sylnotify"
#define SYLNOTIFY_SNARL "snarl"
#define SYLNOTIFY_GROWL "growl"
#define SYLNOTIFYRC "sylnotifyrc"

#define _(String) dgettext("sylnotify", String)
#define N_(String) gettext_noop(String)
#define gettext_noop(String) (String)

#define PLUGIN_NAME N_("SylNotify - notifier plug-in for Sylpheed")
#define PLUGIN_DESC N_("notifier plug-in for Sylpheed")

#define SYLSNARL_PORT  5233
#define SYLGNTP_PORT  23053

struct _SylNotifyOption {
  /* full path to ghostbiffrc*/
  gchar *rcpath;
  /* rcfile */
  GKeyFile *rcfile;

  gboolean plugin_enabled;

  GtkWidget *plugin_on;
  GtkWidget *plugin_off;
  GtkWidget *plugin_switch;
  GtkTooltips *plugin_tooltip;

  gboolean startup_flg;

  gboolean snarl_flg;
  gboolean growl_flg;

  gboolean growl_growlnotify_flg;
  gboolean growl_gntp_flg;

  gboolean snarl_snp_flg;
  gboolean snarl_gntp_flg;
  gboolean snarl_heysnarl_flg;
  gboolean snarl_snarlcmd_flg;
  
  gboolean pattern_summary_flg;
  gboolean pattern_all_flg;

  GtkWidget *window;
  GtkWidget *startup;
  GtkWidget *snarl;
  GtkWidget *growl;
  GtkWidget *pattern_summary;
  GtkWidget *pattern_all;

  GtkWidget *snarl_snp;
  GtkWidget *snarl_gntp;
  GtkWidget *snarl_heysnarl;
  GtkWidget *snarl_heysnarl_path;
  GtkWidget *snarl_snarlcmd;
  GtkWidget *snarl_snarlcmd_path;

  GtkWidget *growl_gntp;
  GtkWidget *growl_growlnotify;
  GtkWidget *growl_growlnotify_path;

  GtkWidget *debug;
};

typedef struct _SylNotifyOption SylNotifyOption;

static void init_done_cb(GObject *obj, gpointer data);
static void app_exit_cb(GObject *obj, gpointer data);
static void app_force_exit_cb(GObject *obj, gpointer data);

static gchar *myprocmsg_get_message_file_path(MsgInfo *msginfo);
static void prefs_ok_cb(GtkWidget *widget, gpointer data);

static void exec_sylnotify_cb(GObject *obj, FolderItem *item, const gchar *file, guint num);
static void exec_sylnotify_menu_cb(void);
static void exec_sylnotify_onoff_cb(void);
static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_snarl_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_growl_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey);

static void command_path_clicked(GtkWidget *widget, gpointer data);
static void inc_start_cb(GObject *obj, PrefsAccount *ac);
static void inc_finished_cb(GObject *obj, gint new_messages);

#define GET_RC_BOOLEAN(section, keyarg) g_key_file_get_boolean(g_opt.rcfile, section, keyarg, NULL)
#define SET_RC_BOOLEAN(section, keyarg,valarg) g_key_file_set_boolean(g_opt.rcfile, section, keyarg, valarg)

#define ALIGN_TOP 3
#define ALIGN_BOTTOM 3
#define ALIGN_LEFT 6
#define ALIGN_RIGHT 6
#define BOX_SPACE 6

#define SNARL_ERROR_FAILED 101
#define SNARL_ERROR_UNKNOWN_COMMAND -1 
#define SNARL_ERROR_TIMED_OUT -1
#define SNARL_ERROR_BAD_SOCKET -106
#define SNARL_ERROR_BAD_PACKET -107
#define SNARL_ERROR_INVALID_ARG -108
#define SNARL_ERROR_ARG_MISSING -109
#define SNARL_ERROR_SYSTEM -1
#define SNARL_ERROR_ACCESS_DENIED -121
#define SNARL_ERROR_NOT_RUNNING -201
#define SNARL_ERROR_NOT_REGISTERED -1
#define SNARL_ERROR_ALREADY_REGISTERED -1
#define SNARL_ERROR_CLASS_ALREADY_EXISTS -1
#define SNARL_ERROR_CLASS_BLOCKED -1
#define SNARL_ERROR_CLASS_NOT_FOUND -1
#define SNARL_ERROR_NOTIFICATION_NOT_FOUND -1
#define SNARL_ERROR_FLOODING -1
#define SNARL_ERROR_DO_NOT_DISTURB -1
#define SNARL_ERROR_COULD_NOT_DISPLAY -1
#define SNARL_ERROR_AUTH_FAILURE -1
#define SNARL_ERROR_DISCARDED -1
#define SNARL_ERROR_NOT_SUBSCRIBED -1


#endif /* __SYLNOTIFY_H__ */
