
libdir = $(sylplugin_dir)
lib_LTLIBRARIES = sylnotify.la

sylnotify_la_SOURCES = \
	sylnotify.c

sylnotify_la_LDFLAGS = -module -no-undefined -avoid-version \
	$(APPINDICATOR_LDFLAGS)

AM_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(LIBAPPINDICATOR_CFLAGS) \
	-I$(includedir) \
	$(SYLPHEED_CFLAGS) \
	$(SYLPF_CFLAGS)

sylnotify_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(LIBAPPINDICATOR_LIBS) \
	$(SYLPHEED_LIBS)
	$(SYLPF_LIBS)

