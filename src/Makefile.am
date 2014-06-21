
AUTOMAKE_OPTIONS = subdir-objects

lib_LTLIBRARIES = sylnotify.la

sylnotify_la_SOURCES = \
	sylnotify.c \
	$(sylplugin_factory_source_dir)/src/sylplugin_factory.c

APPINDICATOR_CFLAGS = `pkg-config --cflags appindicator-0.1`
APPINDICATOR_LDFLAGS = `pkg-config --libs appindicator-0.1`

sylnotify_la_LDFLAGS = -module -no-undefined -avoid-version \
	$(APPINDICATOR_LDFLAGS)

INCLUDES = \
	$(GTK_CFLAGS) \
	$(APPINDICATOR_CFLAGS) \
	-I$(includedir) \
	-I$(sylpheed_build_dir) \
	-I$(sylpheed_build_dir)/libsylph \
	-I$(sylpheed_build_dir)/src \
	-I$(sylplugin_factory_source_dir)/src

sylnotify_la_LIBADD = \
	$(GTK_LIBS) \
	$(sylpheed_build_dir)/src/libsylpheed-plugin-0.la \
	$(sylpheed_build_dir)/libsylph/libsylph-0.la

install:
install-plugin: install-am
