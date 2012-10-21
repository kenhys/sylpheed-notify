
lib_LTLIBRARIES = sylnotify.la

sylpf_srcdir = $(top_srcdir)/../sylplugin_factory/src

sylnotify_la_SOURCES = \
	sylnotify.c \
	$(sylpf_srcdir)/sylpf_utility.c

sylnotify_la_LDFLAGS = -module -no-undefined -avoid-version

INCLUDES = \
	$(GTK_CFLAGS) \
	-I$(includedir) \
	-I$(sylpheed_build_dir) \
	-I$(sylpheed_build_dir)/libsylph \
	-I$(sylpheed_build_dir)/src \
	-I$(sylpf_srcdir)

sylnotify_la_LIBADD = \
	$(GTK_LIBS) \
	$(sylpheed_build_dir)/src/libsylpheed-plugin-0.la \
	$(sylpheed_build_dir)/libsylph/libsylph-0.la

install:
install-plugin: install-am
