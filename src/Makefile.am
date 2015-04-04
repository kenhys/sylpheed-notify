
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
	-I$(sylpheed_build_dir) \
	-I$(sylpheed_build_dir)/libsylph \
	-I$(sylpheed_build_dir)/src \
	$(SYLPF_CFLAGS)

sylnotify_la_LIBADD = \
	$(GTK_LIBS) \
	$(LIBNOTIFY_LIBS) \
	$(LIBAPPINDICATOR_LIBS) \
	$(sylpheed_build_dir)/src/libsylpheed-plugin-0.la \
	$(sylpheed_build_dir)/libsylph/libsylph-0.la \
	$(SYLPF_LIBS)

