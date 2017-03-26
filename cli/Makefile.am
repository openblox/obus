bin_PROGRAMS = obus-cli
obus_cli_SOURCES = main.c \
	../common/conf.c \
	../common/obus.c
obus_cli_CPPFLAGS = $(LGLIB_CFLAGS) $(LZMQ_CFLAGS) $(LJSONC_CFLAGS) -I$(top_srcdir)/common -std=gnu11 -g3
obus_cli_LDADD = $(LGLIB_LIBS) $(LZMQ_LIBS) $(LJSONC_LIBS)
