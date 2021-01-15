AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DG_LOG_DOMAIN=\"lunar-apr\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/lunarx-$(LUNARX_VERSION_API)
extensions_LTLIBRARIES =						\
	lunar-apr.la

lunar_apr_la_SOURCES =							\
	lunar-apr-abstract-page.c					\
	lunar-apr-abstract-page.h					\
	lunar-apr-desktop-page.c					\
	lunar-apr-desktop-page.h					\
	lunar-apr-image-page.c						\
	lunar-apr-image-page.h						\
	lunar-apr-plugin.c						\
	lunar-apr-private.c						\
	lunar-apr-private.h						\
	lunar-apr-provider.c						\
	lunar-apr-provider.h

lunar_apr_la_CFLAGS =							\
	$(EXIF_CFLAGS)							\
	$(ENDO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(PLATFORM_CFLAGS)

lunar_apr_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^lunar_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

lunar_apr_la_LIBADD =							\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la	\
	$(EXIF_LIBS)							\
	$(ENDO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)

lunar_apr_la_DEPENDENCIES =						\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la


EXTRA_DIST =								\
	README.md

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
