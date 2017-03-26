bin_PROGRAMS = obus_daemon
obus_daemon_SOURCES = main.c \
	../common/conf.c \
	../common/obus.c
obus_daemon_CPPFLAGS = $(LGLIB_CFLAGS) $(LZMQ_CFLAGS) $(LJSONC_CFLAGS) -I$(top_srcdir)/common -std=gnu11 -g3
obus_daemon_LDADD = $(LGLIB_LIBS) $(LZMQ_LIBS) $(LJSONC_LIBS)
