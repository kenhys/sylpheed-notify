libdir = $(plugindir)

lib_LTLIBRARIES = sylnotify.la

sylpf_srcdir = $(top_srcdir)/plugin/sylplugin_factory/src

sylnotify_la_SOURCES = sylnotify.c sylnotify.h
sylnotify_la_LDFLAGS = -module -no-undefined -avoid-version

INCLUDES = \
	$(GTK_CFLAGS) \
	-I$(includedir) \
	-I$(top_srcdir) \
	-I$(top_srcdir)/libsylph \
	-I$(top_srcdir)/src \
	-I$(sylpf_srcdir)

sylsnarl_la_LIBADD = \
	$(GTK_LIBS) \
	$(top_builddir)/src/libsylpheed-plugin-0.la \
	$(top_builddir)/libsylph/libsylph-0.la

install:
install-plugin: install-am
