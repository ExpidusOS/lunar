# vi:set ts=8 sw=8 noet ai nocindent syntax=automake:

AM_CPPFLAGS =								\
	-I$(top_builddir)						\
	-I$(top_srcdir)							\
	-DBINDIR=\"$(bindir)\"						\
	-DDATADIR=\"$(datadir)\"					\
	-DENDO_DISABLE_DEPRECATED					\
	-DG_LOG_DOMAIN=\"lunar\"					\
	-DLIBDIR=\"$(libdir)\"						\
	-DHELPERDIR=\"$(HELPER_PATH_PREFIX)\"				\
	-DPACKAGE_LOCALE_DIR=\"$(localedir)\"				\
	-DLUNAR_VERSION_API=\"$(LUNAR_VERSION_API)\"			\
	-DSN_API_NOT_YET_FROZEN						\
	-DG_UDEV_API_IS_SUBJECT_TO_CHANGE				\
	$(PLATFORM_CPPFLAGS)

bin_SCRIPTS =								\
	lunar-settings

bin_PROGRAMS =								\
	lunar

lunar_built_sources =							\
	lunar-marshal.c							\
	lunar-marshal.h							\
	lunar-dbus-freedesktop-interfaces.h		\
	lunar-dbus-freedesktop-interfaces.c		\
	lunar-dbus-service-infos.h					\
	lunar-dbus-service-infos.c					\
	lunar-thumbnailer-proxy.c					\
	lunar-thumbnailer-proxy.h					\
	lunar-thumbnail-cache-proxy.h				\
	lunar-thumbnail-cache-proxy.c				\
	lunar-resources.h							\
	lunar-resources.c


