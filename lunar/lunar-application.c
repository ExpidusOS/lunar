/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2005      Jeff Franks <jcfranks@expidus.org>
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@expidus.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif
#ifdef HAVE_TIME_H
#include <time.h>
#endif

#include <stdlib.h>

#ifdef HAVE_GUDEV
#include <gudev/gudev.h>
#endif

#include <libexpidus1ui/libexpidus1ui.h>

#include <lunar/lunar-application.h>
#include <lunar/lunar-browser.h>
#include <lunar/lunar-dialogs.h>
#include <lunar/lunar-gdk-extensions.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-io-jobs.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-progress-dialog.h>
#include <lunar/lunar-renamer-dialog.h>
#include <lunar/lunar-thumbnail-cache.h>
#include <lunar/lunar-thumbnailer.h>
#include <lunar/lunar-util.h>
#include <lunar/lunar-view.h>
#include <lunar/lunar-session-client.h>
#include <lunar/lunar-dbus-service.h>

#define ACCEL_MAP_PATH "Lunar/accels.scm"



/* option values */
static gboolean opt_version = FALSE;
/* the sm-client-id option is only honored while starting the primary instance,
 * and will be silently ignored on every other invocation */
static gchar   *opt_sm_client_id = NULL;

/* option entries */
static const GOptionEntry option_entries[] =
{
  { "bulk-rename", 'B', 0, G_OPTION_ARG_NONE, NULL, N_ ("Open the bulk rename dialog"), NULL, },
  { "daemon", 0, 0, G_OPTION_ARG_NONE, NULL, N_ ("Run in daemon mode"), NULL, },
  { "sm-client-id", 0, G_OPTION_FLAG_HIDDEN, G_OPTION_ARG_STRING, &opt_sm_client_id, NULL, NULL, },
  { "quit", 'q', 0, G_OPTION_ARG_NONE, NULL, N_ ("Quit a running Lunar instance"), NULL, },
  { "version", 'V', 0, G_OPTION_ARG_NONE, &opt_version, N_ ("Print version information and exit"), NULL, },
  { G_OPTION_REMAINING, 0, 0, G_OPTION_ARG_FILENAME_ARRAY, NULL, NULL, NULL, },
  { NULL, },
};



/* Prototype for the Lunar job launchers */
typedef LunarJob *(*Launcher) (GList *source_path_list,
                                GList *target_path_list);



/* Property identifiers */
enum
{
  PROP_0,
  PROP_DAEMON,
};



static void           lunar_application_finalize               (GObject                *object);
static void           lunar_application_get_property           (GObject                *object,
                                                                 guint                   prop_id,
                                                                 GValue                 *value,
                                                                 GParamSpec             *pspec);
static void           lunar_application_set_property           (GObject                *object,
                                                                 guint                   prop_id,
                                                                 const GValue           *value,
                                                                 GParamSpec             *pspec);
static void           lunar_application_dbus_acquired_cb       (GDBusConnection        *conn,
                                                                 const gchar            *name,
                                                                 gpointer                user_data);
static void           lunar_application_name_acquired_cb       (GDBusConnection        *connection,
                                                                 const gchar            *name,
                                                                 gpointer                user_data);
static void           lunar_application_dbus_name_lost_cb      (GDBusConnection        *connection,
                                                                 const gchar            *name,
                                                                 gpointer                user_data);
static void           lunar_application_dbus_init              (LunarApplication      *application);
static void           lunar_application_startup                (GApplication           *application);
static void           lunar_application_shutdown               (GApplication           *application);
static void           lunar_application_activate               (GApplication           *application);
static int            lunar_application_handle_local_options   (GApplication           *application,
                                                                 GVariantDict           *options);
static int            lunar_application_command_line           (GApplication           *application,
                                                                 GApplicationCommandLine *command_line);
static gboolean       lunar_application_dbus_register          (GApplication           *application,
                                                                 GDBusConnection        *connection,
                                                                 const gchar            *object_path,
                                                                 GError                **error);
static void           lunar_application_load_css               (void);
static void           lunar_application_accel_map_changed      (LunarApplication      *application);
static gboolean       lunar_application_accel_map_save         (gpointer                user_data);
static void           lunar_application_collect_and_launch     (LunarApplication      *application,
                                                                 gpointer                parent,
                                                                 const gchar            *icon_name,
                                                                 const gchar            *title,
                                                                 Launcher                launcher,
                                                                 GList                  *source_file_list,
                                                                 GFile                  *target_file,
                                                                 gboolean                update_source_folders,
                                                                 gboolean                update_target_folders,
                                                                 GClosure               *new_files_closure);
static void           lunar_application_launch_finished        (LunarJob              *job,
                                                                 GList                  *containing_folders);
static void           lunar_application_launch                 (LunarApplication      *application,
                                                                 gpointer                parent,
                                                                 const gchar            *icon_name,
                                                                 const gchar            *title,
                                                                 Launcher                launcher,
                                                                 GList                  *source_path_list,
                                                                 GList                  *target_path_list,
                                                                 gboolean                update_source_folders,
                                                                 gboolean                update_target_folders,
                                                                 GClosure               *new_files_closure);
#ifdef HAVE_GUDEV
static void           lunar_application_uevent                 (GUdevClient            *client,
                                                                 const gchar            *action,
                                                                 GUdevDevice            *device,
                                                                 LunarApplication      *application);
static gboolean       lunar_application_volman_idle            (gpointer                user_data);
static void           lunar_application_volman_idle_destroy    (gpointer                user_data);
static void           lunar_application_volman_watch           (GPid                    pid,
                                                                 gint                    status,
                                                                 gpointer                user_data);
static void           lunar_application_volman_watch_destroy   (gpointer                user_data);
#endif
static gboolean       lunar_application_show_dialogs           (gpointer                user_data);
static void           lunar_application_show_dialogs_destroy   (gpointer                user_data);
static GtkWidget     *lunar_application_get_progress_dialog    (LunarApplication      *application);
static void           lunar_application_process_files          (LunarApplication      *application);



struct _LunarApplicationClass
{
  GtkApplicationClass __parent__;
};

struct _LunarApplication
{
  GtkApplication         __parent__;

  LunarSessionClient   *session_client;

  LunarPreferences     *preferences;
  GtkWidget             *progress_dialog;

  LunarThumbnailCache  *thumbnail_cache;
  LunarThumbnailer     *thumbnailer;

  LunarDBusService     *dbus_service;

  gboolean               daemon;

  guint                  accel_map_save_id;
  GtkAccelMap           *accel_map;

  guint                  show_dialogs_timer_id;

#ifdef HAVE_GUDEV
  GUdevClient           *udev_client;

  GSList                *volman_udis;
  guint                  volman_idle_id;
  guint                  volman_watch_id;
#endif

  GList                 *files_to_launch;

  guint                  dbus_owner_id_expidus;
  guint                  dbus_owner_id_fdo;
};



static GQuark lunar_application_screen_quark;
static GQuark lunar_application_startup_id_quark;
static GQuark lunar_application_file_quark;



G_DEFINE_TYPE_EXTENDED (LunarApplication, lunar_application, GTK_TYPE_APPLICATION, 0,
                        G_IMPLEMENT_INTERFACE (LUNAR_TYPE_BROWSER, NULL))



static void
lunar_application_class_init (LunarApplicationClass *klass)
{
  GObjectClass        *gobject_class = G_OBJECT_CLASS (klass);
  GApplicationClass   *gapplication_class = G_APPLICATION_CLASS (klass);

  /* pre-allocate the required quarks */
  lunar_application_screen_quark =
    g_quark_from_static_string ("lunar-application-screen");
  lunar_application_startup_id_quark =
    g_quark_from_static_string ("lunar-application-startup-id");
  lunar_application_file_quark =
    g_quark_from_static_string ("lunar-application-file");

  gobject_class->finalize = lunar_application_finalize;
  gobject_class->get_property = lunar_application_get_property;
  gobject_class->set_property = lunar_application_set_property;

  gapplication_class->startup              = lunar_application_startup;
  gapplication_class->activate             = lunar_application_activate;
  gapplication_class->shutdown             = lunar_application_shutdown;
  gapplication_class->handle_local_options = lunar_application_handle_local_options;
  gapplication_class->command_line         = lunar_application_command_line;
  gapplication_class->dbus_register        = lunar_application_dbus_register;

  /**
   * LunarApplication:daemon:
   *
   * %TRUE if the application should be run in daemon mode,
   * in which case it will never terminate. %FALSE if the
   * application should terminate once the last window is
   * closed.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DAEMON,
                                   g_param_spec_boolean ("daemon",
                                                         "daemon",
                                                         "daemon",
                                                         FALSE,
                                                         ENDO_PARAM_READWRITE));
}



static void
lunar_application_init (LunarApplication *application)
{
  /* we do most initialization in GApplication::startup since it is only needed
   * in the primary instance anyways */

  application->files_to_launch = NULL;
  application->progress_dialog = NULL;
  application->preferences     = NULL;

  g_application_set_flags (G_APPLICATION (application), G_APPLICATION_HANDLES_COMMAND_LINE);
  g_application_add_main_option_entries (G_APPLICATION (application), option_entries);
}


