AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_builddir)/plugins					\
	-I$(top_srcdir)							\
	-I$(top_srcdir)/plugins						\
	-DBINDIR=\"$(bindir)\"						\
	-DDATADIR=\"$(datadir)\"					\
	-DG_LOG_DOMAIN=\"lunar-uca\"					\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	$(PLATFORM_CPPFLAGS)

extensionsdir = $(libdir)/lunarx-$(LUNARX_VERSION_API)
extensions_LTLIBRARIES =						\
	lunar-uca.la

lunar_uca_la_SOURCES =							\
	lunar-uca-chooser.c						\
	lunar-uca-chooser.h						\
	lunar-uca-context.c						\
	lunar-uca-context.h						\
	lunar-uca-editor.c						\
	lunar-uca-editor.h						\
	lunar-uca-model.c						\
	lunar-uca-model.h						\
	lunar-uca-plugin.c						\
	lunar-uca-private.c						\
	lunar-uca-private.h						\
	lunar-uca-provider.c						\
	lunar-uca-provider.h						\
	lunar-uca.gresource.c

lunar_uca_la_CFLAGS =							\
	$(ENDO_CFLAGS)							\
	$(LIBEXPIDUS1UTIL_CFLAGS)						\
	$(LIBEXPIDUS1UI_CFLAGS)						\
	$(LIBEXPIDUS1KBD_PRIVATE_CFLAGS) 			    \
	$(PLATFORM_CFLAGS)

lunar_uca_la_LDFLAGS =							\
	-avoid-version							\
	-export-dynamic							\
	-no-undefined							\
	-export-symbols-regex "^lunar_extension_(shutdown|initialize|list_types)" \
	-module								\
	$(PLATFORM_LDFLAGS)

lunar_uca_la_LIBADD =							\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la	\
	$(ENDO_LIBS)							\
	$(LIBEXPIDUS1UTIL_LIBS)						\
	$(LIBEXPIDUS1UI_LIBS)							\
	$(LIBEXPIDUS1KBD_PRIVATE_LIBS)

lunar_uca_la_DEPENDENCIES =						\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la

lunar_uca_templates =							\
	lunar-uca-ui.xml

GLIB_COMPILE_RESOURCES = $(shell $(PKG_CONFIG) --variable=glib_compile_resources gio-2.0)

resource_files = $(shell $(GLIB_COMPILE_RESOURCES) --sourcedir=$(srcdir) --generate-dependencies $(srcdir)/lunar-uca.gresource.xml)

lunar-uca.gresource.c: lunar-uca.gresource.xml $(resource_files)
	$(AM_V_GEN) $(GLIB_COMPILE_RESOURCES) $< \
		--target=$@ --sourcedir=$(srcdir) --generate-source

defaultsdir = $(sysconfdir)/xdg/Lunar
defaults_in_files = uca.xml.in
defaults_DATA = $(defaults_in_files:.xml.in=.xml)
@INTLTOOL_XML_RULE@

EXTRA_DIST =								\
	README.md								\
	lunar-uca.gresource.xml					\
	lunar-uca-editor.ui						\
	lunar-uca-chooser.ui						\
	$(defaults_in_files)

CLEANFILES =								\
	$(defaults_DATA)

# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:
