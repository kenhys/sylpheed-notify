
bin_PROGRAMS = test-notification

if WITH_LIBNOTIFY
bin_PROGRAMS += test-libnotify
endif
if WITH_LIBAPPINDICATOR
bin_PROGRAMS += test-libappindicator
endif

test_libnotify_SOURCES = test-libnotify.c

test_libappindicator_SOURCES = test-libappindicator.c

AM_CPPFLAGS = \
	$(GTK_CFLAGS) \
	$(LIBNOTIFY_CFLAGS) \
	$(LIBAPPINDICATOR_CFLAGS) \
	-I$(includedir) \
	-I$(sylpheed_build_dir) \
	-I$(sylpheed_build_dir)/libsylph \
	-I$(sylpheed_build_dir)/src \
	$(SYLPF_CFLAGS)

test_libnotify_LDADD = \
	$(GTK_LIBS) \
	$(LIBNOTIFY_LIBS)

test_libappindicator_LDADD = \
	$(GTK_LIBS) \
	$(LIBAPPINDICATOR_LIBS)