static void
lunar_application_dbus_acquired_cb (GDBusConnection *conn,
                                     const gchar     *name,
                                     gpointer         user_data)
{
  g_debug (_("Acquired the session message bus '%s'\n"), name);
}



static void
lunar_application_name_acquired_cb (GDBusConnection *connection,
                                     const gchar     *name,
                                     gpointer         user_data)
{
    g_debug (_("Acquired the name '%s' on the session message bus\n"), name);
}



static void
lunar_application_dbus_name_lost_cb (GDBusConnection *connection,
                                      const gchar     *name,
                                      gpointer         user_data)
{
  g_warning (_("Name '%s' lost on the message dbus."), name);
}



/* TODO: [GTK3 Port] Check if there's a cleaner way to register */
/* these extra dbus names (besides com.expidus.Lunar) */
static void
lunar_application_dbus_init (LunarApplication *application)
{
    application->dbus_owner_id_expidus = g_bus_own_name (G_BUS_TYPE_SESSION,
                                      "com.expidus.FileManager",
                                      G_BUS_NAME_OWNER_FLAGS_NONE,
                                      lunar_application_dbus_acquired_cb,
                                      lunar_application_name_acquired_cb,
                                      lunar_application_dbus_name_lost_cb,
                                      application,
                                      NULL);

    application->dbus_owner_id_fdo = g_bus_own_name (G_BUS_TYPE_SESSION,
                                     "org.freedesktop.FileManager1",
                                     G_BUS_NAME_OWNER_FLAGS_NONE,
                                     lunar_application_dbus_acquired_cb,
                                     lunar_application_name_acquired_cb,
                                     lunar_application_dbus_name_lost_cb,
                                     application,
                                     NULL);
}



static void
lunar_application_startup (GApplication *gapp)
{
  LunarApplication *application = LUNAR_APPLICATION (gapp);

#ifdef HAVE_GUDEV
  static const gchar *subsystems[] = { "block", "input", "usb", NULL };
#endif
  gchar              *path;

  /* initialize the application */
  application->preferences = lunar_preferences_get ();

#ifdef HAVE_GUDEV
  /* establish connection with udev */
  application->udev_client = g_udev_client_new (subsystems);

  /* connect to the client in order to be notified when devices are plugged in
   * or disconnected from the computer */
  g_signal_connect (application->udev_client, "uevent",
                    G_CALLBACK (lunar_application_uevent), application);
#endif

  lunar_application_dbus_init (application);

  G_APPLICATION_CLASS (lunar_application_parent_class)->startup (gapp);

  /* connect to the session manager */
  application->session_client = lunar_session_client_new (opt_sm_client_id);

  /* check if we have a saved accel map */
  path = expidus_resource_lookup (EXPIDUS_RESOURCE_CONFIG, ACCEL_MAP_PATH);
  if (G_LIKELY (path != NULL))
    {
      /* load the accel map */
      gtk_accel_map_load (path);
      g_free (path);
    }

  /* watch for changes */
  application->accel_map = gtk_accel_map_get ();
  g_signal_connect_swapped (G_OBJECT (application->accel_map), "changed",
      G_CALLBACK (lunar_application_accel_map_changed), application);

  lunar_application_load_css ();
}



static void
lunar_application_shutdown (GApplication *gapp)
{
  LunarApplication *application = LUNAR_APPLICATION (gapp);

  /* unqueue all files waiting to be processed */
  lunar_g_file_list_free (application->files_to_launch);

  /* save the current accel map */
  if (G_UNLIKELY (application->accel_map_save_id != 0))
    {
      g_source_remove (application->accel_map_save_id);
      lunar_application_accel_map_save (application);
    }

  if (application->accel_map != NULL)
    g_object_unref (G_OBJECT (application->accel_map));

#ifdef HAVE_GUDEV
  /* cancel any pending volman watch source */
  if (G_UNLIKELY (application->volman_watch_id != 0))
    g_source_remove (application->volman_watch_id);

  /* cancel any pending volman idle source */
  if (G_UNLIKELY (application->volman_idle_id != 0))
    g_source_remove (application->volman_idle_id);

  /* drop all pending volume manager UDIs */
  g_slist_free_full (application->volman_udis, g_free);

  /* disconnect from the udev client */
  g_object_unref (application->udev_client);
#endif

  /* drop any running "show dialogs" timer */
  if (G_UNLIKELY (application->show_dialogs_timer_id != 0))
    g_source_remove (application->show_dialogs_timer_id);

  /* drop ref on the thumbnailer */
  if (application->thumbnailer != NULL)
    g_object_unref (application->thumbnailer);

  /* release the thumbnail cache */
  if (application->thumbnail_cache != NULL)
    g_object_unref (G_OBJECT (application->thumbnail_cache));

  /* disconnect from the preferences */
  g_object_unref (G_OBJECT (application->preferences));

  /* disconnect from the session manager */
  g_object_unref (G_OBJECT (application->session_client));

  /* remove the dbus service */
  g_clear_pointer (&application->dbus_service, g_object_unref);

  G_APPLICATION_CLASS (lunar_application_parent_class)->shutdown (gapp);
}



static void
lunar_application_finalize (GObject *object)
{
  /* rule of thumb: what gets initialized in GApplication::startup is cleaned up
   * in GApplication::shutdown. Therefore, this method doesn't do very much */

  (*G_OBJECT_CLASS (lunar_application_parent_class)->finalize) (object);
}



static int
lunar_application_handle_local_options (GApplication *gapp,
                                         GVariantDict *options)
{
  /* check if we should print version information */
  if (G_UNLIKELY (opt_version))
    {
      g_print ("%s %s (Expidus %s)\n\n", PACKAGE_NAME, PACKAGE_VERSION, expidus_version_string ());
      g_print ("%s\n", "Copyright (c) 2004-2020");
      g_print ("\t%s\n\n", _("The Lunar development team. All rights reserved."));
      g_print ("%s\n\n", _("Written by Benedikt Meurer <benny@expidus.org>."));
      g_print (_("Please report bugs to <%s>."), PACKAGE_BUGREPORT);
      g_print ("\n");
      return EXIT_SUCCESS;
    }

  /* continue processing on the primary instance */
  return -1;
}



static int
lunar_application_command_line (GApplication            *gapp,
                                 GApplicationCommandLine *command_line)
{
  LunarApplication *application  = LUNAR_APPLICATION (gapp);
  gboolean           bulk_rename  = FALSE;
  gboolean           daemon       = FALSE;
  gboolean           quit         = FALSE;
  GStrv              filenames    = NULL;
  const char        *cwd          = g_application_command_line_get_cwd (command_line);
  GVariantDict      *options_dict = g_application_command_line_get_options_dict (command_line);
  GError            *error        = NULL;
  gchar             *cwd_list[]   = { (gchar *)".", NULL };

  /* retrieve arguments */
  g_variant_dict_lookup (options_dict, "bulk-rename", "b", &bulk_rename);
  g_variant_dict_lookup (options_dict, "quit", "b", &quit);
  g_variant_dict_lookup (options_dict, "daemon", "b", &daemon);
  g_variant_dict_lookup (options_dict, G_OPTION_REMAINING, "^aay", &filenames);

  /* FIXME: --quit should be named --suicide */
  if (G_UNLIKELY (quit))
    {
        g_debug ("quitting");
        g_application_quit (gapp);
        goto out;
    }

  /* check whether we should daemonize */
  if (G_UNLIKELY (daemon))
    {
        lunar_application_set_daemon (application, TRUE);
    }

  /* check if we should open the bulk rename dialog */
  if (G_UNLIKELY (bulk_rename))
    {
      /* try to open the bulk rename dialog */
      if (!lunar_application_bulk_rename (application, cwd, filenames, TRUE, NULL, NULL, &error))
        {
          /* FIXME? */
          g_application_command_line_printerr (command_line, "Lunar: Failed to open bulk rename: %s\n", error->message);
        }
    }
  else if (filenames != NULL)
    {
      if (!lunar_application_process_filenames (application, cwd, filenames, NULL, NULL, &error))
        {
          /* we failed to process the filenames or the bulk rename failed */
          g_application_command_line_printerr (command_line, "Lunar: %s\n", error->message);
        }
    }
  else if (!daemon)
    {
      if (!lunar_application_process_filenames (application, cwd, cwd_list, NULL, NULL, &error))
        {
          /* we failed to process the filenames or the bulk rename failed */
          g_application_command_line_printerr (command_line, "Lunar: %s\n", error->message);
        }
    }

out:
  /* cleanup */
  g_strfreev (filenames);

  if (error)
    {
      g_error_free (error);
      return EXIT_FAILURE;
    }
  else
    return EXIT_SUCCESS;
}



