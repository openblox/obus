bin_PROGRAMS = obus-cli
obus_cli_SOURCES = main.c \
	../common/conf.c \
	../common/obus.c
obus_cli_CPPFLAGS = -I/usr/include/glib-2.0 -I/usr/lib64/glib-2.0/include \
	-I/usr/lib/x86_64-linux-gnu/glib-2.0/include -I$(top_srcdir)/common \
	-I/usr/include/json-c -std=gnu11 -g3
obus_cli_LDADD = -lglib-2.0 -lzmq -ljson-c
