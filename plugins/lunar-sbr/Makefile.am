AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DG_LOG_DOMAIN=\"lunar-sbr\"					\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/lunarx-$(LUNARX_VERSION_API)
extensions_LTLIBRARIES =						\
	lunar-sbr.la

lunar_sbr_la_SOURCES =							\
	lunar-sbr-case-renamer.c					\
	lunar-sbr-case-renamer.h					\
	lunar-sbr-date-renamer.c					\
	lunar-sbr-date-renamer.h					\
	lunar-sbr-enum-types.c						\
	lunar-sbr-enum-types.h						\
	lunar-sbr-insert-renamer.c					\
	lunar-sbr-insert-renamer.h					\
	lunar-sbr-number-renamer.c					\
	lunar-sbr-number-renamer.h					\
	lunar-sbr-plugin.c						\
	lunar-sbr-provider.c						\
	lunar-sbr-provider.h						\
	lunar-sbr-remove-renamer.c					\
	lunar-sbr-remove-renamer.h					\
	lunar-sbr-replace-renamer.c					\
	lunar-sbr-replace-renamer.h

lunar_sbr_la_CFLAGS =							\
	$(EXIF_CFLAGS)							\
	$(ENDO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(PCRE_CFLAGS)							\
	$(PLATFORM_CFLAGS)

lunar_sbr_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^lunar_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

lunar_sbr_la_LIBADD =							\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la	\
	$(EXIF_LIBS)							\
	$(ENDO_LIBS)							\
	$(GLIB_LIBS)							\
	$(PCRE_LIBS)

lunar_sbr_la_DEPENDENCIES =						\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la

EXTRA_DIST =								\
	README.md

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