static gboolean
lunar_application_dbus_register (GApplication           *gapp,
                                  GDBusConnection        *connection,
                                  const gchar            *object_path,
                                  GError                **error)
{
    LunarApplication *application = LUNAR_APPLICATION (gapp);

    if (application->dbus_service) /* WTF? */
        return TRUE;

    application->dbus_service = g_object_new (LUNAR_TYPE_DBUS_SERVICE, NULL);

    return lunar_dbus_service_export_on_connection (application->dbus_service, connection, error);
}



static void
lunar_application_load_css (void)
{
  GtkCssProvider *css_provider;
  GdkScreen *screen;

  css_provider = gtk_css_provider_new ();

  gtk_css_provider_load_from_data (css_provider,
    /* for the pathbar-buttons any margin looks ugly*/
    ".path-bar-button { margin-right: 0; }"
    /* remove extra border between side pane and view */
    ".shortcuts-pane { border-right-width: 0px; }"
    /* add missing top border to side pane */
    ".shortcuts-pane { border-top-style: solid; }"
    /* make border thicker during DnD */
    ".standard-view { border-left-width: 0px; border-right-width: 0px; }"
    ".standard-view:drop(active) { border-width: 2px; }", -1, NULL);
  screen = gdk_screen_get_default ();
  gtk_style_context_add_provider_for_screen (screen, GTK_STYLE_PROVIDER (css_provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  g_object_unref (css_provider);
}



static void
lunar_application_activate (GApplication *gapp)
{
  /* TODO */

  G_APPLICATION_CLASS (lunar_application_parent_class)->activate (gapp);
}



static void
lunar_application_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  LunarApplication *application = LUNAR_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_DAEMON:
      g_value_set_boolean (value, lunar_application_get_daemon (application));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_application_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  LunarApplication *application = LUNAR_APPLICATION (object);

  switch (prop_id)
    {
    case PROP_DAEMON:
      lunar_application_set_daemon (application, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gboolean
lunar_application_accel_map_save (gpointer user_data)
{
  LunarApplication *application = LUNAR_APPLICATION (user_data);
  gchar             *path;

  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), FALSE);

  application->accel_map_save_id = 0;

  /* save the current accel map */
  path = expidus_resource_save_location (EXPIDUS_RESOURCE_CONFIG, ACCEL_MAP_PATH, TRUE);
  if (G_LIKELY (path != NULL))
    {
      /* save the accel map */
      gtk_accel_map_save (path);
      g_free (path);
    }

  return FALSE;
}



static void
lunar_application_accel_map_changed (LunarApplication *application)
{
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* stop pending save */
  if (application->accel_map_save_id != 0)
    {
      g_source_remove (application->accel_map_save_id);
      application->accel_map_save_id = 0;
    }

  /* schedule new save */
  application->accel_map_save_id =
      g_timeout_add_seconds (10, lunar_application_accel_map_save, application);
}



static void
lunar_application_collect_and_launch (LunarApplication *application,
                                       gpointer           parent,
                                       const gchar       *icon_name,
                                       const gchar       *title,
                                       Launcher           launcher,
                                       GList             *source_file_list,
                                       GFile             *target_file,
                                       gboolean           update_source_folders,
                                       gboolean           update_target_folders,
                                       GClosure          *new_files_closure)
{
  GFile  *file;
  GError *err = NULL;
  GList  *target_file_list = NULL;
  GList  *lp;
  gchar  *base_name;

  /* check if we have anything to operate on */
  if (G_UNLIKELY (source_file_list == NULL))
    return;

  /* generate the target path list */
  for (lp = g_list_last (source_file_list); err == NULL && lp != NULL; lp = lp->prev)
    {
      /* verify that we're not trying to collect a root node */
      if (G_UNLIKELY (lunar_g_file_is_root (lp->data)))
        {
          /* tell the user that we cannot perform the requested operation */
          g_set_error (&err, G_FILE_ERROR, G_FILE_ERROR_INVAL, "%s", g_strerror (EINVAL));
        }
      else
        {
          base_name = g_file_get_basename (lp->data);
          file = g_file_resolve_relative_path (target_file, base_name);
          g_free (base_name);

          /* add to the target file list */
          target_file_list = lunar_g_file_list_prepend (target_file_list, file);
          g_object_unref (file);
        }
    }

  /* check if we failed */
  if (G_UNLIKELY (err != NULL))
    {
      /* display an error message to the user */
      lunar_dialogs_show_error (parent, err, _("Failed to launch operation"));

      /* release the error */
      g_error_free (err);
    }
  else
    {
      /* launch the operation */
      lunar_application_launch (application, parent, icon_name, title, launcher,
                                 source_file_list, target_file_list, update_source_folders, update_target_folders, new_files_closure);
    }

  /* release the target path list */
  lunar_g_file_list_free (target_file_list);
}



static void
lunar_application_launch_finished (LunarJob  *job,
                                    GList      *containing_folders)
{
  GList        *lp;
  LunarFile   *file;
  LunarFolder *folder;

  _lunar_return_if_fail (LUNAR_IS_JOB (job));

  for (lp = containing_folders; lp != NULL; lp = lp->next)
    {
      if (lp->data == NULL)
        continue;
      file = lunar_file_get (lp->data, NULL);
      if (file != NULL)
        {
          if (lunar_file_is_directory (file))
            {
              folder = lunar_folder_get_for_file (file);
              if (folder != NULL)
                {
                  /* If the folder is connected to a folder monitor, we dont need to trigger the reload manually */
                  if (!lunar_folder_has_folder_monitor (folder))
                  {
                    lunar_folder_reload (folder, FALSE);
                  }
                  g_object_unref (folder);
                }
            }
          g_object_unref (file);
        }
      /* Unref all containing_folders (refs obtained by g_file_get_parent in lunar_g_file_list_get_parents ) */
      g_object_unref (lp->data);
    }
}



static void
lunar_application_launch (LunarApplication *application,
                           gpointer           parent,
                           const gchar       *icon_name,
                           const gchar       *title,
                           Launcher           launcher,
                           GList             *source_file_list,
                           GList             *target_file_list,
                           gboolean           update_source_folders,
                           gboolean           update_target_folders,
                           GClosure          *new_files_closure)
{
  GtkWidget *dialog;
  GdkScreen *screen;
  LunarJob *job;
  GList     *parent_folder_list = NULL;
  gboolean   has_jobs;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));

  /* parse the parent pointer */
  screen = lunar_util_parse_parent (parent, NULL);

  /* try to allocate a new job for the operation */
  job = (*launcher) (source_file_list, target_file_list);

  if (update_source_folders)
    parent_folder_list = g_list_concat (parent_folder_list, lunar_g_file_list_get_parents (source_file_list));
  if (update_target_folders)
    parent_folder_list = g_list_concat (parent_folder_list, lunar_g_file_list_get_parents (target_file_list));

  /* connect a callback to instantly refresh the parent folders after the operation finished */
  g_signal_connect (G_OBJECT (job), "finished",
                    G_CALLBACK (lunar_application_launch_finished),
                    parent_folder_list);

  /* connect the "new-files" closure (if any) */
  if (G_LIKELY (new_files_closure != NULL))
    g_signal_connect_closure (job, "new-files", new_files_closure, FALSE);

  /* get the shared progress dialog */
  dialog = lunar_application_get_progress_dialog (application);

  /* place the dialog on the given screen */
  if (screen != NULL)
    gtk_window_set_screen (GTK_WINDOW (dialog), screen);

  has_jobs = lunar_progress_dialog_has_jobs (LUNAR_PROGRESS_DIALOG (dialog));

  /* add the job to the dialog */
  lunar_progress_dialog_add_job (LUNAR_PROGRESS_DIALOG (dialog),
                                  job, icon_name, title);

  if (has_jobs)
    {
      /* show the dialog immediately */
      lunar_application_show_dialogs (application);
    }
  else
    {
      /* Set up a timer to show the dialog, to make sure we don't
       * just popup and destroy a dialog for a very short job.
       */
      if (G_LIKELY (application->show_dialogs_timer_id == 0))
        {
          application->show_dialogs_timer_id =
            gdk_threads_add_timeout_full (G_PRIORITY_DEFAULT, 750, lunar_application_show_dialogs,
                                          application, lunar_application_show_dialogs_destroy);
        }
    }

  /* drop our reference on the job */
  g_object_unref (job);
}