lunar_SOURCES =							\
	$(lunar_include_HEADERS)					\
	$(lunar_built_sources)						\
	$(lunar_dbus_sources)						\
	main.c								\
	lunar-abstract-dialog.c					\
	lunar-abstract-dialog.h					\
	lunar-abstract-icon-view.c					\
	lunar-abstract-icon-view.h					\
	lunar-application.c						\
	lunar-application.h						\
	lunar-browser.c						\
	lunar-browser.h						\
	lunar-chooser-button.c						\
	lunar-chooser-button.h						\
	lunar-chooser-dialog.c						\
	lunar-chooser-dialog.h						\
	lunar-chooser-model.c						\
	lunar-chooser-model.h						\
	lunar-clipboard-manager.c					\
	lunar-clipboard-manager.h					\
	lunar-column-editor.c						\
	lunar-column-editor.h						\
	lunar-column-model.c						\
	lunar-column-model.h						\
	lunar-compact-view.c						\
	lunar-compact-view.h						\
	lunar-component.c						\
	lunar-component.h						\
	lunar-dbus-service.c						\
	lunar-dbus-service.h						\
	lunar-deep-count-job.h						\
	lunar-deep-count-job.c						\
	lunar-details-view.c						\
	lunar-details-view.h						\
	lunar-dialogs.c						\
	lunar-dialogs.h						\
	lunar-device.c							\
	lunar-device.h							\
	lunar-device-monitor.c						\
	lunar-device-monitor.h						\
	lunar-dnd.c							\
	lunar-dnd.h							\
	lunar-emblem-chooser.c						\
	lunar-emblem-chooser.h						\
	lunar-enum-types.c						\
	lunar-enum-types.h						\
	lunar-file.c							\
	lunar-file.h							\
	lunar-file-monitor.c						\
	lunar-file-monitor.h						\
	lunar-folder.c							\
	lunar-folder.h							\
	lunar-gdk-extensions.c						\
	lunar-gdk-extensions.h						\
	lunar-gio-extensions.c						\
	lunar-gio-extensions.h						\
	lunar-gobject-extensions.c					\
	lunar-gobject-extensions.h					\
	lunar-gtk-extensions.c						\
	lunar-gtk-extensions.h						\
	lunar-history.c						\
	lunar-history.h						\
	lunar-ice.c							\
	lunar-ice.h							\
	lunar-icon-factory.c						\
	lunar-icon-factory.h						\
	lunar-icon-renderer.c						\
	lunar-icon-renderer.h						\
	lunar-icon-view.c						\
	lunar-icon-view.h						\
	lunar-image.c							\
	lunar-image.h							\
	lunar-io-jobs.c						\
	lunar-io-jobs.h						\
	lunar-io-jobs-util.c						\
	lunar-io-jobs-util.h						\
	lunar-io-scan-directory.c					\
	lunar-io-scan-directory.h					\
	lunar-job.c							\
	lunar-job.h							\
	lunar-launcher.c						\
	lunar-launcher.h						\
	lunar-list-model.c						\
	lunar-list-model.h						\
	lunar-location-bar.c						\
	lunar-location-bar.h						\
	lunar-location-button.c					\
	lunar-location-button.h					\
	lunar-location-buttons.c					\
	lunar-location-buttons.h					\
	lunar-location-entry.c						\
	lunar-location-entry.h						\
	lunar-menu.c							\
	lunar-menu.h							\
	lunar-notify.c							\
	lunar-notify.h							\
	lunar-navigator.c						\
	lunar-navigator.h						\
	lunar-pango-extensions.c					\
	lunar-pango-extensions.h					\
	lunar-path-entry.c						\
	lunar-path-entry.h						\
	lunar-permissions-chooser.c					\
	lunar-permissions-chooser.h					\
	lunar-preferences-dialog.c					\
	lunar-preferences-dialog.h					\
	lunar-preferences.c						\
	lunar-preferences.h						\
	lunar-private.h						\
	lunar-progress-dialog.c					\
	lunar-progress-dialog.h					\
	lunar-progress-view.c						\
	lunar-progress-view.h						\
	lunar-properties-dialog.c					\
	lunar-properties-dialog.h					\
	lunar-renamer-dialog.c						\
	lunar-renamer-dialog.h						\
	lunar-renamer-model.c						\
	lunar-renamer-model.h						\
	lunar-renamer-pair.c						\
	lunar-renamer-pair.h						\
	lunar-renamer-progress.c					\
	lunar-renamer-progress.h					\
	lunar-sendto-model.c						\
	lunar-sendto-model.h						\
	lunar-session-client.c						\
	lunar-session-client.h						\
	lunar-shortcuts-icon-renderer.c				\
	lunar-shortcuts-icon-renderer.h				\
	lunar-shortcuts-model.c					\
	lunar-shortcuts-model.h					\
	lunar-shortcuts-pane.c						\
	lunar-shortcuts-pane.h						\
	lunar-shortcuts-view.c						\
	lunar-shortcuts-view.h						\
	lunar-side-pane.c						\
	lunar-side-pane.h						\
	lunar-simple-job.c						\
	lunar-simple-job.h						\
	lunar-size-label.c						\
	lunar-size-label.h						\
	lunar-standard-view.c						\
	lunar-standard-view.h						\
	lunar-statusbar.c						\
	lunar-statusbar.h						\
	lunar-thumbnail-cache.c					\
	lunar-thumbnail-cache.h					\
	lunar-thumbnailer.c						\
	lunar-thumbnailer.h						\
	lunar-transfer-job.c						\
	lunar-transfer-job.h						\
	lunar-tree-model.c						\
	lunar-tree-model.h						\
	lunar-tree-pane.c						\
	lunar-tree-pane.h						\
	lunar-tree-view.c						\
	lunar-tree-view.h						\
	lunar-user.c							\
	lunar-user.h							\
	lunar-util.c							\
	lunar-util.h							\
	lunar-view.c							\
	lunar-view.h							\
	lunar-window.c							\
	lunar-window.h

lunar_CFLAGS =								\
	$(ENDO_CFLAGS)							\
	$(GIO_CFLAGS)							\
	$(GTHREAD_CFLAGS)						\
	$(GUDEV_CFLAGS)							\
	$(LIBNOTIFY_CFLAGS)						\
	$(LIBSM_CFLAGS)							\
	$(LIBEXPIDUS1UI_CFLAGS)						\
	$(ESCONF_CFLAGS)						\
	$(PANGO_CFLAGS)							\
	$(PLATFORM_CFLAGS)

lunar_LDFLAGS =							\
	-no-undefined							\
	$(LIBSM_LDFLAGS)						\
	$(PLATFORM_LDFLAGS)

lunar_LDADD =								\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la	\
	$(ENDO_LIBS)							\
	$(GIO_LIBS)							\
	$(GTHREAD_LIBS)							\
	$(GUDEV_LIBS)							\
	$(LIBNOTIFY_LIBS)						\
	$(LIBSM_LIBS)							\
	$(LIBEXPIDUS1UI_LIBS)						\
	$(ESCONF_LIBS)							\
	$(PANGO_LIBS)

lunar_DEPENDENCIES =							\
	$(top_builddir)/lunarx/liblunarx-$(LUNARX_VERSION_API).la

