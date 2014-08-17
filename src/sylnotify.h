/*
 * SylNotify -- notifier plug-in for Sylpheed
 *
 * Copyright (c) 2012-2013, HAYASHI Kentaro <kenhys@gmail.com> 
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

#ifndef __SYLNOTIFY_H__
#define __SYLNOTIFY_H__

#include <glib.h>
#include <glib/gi18n-lib.h>
#include <locale.h>

#define SYLPF_FUNC(arg) sylnotify ## _ ## arg

#define SYLNOTIFY "sylnotify"
#define SYLNOTIFY_SNARL "snarl"
#define SYLNOTIFY_GROWL "growl"
#define SYLNOTIFYRC "sylnotifyrc"

#define SYLSNARL_PORT  5233
#define SYLGNTP_PORT  23053

typedef _SylSnarlOption {
  gboolean active;
  gboolean use_snp;
  gboolean use_gntp;
  gboolean use_heysnarl;
  gboolean use_snarlcmd;

  GtkWidget *snp;
  GtkWidget *gntp;
  GtkWidget *heysnarl;
  GtkWidget *heysnarl_path;
  GtkWidget *snarlcmd;
  GtkWidget *snarlcmd_path;
} SylSnarl;

typedef _SylGrowlOption {
  gboolean active;
  gboolean use_growlnotify;
  gboolean use_gntp;

  GtkWidget *gntp;
  GtkWidget *growlnotify;
  GtkWidget *growlnotify_path;
} SylGrowl;

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

  gboolean startup_flag;
  gboolean use_alertpanel;

  SylSnarlOption snarl;
  SylGrowlOption growl;

  gboolean snarl_flag;
  gboolean growl_flag;

  gboolean pattern_summary_flag;
  gboolean pattern_all_flag;

  GtkWidget *window;
  GtkWidget *startup;
  GtkWidget *snarl;
  GtkWidget *growl;
  GtkWidget *pattern_summary;
  GtkWidget *pattern_all;

  GtkWidget *debug;
};

typedef struct _SylNotifyOption SylNotifyOption;

enum SylNotifyAppType {
  SYLNOTIFY_APP_NONE,
  SYLNOTIFY_APP_GFW, /* Growl for Windows */
  SYLNOTIFY_APP_SNARL,
  SYLNOTIFY_APP_GFL, /* Growl For Linux */
  SYLNOTIFY_APP_LIBNOTIFY,
};

typedef struct _SylNotifyAppEntry {
  enum SylNotifyAppType app_type;
  const gchar *app_desc;
} SylNotifyAppEntry;

static void init_done_cb(GObject *obj, gpointer data);
static void app_exit_cb(GObject *obj, gpointer data);
static void app_force_exit_cb(GObject *obj, gpointer data);

static void prefs_ok_cb(GtkWidget *widget, gpointer data);

static void exec_sylnotify_cb(GObject *obj, FolderItem *item, const gchar *file, guint num);
static void exec_sylnotify_menu_cb(void);
static void exec_sylnotify_onoff_cb(void);
static GtkWidget *create_config_main_page(GtkWidget *notebook, GKeyFile *pkey);
static void create_config_gol_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_about_page(GtkWidget *notebook, GKeyFile *pkey);

static void inc_start_cb(GObject *obj, PrefsAccount *ac);
static void inc_finished_cb(GObject *obj, gint new_messages);

#ifdef G_OS_WIN32
static void command_path_clicked(GtkWidget *widget, gpointer data);
static GtkWidget *create_config_snarl_page(GtkWidget *notebook, GKeyFile *pkey);
static GtkWidget *create_config_growl_page(GtkWidget *notebook, GKeyFile *pkey);
static gint send_notification_by_growlnotify(GKeyFile *rcfile,
                                             gchar *from,
                                             gchar *subject);
static gint send_notification_by_snarl(GKeyFile *rcfile,
                                       const gchar *title,
                                       const gchar *message);
static gint send_notification_by_snarl_snp(GKeyFile *rcfile,
                                           const MsgInfo *msginfo);
#endif

static void create_app_indicator(SylNotifyOption *option);

#define GET_RC_BOOLEAN(section, keyarg) g_key_file_get_boolean(SYLPF_OPTION.rcfile, section, keyarg, NULL)
#define SET_RC_BOOLEAN(section, keyarg,valarg) g_key_file_set_boolean(SYLPF_OPTION.rcfile, section, keyarg, valarg)

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