#ifdef HAVE_GUDEV
static void
lunar_application_uevent (GUdevClient       *client,
                           const gchar       *action,
                           GUdevDevice       *device,
                           LunarApplication *application)
{
  const gchar *sysfs_path;
  gboolean     is_cdrom = FALSE;
  gboolean     has_media = FALSE;
  GSList      *lp;

  _lunar_return_if_fail (G_UDEV_IS_CLIENT (client));
  _lunar_return_if_fail (action != NULL && *action != '\0');
  _lunar_return_if_fail (G_UDEV_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (client == application->udev_client);

  /* determine the sysfs path of the device */
  sysfs_path = g_udev_device_get_sysfs_path (device);

  /* check if the device is a CD drive */
  is_cdrom = g_udev_device_get_property_as_boolean (device, "ID_CDROM");
  has_media = g_udev_device_get_property_as_boolean (device, "ID_CDROM_MEDIA");

  /* distinguish between "add", "change" and "remove" actions, ignore "move" */
  if (g_strcmp0 (action, "add") == 0
      || (is_cdrom && has_media && g_strcmp0 (action, "change") == 0))
    {
      /* only insert the path if we don't have it already */
      if (g_slist_find_custom (application->volman_udis, sysfs_path,
                               (GCompareFunc) (void (*)(void)) g_utf8_collate) == NULL)
        {
          application->volman_udis = g_slist_prepend (application->volman_udis,
                                                      g_strdup (sysfs_path));

          /* check if there's currently no active or scheduled handler */
          if (G_LIKELY (application->volman_idle_id == 0
                        && application->volman_watch_id == 0))
            {
              /* schedule a new handler using the idle source, which invokes the handler */
              application->volman_idle_id =
                g_idle_add_full (G_PRIORITY_LOW, lunar_application_volman_idle,
                                 application, lunar_application_volman_idle_destroy);
            }
        }
    }
  else if (g_strcmp0 (action, "remove") == 0)
    {
      /* look for the sysfs path in the list of pending paths */
      lp = g_slist_find_custom (application->volman_udis, sysfs_path,
                                (GCompareFunc) (void (*)(void)) g_utf8_collate);

      if (G_LIKELY (lp != NULL))
        {
          /* free the sysfs path string */
          g_free (lp->data);

          /* drop the sysfs path from the list of pending device paths */
          application->volman_udis = g_slist_delete_link (application->volman_udis, lp);
        }
    }
}



static gboolean
lunar_application_volman_idle (gpointer user_data)
{
  LunarApplication *application = LUNAR_APPLICATION (user_data);
  GdkScreen         *screen;
  gboolean           misc_volume_management;
  GError            *err = NULL;
  gchar            **argv;
  GPid               pid;
  char              *display = NULL;

  /* check if volume management is enabled (otherwise, we don't spawn anything, but clear the list here) */
  g_object_get (G_OBJECT (application->preferences), "misc-volume-management", &misc_volume_management, NULL);
  if (G_LIKELY (misc_volume_management))
    {
      /* check if we don't already have a handler, and we have a pending UDI */
      if (application->volman_watch_id == 0 && application->volman_udis != NULL)
        {
          /* generate the argument list for the volman */
          argv = g_new (gchar *, 4);
          argv[0] = g_strdup ("lunar-volman");
          argv[1] = g_strdup ("--device-added");
          argv[2] = application->volman_udis->data;
          argv[3] = NULL;

          /* remove the first list item from the pending list */
          application->volman_udis = g_slist_delete_link (application->volman_udis, application->volman_udis);

          /* locate the currently active screen (the one with the pointer) */
          screen = expidus_gdk_screen_get_active (NULL);

          if (screen != NULL)
            display = g_strdup (gdk_display_get_name (gdk_screen_get_display (screen)));

          /* try to spawn the volman on the active screen */
          if (g_spawn_async (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, lunar_setup_display_cb, display, &pid, &err))
            {
              /* add a child watch for the volman handler */
              application->volman_watch_id = g_child_watch_add_full (G_PRIORITY_LOW, pid, lunar_application_volman_watch,
                                                                     application, lunar_application_volman_watch_destroy);
            }
          else
            {
              /* failed to spawn, tell the user, giving a hint to install the lunar-volman package */
              g_warning ("Failed to launch the volume manager (%s), make sure you have the \"lunar-volman\" package installed.", err->message);
              g_error_free (err);
            }

          /* cleanup */
          g_free (display);
          g_strfreev (argv);
        }

    }
  else
    {
      /* drop all pending HAL device UDIs */
      g_slist_free_full (application->volman_udis, g_free);
      application->volman_udis = NULL;
    }

  /* keep the idle source alive as long as no handler is
   * active and we have pending UDIs that must be handled
   */
  return (application->volman_watch_id == 0
       && application->volman_udis != NULL);
}



static void
lunar_application_volman_idle_destroy (gpointer user_data)
{
  LUNAR_APPLICATION (user_data)->volman_idle_id = 0;
}



static void
lunar_application_volman_watch (GPid     pid,
                                 gint     status,
                                 gpointer user_data)
{
  LunarApplication *application = LUNAR_APPLICATION (user_data);

  /* check if the idle source isn't active, but we have pending UDIs */
  if (application->volman_idle_id == 0 && application->volman_udis != NULL)
    {
      /* schedule a new handler using the idle source, which invokes the handler */
      application->volman_idle_id = gdk_threads_add_idle_full (G_PRIORITY_LOW, lunar_application_volman_idle,
                                                               application, lunar_application_volman_idle_destroy);
    }

  /* be sure to close the pid handle */
  g_spawn_close_pid (pid);
}



static void
lunar_application_volman_watch_destroy (gpointer user_data)
{
  LUNAR_APPLICATION (user_data)->volman_watch_id = 0;
}
#endif /* HAVE_GUDEV */



static gboolean
lunar_application_show_dialogs (gpointer user_data)
{
  LunarApplication *application = LUNAR_APPLICATION (user_data);

  /* show the progress dialog */
  if (application->progress_dialog != NULL)
    gtk_window_present (GTK_WINDOW (application->progress_dialog));

  return FALSE;
}



static void
lunar_application_show_dialogs_destroy (gpointer user_data)
{
  LUNAR_APPLICATION (user_data)->show_dialogs_timer_id = 0;
}



/**
 * lunar_application_get:
 *
 * Returns the global shared #LunarApplication instance.
 * This method takes a reference on the global instance
 * for the caller, so you must call g_object_unref()
 * on it when done.
 *
 * Return value: the shared #LunarApplication instance.
 **/
LunarApplication*
lunar_application_get (void)
{
  GApplication *default_app = g_application_get_default ();

  if (default_app)
    return LUNAR_APPLICATION (g_object_ref (default_app));
  else
    return g_object_ref_sink (g_object_new (LUNAR_TYPE_APPLICATION, "application-id", "com.expidus.Lunar", NULL));
}



/**
 * lunar_application_quit:
 * @application : a #LunarApplication.
 *
 * Attempts to exit daemon mode(required if application is on hold) and leaves the gtk main loop
 **/
void
lunar_application_quit (LunarApplication *application)
{
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  lunar_application_set_daemon(application, FALSE);

  /* For some unknown reason we need to close all open lunar windows before quit */
  /* Otherwise lunar will hangup on quit and lunar_application_shutdown will not be called */
  lunar_application_close_all_windows (application);

  g_application_quit (G_APPLICATION (application));
}



/**
 * lunar_application_get_daemon:
 * @application : a #LunarApplication.
 *
 * Returns %TRUE if @application is in daemon mode.
 *
 * Return value: %TRUE if @application is in daemon mode.
 **/
gboolean
lunar_application_get_daemon (LunarApplication *application)
{
  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), FALSE);
  return application->daemon;
}



/**
 * lunar_application_set_daemon:
 * @application : a #LunarApplication.
 * @daemonize   : %TRUE to set @application into daemon mode.
 *
 * If @daemon is %TRUE, @application will be set into daemon mode.
 **/
void
lunar_application_set_daemon (LunarApplication *application,
                               gboolean           daemonize)
{
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  if (application->daemon != daemonize)
    {
      application->daemon = daemonize;
      g_object_notify (G_OBJECT (application), "daemon");

      if (daemonize)
        g_application_hold (G_APPLICATION (application));
      else
        g_application_release (G_APPLICATION (application));
    }
}



/**
 * lunar_application_close_all_windows:
 * @application : a #LunarApplication.
 *
 * Closes all #LunarWindows currently handled by @application.
 **/
void
lunar_application_close_all_windows (LunarApplication *application)
{
  GList *gtk_windows, *lp;

  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  gtk_windows = gtk_application_get_windows (GTK_APPLICATION (application));

  for (lp = gtk_windows; lp != NULL; lp = lp->next)
    if (G_LIKELY (LUNAR_IS_WINDOW (lp->data)))
      gtk_window_close (lp->data);
}



/**
 * lunar_application_get_windows:
 * @application : a #LunarApplication.
 *
 * Returns the list of regular #LunarWindows currently handled by
 * @application. The returned list is owned by the caller and
 * must be freed using g_list_free().
 *
 * Return value: the list of regular #LunarWindows in @application.
 **/