if HAVE_GIO_UNIX
lunar_CFLAGS +=							\
	$(GIO_UNIX_CFLAGS)

lunar_LDADD +=								\
	$(GIO_UNIX_LIBS)
endif

desktopdir = $(datadir)/applications
desktop_in_files = lunar-settings.desktop.in
desktop_DATA = $(desktop_in_files:.desktop.in=.desktop)
@INTLTOOL_DESKTOP_RULE@

# install symlink to 'lunar'
install-exec-hook:
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	-( cd $(DESTDIR)$(bindir) ; test -x Lunar || ln -sf lunar Lunar )

uninstall-local:
	-( cd $(DESTDIR)$(bindir) ; test -h Lunar && rm -f Lunar )

clean-local:
	rm -f *.core core core.* *.o

DISTCLEANFILES =							\
	lunar-settings.desktop

if MAINTAINER_MODE
CLEANFILES =								\
	xgen-tmc							\
	xgen-tmh

DISTCLEANFILES +=							\
	$(lunar_built_sources)						\
	stamp-lunar-marshal.h

BUILT_SOURCES =								\
	$(lunar_built_sources)

lunar-dbus-freedesktop-interfaces.h lunar-dbus-freedesktop-interfaces.c: $(srcdir)/lunar-dbus-freedesktop-interfaces.xml Makefile
	$(AM_V_GEN) gdbus-codegen --c-namespace Lunar --generate-c-code=lunar-dbus-freedesktop-interfaces $(srcdir)/lunar-dbus-freedesktop-interfaces.xml

lunar-dbus-service-infos.h lunar-dbus-service-infos.c: $(srcdir)/lunar-dbus-service-infos.xml Makefile
	$(AM_V_GEN) gdbus-codegen --c-namespace Lunar --generate-c-code=lunar-dbus-service-infos $(srcdir)/lunar-dbus-service-infos.xml

lunar-thumbnailer-proxy.h lunar-thumbnailer-proxy.c: $(srcdir)/lunar-thumbnailer-dbus.xml Makefile
	$(AM_V_GEN) gdbus-codegen --c-namespace Lunar --generate-c-code lunar-thumbnailer-proxy $(srcdir)/lunar-thumbnailer-dbus.xml

lunar-thumbnail-cache-proxy.h lunar-thumbnail-cache-proxy.c: $(srcdir)/lunar-thumbnail-cache-dbus.xml Makefile
	$(AM_V_GEN) gdbus-codegen --c-namespace Lunar --generate-c-code lunar-thumbnail-cache-proxy $(srcdir)/lunar-thumbnail-cache-dbus.xml

lunar-marshal.h: stamp-lunar-marshal.h
	@true

stamp-lunar-marshal.h: lunar-marshal.list Makefile
	$(AM_V_GEN) ( \
		cd $(srcdir) \
		&& echo "#ifndef __LUNAR_MARSHAL_H__" > xgen-tmh \
		&& echo "#define __LUNAR_MARSHAL_H__" >> xgen-tmh \
		&& ( glib-genmarshal \
			--prefix=_lunar_marshal \
			--header lunar-marshal.list ) >> xgen-tmh \
		&& echo "#endif /* !__LUNAR_MARSHAL_H__ */" >> xgen-tmh \
		&& ( cmp -s xgen-tmh lunar-marshal.h || cp xgen-tmh lunar-marshal.h ) \
		&& rm -f xgen-tmh \
		&& echo timestamp > $(@F) \
	)

lunar-marshal.c: lunar-marshal.list Makefile
	$(AM_V_GEN) ( \
		cd $(srcdir) \
		&& echo "#include <lunar/lunar-marshal.h>" > xgen-tmc \
		&& ( glib-genmarshal \
			--prefix=_lunar_marshal \
			--body lunar-marshal.list ) >> xgen-tmc \
		&& cp xgen-tmc lunar-marshal.c \
		&& rm -f xgen-tmc \
	)

lunar-resources.c: lunar.gresource.xml
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-source --c-name lunar $(srcdir)/lunar.gresource.xml

lunar-resources.h: lunar.gresource.xml
	glib-compile-resources --target=$@ --sourcedir=$(srcdir) --generate-header --c-name lunar $(srcdir)/lunar.gresource.xml
endif

EXTRA_DIST =								\
	lunar-dbus-freedesktop-interfaces.xml	\
	lunar-dbus-service-infos.xml			\
	lunar-marshal.list						\
	lunar-thumbnail-frame.png				\
	lunar-settings							\
	lunar.gresource.xml					\
	$(desktop_in_files)
