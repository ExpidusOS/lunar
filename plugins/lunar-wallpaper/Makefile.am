AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DG_LOG_DOMAIN=\"lunar-wallpaper-plugin\"			\
	-DENDO_API_SUBJECT_TO_CHANGE					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/lunarx-$(LUNARX_VERSION_API)
extensions_LTLIBRARIES =						\
	lunar-wallpaper-plugin.la

lunar_wallpaper_plugin_la_SOURCES =					\
	twp-provider.h							\
	twp-provider.c							\
	lunar-wallpaper-plugin.c

lunar_wallpaper_plugin_la_CFLAGS =					\
	$(ENDO_CFLAGS)							\
	$(GLIB_CFLAGS)							\
	$(GTK_CFLAGS)							\
	$(LIBX11_CFLAGS)						\
	$(ESCONF_CFLAGS)						\
	$(PLATFORM_CFLAGS)

lunar_wallpaper_plugin_la_LDFLAGS =					\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^lunar_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

lunar_wallpaper_plugin_la_LIBADD =					\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la	\
	$(ENDO_LIBS)							\
	$(GLIB_LIBS)							\
	$(GTK_LIBS)							\
	$(ESCONF_LIBS)						\
	$(LIBX11_LIBS)

lunar_wallpaper_plugin_la_DEPENDENCIES =				\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