GList*
lunar_application_get_windows (LunarApplication *application)
{
  GList *gtk_windows, *lunar_windows = NULL, *lp;

  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), NULL);

  gtk_windows = gtk_application_get_windows (GTK_APPLICATION (application));

  for (lp = gtk_windows; lp != NULL; lp = lp->next)
    if (G_LIKELY (LUNAR_IS_WINDOW (lp->data)))
      lunar_windows = g_list_prepend (lunar_windows, lp->data);

  return lunar_windows;
}


/**
 * lunar_application_has_windows:
 * @application : a #LunarApplication.
 *
 * Returns %TRUE if @application controls atleast one window.
 *
 * Return value: %TRUE if @application controls atleast one window.
 **/
gboolean
lunar_application_has_windows (LunarApplication *application)
{
  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), FALSE);

  /* FIXME: this is possible inconsisitent with lunar_application_get_windows() */
  return gtk_application_get_windows (GTK_APPLICATION (application)) != NULL;
}



/**
 * lunar_application_take_window:
 * @application : a #LunarApplication.
 * @window      : a #GtkWindow.
 *
 * Lets @application take over control of the specified @window.
 * @application will not exit until the last controlled #GtkWindow
 * is closed by the user.
 *
 * If the @window has no transient window, it will also create a
 * new #GtkWindowGroup for this window. This will make different
 * windows work independant (think gtk_dialog_run).
 **/
void
lunar_application_take_window (LunarApplication *application,
                                GtkWindow         *window)
{
  GtkWindowGroup *group;

  _lunar_return_if_fail (GTK_IS_WINDOW (window));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* only windows without a parent get a new window group */
  if (gtk_window_get_transient_for (window) == NULL && !gtk_window_has_group (window))
    {
      group = gtk_window_group_new ();
      gtk_window_group_add_window (group, window);
      g_object_weak_ref (G_OBJECT (window), (GWeakNotify) (void (*)(void)) g_object_unref, group);
    }

  /* add the ourselves to the window */
  gtk_window_set_application (window, GTK_APPLICATION (application));
}



/**
 * lunar_application_open_window:
 * @application      : a #LunarApplication.
 * @directory        : the directory to open.
 * @screen           : the #GdkScreen on which to open the window or %NULL
 *                     to open on the default screen.
 * @startup_id       : startup id from startup notification passed along
 *                     with dbus to make focus stealing work properly.
 * @force_new_window : set this flag to force a new window, even if misc-open-new-window-as-tab is set
 *
 * Opens a new #LunarWindow (or tab if preferred) for @application, displaying the
 * given @directory.
 *
 * Return value: the newly allocated #LunarWindow.
 **/
GtkWidget*
lunar_application_open_window (LunarApplication *application,
                                LunarFile        *directory,
                                GdkScreen         *screen,
                                const gchar       *startup_id,
                                gboolean           force_new_window)
{
  GList*     list;
  GtkWidget *window;
  gchar     *role;
  gboolean   open_new_window_as_tab;

  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), NULL);
  _lunar_return_val_if_fail (directory == NULL || LUNAR_IS_FILE (directory), NULL);
  _lunar_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), NULL);

  if (G_UNLIKELY (screen == NULL))
    screen = gdk_screen_get_default ();

  /* open as tab instead, if preferred */
  g_object_get (G_OBJECT (application->preferences), "misc-open-new-window-as-tab", &open_new_window_as_tab, NULL);
  if (G_UNLIKELY (!force_new_window && open_new_window_as_tab))
    {
      list = lunar_application_get_windows (application);
      if (list != NULL)  
        {
          /* this will be the topmost Window */
          list = g_list_last (list);

          if (directory != NULL)
              lunar_window_notebook_open_new_tab (LUNAR_WINDOW (list->data), directory);
          
          /* bring the window to front */
          gtk_window_present (list->data);
          return list->data;
        }
    }

  /* generate a unique role for the new window (for session management) */
  role = g_strdup_printf ("Lunar-%u-%u", (guint) time (NULL), (guint) g_random_int ());

  /* allocate the window */
  window = g_object_new (LUNAR_TYPE_WINDOW,
                         "role", role,
                         "screen", screen,
                         NULL);

  /* cleanup */
  g_free (role);

  /* set the startup id */
  if (startup_id != NULL)
    gtk_window_set_startup_id (GTK_WINDOW (window), startup_id);

  /* hook up the window */
  lunar_application_take_window (application, GTK_WINDOW (window));

  /* show the new window */
  gtk_widget_show (window);

  /* change the directory */
  if (directory != NULL)
    lunar_window_set_current_directory (LUNAR_WINDOW (window), directory);

  return window;
}



/**
 * lunar_application_bulk_rename:
 * @application       : a #LunarApplication.
 * @working_directory : the default working directory for the bulk rename dialog.
 * @filenames         : the list of file names that should be renamed or the empty
 *                      list to start with an empty rename dialog. The file names
 *                      can either be absolute paths, supported URIs or relative file
 *                      names to @working_directory.
 * @standalone        : %TRUE to display the bulk rename dialog like a standalone
 *                      application.
 * @screen            : the #GdkScreen on which to rename the @filenames or %NULL
 *                      to use the default #GdkScreen.
 * @startup_id        : startup notification id to properly finish startup notification
 *                      and focus the window when focus stealing is enabled or %NULL.
 * @error             : return location for errors or %NULL.
 *
 * Tries to popup the bulk rename dialog.
 *
 * Return value: %TRUE if the dialog was opened successfully, otherwise %FALSE.
 **/
gboolean
lunar_application_bulk_rename (LunarApplication *application,
                                const gchar       *working_directory,
                                gchar            **filenames,
                                gboolean           standalone,
                                GdkScreen         *screen,
                                const gchar       *startup_id,
                                GError           **error)
{
  LunarFile *current_directory = NULL;
  LunarFile *file;
  gboolean    result = FALSE;
  GList      *file_list = NULL;
  gchar      *filename;
  gchar      *empty_strv[] = { NULL };
  gint        n;

  _lunar_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), FALSE);
  _lunar_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  _lunar_return_val_if_fail (working_directory != NULL, FALSE);

  if (!filenames)
    filenames = empty_strv;

  /* determine the file for the working directory */
  current_directory = lunar_file_get_for_uri (working_directory, error);
  if (G_UNLIKELY (current_directory == NULL))
    return FALSE;

  /* check if we should use the default screen */
  if (G_LIKELY (screen == NULL))
    screen = gdk_screen_get_default ();

  /* try to process all filenames and convert them to the appropriate file objects */
  for (n = 0; filenames[n] != NULL; ++n)
    {
      /* check if the filename is an absolute path or looks like an URI */
      if (g_path_is_absolute (filenames[n]) || endo_str_looks_like_an_uri (filenames[n]))
        {
          /* determine the file for the filename directly */
          file = lunar_file_get_for_uri (filenames[n], error);
        }
      else
        {
          /* translate the filename into an absolute path first */
          filename = g_build_filename (working_directory, filenames[n], NULL);
          file = lunar_file_get_for_uri (filename, error);
          g_free (filename);
        }

      /* verify that we have a valid file */
      if (G_LIKELY (file != NULL))
        file_list = g_list_append (file_list, file);
      else
        break;
    }

  /* check if the filenames where resolved successfully */
  if (G_LIKELY (filenames[n] == NULL))
    {
      /* popup the bulk rename dialog */
      lunar_show_renamer_dialog (screen, current_directory, file_list, standalone, startup_id);

      /* we succeed */
      result = TRUE;
    }

  /* cleanup */
  g_object_unref (G_OBJECT (current_directory));
  lunar_g_file_list_free (file_list);

  return result;
}



static GtkWidget *
lunar_application_get_progress_dialog (LunarApplication *application)
{
  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), NULL);

  if (application->progress_dialog == NULL)
    {
      application->progress_dialog = lunar_progress_dialog_new ();

      g_object_add_weak_pointer (G_OBJECT (application->progress_dialog),
                                 (gpointer) &application->progress_dialog);

      lunar_application_take_window (application,
                                      GTK_WINDOW (application->progress_dialog));
    }

  return application->progress_dialog;
}



