AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins				\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins					\
	-DG_LOG_DOMAIN=\"liblunar-tpa\"		\
	$(PLATFORM_CPPFLAGS)

plugindir = $(libdir)/expidus1/panel/plugins
plugin_LTLIBRARIES =							\
	liblunar-tpa.la

liblunar_tpa_la_SOURCES =						\
	lunar-tpa.c								\
	lunar-tpa-bindings.c						\
	lunar-tpa-bindings.h

liblunar_tpa_la_CFLAGS =						\
	$(LIBEXPIDUS1UI_CFLAGS)						\
	$(LIBEXPIDUS1UTIL_CFLAGS)						\
	$(LIBEXPIDUS1PANEL_CFLAGS)						\
	$(PLATFORM_CFLAGS)

liblunar_tpa_la_LIBADD =						\
	$(LIBEXPIDUS1UTIL_LIBS)						\
	$(LIBEXPIDUS1UI_LIBS)							\
	$(LIBEXPIDUS1PANEL_LIBS)

liblunar_tpa_la_LDFLAGS =					\
	-avoid-version							\
	-module									\
	-no-undefined							\
	-export-symbols-regex '^expidus_panel_module_(preinit|init|construct)' \
	$(PLATFORM_LDFLAGS)

desktopdir = $(datadir)/expidus1/panel/plugins
desktop_in_files = lunar-tpa.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

if MAINTAINER_MODE
BUILT_SOURCES =								\
	lunar-tpa-bindings.c					\
	lunar-tpa-bindings.h

DISTCLEANFILES =							\
	lunar-tpa-bindings.c					\
	lunar-tpa-bindings.h

lunar-tpa-bindings.c lunar-tpa-bindings.h: $(srcdir)/lunar-tpa-bindings.xml Makefile
	$(AM_V_GEN) gdbus-codegen --generate-c-code lunar-tpa-bindings --c-namespace lunarTPA --interface-prefix com.expidus. $(srcdir)/lunar-tpa-bindings.xml
endif

EXTRA_DIST =								\
	$(desktop_in_files)						\
	lunar-tpa-bindings.xml					\
	README.md

CLEANFILES =								\
	$(desktop_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