static void
lunar_application_process_files_finish (LunarBrowser *browser,
                                         LunarFile    *file,
                                         LunarFile    *target_file,
                                         GError        *error,
                                         gpointer       unused)
{
  LunarApplication *application = LUNAR_APPLICATION (browser);
  GdkScreen         *screen;
  const gchar       *startup_id;

  _lunar_return_if_fail (LUNAR_IS_BROWSER (browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* determine and reset the screen of the file */
  screen = g_object_get_qdata (G_OBJECT (file), lunar_application_screen_quark);
  g_object_set_qdata (G_OBJECT (file), lunar_application_screen_quark, NULL);

  /* determine and the startup id of the file */
  startup_id = g_object_get_qdata (G_OBJECT (file), lunar_application_startup_id_quark);

  /* check if resolving/mounting failed */
  if (error != NULL)
    {
      /* don't display cancel errors */
      if (error->domain != G_IO_ERROR || error->code != G_IO_ERROR_CANCELLED)
        {
          /* tell the user that we were unable to launch the file specified */
          lunar_dialogs_show_error (screen, error, _("Failed to open \"%s\""),
                                     lunar_file_get_display_name (file));
        }

      /* stop processing files */
      lunar_g_file_list_free (application->files_to_launch);
      application->files_to_launch = NULL;
    }
  else
    {
      /* try to open the file or directory */
      lunar_file_launch (target_file, screen, startup_id, &error);

      /* remove the file from the list */
      application->files_to_launch = g_list_delete_link (application->files_to_launch,
                                                         application->files_to_launch);

      /* release the file */
      g_object_unref (file);

      /* check if we have more files to process */
      if (application->files_to_launch != NULL)
        {
          /* continue processing the next file */
          lunar_application_process_files (application);
        }
    }

  /* unset the startup id */
  if (startup_id != NULL)
    g_object_set_qdata (G_OBJECT (file), lunar_application_startup_id_quark, NULL);

  /* release the application */
  g_application_release (G_APPLICATION (application));
}



static void
lunar_application_process_files (LunarApplication *application)
{
  LunarFile *file;
  GdkScreen  *screen;

  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* don't do anything if no files are to be processed */
  if (application->files_to_launch == NULL)
    return;

  /* take the next file from the queue */
  file = LUNAR_FILE (application->files_to_launch->data);

  /* retrieve the screen we need to launch the file on */
  screen = g_object_get_qdata (G_OBJECT (file), lunar_application_screen_quark);

  /* make sure to hold a reference to the application while file processing is going on */
  g_application_hold (G_APPLICATION (application));

  /* resolve the file and/or mount its enclosing volume
   * before handling it in the callback */
  lunar_browser_poke_file (LUNAR_BROWSER (application), file, screen,
                            lunar_application_process_files_finish, NULL);
}



/**
 * lunar_application_process_filenames:
 * @application       : a #LunarApplication.
 * @working_directory : the working directory relative to which the @filenames should
 *                      be interpreted.
 * @filenames         : a list of supported URIs or filenames. If a filename is specified
 *                      it can be either an absolute path or a path relative to the
 *                      @working_directory.
 * @screen            : the #GdkScreen on which to process the @filenames, or %NULL to
 *                      use the default screen.
 * @startup_id        : startup id to finish startup notification and properly focus the
 *                      window when focus stealing is enabled or %NULL.
 * @error             : return location for errors or %NULL.
 *
 * Tells @application to process the given @filenames and launch them appropriately.
 *
 * Return value: %TRUE on success, %FALSE if @error is set.
 **/
gboolean
lunar_application_process_filenames (LunarApplication *application,
                                      const gchar       *working_directory,
                                      gchar            **filenames,
                                      GdkScreen         *screen,
                                      const gchar       *startup_id,
                                      GError           **error)
{
  LunarFile *file;
  GError     *derror = NULL;
  gchar      *filename;
  GList      *file_list = NULL;
  GList      *lp;
  gint        n;

  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), FALSE);
  _lunar_return_val_if_fail (working_directory != NULL, FALSE);
  _lunar_return_val_if_fail (filenames != NULL, FALSE);
  _lunar_return_val_if_fail (*filenames != NULL, FALSE);
  _lunar_return_val_if_fail (screen == NULL || GDK_IS_SCREEN (screen), FALSE);
  _lunar_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  /* try to process all filenames and convert them to the appropriate file objects */
  for (n = 0; filenames[n] != NULL; ++n)
    {
      /* check if the filename is an absolute path or looks like an URI */
      if (g_path_is_absolute (filenames[n]) || endo_str_looks_like_an_uri (filenames[n]))
        {
          /* determine the file for the filename directly */
          file = lunar_file_get_for_uri (filenames[n], &derror);
        }
      else
        {
          /* translate the filename into an absolute path first */
          filename = g_build_filename (working_directory, filenames[n], NULL);
          file = lunar_file_get_for_uri (filename, &derror);
          g_free (filename);
        }

      /* verify that we have a valid file */
      if (G_LIKELY (file != NULL))
        {
          file_list = g_list_append (file_list, file);
        }
      else
        {
          /* tell the user that we were unable to launch the file specified */
          lunar_dialogs_show_error (screen, derror, _("Failed to open \"%s\""),
                                     filenames[n]);

          g_set_error (error, derror->domain, derror->code,
                       _("Failed to open \"%s\": %s"), filenames[n], derror->message);
          g_error_free (derror);

          lunar_g_file_list_free (file_list);

          return FALSE;
        }
    }

  /* loop over all files */
  for (lp = file_list; lp != NULL; lp = lp->next)
    {
      /* remember the screen to launch the file on */
      g_object_set_qdata (G_OBJECT (lp->data), lunar_application_screen_quark, screen);

      /* remember the startup id to set on the window */
      if (G_LIKELY (startup_id != NULL && *startup_id != '\0'))
        g_object_set_qdata_full (G_OBJECT (lp->data), lunar_application_startup_id_quark,
                                 g_strdup (startup_id), (GDestroyNotify) g_free);

      /* append the file to the list of files we need to launch */
      application->files_to_launch = g_list_append (application->files_to_launch,
                                                    lp->data);
    }

  /* start processing files if we have any to launch */
  if (application->files_to_launch != NULL)
    lunar_application_process_files (application);

  /* free the file list */
  g_list_free (file_list);

  return TRUE;
}



static void
lunar_application_rename_file_error (EndoJob            *job,
                                      GError            *error,
                                      LunarApplication *application)
{
  LunarFile *file;
  GdkScreen  *screen;

  _lunar_return_if_fail (ENDO_IS_JOB (job));
  _lunar_return_if_fail (error != NULL);
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  screen = g_object_get_qdata (G_OBJECT (job), lunar_application_screen_quark);
  file = g_object_get_qdata (G_OBJECT (job), lunar_application_file_quark);

  g_assert (screen != NULL);
  g_assert (file != NULL);

  lunar_dialogs_show_error (screen, error, _("Failed to rename \"%s\""),
                             lunar_file_get_display_name (file));
}



static void
lunar_application_rename_file_finished (EndoJob  *job,
                                         gpointer user_data)
{
  _lunar_return_if_fail (ENDO_IS_JOB (job));

  /* destroy the job object */
  g_object_unref (job);
}



/**
 * lunar_application_rename_file:
 * @application : a #LunarApplication.
 * @file        : a #LunarFile to be renamed.
 * @screen      : the #GdkScreen on which to open the window or %NULL
 *                to open on the default screen.
 * @startup_id  : startup id from startup notification passed along
 *                with dbus to make focus stealing work properly.
 *
 * Prompts the user to rename the @file.
 **/
void
lunar_application_rename_file (LunarApplication *application,
                                LunarFile        *file,
                                GdkScreen         *screen,
                                const gchar       *startup_id)
{
  LunarJob *job;

  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (GDK_IS_SCREEN (screen));
  _lunar_return_if_fail (startup_id != NULL);

  /* TODO pass the startup ID to the rename dialog */

  /* run the rename dialog */
  job = lunar_dialogs_show_rename_file (screen, file);
  if (G_LIKELY (job != NULL))
    {
      /* remember the screen and file */
      g_object_set_qdata (G_OBJECT (job), lunar_application_screen_quark, screen);
      g_object_set_qdata_full (G_OBJECT (job), lunar_application_file_quark,
                               g_object_ref (file), g_object_unref);

      /* handle rename errors */
      g_signal_connect (job, "error",
                        G_CALLBACK (lunar_application_rename_file_error), application);

      /* destroy the job when it has finished */
      g_signal_connect (job, "finished",
                        G_CALLBACK (lunar_application_rename_file_finished), NULL);
    }
}



/**
 * lunar_application_create_file:
 * @application      : a #LunarApplication.
 * @parent_directory : the #LunarFile of the parent directory.
 * @content_type     : the content type of the new file.
 * @screen           : the #GdkScreen on which to open the window or %NULL
 *                     to open on the default screen.
 * @startup_id       : startup id from startup notification passed along
 *                     with dbus to make focus stealing work properly.
 *
 * Prompts the user to create a new file or directory in @parent_directory.
 * The @content_type defines the icon and other elements in the filename
 * prompt dialog.
 **/
void
lunar_application_create_file (LunarApplication *application,
                                LunarFile        *parent_directory,
                                const gchar       *content_type,
                                GdkScreen         *screen,
                                const gchar       *startup_id)
{
  const gchar *dialog_title;
  const gchar *title;
  gboolean     is_directory;
  GList        path_list;
  gchar       *name;

  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (LUNAR_IS_FILE (parent_directory));
  _lunar_return_if_fail (content_type != NULL && *content_type != '\0');
  _lunar_return_if_fail (GDK_IS_SCREEN (screen));
  _lunar_return_if_fail (startup_id != NULL);

  is_directory = (g_strcmp0 (content_type, "inode/directory") == 0);

  if (is_directory)
    {
      dialog_title = _("New Folder");
      title = _("Create New Folder");
    }
  else
    {
      dialog_title = _("New File");
      title = _("Create New File");
    }

  /* TODO pass the startup ID to the rename dialog */

  /* ask the user to enter a name for the new folder */
  name = lunar_dialogs_show_create (screen, content_type, dialog_title, title);
  if (G_LIKELY (name != NULL))
    {
      path_list.data = g_file_get_child (lunar_file_get_file (parent_directory), name);
      path_list.next = path_list.prev = NULL;

      /* launch the operation */
      if (is_directory)
        lunar_application_mkdir (application, screen, &path_list, NULL);
      else
        lunar_application_creat (application, screen, &path_list, NULL, NULL);

      g_object_unref (path_list.data);
      g_free (name);
    }
}



/**
 * lunar_application_create_file_from_template:
 * @application      : a #LunarApplication.
 * @parent_directory : the #LunarFile of the parent directory.
 * @template_file    : the #LunarFile of the template.
 * @screen           : the #GdkScreen on which to open the window or %NULL
 *                     to open on the default screen.
 * @startup_id       : startup id from startup notification passed along
 *                     with dbus to make focus stealing work properly.
 *
 * Prompts the user to create a new file or directory in @parent_directory
 * from an existing @template_file which predefines the name and extension
 * in the create dialog.
 **/
void
lunar_application_create_file_from_template (LunarApplication *application,
                                              LunarFile        *parent_directory,
                                              LunarFile        *template_file,
                                              GdkScreen         *screen,
                                              const gchar       *startup_id)
{
  GList  target_path_list;
  gchar *name;
  gchar *title;

  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (LUNAR_IS_FILE (parent_directory));
  _lunar_return_if_fail (LUNAR_IS_FILE (template_file));
  _lunar_return_if_fail (GDK_IS_SCREEN (screen));
  _lunar_return_if_fail (startup_id != NULL);

  /* generate a title for the create dialog */
  title = g_strdup_printf (_("Create Document from template \"%s\""),
                           lunar_file_get_display_name (template_file));

  /* TODO pass the startup ID to the rename dialog */

  /* ask the user to enter a name for the new document */
  name = lunar_dialogs_show_create (screen,
                                     lunar_file_get_content_type (template_file),
                                     lunar_file_get_display_name (template_file),
                                     title);
  if (G_LIKELY (name != NULL))
    {
      /* fake the target path list */
      target_path_list.data = g_file_get_child (lunar_file_get_file (parent_directory), name);
      target_path_list.next = target_path_list.prev = NULL;

      /* launch the operation */
      lunar_application_creat (application, screen,
                                &target_path_list,
                                lunar_file_get_file (template_file),
                                NULL);

      /* release the target path */
      g_object_unref (target_path_list.data);

      /* release the file name */
      g_free (name);
    }

  /* clean up */
  g_free (title);
}



/**
 * lunar_application_copy_to:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the lst of #GFile<!---->s that should be copied.
 * @target_file_list  : the list of #GFile<!---->s where files should be copied to.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Copies all files from @source_file_list to their locations specified in
 * @target_file_list.
 *
 * @source_file_list and @target_file_list must be of the same length.
 **/
void
lunar_application_copy_to (LunarApplication *application,
                            gpointer           parent,
                            GList             *source_file_list,
                            GList             *target_file_list,
                            GClosure          *new_files_closure)
{
  _lunar_return_if_fail (g_list_length (source_file_list) == g_list_length (target_file_list));
  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* launch the operation */
  lunar_application_launch (application, parent, "edit-copy",
                             _("Copying files..."), lunar_io_jobs_copy_files,
                             source_file_list, target_file_list, FALSE, TRUE, new_files_closure);
}



/**
 * lunar_application_copy_into:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the list of #GFile<!---->s that should be copied.
 * @target_file       : the #GFile to the target directory.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Copies all files referenced by the @source_file_list to the directory
 * referenced by @target_file. This method takes care of all user interaction.
 **/
void
lunar_application_copy_into (LunarApplication *application,
                              gpointer           parent,
                              GList             *source_file_list,
                              GFile             *target_file,
                              GClosure          *new_files_closure)
{
  gchar *display_name;
  gchar *title;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (G_IS_FILE (target_file));

   /* generate a title for the progress dialog */
   display_name = lunar_file_cached_display_name (target_file);
   title = g_strdup_printf (_("Copying files to \"%s\"..."), display_name);
   g_free (display_name);

  /* collect the target files and launch the job */
  lunar_application_collect_and_launch (application, parent, "edit-copy",
                                         title, lunar_io_jobs_copy_files,
                                         source_file_list, target_file,
                                         FALSE, TRUE,
                                         new_files_closure);

  /* free the title */
  g_free (title);
}



/**
 * lunar_application_link_into:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the list of #GFile<!---->s that should be symlinked.
 * @target_file       : the target directory.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Symlinks all files referenced by the @source_file_list to the directory
 * referenced by @target_file. This method takes care of all user
 * interaction.
 **/
void
lunar_application_link_into (LunarApplication *application,
                              gpointer           parent,
                              GList             *source_file_list,
                              GFile             *target_file,
                              GClosure          *new_files_closure)
{
  gchar *display_name;
  gchar *title;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (G_IS_FILE (target_file));

  /* generate a title for the progress dialog */
  display_name = lunar_file_cached_display_name (target_file);
  title = g_strdup_printf (_("Creating symbolic links in \"%s\"..."), display_name);
  g_free (display_name);

  /* collect the target files and launch the job */
  lunar_application_collect_and_launch (application, parent, "insert-link",
                                         title, lunar_io_jobs_link_files,
                                         source_file_list, target_file,
                                         FALSE, TRUE,
                                         new_files_closure);

  /* free the title */
  g_free (title);
}



/**
 * lunar_application_move_into:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @source_file_list  : the list of #GFile<!---->s that should be moved.
 * @target_file       : the target directory.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Moves all files referenced by the @source_file_list to the directory
 * referenced by @target_file. This method takes care of all user
 * interaction.
 **/
void
lunar_application_move_into (LunarApplication *application,
                              gpointer           parent,
                              GList             *source_file_list,
                              GFile             *target_file,
                              GClosure          *new_files_closure)
{
  gchar *display_name;
  gchar *title;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (target_file != NULL);

  /* launch the appropriate operation depending on the target file */
  if (lunar_g_file_is_trashed (target_file))
    {
      lunar_application_trash (application, parent, source_file_list);
    }
  else
    {
      /* generate a title for the progress dialog */
      display_name = lunar_file_cached_display_name (target_file);
      title = g_strdup_printf (_("Moving files into \"%s\"..."), display_name);
      g_free (display_name);

      /* collect the target files and launch the job */
      lunar_application_collect_and_launch (application, parent,
                                             "stock_folder-move", title,
                                             lunar_io_jobs_move_files,
                                             source_file_list, target_file,
                                             TRUE, TRUE,
                                             new_files_closure);

      /* free the title */
      g_free (title);
    }
}



static LunarJob *
unlink_stub (GList *source_path_list,
             GList *target_path_list)
{
  return lunar_io_jobs_unlink_files (source_path_list);
}



/**
 * lunar_application_unlink_files:
 * @application : a #LunarApplication.
 * @parent      : a #GdkScreen, a #GtkWidget or %NULL.
 * @file_list   : the list of #LunarFile<!---->s that should be deleted.
 * @permanently : whether to unlink the files permanently.
 *
 * Deletes all files in the @file_list and takes care of all user interaction.
 *
 * If the user pressed the shift key while triggering the delete action,
 * the files will be deleted permanently (after confirming the action),
 * otherwise the files will be moved to the trash.
 **/
void
lunar_application_unlink_files (LunarApplication *application,
                                 gpointer           parent,
                                 GList             *file_list,
                                 gboolean           permanently)
{
  GtkWidget *dialog;
  GtkWindow *window;
  GdkScreen *screen;
  GList     *path_list = NULL;
  GList     *lp;
  gchar     *message;
  guint      n_path_list = 0;
  gint       response;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* determine the paths for the files */
  for (lp = g_list_last (file_list); lp != NULL; lp = lp->prev, ++n_path_list)
    {
      /* prepend the path to the path list */
      path_list = lunar_g_file_list_prepend (path_list, lunar_file_get_file (lp->data));

      /* permanently delete if at least one of the file is not a local
       * file (e.g. resides in the trash) or cannot be trashed */
      if (!lunar_file_is_local (lp->data) || !lunar_file_can_be_trashed (lp->data))
        permanently = TRUE;
    }

  /* nothing to do if we don't have any paths */
  if (G_UNLIKELY (n_path_list == 0))
    return;

  /* ask the user to confirm if deleting permanently */
  if (G_UNLIKELY (permanently))
    {
      /* parse the parent pointer */
      screen = lunar_util_parse_parent (parent, &window);

      /* generate the question to confirm the delete operation */
      if (G_LIKELY (n_path_list == 1))
        {
          message = g_strdup_printf (_("Are you sure that you want to\npermanently delete \"%s\"?"),
                                     lunar_file_get_display_name (LUNAR_FILE (file_list->data)));
        }
      else
        {
          message = g_strdup_printf (ngettext ("Are you sure that you want to permanently\ndelete the selected file?",
                                               "Are you sure that you want to permanently\ndelete the %u selected files?",
                                               n_path_list),
                                     n_path_list);
        }

      /* ask the user to confirm the delete operation */
      dialog = gtk_message_dialog_new (window,
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_NONE,
                                       "%s", message);
      if (G_UNLIKELY (window == NULL && screen != NULL))
        gtk_window_set_screen (GTK_WINDOW (dialog), screen);
      gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                              _("_Cancel"), GTK_RESPONSE_CANCEL,
                              _("_Delete"), GTK_RESPONSE_YES,
                              NULL);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                _("If you delete a file, it is permanently lost."));
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (message);

      /* perform the delete operation */
      if (G_LIKELY (response == GTK_RESPONSE_YES))
        {
          /* launch the "Delete" operation */
          lunar_application_launch (application, parent, "edit-delete",
                                     _("Deleting files..."), unlink_stub,
                                     path_list, path_list, TRUE, FALSE, NULL);
        }
    }
  else
    {
      /* launch the "Move to Trash" operation */
      lunar_application_trash (application, parent, path_list);
    }

  /* release the path list */
  lunar_g_file_list_free (path_list);
}



static LunarJob *
trash_stub (GList *source_file_list,
            GList *target_file_list)
{
  return lunar_io_jobs_trash_files (source_file_list);
}



void
lunar_application_trash (LunarApplication *application,
                          gpointer           parent,
                          GList             *file_list)
{
  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (file_list != NULL);

  lunar_application_launch (application, parent, "user-trash-full",
                             _("Moving files into the trash..."), trash_stub,
                             file_list, NULL, TRUE, FALSE, NULL);
}



static LunarJob *
creat_stub (GList *template_file,
            GList *target_path_list)
{
   _lunar_return_val_if_fail (template_file->data == NULL || G_IS_FILE (template_file->data), NULL);
  return lunar_io_jobs_create_files (target_path_list, template_file->data);
}



/**
 * lunar_application_creat:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @file_list         : the list of files to create.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Creates empty files for all #GFile<!---->s listed in @file_list. This
 * method takes care of all user interaction.
 **/
void
lunar_application_creat (LunarApplication *application,
                          gpointer           parent,
                          GList             *file_list,
                          GFile             *template_file,
                          GClosure          *new_files_closure)
{
  GList template_list;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  template_list.next = template_list.prev = NULL;
  template_list.data = template_file;

  /* launch the operation */
  lunar_application_launch (application, parent, "document-new",
                             _("Creating files..."), creat_stub,
                             &template_list, file_list, FALSE, TRUE, new_files_closure);
}



static LunarJob *
mkdir_stub (GList *source_path_list,
            GList *target_path_list)
{
  return lunar_io_jobs_make_directories (source_path_list);
}



/**
 * lunar_application_mkdir:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @file_list         : the list of directories to create.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Creates all directories referenced by the @file_list. This method takes care of all user
 * interaction.
 **/
void
lunar_application_mkdir (LunarApplication *application,
                          gpointer           parent,
                          GList             *file_list,
                          GClosure          *new_files_closure)
{
  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  /* launch the operation */
  lunar_application_launch (application, parent, "folder-new",
                             _("Creating directories..."), mkdir_stub,
                             file_list, file_list, TRUE, FALSE, new_files_closure);
}



/**
 * lunar_application_empty_trash:
 * @application : a #LunarApplication.
 * @parent      : the parent, which can either be the associated
 *                #GtkWidget, the screen on which display the
 *                progress and the confirmation, or %NULL.
 *
 * Deletes all files and folders in the Trash after asking
 * the user to confirm the operation.
 **/
void
lunar_application_empty_trash (LunarApplication *application,
                                gpointer           parent,
                                const gchar       *startup_id)
{
  GtkWidget *dialog;
  GtkWindow *window;
  GdkScreen *screen;
  GList      file_list;
  gint       response;

  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));
  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));

  /* parse the parent pointer */
  screen = lunar_util_parse_parent (parent, &window);

  /* ask the user to confirm the operation */
  dialog = gtk_message_dialog_new (window,
                                   GTK_DIALOG_MODAL
                                   | GTK_DIALOG_DESTROY_WITH_PARENT,
                                   GTK_MESSAGE_QUESTION,
                                   GTK_BUTTONS_NONE,
                                   _("Remove all files and folders from the Trash?"));
  if (G_UNLIKELY (window == NULL && screen != NULL))
    gtk_window_set_screen (GTK_WINDOW (dialog), screen);
  gtk_window_set_startup_id (GTK_WINDOW (dialog), startup_id);
  gtk_dialog_add_buttons (GTK_DIALOG (dialog),
                          _("_Cancel"), GTK_RESPONSE_CANCEL,
                          _("_Empty Trash"), GTK_RESPONSE_YES,
                          NULL);
  gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
  gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                            _("If you choose to empty the Trash, all items in it will be permanently lost. "
                                              "Please note that you can also delete them separately."));
  response = gtk_dialog_run (GTK_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  /* check if the user confirmed */
  if (G_LIKELY (response == GTK_RESPONSE_YES))
    {
      /* fake a path list with only the trash root (the root
       * folder itself will never be unlinked, so this is safe)
       */
      file_list.data = lunar_g_file_new_for_trash ();
      file_list.next = NULL;
      file_list.prev = NULL;

      /* launch the operation */
      lunar_application_launch (application, parent, "user-trash",
                                 _("Emptying the Trash..."),
                                 unlink_stub, &file_list, NULL, TRUE, FALSE, NULL);

      /* cleanup */
      g_object_unref (file_list.data);
    }
}



/**
 * lunar_application_restore_files:
 * @application       : a #LunarApplication.
 * @parent            : a #GdkScreen, a #GtkWidget or %NULL.
 * @trash_file_list   : a #GList of #LunarFile<!---->s in the trash.
 * @new_files_closure : a #GClosure to connect to the job's "new-files" signal,
 *                      which will be emitted when the job finishes with the
 *                      list of #GFile<!---->s created by the job, or
 *                      %NULL if you're not interested in the signal.
 *
 * Restores all #LunarFile<!---->s in the @trash_file_list to their original
 * location.
 **/
void
lunar_application_restore_files (LunarApplication *application,
                                  gpointer           parent,
                                  GList             *trash_file_list,
                                  GClosure          *new_files_closure)
{
  const gchar *original_uri;
  GError      *err = NULL;
  GFile       *target_path;
  GList       *source_path_list = NULL;
  GList       *target_path_list = NULL;
  GList       *lp;

  _lunar_return_if_fail (parent == NULL || GDK_IS_SCREEN (parent) || GTK_IS_WIDGET (parent));
  _lunar_return_if_fail (LUNAR_IS_APPLICATION (application));

  for (lp = trash_file_list; lp != NULL; lp = lp->next)
    {
      original_uri = lunar_file_get_original_path (lp->data);
      if (G_UNLIKELY (original_uri == NULL))
        {
          /* no OriginalPath, impossible to continue */
          g_set_error (&err, G_FILE_ERROR, G_FILE_ERROR_INVAL,
                       _("Failed to determine the original path for \"%s\""),
                       lunar_file_get_display_name (lp->data));
          break;
        }

      /* TODO we might have to distinguish between URIs and paths here */
      target_path = g_file_new_for_commandline_arg (original_uri);

      source_path_list = lunar_g_file_list_append (source_path_list, lunar_file_get_file (lp->data));
      target_path_list = lunar_g_file_list_append (target_path_list, target_path);

      g_object_unref (target_path);
    }

  if (G_UNLIKELY (err != NULL))
    {
      /* display an error dialog */
      lunar_dialogs_show_error (parent, err, _("Could not restore \"%s\""),
                                 lunar_file_get_display_name (lp->data));
      g_error_free (err);
    }
  else
    {
      /* launch the operation */
      lunar_application_launch (application, parent, "stock_folder-move",
                                 _("Restoring files..."), lunar_io_jobs_restore_files,
                                 source_path_list, target_path_list, TRUE, TRUE, new_files_closure);
    }

  /* free path lists */
  lunar_g_file_list_free (source_path_list);
  lunar_g_file_list_free (target_path_list);
}



LunarThumbnailCache *
lunar_application_get_thumbnail_cache (LunarApplication *application)
{
  _lunar_return_val_if_fail (LUNAR_IS_APPLICATION (application), NULL);

  if (application->thumbnail_cache == NULL)
    application->thumbnail_cache = lunar_thumbnail_cache_new ();

  return g_object_ref (application->thumbnail_cache);
}


