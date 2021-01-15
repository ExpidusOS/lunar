/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
 * Copyright (c) 2020 Alexander Schwinn <alexxcons@expidus.org>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <lunar/lunar-application.h>
#include <lunar/lunar-browser.h>
#include <lunar/lunar-chooser-dialog.h>
#include <lunar/lunar-clipboard-manager.h>
#include <lunar/lunar-dialogs.h>
#include <lunar/lunar-gio-extensions.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-gtk-extensions.h>
#include <lunar/lunar-icon-factory.h>
#include <lunar/lunar-io-scan-directory.h>
#include <lunar/lunar-launcher.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-properties-dialog.h>
#include <lunar/lunar-renamer-dialog.h>
#include <lunar/lunar-sendto-model.h>
#include <lunar/lunar-shortcuts-pane.h>
#include <lunar/lunar-simple-job.h>
#include <lunar/lunar-device-monitor.h>
#include <lunar/lunar-tree-view.h>
#include <lunar/lunar-util.h>
#include <lunar/lunar-window.h>

#include <libexpidus1ui/libexpidus1ui.h>



/**
 * SECTION:lunar-launcher
 * @Short_description: Manages creation and execution of menu-item
 * @Title: LunarLauncher
 *
 * The #LunarLauncher class manages the creation and execution of menu-item which are used by multiple menus.
 * The management is done in a central way to prevent code duplication on various places.
 * ExpidusGtkActionEntry is used in order to define a list of the managed items and ease the setup of single items.
 *
 * #LunarLauncher implements the #LunarNavigator interface in order to use the "open in new tab" and "change directory" service.
 * It as well tracks the current directory via #LunarNavigator.
 *
 * #LunarLauncher implements the #LunarComponent interface in order to track the currently selected files.
 * Based on to the current selection (and some other criteria), some menu items will not be shown, or will be insensitive.
 *
 * Files which are opened via #LunarLauncher are poked first in order to e.g do missing mount operations.
 *
 * As well menu-item related services, like activation of selected files and opening tabs/new windows,
 * are provided by #LunarLauncher.
 *
 * It is required to keep an instance of #LunarLauncher open, in order to listen to accellerators which target
 * menu-items managed by #LunarLauncher.
 * Typically a single instance of #LunarLauncher is provided by each #LunarWindow.
 */



typedef struct _LunarLauncherPokeData LunarLauncherPokeData;



/* Property identifiers */
enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
  PROP_SELECTED_FILES,
  PROP_WIDGET,
  PROP_SELECT_FILES_CLOSURE,
  PROP_SELECTED_DEVICE,
  N_PROPERTIES
};



static void                    lunar_launcher_component_init             (LunarComponentIface           *iface);
static void                    lunar_launcher_navigator_init             (LunarNavigatorIface           *iface);
static void                    lunar_launcher_dispose                    (GObject                        *object);
static void                    lunar_launcher_finalize                   (GObject                        *object);
static void                    lunar_launcher_get_property               (GObject                        *object,
                                                                           guint                           prop_id,
                                                                           GValue                         *value,
                                                                           GParamSpec                     *pspec);
static void                    lunar_launcher_set_property               (GObject                        *object,
                                                                           guint                           prop_id,
                                                                           const GValue                   *value,
                                                                           GParamSpec                     *pspec);
static LunarFile             *lunar_launcher_get_current_directory      (LunarNavigator                *navigator);
static void                    lunar_launcher_set_current_directory      (LunarNavigator                *navigator,
                                                                           LunarFile                     *current_directory);
static void                    lunar_launcher_set_selected_files         (LunarComponent                *component,
                                                                           GList                          *selected_files);
static void                    lunar_launcher_execute_files              (LunarLauncher                 *launcher,
                                                                           GList                          *files);
static void                    lunar_launcher_open_file                  (LunarLauncher                 *launcher,
                                                                           LunarFile                     *file,
                                                                           GAppInfo                       *application_to_use);
static void                    lunar_launcher_open_files                 (LunarLauncher                 *launcher,
                                                                           GList                          *files,
                                                                           GAppInfo                       *application_to_use);
static void                    lunar_launcher_open_paths                 (GAppInfo                       *app_info,
                                                                           GList                          *file_list,
                                                                           LunarLauncher                 *launcher);
static void                    lunar_launcher_open_windows               (LunarLauncher                 *launcher,
                                                                           GList                          *directories);
static void                    lunar_launcher_poke                       (LunarLauncher                 *launcher,
                                                                           GAppInfo                       *application_to_use,
                                                                           LunarLauncherFolderOpenAction  folder_open_action);
static void                    lunar_launcher_poke_device_finish         (LunarBrowser                  *browser,
                                                                           LunarDevice                   *volume,
                                                                           LunarFile                     *mount_point,
                                                                           GError                         *error,
                                                                           gpointer                        user_data,
                                                                           gboolean                        cancelled);
static void                    lunar_launcher_poke_files_finish          (LunarBrowser                  *browser,
                                                                           LunarFile                     *file,
                                                                           LunarFile                     *target_file,
                                                                           GError                         *error,
                                                                           gpointer                        user_data);
static LunarLauncherPokeData *lunar_launcher_poke_data_new              (GList                          *files_to_poke,
                                                                           GAppInfo                       *application_to_use,
                                                                           LunarLauncherFolderOpenAction  folder_open_action);
static void                    lunar_launcher_poke_data_free             (LunarLauncherPokeData         *data);
static void                    lunar_launcher_widget_destroyed           (LunarLauncher                 *launcher,
                                                                           GtkWidget                      *widget);
static void                    lunar_launcher_action_open                (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_open_in_new_tabs    (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_open_in_new_windows (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_open_with_other     (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_sendto_desktop      (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_properties          (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_sendto_device       (LunarLauncher                 *launcher,
                                                                           GObject                        *object);
static void                    lunar_launcher_action_add_shortcuts       (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_make_link           (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_duplicate           (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_rename              (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_restore             (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_move_to_trash       (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_delete              (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_trash_delete        (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_empty_trash         (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_cut                 (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_copy                (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_paste               (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_paste_into_folder   (LunarLauncher                 *launcher);
static void                    lunar_launcher_sendto_device              (LunarLauncher                 *launcher,
                                                                           LunarDevice                   *device);
static void                    lunar_launcher_sendto_mount_finish        (LunarDevice                   *device,
                                                                           const GError                   *error,
                                                                           gpointer                        user_data);
static GtkWidget              *lunar_launcher_build_sendto_submenu       (LunarLauncher                 *launcher);
static void                    lunar_launcher_menu_item_activated        (LunarLauncher                 *launcher,
                                                                           GtkWidget                      *menu_item);
static void                    lunar_launcher_action_create_folder       (LunarLauncher                 *launcher);
static void                    lunar_launcher_action_create_document     (LunarLauncher                 *launcher,
                                                                           GtkWidget                      *menu_item);
static GtkWidget              *lunar_launcher_create_document_submenu_new(LunarLauncher                 *launcher);



struct _LunarLauncherClass
{
  GObjectClass __parent__;
};

struct _LunarLauncher
{
  GObject __parent__;

  LunarFile             *current_directory;
  GList                  *files_to_process;
  LunarDevice           *device_to_process;

  gint                    n_files_to_process;
  gint                    n_directories_to_process;
  gint                    n_executables_to_process;
  gint                    n_regulars_to_process;
  gboolean                files_to_process_trashable;
  gboolean                files_are_selected;
  gboolean                single_directory_to_process;

  LunarFile             *single_folder;
  LunarFile             *parent_folder;

  GClosure               *select_files_closure;

  LunarPreferences      *preferences;

  /* Parent widget which holds the instance of the launcher */
  GtkWidget              *widget;
};

static GQuark lunar_launcher_appinfo_quark;
static GQuark lunar_launcher_device_quark;
static GQuark lunar_launcher_file_quark;

struct _LunarLauncherPokeData
{
  GList                          *files_to_poke;
  GList                          *files_poked;
  GAppInfo                       *application_to_use;
  LunarLauncherFolderOpenAction  folder_open_action;
};

static GParamSpec *launcher_props[N_PROPERTIES] = { NULL, };

static ExpidusGtkActionEntry lunar_launcher_action_entries[] =
{
    { LUNAR_LAUNCHER_ACTION_OPEN,             "<Actions>/LunarLauncher/open",                    "<Primary>O",        EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                                   NULL,                                                                                            "document-open",        G_CALLBACK (lunar_launcher_action_open),                },
    { LUNAR_LAUNCHER_ACTION_EXECUTE,          "<Actions>/LunarLauncher/execute",                 "",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                                   NULL,                                                                                            "system-run",           G_CALLBACK (lunar_launcher_action_open),                },
    { LUNAR_LAUNCHER_ACTION_OPEN_IN_TAB,      "<Actions>/LunarLauncher/open-in-new-tab",         "<Primary><shift>P", EXPIDUS_GTK_MENU_ITEM,       NULL,                                   NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_open_in_new_tabs),    },
    { LUNAR_LAUNCHER_ACTION_OPEN_IN_WINDOW,   "<Actions>/LunarLauncher/open-in-new-window",      "<Primary><shift>O", EXPIDUS_GTK_MENU_ITEM,       NULL,                                   NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_open_in_new_windows), },
    { LUNAR_LAUNCHER_ACTION_OPEN_WITH_OTHER,  "<Actions>/LunarLauncher/open-with-other",         "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("Open With Other _Application..."), N_ ("Choose another application with which to open the selected file"),                          NULL,                   G_CALLBACK (lunar_launcher_action_open_with_other),     },

    /* For backward compatibility the old accel paths are re-used. Currently not possible to automatically migrate to new accel paths. */
    /* Waiting for https://gitlab.gnome.org/GNOME/gtk/issues/2375 to be able to fix that */
    { LUNAR_LAUNCHER_ACTION_SENDTO_MENU,      "<Actions>/LunarWindow/sendto-menu",               "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("_Send To"),                        NULL,                                                                                            NULL,                   NULL,                                                    },
    { LUNAR_LAUNCHER_ACTION_SENDTO_SHORTCUTS, "<Actions>/LunarShortcutsPane/sendto-shortcuts",   "",                  EXPIDUS_GTK_MENU_ITEM,       NULL,                                   NULL,                                                                                            "bookmark-new",         G_CALLBACK (lunar_launcher_action_add_shortcuts),       },
    { LUNAR_LAUNCHER_ACTION_SENDTO_DESKTOP,   "<Actions>/LunarLauncher/sendto-desktop",          "",                  EXPIDUS_GTK_MENU_ITEM,       NULL,                                   NULL,                                                                                            "user-desktop",         G_CALLBACK (lunar_launcher_action_sendto_desktop),      },
    { LUNAR_LAUNCHER_ACTION_PROPERTIES,       "<Actions>/LunarStandardView/properties",          "<Alt>Return",       EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Properties..."),                  N_ ("View the properties of the selected file"),                                                 "document-properties",  G_CALLBACK (lunar_launcher_action_properties),          },
    { LUNAR_LAUNCHER_ACTION_MAKE_LINK,        "<Actions>/LunarStandardView/make-link",           "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("Ma_ke Link"),                      NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_make_link),           },
    { LUNAR_LAUNCHER_ACTION_DUPLICATE,        "<Actions>/LunarStandardView/duplicate",           "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("Du_plicate"),                      NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_duplicate),           },
    { LUNAR_LAUNCHER_ACTION_RENAME,           "<Actions>/LunarStandardView/rename",              "F2",                EXPIDUS_GTK_MENU_ITEM,       N_ ("_Rename..."),                      NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_rename),              },
    { LUNAR_LAUNCHER_ACTION_EMPTY_TRASH,      "<Actions>/LunarWindow/empty-trash",               "",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Empty Trash"),                    N_ ("Delete all files and folders in the Trash"),                                                NULL,                   G_CALLBACK (lunar_launcher_action_empty_trash),         },
    { LUNAR_LAUNCHER_ACTION_CREATE_FOLDER,    "<Actions>/LunarStandardView/create-folder",       "<Primary><shift>N", EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Create _Folder..."),               N_ ("Create an empty folder within the current folder"),                                         "folder-new",           G_CALLBACK (lunar_launcher_action_create_folder),       },
    { LUNAR_LAUNCHER_ACTION_CREATE_DOCUMENT,  "<Actions>/LunarStandardView/create-document",     "",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Create _Document"),                N_ ("Create a new document from a template"),                                                    "document-new",         G_CALLBACK (NULL),                                       },

    { LUNAR_LAUNCHER_ACTION_RESTORE,          "<Actions>/LunarLauncher/restore",                 "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("_Restore"),                        NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_restore),             },
    { LUNAR_LAUNCHER_ACTION_MOVE_TO_TRASH,    "<Actions>/LunarLauncher/move-to-trash",           "",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Mo_ve to Trash"),                  NULL,                                                                                            "user-trash",           G_CALLBACK (lunar_launcher_action_move_to_trash),       },
    { LUNAR_LAUNCHER_ACTION_DELETE,           "<Actions>/LunarLauncher/delete",                  "",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Delete"),                         NULL,                                                                                            "edit-delete",          G_CALLBACK (lunar_launcher_action_delete),              },
    { LUNAR_LAUNCHER_ACTION_DELETE,           "<Actions>/LunarLauncher/delete-2",                "<Shift>Delete",     EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                                   NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_delete),              },
    { LUNAR_LAUNCHER_ACTION_DELETE,           "<Actions>/LunarLauncher/delete-3",                "<Shift>KP_Delete",  EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                                   NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_delete),              },
    { LUNAR_LAUNCHER_ACTION_TRASH_DELETE,     "<Actions>/LunarLauncher/trash-delete",            "Delete",            EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                                   NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_trash_delete),        },
    { LUNAR_LAUNCHER_ACTION_TRASH_DELETE,     "<Actions>/LunarLauncher/trash-delete-2",          "KP_Delete",         EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                                   NULL,                                                                                            NULL,                   G_CALLBACK (lunar_launcher_action_trash_delete),        },
    { LUNAR_LAUNCHER_ACTION_PASTE,            "<Actions>/LunarLauncher/paste",                   "<Primary>V",        EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Paste"),                          N_ ("Move or copy files previously selected by a Cut or Copy command"),                          "edit-paste",           G_CALLBACK (lunar_launcher_action_paste),               },
    { LUNAR_LAUNCHER_ACTION_PASTE_INTO_FOLDER,NULL,                                               "",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Paste Into Folder"),              N_ ("Move or copy files previously selected by a Cut or Copy command into the selected folder"), "edit-paste",           G_CALLBACK (lunar_launcher_action_paste_into_folder),   },
    { LUNAR_LAUNCHER_ACTION_COPY,             "<Actions>/LunarLauncher/copy",                    "<Primary>C",        EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Copy"),                           N_ ("Prepare the selected files to be copied with a Paste command"),                             "edit-copy",            G_CALLBACK (lunar_launcher_action_copy),                },
    { LUNAR_LAUNCHER_ACTION_CUT,              "<Actions>/LunarLauncher/cut",                     "<Primary>X",        EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Cu_t"),                            N_ ("Prepare the selected files to be moved with a Paste command"),                              "edit-cut",             G_CALLBACK (lunar_launcher_action_cut),                 },

    { LUNAR_LAUNCHER_ACTION_MOUNT,            NULL,                                               "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("_Mount"),                          N_ ("Mount the selected device"),                                                                NULL,                   G_CALLBACK (lunar_launcher_action_open),                },
    { LUNAR_LAUNCHER_ACTION_UNMOUNT,          NULL,                                               "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("_Unmount"),                        N_ ("Unmount the selected device"),                                                              NULL,                   G_CALLBACK (lunar_launcher_action_unmount),             },
    { LUNAR_LAUNCHER_ACTION_EJECT,            NULL,                                               "",                  EXPIDUS_GTK_MENU_ITEM,       N_ ("_Eject"),                          N_ ("Eject the selected device"),                                                                NULL,                   G_CALLBACK (lunar_launcher_action_eject),               },
};

#define get_action_entry(id) expidus_gtk_get_action_entry_by_id(lunar_launcher_action_entries,G_N_ELEMENTS(lunar_launcher_action_entries),id)


G_DEFINE_TYPE_WITH_CODE (LunarLauncher, lunar_launcher, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (LUNAR_TYPE_BROWSER, NULL)
    G_IMPLEMENT_INTERFACE (LUNAR_TYPE_NAVIGATOR, lunar_launcher_navigator_init)
    G_IMPLEMENT_INTERFACE (LUNAR_TYPE_COMPONENT, lunar_launcher_component_init))



static void
lunar_launcher_class_init (LunarLauncherClass *klass)
{
  GObjectClass *gobject_class;
  gpointer      g_iface;

  /* determine all used quarks */
  lunar_launcher_appinfo_quark = g_quark_from_static_string ("lunar-launcher-appinfo");
  lunar_launcher_device_quark = g_quark_from_static_string ("lunar-launcher-device");
  lunar_launcher_file_quark = g_quark_from_static_string ("lunar-launcher-file");

  expidus_gtk_translate_action_entries (lunar_launcher_action_entries, G_N_ELEMENTS (lunar_launcher_action_entries));

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = lunar_launcher_dispose;
  gobject_class->finalize = lunar_launcher_finalize;
  gobject_class->get_property = lunar_launcher_get_property;
  gobject_class->set_property = lunar_launcher_set_property;

  /**
   * LunarLauncher:widget:
   *
   * The #GtkWidget with which this launcher is associated.
   **/
  launcher_props[PROP_WIDGET] =
      g_param_spec_object ("widget",
                           "widget",
                           "widget",
                           GTK_TYPE_WIDGET,
                           ENDO_PARAM_WRITABLE);

  /**
   * LunarLauncher:select-files-closure:
   *
   * The #GClosure which will be called if the selected file should be updated after a launcher operation
   **/
  launcher_props[PROP_SELECT_FILES_CLOSURE] =
     g_param_spec_pointer ("select-files-closure",
                           "select-files-closure",
                           "select-files-closure",
                           G_PARAM_WRITABLE
                           | G_PARAM_CONSTRUCT_ONLY);

  /**
   * LunarLauncher:select-device:
   *
   * The #LunarDevice which currently is selected (or NULL if no #LunarDevice is selected)
   **/
  launcher_props[PROP_SELECTED_DEVICE] =
     g_param_spec_pointer ("selected-device",
                           "selected-device",
                           "selected-device",
                           G_PARAM_WRITABLE);

  /* Override LunarNavigator's properties */
  g_iface = g_type_default_interface_peek (LUNAR_TYPE_NAVIGATOR);
  launcher_props[PROP_CURRENT_DIRECTORY] =
      g_param_spec_override ("current-directory",
                             g_object_interface_find_property (g_iface, "current-directory"));

  /* Override LunarComponent's properties */
  g_iface = g_type_default_interface_peek (LUNAR_TYPE_COMPONENT);
  launcher_props[PROP_SELECTED_FILES] =
      g_param_spec_override ("selected-files",
                             g_object_interface_find_property (g_iface, "selected-files"));

  /* install properties */
  g_object_class_install_properties (gobject_class, N_PROPERTIES, launcher_props);
}



static void
lunar_launcher_component_init (LunarComponentIface *iface)
{
  iface->get_selected_files = (gpointer) endo_noop_null;
  iface->set_selected_files = lunar_launcher_set_selected_files;
}



static void
lunar_launcher_navigator_init (LunarNavigatorIface *iface)
{
  iface->get_current_directory = lunar_launcher_get_current_directory;
  iface->set_current_directory = lunar_launcher_set_current_directory;
}



static void
lunar_launcher_init (LunarLauncher *launcher)
{
  launcher->files_to_process = NULL;
  launcher->select_files_closure = NULL;
  launcher->device_to_process = NULL;

  /* grab a reference on the preferences */
  launcher->preferences = lunar_preferences_get ();
}



static void
lunar_launcher_dispose (GObject *object)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (object);

  /* reset our properties */
  lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (launcher), NULL);
  lunar_launcher_set_widget (LUNAR_LAUNCHER (launcher), NULL);

  /* disconnect from the currently selected files */
  lunar_g_file_list_free (launcher->files_to_process);
  launcher->files_to_process = NULL;

  /* unref parent, if any */
  if (launcher->parent_folder != NULL)
      g_object_unref (launcher->parent_folder);

  (*G_OBJECT_CLASS (lunar_launcher_parent_class)->dispose) (object);
}



static void
lunar_launcher_finalize (GObject *object)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (object);

  /* release the preferences reference */
  g_object_unref (launcher->preferences);

  (*G_OBJECT_CLASS (lunar_launcher_parent_class)->finalize) (object);
}



static void
lunar_launcher_get_property (GObject    *object,
                              guint       prop_id,
                              GValue     *value,
                              GParamSpec *pspec)
{
  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      g_value_set_object (value, lunar_navigator_get_current_directory (LUNAR_NAVIGATOR (object)));
      break;

    case PROP_SELECTED_FILES:
      g_value_set_boxed (value, lunar_component_get_selected_files (LUNAR_COMPONENT (object)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_launcher_set_property (GObject      *object,
                              guint         prop_id,
                              const GValue *value,
                              GParamSpec   *pspec)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (object);

  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (object), g_value_get_object (value));
      break;

    case PROP_SELECTED_FILES:
      lunar_component_set_selected_files (LUNAR_COMPONENT (object), g_value_get_boxed (value));
      break;

    case PROP_WIDGET:
      lunar_launcher_set_widget (launcher, g_value_get_object (value));
      break;

    case PROP_SELECT_FILES_CLOSURE:
      launcher->select_files_closure = g_value_get_pointer (value);
      break;

    case PROP_SELECTED_DEVICE:
      launcher->device_to_process = g_value_get_pointer (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static LunarFile*
lunar_launcher_get_current_directory (LunarNavigator *navigator)
{
  return LUNAR_LAUNCHER (navigator)->current_directory;
}



static void
lunar_launcher_set_current_directory (LunarNavigator *navigator,
                                       LunarFile      *current_directory)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (navigator);

  /* disconnect from the previous directory */
  if (G_LIKELY (launcher->current_directory != NULL))
    g_object_unref (G_OBJECT (launcher->current_directory));

  /* activate the new directory */
  launcher->current_directory = current_directory;

  /* connect to the new directory */
  if (G_LIKELY (current_directory != NULL))
    {
      g_object_ref (G_OBJECT (current_directory));

      /* update files_to_process if not initialized yet */
      if (launcher->files_to_process == NULL)
        lunar_launcher_set_selected_files (LUNAR_COMPONENT (navigator), NULL);
    }

  /* notify listeners */
  g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_CURRENT_DIRECTORY]);
}



static void
lunar_launcher_set_selected_files (LunarComponent *component,
                                    GList           *selected_files)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (component);
  GList          *lp;

  /* That happens at startup for some reason */
  if (launcher->current_directory == NULL)
    return;

  /* disconnect from the previous files to process */
  if (launcher->files_to_process != NULL)
    lunar_g_file_list_free (launcher->files_to_process);
  launcher->files_to_process = NULL;

  /* notify listeners */
  g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_SELECTED_FILES]);

  /* unref previous parent, if any */
  if (launcher->parent_folder != NULL)
    g_object_unref (launcher->parent_folder);
  launcher->parent_folder = NULL;

  launcher->files_are_selected = TRUE;
  if (selected_files == NULL || g_list_length (selected_files) == 0)
    launcher->files_are_selected = FALSE;

  launcher->files_to_process_trashable = TRUE;
  launcher->n_files_to_process         = 0;
  launcher->n_directories_to_process   = 0;
  launcher->n_executables_to_process   = 0;
  launcher->n_regulars_to_process      = 0;
  launcher->single_directory_to_process = FALSE;
  launcher->single_folder = NULL;
  launcher->parent_folder = NULL;

  /* if nothing is selected, the current directory is the folder to use for all menus */
  if (launcher->files_are_selected)
    launcher->files_to_process = lunar_g_file_list_copy (selected_files);
  else
    launcher->files_to_process = g_list_append (launcher->files_to_process, launcher->current_directory);

  /* determine the number of files/directories/executables */
  for (lp = launcher->files_to_process; lp != NULL; lp = lp->next, ++launcher->n_files_to_process)
    {
      /* Keep a reference on all selected files */
      g_object_ref (lp->data);

      if (lunar_file_is_directory (lp->data)
          || lunar_file_is_shortcut (lp->data)
          || lunar_file_is_mountable (lp->data))
        {
          ++launcher->n_directories_to_process;
        }
      else
        {
          if (lunar_file_is_executable (lp->data))
            ++launcher->n_executables_to_process;
          ++launcher->n_regulars_to_process;
        }

      if (!lunar_file_can_be_trashed (lp->data))
        launcher->files_to_process_trashable = FALSE;
    }

  launcher->single_directory_to_process = (launcher->n_directories_to_process == 1 && launcher->n_files_to_process == 1);
  if (launcher->single_directory_to_process)
    {
      /* grab the folder of the first selected item */
      launcher->single_folder = LUNAR_FILE (launcher->files_to_process->data);
    }

  if (launcher->files_to_process != NULL)
    {
      /* just grab the folder of the first selected item */
      launcher->parent_folder = lunar_file_get_parent (LUNAR_FILE (launcher->files_to_process->data), NULL);
    }
}



 /**
 * lunar_launcher_set_widget:
 * @launcher : a #LunarLauncher.
 * @widget   : a #GtkWidget or %NULL.
 *
 * Associates @launcher with @widget.
 **/
void
lunar_launcher_set_widget (LunarLauncher *launcher,
                            GtkWidget      *widget)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));
  _lunar_return_if_fail (widget == NULL || GTK_IS_WIDGET (widget));

  /* disconnect from the previous widget */
  if (G_UNLIKELY (launcher->widget != NULL))
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (launcher->widget), lunar_launcher_widget_destroyed, launcher);
      g_object_unref (G_OBJECT (launcher->widget));
    }

  launcher->widget = widget;

  /* connect to the new widget */
  if (G_LIKELY (widget != NULL))
    {
      g_object_ref (G_OBJECT (widget));
      g_signal_connect_swapped (G_OBJECT (widget), "destroy", G_CALLBACK (lunar_launcher_widget_destroyed), launcher);
    }

  /* notify listeners */
  g_object_notify_by_pspec (G_OBJECT (launcher), launcher_props[PROP_WIDGET]);
}



static void
lunar_launcher_menu_item_activated (LunarLauncher *launcher,
                                     GtkWidget      *menu_item)
{
  GAppInfo *app_info;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_UNLIKELY (launcher->files_to_process == NULL))
    return;

  /* if we have a mime handler associated with the menu_item, we pass it to the launcher (g_object_get_qdata will return NULL otherwise)*/
  app_info = g_object_get_qdata (G_OBJECT (menu_item), lunar_launcher_appinfo_quark);
  lunar_launcher_activate_selected_files (launcher, LUNAR_LAUNCHER_CHANGE_DIRECTORY, app_info);
}



/**
 * lunar_launcher_activate_selected_files:
 * @launcher : a #LunarLauncher instance
 * @action   : the #LunarLauncherFolderOpenAction to use, if there are folders among the selected files
 * @app_info : a #GAppInfo instance
 *
 * Will try to open all selected files with the provided #GAppInfo
 **/
void
lunar_launcher_activate_selected_files (LunarLauncher                 *launcher,
                                         LunarLauncherFolderOpenAction  action,
                                         GAppInfo                       *app_info)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  lunar_launcher_poke (launcher, app_info, action);
}



static void
lunar_launcher_execute_files (LunarLauncher *launcher,
                               GList          *files)
{
  GError *error = NULL;
  GFile  *working_directory;
  GList  *lp;

  /* execute all selected files */
  for (lp = files; lp != NULL; lp = lp->next)
    {
      working_directory = lunar_file_get_file (launcher->current_directory);

      if (!lunar_file_execute (lp->data, working_directory, launcher->widget, NULL, NULL, &error))
        {
          /* display an error message to the user */
          lunar_dialogs_show_error (launcher->widget, error, _("Failed to execute file \"%s\""), lunar_file_get_display_name (lp->data));
          g_error_free (error);
          break;
        }
    }
}



static guint
lunar_launcher_g_app_info_hash (gconstpointer app_info)
{
  return 0;
}



static void
lunar_launcher_open_file (LunarLauncher *launcher,
                           LunarFile     *file,
                           GAppInfo       *application_to_use)
{
  GList *files = NULL;

  files = g_list_append (files, file);
  lunar_launcher_open_files (launcher, files, application_to_use);
  g_list_free (files);
}



static void
lunar_launcher_open_files (LunarLauncher *launcher,
                            GList          *files,
                            GAppInfo       *application_to_use)
{
  GHashTable *applications;
  GAppInfo   *app_info;
  GList      *file_list;
  GList      *lp;

  /* allocate a hash table to associate applications to URIs. since GIO allocates
   * new GAppInfo objects every time, g_direct_hash does not work. we therefore use
   * a fake hash function to always hit the collision list of the hash table and
   * avoid storing multiple equal GAppInfos by means of g_app_info_equal(). */
  applications = g_hash_table_new_full (lunar_launcher_g_app_info_hash,
                                        (GEqualFunc) g_app_info_equal,
                                        (GDestroyNotify) g_object_unref,
                                        (GDestroyNotify) lunar_g_file_list_free);

  for (lp = files; lp != NULL; lp = lp->next)
    {
      /* Because we created the hash_table with g_hash_table_new_full
       * g_object_unref on each hash_table key and value will be called by g_hash_table_destroy */
      if (application_to_use)
        {
          app_info = g_app_info_dup (application_to_use);
        }
      else
        {
          /* determine the default application for the MIME type */
          app_info = lunar_file_get_default_handler (lp->data);
        }

      /* check if we have an application here */
      if (G_LIKELY (app_info != NULL))
        {
          /* check if we have that application already */
          file_list = g_hash_table_lookup (applications, app_info);
          if (G_LIKELY (file_list != NULL))
            {
              /* take a copy of the list as the old one will be dropped by the insert */
              file_list = lunar_g_file_list_copy (file_list);
            }

          /* append our new URI to the list */
          file_list = lunar_g_file_list_append (file_list, lunar_file_get_file (lp->data));

          /* (re)insert the URI list for the application */
          g_hash_table_insert (applications, app_info, file_list);
        }
      else
        {
          /* display a chooser dialog for the file and stop */
          lunar_show_chooser_dialog (launcher->widget, lp->data, TRUE);
          break;
        }
    }

  /* run all collected applications */
  g_hash_table_foreach (applications, (GHFunc) lunar_launcher_open_paths, launcher);

  /* drop the applications hash table */
  g_hash_table_destroy (applications);
}



static void
lunar_launcher_open_paths (GAppInfo       *app_info,
                            GList          *path_list,
                            LunarLauncher *launcher)
{
  GdkAppLaunchContext *context;
  GdkScreen           *screen;
  GError              *error = NULL;
  GFile               *working_directory = NULL;
  gchar               *message;
  gchar               *name;
  guint                n;

  /* determine the screen on which to launch the application */
  screen = gtk_widget_get_screen (launcher->widget);

  /* create launch context */
  context = gdk_display_get_app_launch_context (gdk_screen_get_display (screen));
  gdk_app_launch_context_set_screen (context, screen);
  gdk_app_launch_context_set_timestamp (context, gtk_get_current_event_time ());
  gdk_app_launch_context_set_icon (context, g_app_info_get_icon (app_info));

  /* determine the working directory */
  if (launcher->current_directory != NULL)
    working_directory = lunar_file_get_file (launcher->current_directory);

  /* try to execute the application with the given URIs */
  if (!lunar_g_app_info_launch (app_info, working_directory, path_list, G_APP_LAUNCH_CONTEXT (context), &error))
    {
      /* figure out the appropriate error message */
      n = g_list_length (path_list);
      if (G_LIKELY (n == 1))
        {
          /* we can give a precise error message here */
          name = g_filename_display_name (g_file_get_basename (path_list->data));
          message = g_strdup_printf (_("Failed to open file \"%s\""), name);
          g_free (name);
        }
      else
        {
          /* we can just tell that n files failed to open */
          message = g_strdup_printf (ngettext ("Failed to open %d file", "Failed to open %d files", n), n);
        }

      /* display an error dialog to the user */
      lunar_dialogs_show_error (launcher->widget, error, "%s", message);
      g_error_free (error);
      g_free (message);
    }

  /* destroy the launch context */
  g_object_unref (context);
}



static void
lunar_launcher_open_windows (LunarLauncher *launcher,
                              GList          *directories)
{
  LunarApplication *application;
  GtkWidget         *dialog;
  GtkWidget         *window;
  GdkScreen         *screen;
  gchar             *label;
  GList             *lp;
  gint               response = GTK_RESPONSE_YES;
  gint               n;

  /* ask the user if we would open more than one new window */
  n = g_list_length (directories);
  if (G_UNLIKELY (n > 1))
    {
      /* open a message dialog */
      window = gtk_widget_get_toplevel (launcher->widget);
      dialog = gtk_message_dialog_new ((GtkWindow *) window,
                                       GTK_DIALOG_DESTROY_WITH_PARENT
                                       | GTK_DIALOG_MODAL,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_NONE,
                                       _("Are you sure you want to open all folders?"));
      gtk_message_dialog_format_secondary_text (GTK_MESSAGE_DIALOG (dialog),
                                                ngettext ("This will open %d separate file manager window.",
                                                          "This will open %d separate file manager windows.",
                                                          n), n);
      label = g_strdup_printf (ngettext ("Open %d New Window", "Open %d New Windows", n), n);
      gtk_dialog_add_button (GTK_DIALOG (dialog), _("_Cancel"), GTK_RESPONSE_CANCEL);
      gtk_dialog_add_button (GTK_DIALOG (dialog), label, GTK_RESPONSE_YES);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (label);
    }

  /* open n new windows if the user approved it */
  if (G_LIKELY (response == GTK_RESPONSE_YES))
    {
      /* query the application object */
      application = lunar_application_get ();

      /* determine the screen on which to open the new windows */
      screen = gtk_widget_get_screen (launcher->widget);

      /* open all requested windows */
      for (lp = directories; lp != NULL; lp = lp->next)
        lunar_application_open_window (application, lp->data, screen, NULL, TRUE);

      /* release the application object */
      g_object_unref (G_OBJECT (application));
    }
}



static void
lunar_launcher_poke (LunarLauncher                 *launcher,
                      GAppInfo                       *application_to_use,
                      LunarLauncherFolderOpenAction  folder_open_action)
{
  LunarLauncherPokeData *poke_data;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

   if (launcher->files_to_process == NULL)
     {
       g_warning("No files to process. lunar_launcher_poke aborted.");
       return;
     }

   poke_data = lunar_launcher_poke_data_new (launcher->files_to_process, application_to_use, folder_open_action);

   if (launcher->device_to_process != NULL)
     {
       lunar_browser_poke_device (LUNAR_BROWSER (launcher), launcher->device_to_process,
                                   launcher->widget, lunar_launcher_poke_device_finish,
                                  poke_data);
     }
   else
     {
      // We will only poke one file at a time, in order to dont use all available CPU's
      // TODO: Check if that could cause slowness
      lunar_browser_poke_file (LUNAR_BROWSER (launcher), poke_data->files_to_poke->data,
                                launcher->widget, lunar_launcher_poke_files_finish,
                                poke_data);
     }
}



static void lunar_launcher_poke_device_finish (LunarBrowser *browser,
                                                LunarDevice  *volume,
                                                LunarFile    *mount_point,
                                                GError        *error,
                                                gpointer       user_data,
                                                gboolean       cancelled)
{
  LunarLauncherPokeData *poke_data = user_data;
  gchar                  *device_name;

  if (error != NULL)
    {
      device_name = lunar_device_get_name (volume);
      lunar_dialogs_show_error (GTK_WIDGET (LUNAR_LAUNCHER (browser)->widget), error, _("Failed to mount \"%s\""), device_name);
      g_free (device_name);
    }

  if (cancelled == TRUE || error != NULL || mount_point == NULL)
    {
      lunar_launcher_poke_data_free (poke_data);
      return;
    }

  if (poke_data->folder_open_action == LUNAR_LAUNCHER_OPEN_AS_NEW_TAB)
    {
      lunar_navigator_open_new_tab (LUNAR_NAVIGATOR (browser), mount_point);
    }
  else if (poke_data->folder_open_action == LUNAR_LAUNCHER_OPEN_AS_NEW_WINDOW)
    {
      GList *directories = NULL;
      directories = g_list_append (directories, mount_point);
      lunar_launcher_open_windows (LUNAR_LAUNCHER (browser), directories);
      g_list_free (directories);
    }
  else if (poke_data->folder_open_action == LUNAR_LAUNCHER_CHANGE_DIRECTORY)
    {
      lunar_navigator_change_directory (LUNAR_NAVIGATOR (browser), mount_point);
    }

  lunar_launcher_poke_data_free (poke_data);
}



static void
lunar_launcher_poke_files_finish (LunarBrowser *browser,
                                   LunarFile    *file,
                                   LunarFile    *target_file,
                                   GError        *error,
                                   gpointer       user_data)
{
  LunarLauncherPokeData *poke_data = user_data;
  gboolean                executable = TRUE;
  gboolean                open_new_window_as_tab = TRUE;
  GList                  *directories = NULL;
  GList                  *files = NULL;
  GList                  *lp;

  _lunar_return_if_fail (LUNAR_IS_BROWSER (browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (poke_data != NULL);
  _lunar_return_if_fail (poke_data->files_to_poke != NULL);

  /* check if poking succeeded */
  if (error == NULL)
    {
      /* add the resolved file to the list of file to be opened/executed later */
      poke_data->files_poked = g_list_prepend (poke_data->files_poked,g_object_ref (target_file));
    }

  /* release and remove the just poked file from the list */
  g_object_unref (poke_data->files_to_poke->data);
  poke_data->files_to_poke = g_list_delete_link (poke_data->files_to_poke, poke_data->files_to_poke);

  if (poke_data->files_to_poke == NULL)
    {
      /* separate files and directories in the selected files list */
      for (lp = poke_data->files_poked; lp != NULL; lp = lp->next)
        {
          if (lunar_file_is_directory (lp->data))
            {
              /* add to our directory list */
              directories = g_list_prepend (directories, lp->data);
            }
          else
            {
              /* add to our file list */
              files = g_list_prepend (files, lp->data);

              /* check if the file is executable */
              executable = (executable && lunar_file_is_executable (lp->data));
            }
        }

      /* check if we have any directories to process */
      if (G_LIKELY (directories != NULL))
        {
          if (poke_data->application_to_use != NULL)
            {
              /* open them separately, using some specific application */
              for (lp = directories; lp != NULL; lp = lp->next)
                lunar_launcher_open_file (LUNAR_LAUNCHER (browser), lp->data, poke_data->application_to_use);
            }
          else if (poke_data->folder_open_action == LUNAR_LAUNCHER_OPEN_AS_NEW_TAB)
            {
              for (lp = directories; lp != NULL; lp = lp->next)
                lunar_navigator_open_new_tab (LUNAR_NAVIGATOR (browser), lp->data);
            }
          else if (poke_data->folder_open_action == LUNAR_LAUNCHER_OPEN_AS_NEW_WINDOW)
            {
              lunar_launcher_open_windows (LUNAR_LAUNCHER (browser), directories);
            }
          else if (poke_data->folder_open_action == LUNAR_LAUNCHER_CHANGE_DIRECTORY)
            {
              /* If multiple directories are passed, we assume that we should open them all */
              if (directories->next == NULL)
                lunar_navigator_change_directory (LUNAR_NAVIGATOR (browser), directories->data);
              else
                {
                  g_object_get (G_OBJECT (LUNAR_LAUNCHER (browser)->preferences), "misc-open-new-window-as-tab", &open_new_window_as_tab, NULL);
                  if (open_new_window_as_tab)
                    {
                      for (lp = directories; lp != NULL; lp = lp->next)
                        lunar_navigator_open_new_tab (LUNAR_NAVIGATOR (browser), lp->data);
                    }
                  else
                    lunar_launcher_open_windows (LUNAR_LAUNCHER (browser), directories);
                }
            }
          else if (poke_data->folder_open_action == LUNAR_LAUNCHER_NO_ACTION)
            {
              // nothing to do
            }
          else
              g_warning("'folder_open_action' was not defined");
          g_list_free (directories);
        }

      /* check if we have any files to process */
      if (G_LIKELY (files != NULL))
        {
          /* if all files are executable, we just run them here */
          if (G_UNLIKELY (executable) && poke_data->application_to_use == NULL)
            {
              /* try to execute all given files */
              lunar_launcher_execute_files (LUNAR_LAUNCHER (browser), files);
            }
          else
            {
              /* try to open all files */
              lunar_launcher_open_files (LUNAR_LAUNCHER (browser), files, poke_data->application_to_use);
            }

          /* cleanup */
          g_list_free (files);
        }

      /* free the poke data */
      lunar_launcher_poke_data_free (poke_data);
    }
  else
    {
      /* we need to continue this until all files have been resolved */
      // We will only poke one file at a time, in order to dont use all available CPU's
      // TODO: Check if that could cause slowness
      lunar_browser_poke_file (browser, poke_data->files_to_poke->data,
                                (LUNAR_LAUNCHER (browser))->widget, lunar_launcher_poke_files_finish, poke_data);
    }
}



static LunarLauncherPokeData *
lunar_launcher_poke_data_new (GList                          *files_to_poke,
                               GAppInfo                       *application_to_use,
                               LunarLauncherFolderOpenAction  folder_open_action)
{
  LunarLauncherPokeData *data;

  data = g_slice_new0 (LunarLauncherPokeData);
  data->files_to_poke = lunar_g_file_list_copy (files_to_poke);
  data->files_poked = NULL;
  data->application_to_use = application_to_use;

  /* keep a reference on the appdata */
  if (application_to_use != NULL)
    g_object_ref (application_to_use);
  data->folder_open_action = folder_open_action;

  return data;
}



static void
lunar_launcher_poke_data_free (LunarLauncherPokeData *data)
{
  _lunar_return_if_fail (data != NULL);

  lunar_g_file_list_free (data->files_to_poke);
  lunar_g_file_list_free (data->files_poked);

  if (data->application_to_use != NULL)
    g_object_unref (data->application_to_use);
  g_slice_free (LunarLauncherPokeData, data);
}



static void
lunar_launcher_widget_destroyed (LunarLauncher *launcher,
                                  GtkWidget      *widget)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));
  _lunar_return_if_fail (launcher->widget == widget);
  _lunar_return_if_fail (GTK_IS_WIDGET (widget));

  /* just reset the widget property for the launcher */
  lunar_launcher_set_widget (launcher, NULL);
}



/**
 * lunar_launcher_open_selected_folders:
 * @launcher : a #LunarLauncher instance
 * @open_in_tabs : TRUE to open each folder in a new tab, FALSE to open each folder in a new window
 *
 * Will open each selected folder in a new tab/window
 **/
void lunar_launcher_open_selected_folders (LunarLauncher *launcher,
                                            gboolean        open_in_tabs)
{
  GList *lp;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  for (lp = launcher->files_to_process; lp != NULL; lp = lp->next)
    _lunar_return_if_fail (lunar_file_is_directory (LUNAR_FILE (lp->data)));

  if (open_in_tabs)
    lunar_launcher_poke (launcher, NULL, LUNAR_LAUNCHER_OPEN_AS_NEW_TAB);
  else
    lunar_launcher_poke (launcher, NULL, LUNAR_LAUNCHER_OPEN_AS_NEW_WINDOW);
}



static void
lunar_launcher_action_open (LunarLauncher *launcher)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_UNLIKELY (launcher->files_to_process == NULL))
    return;

  lunar_launcher_activate_selected_files (launcher, LUNAR_LAUNCHER_CHANGE_DIRECTORY, NULL);
}



/**
 * lunar_launcher_action_open_in_new_tabs:
 * @launcher : a #LunarLauncher instance
 *
 * Will open each selected folder in a new tab
 **/
static void
lunar_launcher_action_open_in_new_tabs (LunarLauncher *launcher)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_UNLIKELY (launcher->files_to_process == NULL))
    return;

  lunar_launcher_open_selected_folders (launcher, TRUE);
}



static void
lunar_launcher_action_open_in_new_windows (LunarLauncher *launcher)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_UNLIKELY (launcher->files_to_process == NULL))
    return;

  lunar_launcher_open_selected_folders (launcher, FALSE);
}



static void
lunar_launcher_action_open_with_other (LunarLauncher *launcher)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->n_files_to_process == 1)
    lunar_show_chooser_dialog (launcher->widget, launcher->files_to_process->data, TRUE);
}



/**
 * lunar_launcher_append_accelerators:
 * @launcher    : a #LunarLauncher.
 * @accel_group : a #GtkAccelGroup to be used used for new menu items
 *
 * Connects all accelerators and corresponding default keys of this widget to the global accelerator list
 **/
void lunar_launcher_append_accelerators (LunarLauncher *launcher,
                                          GtkAccelGroup  *accel_group)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  expidus_gtk_accel_map_add_entries (lunar_launcher_action_entries, G_N_ELEMENTS (lunar_launcher_action_entries));
  expidus_gtk_accel_group_connect_action_entries (accel_group,
                                               lunar_launcher_action_entries,
                                               G_N_ELEMENTS (lunar_launcher_action_entries),
                                               launcher);
}



static gboolean
lunar_launcher_show_trash (LunarLauncher *launcher)
{
  if (launcher->parent_folder == NULL)
    return FALSE;

  /* If the folder is read only, always show trash insensitive */
  /* If we are outside waste basket, the selection is trashable and we support trash, show trash */
  return !lunar_file_is_writable (launcher->parent_folder) || ( !lunar_file_is_trashed (launcher->parent_folder) && launcher->files_to_process_trashable && lunar_g_vfs_is_uri_scheme_supported ("trash"));
}



/**
 * lunar_launcher_append_menu_item:
 * @launcher       : Instance of a  #LunarLauncher
 * @menu           : #GtkMenuShell to which the item should be added
 * @action         : #LunarLauncherAction to select which item should be added
 * @force          : force to generate the item. If it cannot be used, it will be shown as insensitive
 *
 * Adds the selected, widget specific #GtkMenuItem to the passed #GtkMenuShell
 *
 * Return value: (transfer none): The added #GtkMenuItem
 **/
GtkWidget*
lunar_launcher_append_menu_item (LunarLauncher       *launcher,
                                  GtkMenuShell         *menu,
                                  LunarLauncherAction  action,
                                  gboolean              force)
{
  GtkWidget                *item = NULL;
  GtkWidget                *submenu;
  GtkWidget                *focused_widget;
  gchar                    *label_text;
  gchar                    *tooltip_text;
  const ExpidusGtkActionEntry *action_entry = get_action_entry (action);
  gboolean                  show_delete_item;
  gboolean                  show_item;
  LunarClipboardManager   *clipboard;
  LunarFile               *parent;
  gint                      n;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), NULL);
  _lunar_return_val_if_fail (action_entry != NULL, NULL);

  /* This may occur when the lunar-window is build */
  if (G_UNLIKELY (launcher->files_to_process == NULL) && launcher->device_to_process == NULL)
    return NULL;

  switch (action)
    {
      case LUNAR_LAUNCHER_ACTION_OPEN: /* aka "activate" */
        return expidus_gtk_image_menu_item_new_from_icon_name (_("_Open"), ngettext ("Open the selected file", "Open the selected files", launcher->n_files_to_process),
                                           action_entry->accel_path, action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);

      case LUNAR_LAUNCHER_ACTION_EXECUTE:
        return expidus_gtk_image_menu_item_new_from_icon_name (_("_Execute"), ngettext ("Execute the selected file", "Execute the selected files", launcher->n_files_to_process),
                                           action_entry->accel_path, action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);

      case LUNAR_LAUNCHER_ACTION_OPEN_IN_TAB:
        n = launcher->n_files_to_process > 0 ? launcher->n_files_to_process : 1;
        label_text = g_strdup_printf (ngettext ("Open in New _Tab", "Open in %d New _Tabs", n), n);
        tooltip_text = g_strdup_printf (ngettext ("Open the selected directory in new tab",
                                                  "Open the selected directories in %d new tabs", n), n);
        item = expidus_gtk_menu_item_new (label_text, tooltip_text, action_entry->accel_path, action_entry->callback, G_OBJECT (launcher), menu);
        g_free (tooltip_text);
        g_free (label_text);
        return item;

      case LUNAR_LAUNCHER_ACTION_OPEN_IN_WINDOW:
        n = launcher->n_files_to_process > 0 ? launcher->n_files_to_process : 1;
        label_text = g_strdup_printf (ngettext ("Open in New _Window", "Open in %d New _Windows", n), n);
        tooltip_text = g_strdup_printf (ngettext ("Open the selected directory in new window",
                                                  "Open the selected directories in %d new windows",n), n);
        item = expidus_gtk_menu_item_new (label_text, tooltip_text, action_entry->accel_path, action_entry->callback, G_OBJECT (launcher), menu);
        g_free (tooltip_text);
        g_free (label_text);
        return item;

      case LUNAR_LAUNCHER_ACTION_OPEN_WITH_OTHER:
        return expidus_gtk_menu_item_new (action_entry->menu_item_label_text, action_entry->menu_item_tooltip_text,
                                       action_entry->accel_path, action_entry->callback, G_OBJECT (launcher), menu);

      case LUNAR_LAUNCHER_ACTION_SENDTO_MENU:
        if (launcher->files_are_selected == FALSE)
          return NULL;
        item = expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
        submenu = lunar_launcher_build_sendto_submenu (launcher);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
        return item;

      case LUNAR_LAUNCHER_ACTION_MAKE_LINK:
        show_item = lunar_file_is_writable (launcher->current_directory) &&
                    launcher->files_are_selected &&
                    lunar_file_is_trashed (launcher->current_directory) == FALSE;
        if (!show_item && !force)
          return NULL;

        label_text = ngettext ("Ma_ke Link", "Ma_ke Links", launcher->n_files_to_process);
        tooltip_text = ngettext ("Create a symbolic link for the selected file",
                                 "Create a symbolic link for each selected file", launcher->n_files_to_process);
        item = expidus_gtk_menu_item_new (label_text, tooltip_text, action_entry->accel_path, action_entry->callback,
                                       G_OBJECT (launcher), menu);
        gtk_widget_set_sensitive (item, show_item && launcher->parent_folder != NULL && lunar_file_is_writable (launcher->parent_folder));
        return item;

      case LUNAR_LAUNCHER_ACTION_DUPLICATE:
        show_item = lunar_file_is_writable (launcher->current_directory) &&
                    launcher->files_are_selected &&
                    lunar_file_is_trashed (launcher->current_directory) == FALSE;
        if (!show_item && !force)
          return NULL;
        item = expidus_gtk_menu_item_new (action_entry->menu_item_label_text, action_entry->menu_item_tooltip_text,
                                       action_entry->accel_path, action_entry->callback, G_OBJECT (launcher), menu);
        gtk_widget_set_sensitive (item, show_item && launcher->parent_folder != NULL && lunar_file_is_writable (launcher->parent_folder));
        return item;

      case LUNAR_LAUNCHER_ACTION_RENAME:
        show_item = lunar_file_is_writable (launcher->current_directory) &&
                    launcher->files_are_selected &&
                    lunar_file_is_trashed (launcher->current_directory) == FALSE;
        if (!show_item && !force)
          return NULL;
        tooltip_text = ngettext ("Rename the selected file",
                                 "Rename the selected files", launcher->n_files_to_process);
        item = expidus_gtk_menu_item_new (action_entry->menu_item_label_text, tooltip_text, action_entry->accel_path,
                                       action_entry->callback, G_OBJECT (launcher), menu);
        gtk_widget_set_sensitive (item, show_item && launcher->parent_folder != NULL && lunar_file_is_writable (launcher->parent_folder));
        return item;

      case LUNAR_LAUNCHER_ACTION_RESTORE:
        if (launcher->files_are_selected && lunar_file_is_trashed (launcher->current_directory))
          {
            tooltip_text = ngettext ("Restore the selected file to its original location",
                                     "Restore the selected files to its original location", launcher->n_files_to_process);
            item = expidus_gtk_menu_item_new (action_entry->menu_item_label_text, tooltip_text, action_entry->accel_path,
                                           action_entry->callback, G_OBJECT (launcher), menu);
            gtk_widget_set_sensitive (item, lunar_file_is_writable (launcher->current_directory));
            return item;
          }
        return NULL;

      case LUNAR_LAUNCHER_ACTION_MOVE_TO_TRASH:
        if (!lunar_launcher_show_trash (launcher))
          return NULL;

        show_item = launcher->files_are_selected;
        if (!show_item && !force)
          return NULL;

        tooltip_text = ngettext ("Move the selected file to the Trash",
                                 "Move the selected files to the Trash", launcher->n_files_to_process);
        item = expidus_gtk_image_menu_item_new_from_icon_name (action_entry->menu_item_label_text, tooltip_text, action_entry->accel_path,
                                                            action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);
        gtk_widget_set_sensitive (item, show_item && launcher->parent_folder != NULL && lunar_file_is_writable (launcher->parent_folder));
        return item;


      case LUNAR_LAUNCHER_ACTION_DELETE:
        g_object_get (G_OBJECT (launcher->preferences), "misc-show-delete-action", &show_delete_item, NULL);
        if (lunar_launcher_show_trash (launcher) && !show_delete_item)
          return NULL;

        show_item = launcher->files_are_selected;
        if (!show_item && !force)
          return NULL;

        tooltip_text = ngettext ("Permanently delete the selected file",
                                 "Permanently delete the selected files", launcher->n_files_to_process);
        item = expidus_gtk_image_menu_item_new_from_icon_name (action_entry->menu_item_label_text, tooltip_text, action_entry->accel_path,
                                                            action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);
        gtk_widget_set_sensitive (item, show_item && launcher->parent_folder != NULL && lunar_file_is_writable (launcher->parent_folder));
        return item;

      case LUNAR_LAUNCHER_ACTION_EMPTY_TRASH:
        if (launcher->single_directory_to_process == TRUE)
          {
            if (lunar_file_is_root (launcher->single_folder) && lunar_file_is_trashed (launcher->single_folder))
              {
                item = expidus_gtk_image_menu_item_new_from_icon_name (action_entry->menu_item_label_text, action_entry->menu_item_tooltip_text, action_entry->accel_path,
                                                                    action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);
                gtk_widget_set_sensitive (item, lunar_file_get_item_count (launcher->single_folder) > 0);
                return item;
              }
          }
        return NULL;

      case LUNAR_LAUNCHER_ACTION_CREATE_FOLDER:
        if (LUNAR_IS_TREE_VIEW (launcher->widget) && launcher->files_are_selected && launcher->single_directory_to_process)
          parent = launcher->single_folder;
        else
          parent = launcher->current_directory;
        if (lunar_file_is_trashed (parent))
          return NULL;
        item = expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
        gtk_widget_set_sensitive (item, lunar_file_is_writable (parent));
        return item;

      case LUNAR_LAUNCHER_ACTION_CREATE_DOCUMENT:
        if (LUNAR_IS_TREE_VIEW (launcher->widget) && launcher->files_are_selected && launcher->single_directory_to_process)
          parent = launcher->single_folder;
        else
          parent = launcher->current_directory;
        if (lunar_file_is_trashed (parent))
          return NULL;
        item = expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
        submenu = lunar_launcher_create_document_submenu_new (launcher);
        gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);
        gtk_widget_set_sensitive (item, lunar_file_is_writable (parent));
        return item;

      case LUNAR_LAUNCHER_ACTION_CUT:
        focused_widget = lunar_gtk_get_focused_widget();
        if (focused_widget && GTK_IS_EDITABLE (focused_widget))
          {
            item = expidus_gtk_image_menu_item_new_from_icon_name (
                          action_entry->menu_item_label_text,
                          N_ ("Cut the selection"),
                          action_entry->accel_path, G_CALLBACK (gtk_editable_cut_clipboard),
                          G_OBJECT (focused_widget), action_entry->menu_item_icon_name, menu);
            gtk_widget_set_sensitive (item, lunar_gtk_editable_can_cut (GTK_EDITABLE (focused_widget)));
          }
        else
          {
            show_item = launcher->files_are_selected;
            if (!show_item && !force)
              return NULL;
            tooltip_text = ngettext ("Prepare the selected file to be moved with a Paste command",
                                     "Prepare the selected files to be moved with a Paste command", launcher->n_files_to_process);
            item = expidus_gtk_image_menu_item_new_from_icon_name (action_entry->menu_item_label_text, tooltip_text, action_entry->accel_path,
                                                                action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);
            gtk_widget_set_sensitive (item, show_item && launcher->parent_folder != NULL && lunar_file_is_writable (launcher->parent_folder));
          }
        return item;

      case LUNAR_LAUNCHER_ACTION_COPY:
        focused_widget = lunar_gtk_get_focused_widget();
        if (focused_widget && GTK_IS_EDITABLE (focused_widget))
          {
            item = expidus_gtk_image_menu_item_new_from_icon_name (
                          action_entry->menu_item_label_text,
                          N_ ("Copy the selection"),
                          action_entry->accel_path,G_CALLBACK (gtk_editable_copy_clipboard),
                          G_OBJECT (focused_widget), action_entry->menu_item_icon_name, menu);
            gtk_widget_set_sensitive (item, lunar_gtk_editable_can_copy (GTK_EDITABLE (focused_widget)));
          }
        else
          {
            show_item = launcher->files_are_selected;
            if (!show_item && !force)
              return NULL;
            tooltip_text = ngettext ("Prepare the selected file to be copied with a Paste command",
                                    "Prepare the selected files to be copied with a Paste command", launcher->n_files_to_process);
            item = expidus_gtk_image_menu_item_new_from_icon_name (action_entry->menu_item_label_text, tooltip_text, action_entry->accel_path,
                                                                action_entry->callback, G_OBJECT (launcher), action_entry->menu_item_icon_name, menu);
            gtk_widget_set_sensitive (item, show_item);
          }
        return item;

      case LUNAR_LAUNCHER_ACTION_PASTE_INTO_FOLDER:
        if (!launcher->single_directory_to_process)
          return NULL;
        clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (launcher->widget));
        item = expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
        gtk_widget_set_sensitive (item, lunar_clipboard_manager_get_can_paste (clipboard) && lunar_file_is_writable (launcher->single_folder));
        g_object_unref (clipboard);
        return item;

      case LUNAR_LAUNCHER_ACTION_PASTE:
        focused_widget = lunar_gtk_get_focused_widget();
        if (focused_widget && GTK_IS_EDITABLE (focused_widget))
          {
            item = expidus_gtk_image_menu_item_new_from_icon_name (
                          action_entry->menu_item_label_text,
                          N_ ("Paste the clipboard"),
                          action_entry->accel_path,G_CALLBACK (gtk_editable_paste_clipboard),
                          G_OBJECT (focused_widget), action_entry->menu_item_icon_name, menu);
            gtk_widget_set_sensitive (item, lunar_gtk_editable_can_paste (GTK_EDITABLE (focused_widget)));
          }
        else
          {
            if (launcher->single_directory_to_process && launcher->files_are_selected)
                return lunar_launcher_append_menu_item (launcher, menu, LUNAR_LAUNCHER_ACTION_PASTE_INTO_FOLDER, force);
            clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (launcher->widget));
            item = expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
            gtk_widget_set_sensitive (item, lunar_clipboard_manager_get_can_paste (clipboard) && lunar_file_is_writable (launcher->current_directory));
            g_object_unref (clipboard);
          }
        return item;

      case LUNAR_LAUNCHER_ACTION_MOUNT:
        if (launcher->device_to_process == NULL || lunar_device_is_mounted (launcher->device_to_process) == TRUE)
          return NULL;
        return expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));

      case LUNAR_LAUNCHER_ACTION_UNMOUNT:
        if (launcher->device_to_process == NULL || lunar_device_is_mounted (launcher->device_to_process) == FALSE)
          return NULL;
        return expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));

      case LUNAR_LAUNCHER_ACTION_EJECT:
        if (launcher->device_to_process == NULL || lunar_device_get_kind (launcher->device_to_process) != LUNAR_DEVICE_KIND_VOLUME)
          return NULL;
        item = expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
        gtk_widget_set_sensitive (item, lunar_device_can_eject (launcher->device_to_process));
        return item;

      default:
        return expidus_gtk_menu_item_new_from_action_entry (action_entry, G_OBJECT (launcher), GTK_MENU_SHELL (menu));
    }
  return NULL;
}




static void
lunar_launcher_action_sendto_desktop (LunarLauncher *launcher)
{
  LunarApplication *application;
  GFile             *desktop_file;
  GList             *files;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  /* determine the source paths */
  files = lunar_file_list_to_lunar_g_file_list (launcher->files_to_process);
  if (G_UNLIKELY (files == NULL))
    return;

  /* determine the file to the ~/Desktop folder */
  desktop_file = lunar_g_file_new_for_desktop ();

  /* launch the link job */
  application = lunar_application_get ();
  lunar_application_link_into (application, launcher->widget, files, desktop_file, NULL);
  g_object_unref (G_OBJECT (application));

  /* cleanup */
  g_object_unref (desktop_file);
  lunar_g_file_list_free (files);
}



static void
lunar_launcher_sendto_device (LunarLauncher *launcher,
                               LunarDevice   *device)
{
  LunarApplication *application;
  GFile             *mount_point;
  GList             *files;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));

  if (!lunar_device_is_mounted (device))
    return;

  /* determine the source paths */
  files = lunar_file_list_to_lunar_g_file_list (launcher->files_to_process);
  if (G_UNLIKELY (files == NULL))
    return;

  mount_point = lunar_device_get_root (device);
  if (mount_point != NULL)
    {
      /* copy the files onto the specified device */
      application = lunar_application_get ();
      lunar_application_copy_into (application, launcher->widget, files, mount_point, NULL);
      g_object_unref (application);
      g_object_unref (mount_point);
    }

  /* cleanup */
  lunar_g_file_list_free (files);
}



static void
lunar_launcher_sendto_mount_finish (LunarDevice *device,
                                     const GError *error,
                                     gpointer      user_data)
{
  LunarLauncher *launcher = user_data;
  gchar          *device_name;

  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (user_data));

  if (error != NULL)
    {
      /* tell the user that we were unable to mount the device, which is
       * required to send files to it */
      device_name = lunar_device_get_name (device);
      lunar_dialogs_show_error (launcher->widget, error, _("Failed to mount \"%s\""), device_name);
      g_free (device_name);
    }
  else
    {
      lunar_launcher_sendto_device (launcher, device);
    }
}



static void
lunar_launcher_action_sendto_device (LunarLauncher *launcher,
                                      GObject        *object)
{
  GMountOperation *mount_operation;
  LunarDevice    *device;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  /* determine the device to which to send */
  device = g_object_get_qdata (G_OBJECT (object), lunar_launcher_device_quark);
  if (G_UNLIKELY (device == NULL))
    return;

  /* make sure to mount the device first, if it's not already mounted */
  if (!lunar_device_is_mounted (device))
    {
      /* allocate a GTK+ mount operation */
      mount_operation = lunar_gtk_mount_operation_new (launcher->widget);

      /* try to mount the device and later start sending the files */
      lunar_device_mount (device,
                           mount_operation,
                           NULL,
                           lunar_launcher_sendto_mount_finish,
                           launcher);

      g_object_unref (mount_operation);
    }
  else
    {
      lunar_launcher_sendto_device (launcher, device);
    }
}


static void
lunar_launcher_action_add_shortcuts (LunarLauncher *launcher)
{
  GList           *lp;
  GtkWidget       *window;
  const GtkWidget *sidepane;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  /* determine the toplevel window we belong to */
  window = gtk_widget_get_toplevel (launcher->widget);
  if (LUNAR_IS_WINDOW (window) == FALSE)
    return;
  if (lunar_window_has_shortcut_sidepane (LUNAR_WINDOW (window)) == FALSE)
    return;

  sidepane = lunar_window_get_sidepane (LUNAR_WINDOW (window));
  if (sidepane != NULL  && LUNAR_IS_SHORTCUTS_PANE (sidepane))
    {
      for (lp = launcher->files_to_process; lp != NULL; lp = lp->next)
        lunar_shortcuts_pane_add_shortcut (LUNAR_SHORTCUTS_PANE (sidepane), lp->data);
    }

}



static GtkWidget*
lunar_launcher_build_sendto_submenu (LunarLauncher *launcher)
{
  GList                    *lp;
  gboolean                  linkable = TRUE;
  gchar                    *label_text;
  gchar                    *tooltip_text;
  GtkWidget                *image;
  GtkWidget                *item;
  GtkWidget                *submenu;
  GtkWidget                *window;
  GList                    *devices;
  GList                    *appinfo_list;
  GIcon                    *icon;
  LunarDeviceMonitor      *device_monitor;
  LunarSendtoModel        *sendto_model;
  const ExpidusGtkActionEntry *action_entry;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), NULL);

  submenu = gtk_menu_new();

  /* show "sent to shortcut" if only directories are selected */
  if (launcher->n_directories_to_process > 0 && launcher->n_directories_to_process == launcher->n_files_to_process)
    {
      /* determine the toplevel window we belong to */
      window = gtk_widget_get_toplevel (launcher->widget);
      if (LUNAR_IS_WINDOW (window) && lunar_window_has_shortcut_sidepane (LUNAR_WINDOW (window)))
        {
          action_entry = get_action_entry (LUNAR_LAUNCHER_ACTION_SENDTO_SHORTCUTS);
          if (action_entry != NULL)
            {
              label_text   = ngettext ("Side Pane (Create Shortcut)", "Side Pane (Create Shortcuts)", launcher->n_files_to_process);
              tooltip_text = ngettext ("Add the selected folder to the shortcuts side pane",
                                       "Add the selected folders to the shortcuts side pane", launcher->n_files_to_process);
              item = expidus_gtk_image_menu_item_new_from_icon_name (label_text, tooltip_text, action_entry->accel_path, action_entry->callback,
                                                                  G_OBJECT (launcher), action_entry->menu_item_icon_name, GTK_MENU_SHELL (submenu));
            }
        }
    }

  /* Check whether at least one files is located in the trash (to en-/disable the "sendto-desktop" action). */
  for (lp = launcher->files_to_process; lp != NULL; lp = lp->next)
    {
      if (G_UNLIKELY (lunar_file_is_trashed (lp->data)))
        linkable = FALSE;
    }
  if (linkable)
    {
      action_entry = get_action_entry (LUNAR_LAUNCHER_ACTION_SENDTO_DESKTOP);
      if (action_entry != NULL)
        {
          label_text   = ngettext ("Desktop (Create Link)", "Desktop (Create Links)", launcher->n_files_to_process);
          tooltip_text = ngettext ("Create a link to the selected file on the desktop",
                                   "Create links to the selected files on the desktop", launcher->n_files_to_process);
          item = expidus_gtk_image_menu_item_new_from_icon_name (label_text, tooltip_text, action_entry->accel_path, action_entry->callback,
                                                              G_OBJECT (launcher), action_entry->menu_item_icon_name, GTK_MENU_SHELL (submenu));
        }
    }

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
  gtk_widget_show (item);

  /* determine the currently active devices */
  device_monitor = lunar_device_monitor_get ();
  devices = lunar_device_monitor_get_devices (device_monitor);
  g_object_unref (device_monitor);

  /* add removable (and writable) drives and media */
  for (lp = devices; lp != NULL; lp = lp->next)
    {
      /* generate a unique name and tooltip for the device */
      label_text = lunar_device_get_name (lp->data);
      tooltip_text = g_strdup_printf (ngettext ("Send the selected file to \"%s\"",
                                                "Send the selected files to \"%s\"", launcher->n_files_to_process), label_text);
      icon = lunar_device_get_icon (lp->data);
      image = NULL;
      if (G_LIKELY (icon != NULL))
        {
          image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_MENU);
          g_object_unref (icon);
        }
      item = expidus_gtk_image_menu_item_new (label_text, tooltip_text, NULL, G_CALLBACK (lunar_launcher_action_sendto_device),
                                           G_OBJECT (launcher), image, GTK_MENU_SHELL (submenu));
      g_object_set_qdata_full (G_OBJECT (item), lunar_launcher_device_quark, lp->data, g_object_unref);
      g_object_set_data (G_OBJECT (lp->data), "skip-app-info-update", GUINT_TO_POINTER (1));

      /* cleanup */
      g_free (tooltip_text);
      g_free (label_text);
    }

  /* free the devices list */
  g_list_free (devices);

  item = gtk_separator_menu_item_new ();
  gtk_menu_shell_append (GTK_MENU_SHELL (submenu), item);
  gtk_widget_show (item);

  /* determine the sendto appInfos for the selected files */
  sendto_model = lunar_sendto_model_get_default ();
  appinfo_list = lunar_sendto_model_get_matching (sendto_model, launcher->files_to_process);
  g_object_unref (sendto_model);

  if (G_LIKELY (appinfo_list != NULL))
    {
      /* add all handlers to the user interface */
      for (lp = appinfo_list; lp != NULL; lp = lp->next)
        {
          /* generate a unique name and tooltip for the handler */
          label_text = g_strdup (g_app_info_get_name (lp->data));
          tooltip_text = g_strdup_printf (ngettext ("Send the selected file to \"%s\"",
                                                    "Send the selected files to \"%s\"", launcher->n_files_to_process), label_text);

          icon = g_app_info_get_icon (lp->data);
          image = NULL;
          if (G_LIKELY (icon != NULL))
              image = gtk_image_new_from_gicon (icon, GTK_ICON_SIZE_MENU);

          item = expidus_gtk_image_menu_item_new (label_text, tooltip_text, NULL, G_CALLBACK (lunar_launcher_menu_item_activated),
                                               G_OBJECT (launcher), image, GTK_MENU_SHELL (submenu));
          g_object_set_qdata_full (G_OBJECT (item), lunar_launcher_appinfo_quark, g_object_ref (lp->data), g_object_unref);

          /* cleanup */
          g_free (label_text);
          g_free (tooltip_text);
        }

      /* release the appinfo list */
      g_list_free_full (appinfo_list, g_object_unref);
    }

  return submenu;
}



static void
lunar_launcher_action_properties (LunarLauncher *launcher)
{
  GtkWidget *toplevel;
  GtkWidget *dialog;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  /* popup the files dialog */
  toplevel = gtk_widget_get_toplevel (launcher->widget);
  if (G_LIKELY (toplevel != NULL))
    {
      dialog = lunar_properties_dialog_new (GTK_WINDOW (toplevel));

      /* check if no files are currently selected */
      if (launcher->files_to_process == NULL)
        {
          /* if we don't have any files selected, we just popup the properties dialog for the current folder. */
          lunar_properties_dialog_set_file (LUNAR_PROPERTIES_DIALOG (dialog), launcher->current_directory);
        }
      else
        {
          /* popup the properties dialog for all file(s) */
          lunar_properties_dialog_set_files (LUNAR_PROPERTIES_DIALOG (dialog), launcher->files_to_process);
        }
      gtk_widget_show (dialog);
    }
}



static void
lunar_launcher_action_make_link (LunarLauncher *launcher)
{
  LunarApplication *application;
  GList             *g_files = NULL;
  GList             *lp;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_UNLIKELY (launcher->current_directory == NULL))
    return;
  if (launcher->files_are_selected == FALSE || lunar_file_is_trashed (launcher->current_directory))
    return;

  for (lp = launcher->files_to_process; lp != NULL; lp = lp->next)
    {
      g_files = g_list_append (g_files, lunar_file_get_file (lp->data));
    }
  /* link the selected files into the current directory, which effectively
   * creates new unique links for the files.
   */
  application = lunar_application_get ();
  lunar_application_link_into (application, launcher->widget, g_files,
                                lunar_file_get_file (launcher->current_directory), launcher->select_files_closure);
  g_object_unref (G_OBJECT (application));
  g_list_free (g_files);
}



static void
lunar_launcher_action_duplicate (LunarLauncher *launcher)
{
  LunarApplication *application;
  GList             *files_to_process;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_UNLIKELY (launcher->current_directory == NULL))
    return;
  if (launcher->files_are_selected == FALSE || lunar_file_is_trashed (launcher->current_directory))
    return;

  /* determine the selected files for the view */
  files_to_process = lunar_file_list_to_lunar_g_file_list (launcher->files_to_process);
  if (G_LIKELY (files_to_process != NULL))
    {
      /* copy the selected files into the current directory, which effectively
       * creates duplicates of the files.
       */
      application = lunar_application_get ();
      lunar_application_copy_into (application, launcher->widget, files_to_process,
                                    lunar_file_get_file (launcher->current_directory), launcher->select_files_closure);
      g_object_unref (G_OBJECT (application));

      /* clean up */
      lunar_g_file_list_free (files_to_process);
    }
}



/**
 * lunar_launcher_append_custom_actions:
 * @launcher : a #LunarLauncher instance
 * @menu : #GtkMenuShell on which the custom actions should be appended
 *
 * Will append all custom actions which match the file-type to the provided #GtkMenuShell
 *
 * Return value: TRUE if any custom action was added
 **/
gboolean
lunar_launcher_append_custom_actions (LunarLauncher *launcher,
                                       GtkMenuShell   *menu)
{
  gboolean                uca_added = FALSE;
  GtkWidget              *window;
  GtkWidget              *gtk_menu_item;
  LunarxProviderFactory *provider_factory;
  GList                  *providers;
  GList                  *lunarx_menu_items = NULL;
  GList                  *lp_provider;
  GList                  *lp_item;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), FALSE);
  _lunar_return_val_if_fail (GTK_IS_MENU (menu), FALSE);

  /* determine the toplevel window we belong to */
  window = gtk_widget_get_toplevel (launcher->widget);

  /* load the menu providers from the provider factory */
  provider_factory = lunarx_provider_factory_get_default ();
  providers = lunarx_provider_factory_list_providers (provider_factory, LUNARX_TYPE_MENU_PROVIDER);
  g_object_unref (provider_factory);

  if (G_UNLIKELY (providers == NULL))
    return FALSE;

  /* This may occur when the lunar-window is build */
  if (G_UNLIKELY (launcher->files_to_process == NULL))
    return FALSE;

  /* load the menu items offered by the menu providers */
  for (lp_provider = providers; lp_provider != NULL; lp_provider = lp_provider->next)
    {
      if (launcher->files_are_selected == FALSE)
        lunarx_menu_items = lunarx_menu_provider_get_folder_menu_items (lp_provider->data, window, LUNARX_FILE_INFO (launcher->current_directory));
      else
        lunarx_menu_items = lunarx_menu_provider_get_file_menu_items (lp_provider->data, window, launcher->files_to_process);

      for (lp_item = lunarx_menu_items; lp_item != NULL; lp_item = lp_item->next)
        {
          gtk_menu_item = lunar_gtk_menu_lunarx_menu_item_new (lp_item->data, menu);

          /* Each lunarx_menu_item will be destroyed together with its related gtk_menu_item*/
          g_signal_connect_swapped (G_OBJECT (gtk_menu_item), "destroy", G_CALLBACK (g_object_unref), lp_item->data);
          uca_added = TRUE;
        }
      g_list_free (lunarx_menu_items);
    }
  g_list_free_full (providers, g_object_unref);
  return uca_added;
}



gboolean
lunar_launcher_check_uca_key_activation (LunarLauncher *launcher,
                                          GdkEventKey    *key_event)
{
  GtkWidget              *window;
  LunarxProviderFactory *provider_factory;
  GList                  *providers;
  GList                  *lunarx_menu_items = NULL;
  GList                  *lp_provider;
  GList                  *lp_item;
  GtkAccelKey             uca_key;
  gchar                  *name, *accel_path;
  gboolean                uca_activated = FALSE;

  /* determine the toplevel window we belong to */
  window = gtk_widget_get_toplevel (launcher->widget);

  /* load the menu providers from the provider factory */
  provider_factory = lunarx_provider_factory_get_default ();
  providers = lunarx_provider_factory_list_providers (provider_factory, LUNARX_TYPE_MENU_PROVIDER);
  g_object_unref (provider_factory);

  if (G_UNLIKELY (providers == NULL))
    return uca_activated;

  /* load the menu items offered by the menu providers */
  for (lp_provider = providers; lp_provider != NULL; lp_provider = lp_provider->next)
    {
      if (launcher->files_are_selected == FALSE)
        lunarx_menu_items = lunarx_menu_provider_get_folder_menu_items (lp_provider->data, window, LUNARX_FILE_INFO (launcher->current_directory));
      else
        lunarx_menu_items = lunarx_menu_provider_get_file_menu_items (lp_provider->data, window, launcher->files_to_process);
      for (lp_item = lunarx_menu_items; lp_item != NULL; lp_item = lp_item->next)
        {
          g_object_get (G_OBJECT (lp_item->data), "name", &name, NULL);
          accel_path = g_strconcat ("<Actions>/LunarActions/", name, NULL);
          if (gtk_accel_map_lookup_entry (accel_path, &uca_key) == TRUE)
            {
              if (g_ascii_tolower (key_event->keyval) == g_ascii_tolower (uca_key.accel_key))
                {
                  if ((key_event->state & gtk_accelerator_get_default_mod_mask ()) == uca_key.accel_mods)
                    {
                      lunarx_menu_item_activate (lp_item->data);
                      uca_activated = TRUE;
                    }
                }
            }
          g_free (name);
          g_free (accel_path);
          g_object_unref (lp_item->data);
        }
      g_list_free (lunarx_menu_items);
    }
  g_list_free_full (providers, g_object_unref);
  return uca_activated;
}



static void
lunar_launcher_rename_error (EndoJob    *job,
                              GError    *error,
                              GtkWidget *widget)
{
  GArray     *param_values;
  LunarFile *file;

  _lunar_return_if_fail (ENDO_IS_JOB (job));
  _lunar_return_if_fail (error != NULL);

  param_values = lunar_simple_job_get_param_values (LUNAR_SIMPLE_JOB (job));
  file = g_value_get_object (&g_array_index (param_values, GValue, 0));

  lunar_dialogs_show_error (GTK_WIDGET (widget), error,
                             _("Failed to rename \"%s\""),
                             lunar_file_get_display_name (file));
  g_object_unref (file);
}



static void
lunar_launcher_rename_finished (EndoJob    *job,
                                 GtkWidget *widget)
{
  _lunar_return_if_fail (ENDO_IS_JOB (job));

  /* destroy the job */
  g_signal_handlers_disconnect_matched (job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, widget);
  g_object_unref (job);
}



static void
lunar_launcher_action_rename (LunarLauncher *launcher)
{
  LunarJob *job;
  GtkWidget *window;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->files_to_process == NULL || g_list_length (launcher->files_to_process) == 0)
    return;
  if (launcher->files_are_selected == FALSE || lunar_file_is_trashed (launcher->current_directory))
    return;

  /* get the window */
  window = gtk_widget_get_toplevel (launcher->widget);

  /* start renaming if we have exactly one selected file */
  if (g_list_length (launcher->files_to_process) == 1)
    {
      /* run the rename dialog */
      job = lunar_dialogs_show_rename_file (GTK_WINDOW (window), LUNAR_FILE (launcher->files_to_process->data));
      if (G_LIKELY (job != NULL))
        {
          g_signal_connect (job, "error", G_CALLBACK (lunar_launcher_rename_error), launcher->widget);
          g_signal_connect (job, "finished", G_CALLBACK (lunar_launcher_rename_finished), launcher->widget);
        }
    }
  else
    {
      /* display the bulk rename dialog */
      lunar_show_renamer_dialog (GTK_WIDGET (window), launcher->current_directory, launcher->files_to_process, FALSE, NULL);
    }
}



static void
lunar_launcher_action_restore (LunarLauncher *launcher)
{
  LunarApplication *application;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->files_are_selected == FALSE || !lunar_file_is_trashed (launcher->current_directory))
    return;

  /* restore the selected files */
  application = lunar_application_get ();
  lunar_application_restore_files (application, launcher->widget, launcher->files_to_process, NULL);
  g_object_unref (G_OBJECT (application));
}


static void
lunar_launcher_action_move_to_trash (LunarLauncher *launcher)
{
  LunarApplication *application;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->parent_folder == NULL || launcher->files_are_selected == FALSE)
    return;

  application = lunar_application_get ();
  lunar_application_unlink_files (application, launcher->widget, launcher->files_to_process, FALSE);
  g_object_unref (G_OBJECT (application));
}



static void
lunar_launcher_action_delete (LunarLauncher *launcher)
{
  LunarApplication *application;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->parent_folder == NULL || launcher->files_are_selected == FALSE)
    return;

  application = lunar_application_get ();
  lunar_application_unlink_files (application, launcher->widget, launcher->files_to_process, TRUE);
  g_object_unref (G_OBJECT (application));
}



static void
lunar_launcher_action_trash_delete (LunarLauncher *launcher)
{
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (lunar_g_vfs_is_uri_scheme_supported ("trash"))
    lunar_launcher_action_move_to_trash (launcher);
  else
    lunar_launcher_action_delete (launcher);
}



static void
lunar_launcher_action_empty_trash (LunarLauncher *launcher)
{
  LunarApplication *application;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->single_directory_to_process == FALSE)
    return;
  if (!lunar_file_is_root (launcher->single_folder) || !lunar_file_is_trashed (launcher->single_folder))
    return;

  application = lunar_application_get ();
  lunar_application_empty_trash (application, launcher->widget, NULL);
  g_object_unref (G_OBJECT (application));
}



static void
lunar_launcher_action_create_folder (LunarLauncher *launcher)
{
  LunarApplication *application;
  GList              path_list;
  gchar             *name;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (lunar_file_is_trashed (launcher->current_directory))
    return;

  /* ask the user to enter a name for the new folder */
  name = lunar_dialogs_show_create (launcher->widget,
                                     "inode/directory",
                                     _("New Folder"),
                                     _("Create New Folder"));
  if (G_LIKELY (name != NULL))
    {
      /* fake the path list */
      if (LUNAR_IS_TREE_VIEW (launcher->widget) && launcher->files_are_selected && launcher->single_directory_to_process)
        path_list.data = g_file_resolve_relative_path (lunar_file_get_file (launcher->single_folder), name);
      else
        path_list.data = g_file_resolve_relative_path (lunar_file_get_file (launcher->current_directory), name);
      path_list.next = path_list.prev = NULL;

      /* launch the operation */
      application = lunar_application_get ();
      lunar_application_mkdir (application, launcher->widget, &path_list, launcher->select_files_closure);
      g_object_unref (G_OBJECT (application));

      /* release the path */
      g_object_unref (path_list.data);

      /* release the file name */
      g_free (name);
    }
}



static void
lunar_launcher_action_create_document (LunarLauncher *launcher,
                                        GtkWidget      *menu_item)
{
  LunarApplication *application;
  GList              target_path_list;
  gchar             *name;
  gchar             *title;
  LunarFile        *template_file;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (lunar_file_is_trashed (launcher->current_directory))
    return;

  template_file = g_object_get_qdata (G_OBJECT (menu_item), lunar_launcher_file_quark);

  if (template_file != NULL)
    {
      /* generate a title for the create dialog */
      title = g_strdup_printf (_("Create Document from template \"%s\""),
                               lunar_file_get_display_name (template_file));

      /* ask the user to enter a name for the new document */
      name = lunar_dialogs_show_create (launcher->widget,
                                         lunar_file_get_content_type (LUNAR_FILE (template_file)),
                                         lunar_file_get_display_name (template_file),
                                         title);
      /* cleanup */
      g_free (title);
    }
  else
    {
      /* ask the user to enter a name for the new empty file */
      name = lunar_dialogs_show_create (launcher->widget,
                                         "text/plain",
                                         _("New Empty File"),
                                         _("New Empty File..."));
    }

  if (G_LIKELY (name != NULL))
    {
      if (G_LIKELY (launcher->parent_folder != NULL))
        {
          /* fake the target path list */
          if (LUNAR_IS_TREE_VIEW (launcher->widget) && launcher->files_are_selected && launcher->single_directory_to_process)
            target_path_list.data = g_file_get_child (lunar_file_get_file (launcher->single_folder), name);
          else
            target_path_list.data = g_file_get_child (lunar_file_get_file (launcher->current_directory), name);
          target_path_list.next = NULL;
          target_path_list.prev = NULL;

          /* launch the operation */
          application = lunar_application_get ();
          lunar_application_creat (application, launcher->widget, &target_path_list,
                                    template_file != NULL ? lunar_file_get_file (template_file) : NULL,
                                    launcher->select_files_closure);
          g_object_unref (G_OBJECT (application));

          /* release the target path */
          g_object_unref (target_path_list.data);
        }

      /* release the file name */
      g_free (name);
    }
}



/* helper method in order to find the parent menu for a menu item */
static GtkWidget *
lunar_launcher_create_document_submenu_templates_find_parent_menu (LunarFile *file,
                                                                    GList      *dirs,
                                                                    GList      *items)
{
  GtkWidget *parent_menu = NULL;
  GFile     *parent;
  GList     *lp;
  GList     *ip;

  /* determine the parent of the file */
  parent = g_file_get_parent (lunar_file_get_file (file));

  /* check if the file has a parent at all */
  if (parent == NULL)
    return NULL;

  /* iterate over all dirs and menu items */
  for (lp = g_list_first (dirs), ip = g_list_first (items);
       parent_menu == NULL && lp != NULL && ip != NULL;
       lp = lp->next, ip = ip->next)
    {
      /* check if the current dir/item is the parent of our file */
      if (g_file_equal (parent, lunar_file_get_file (lp->data)))
        {
          /* we want to insert an item for the file in this menu */
          parent_menu = gtk_menu_item_get_submenu (ip->data);
        }
    }

  /* destroy the parent GFile */
  g_object_unref (parent);

  return parent_menu;
}



/* recursive helper method in order to create menu items for all available templates */
static gboolean
lunar_launcher_create_document_submenu_templates (LunarLauncher *launcher,
                                                   GtkWidget      *create_file_submenu,
                                                   GList          *files)
{
  LunarIconFactory *icon_factory;
  LunarFile        *file;
  GdkPixbuf         *icon;
  GtkWidget         *parent_menu;
  GtkWidget         *submenu;
  GtkWidget         *image;
  GtkWidget         *item;
  GList             *lp;
  GList             *dirs = NULL;
  GList             *items = NULL;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), FALSE);

  /* do nothing if there is no menu */
  if (create_file_submenu == NULL)
    return FALSE;

  /* get the icon factory */
  icon_factory = lunar_icon_factory_get_default ();

  /* sort items so that directories come before files and ancestors come
   * before descendants */
  files = g_list_sort (files, (GCompareFunc) (void (*)(void)) lunar_file_compare_by_type);

  for (lp = g_list_first (files); lp != NULL; lp = lp->next)
    {
      file = lp->data;

      /* determine the parent menu for this file/directory */
      parent_menu = lunar_launcher_create_document_submenu_templates_find_parent_menu (file, dirs, items);
      if (parent_menu == NULL)
        parent_menu = create_file_submenu;

      /* determine the icon for this file/directory */
      icon = lunar_icon_factory_load_file_icon (icon_factory, file, LUNAR_FILE_ICON_STATE_DEFAULT, 16);

      /* allocate an image based on the icon */
      image = gtk_image_new_from_pixbuf (icon);

      item = expidus_gtk_image_menu_item_new (lunar_file_get_display_name (file), NULL, NULL, NULL, NULL, image, GTK_MENU_SHELL (parent_menu));
      if (lunar_file_is_directory (file))
        {
          /* allocate a new submenu for the directory */
          submenu = gtk_menu_new ();

          /* allocate a new menu item for the directory */
          gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), submenu);

          /* prepend the directory, its item and the parent menu it should
           * later be added to to the respective lists */
          dirs = g_list_prepend (dirs, file);
          items = g_list_prepend (items, item);
        }
      else
        {
          g_signal_connect_swapped (G_OBJECT (item), "activate", G_CALLBACK (lunar_launcher_action_create_document), launcher);
          g_object_set_qdata_full (G_OBJECT (item), lunar_launcher_file_quark, g_object_ref (file), g_object_unref);
        }
      /* release the icon reference */
      g_object_unref (icon);
    }

  /* destroy lists */
  g_list_free (dirs);
  g_list_free (items);

  /* release the icon factory */
  g_object_unref (icon_factory);

  /* let the job destroy the file list */
  return FALSE;
}



/**
 * lunar_launcher_create_document_submenu_new:
 * @launcher : a #LunarLauncher instance
 *
 * Will create a complete 'create_document* #GtkMenu and return it
 *
 * Return value: (transfer full): the created #GtkMenu
 **/
static GtkWidget*
lunar_launcher_create_document_submenu_new (LunarLauncher *launcher)
{
  GList           *files = NULL;
  GFile           *home_dir;
  GFile           *templates_dir = NULL;
  const gchar     *path;
  gchar           *template_path;
  gchar           *label_text;
  GtkWidget       *submenu;
  GtkWidget       *item;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), NULL);

  home_dir = lunar_g_file_new_for_home ();
  path = g_get_user_special_dir (G_USER_DIRECTORY_TEMPLATES);

  if (G_LIKELY (path != NULL))
    templates_dir = g_file_new_for_path (path);

  /* If G_USER_DIRECTORY_TEMPLATES not found, set "~/Templates" directory as default */
  if (G_UNLIKELY (path == NULL) || G_UNLIKELY (g_file_equal (templates_dir, home_dir)))
    {
      if (templates_dir != NULL)
        g_object_unref (templates_dir);
      templates_dir = g_file_resolve_relative_path (home_dir, "Templates");
    }

  if (G_LIKELY (templates_dir != NULL))
    {
      /* load the LunarFiles */
      files = lunar_io_scan_directory (NULL, templates_dir, G_FILE_QUERY_INFO_NONE, TRUE, FALSE, TRUE, NULL);
    }

  submenu = gtk_menu_new();
  if (files == NULL)
    {
      template_path = g_file_get_path (templates_dir);
      label_text = g_strdup_printf (_("No templates installed in \"%s\""), template_path);
      item = expidus_gtk_image_menu_item_new (label_text, NULL, NULL, NULL, NULL, NULL, GTK_MENU_SHELL (submenu));
      gtk_widget_set_sensitive (item, FALSE);
      g_free (template_path);
      g_free (label_text);
    }
  else
    {
      lunar_launcher_create_document_submenu_templates (launcher, submenu, files);
      lunar_g_file_list_free (files);
    }

  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (submenu));
  expidus_gtk_image_menu_item_new_from_icon_name (_("_Empty File"), NULL, NULL, G_CALLBACK (lunar_launcher_action_create_document),
                                               G_OBJECT (launcher), "text-x-generic", GTK_MENU_SHELL (submenu));


  g_object_unref (templates_dir);
  g_object_unref (home_dir);

  return submenu;
}



static void
lunar_launcher_action_cut (LunarLauncher *launcher)
{
  LunarClipboardManager *clipboard;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->files_are_selected == FALSE || launcher->parent_folder == NULL)
    return;

  clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (launcher->widget));
  lunar_clipboard_manager_cut_files (clipboard, launcher->files_to_process);
  g_object_unref (G_OBJECT (clipboard));
}



static void
lunar_launcher_action_copy (LunarLauncher *launcher)
{
  LunarClipboardManager *clipboard;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (launcher->files_are_selected == FALSE)
    return;

  clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (launcher->widget));
  lunar_clipboard_manager_copy_files (clipboard, launcher->files_to_process);
  g_object_unref (G_OBJECT (clipboard));
}



static void
lunar_launcher_action_paste (LunarLauncher *launcher)
{
  LunarClipboardManager *clipboard;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (launcher->widget));
  lunar_clipboard_manager_paste_files (clipboard, lunar_file_get_file (launcher->current_directory), launcher->widget, launcher->select_files_closure);
  g_object_unref (G_OBJECT (clipboard));
}



static void
lunar_launcher_action_paste_into_folder (LunarLauncher *launcher)
{
  LunarClipboardManager *clipboard;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (!launcher->single_directory_to_process)
    return;

  clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (launcher->widget));
  lunar_clipboard_manager_paste_files (clipboard, lunar_file_get_file (launcher->single_folder), launcher->widget, launcher->select_files_closure);
  g_object_unref (G_OBJECT (clipboard));
}


/**
 * lunar_launcher_action_mount:
 * @launcher : a #LunarLauncher instance
*
 * Will mount the selected device, if any. The related folder will not be opened.
 **/
void
lunar_launcher_action_mount (LunarLauncher *launcher)
{
  lunar_launcher_poke (launcher, NULL,LUNAR_LAUNCHER_NO_ACTION);
}



static void
lunar_launcher_action_eject_finish (LunarDevice  *device,
                                      const GError *error,
                                      gpointer      user_data)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (user_data);
  gchar          *device_name;

  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  /* check if there was an error */
  if (error != NULL)
    {
      /* display an error dialog to inform the user */
      device_name = lunar_device_get_name (device);
      lunar_dialogs_show_error (GTK_WIDGET (launcher->widget), error, _("Failed to eject \"%s\""), device_name);
      g_free (device_name);
    }
  else
    launcher->device_to_process = NULL;

  g_object_unref (launcher);
}



/**
 * lunar_launcher_action_eject:
 * @launcher : a #LunarLauncher instance
*
 * Will eject the selected device, if any
 **/
void
lunar_launcher_action_eject (LunarLauncher *launcher)
{
  GMountOperation *mount_operation;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_LIKELY (launcher->device_to_process != NULL))
    {
      /* prepare a mount operation */
      mount_operation = lunar_gtk_mount_operation_new (GTK_WIDGET (launcher->widget));

      /* eject */
      lunar_device_eject (launcher->device_to_process,
                           mount_operation,
                           NULL,
                           lunar_launcher_action_eject_finish,
                           g_object_ref (launcher));

      g_object_unref (mount_operation);
    }
}



static void
lunar_launcher_action_unmount_finish (LunarDevice *device,
                                       const GError *error,
                                       gpointer      user_data)
{
  LunarLauncher *launcher = LUNAR_LAUNCHER (user_data);
  gchar          *device_name;

  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  /* check if there was an error */
  if (error != NULL)
    {
      /* display an error dialog to inform the user */
      device_name = lunar_device_get_name (device);
      lunar_dialogs_show_error (GTK_WIDGET (launcher->widget), error, _("Failed to unmount \"%s\""), device_name);
      g_free (device_name);
    }
  else
    launcher->device_to_process = NULL;

  g_object_unref (launcher);
}



/**
 * lunar_launcher_action_eject:
 * @launcher : a #LunarLauncher instance
*
 * Will unmount the selected device, if any
 **/
void
lunar_launcher_action_unmount (LunarLauncher *launcher)
{
  GMountOperation *mount_operation;

  _lunar_return_if_fail (LUNAR_IS_LAUNCHER (launcher));

  if (G_LIKELY (launcher->device_to_process != NULL))
    {
      /* prepare a mount operation */
      mount_operation = lunar_gtk_mount_operation_new (GTK_WIDGET (launcher->widget));

      /* eject */
      lunar_device_unmount (launcher->device_to_process,
                             mount_operation,
                             NULL,
                             lunar_launcher_action_unmount_finish,
                             g_object_ref (launcher));

      /* release the device */
      g_object_unref (mount_operation);
    }
}



static GtkWidget*
lunar_launcher_build_application_submenu (LunarLauncher *launcher,
                                           GList          *applications)
{
  GList     *lp;
  GtkWidget *submenu;
  GtkWidget *image;
  GtkWidget *item;
  gchar     *label_text;
  gchar     *tooltip_text;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), NULL);

  submenu =  gtk_menu_new();
  /* add open with subitem per application */
  for (lp = applications; lp != NULL; lp = lp->next)
    {
      label_text = g_strdup_printf (_("Open With \"%s\""), g_app_info_get_name (lp->data));
      tooltip_text = g_strdup_printf (ngettext ("Use \"%s\" to open the selected file",
                                           "Use \"%s\" to open the selected files",
                                           launcher->n_files_to_process), g_app_info_get_name (lp->data));
      image = gtk_image_new_from_gicon (g_app_info_get_icon (lp->data), GTK_ICON_SIZE_MENU);
      item = expidus_gtk_image_menu_item_new (label_text, tooltip_text, NULL, G_CALLBACK (lunar_launcher_menu_item_activated), G_OBJECT (launcher), image, GTK_MENU_SHELL (submenu));
      g_object_set_qdata_full (G_OBJECT (item), lunar_launcher_appinfo_quark, g_object_ref (lp->data), g_object_unref);
      g_free (tooltip_text);
      g_free (label_text);
    }

  if (launcher->n_files_to_process == 1)
    {
      expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (submenu));
      lunar_launcher_append_menu_item (launcher, GTK_MENU_SHELL (submenu), LUNAR_LAUNCHER_ACTION_OPEN_WITH_OTHER, FALSE);
    }

  return submenu;
}



/**
 * lunar_launcher_append_open_section:
 * @launcher                 : a #LunarLauncher instance
 * @menu                     : #GtkMenuShell on which the open section should be appended
 * @support_tabs             : Set to TRUE if 'open in new tab' should be shown
 * @support_change_directory : Set to TRUE if 'open' should be shown
 * @force                    : Append the open section, even if the selected folder is the current folder
 *
 * Will append the section "open/open in new window/open in new tab/open with" to the provided #GtkMenuShell
 *
 * Return value: TRUE if the section was added
 **/
gboolean
lunar_launcher_append_open_section (LunarLauncher *launcher,
                                     GtkMenuShell   *menu,
                                     gboolean        support_tabs,
                                     gboolean        support_change_directory,
                                     gboolean        force)
{
  GList     *applications;
  gchar     *label_text;
  gchar     *tooltip_text;
  GtkWidget *image;
  GtkWidget *menu_item;
  GtkWidget *submenu;

  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), FALSE);

  /* Usually it is not required to open the current directory */
  if (launcher->files_are_selected == FALSE && !force)
    return FALSE;

  /* determine the set of applications that work for all selected files */
  applications = lunar_file_list_get_applications (launcher->files_to_process);

  /* Execute OR Open OR OpenWith */
  if (G_UNLIKELY (launcher->n_executables_to_process == launcher->n_files_to_process))
    lunar_launcher_append_menu_item (launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_EXECUTE, FALSE);
  else if (G_LIKELY (launcher->n_directories_to_process >= 1))
    {
      if (support_change_directory)
        lunar_launcher_append_menu_item (launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_OPEN, FALSE);
    }
  else if (G_LIKELY (applications != NULL))
    {
      label_text = g_strdup_printf (_("_Open With \"%s\""), g_app_info_get_name (applications->data));
      tooltip_text = g_strdup_printf (ngettext ("Use \"%s\" to open the selected file",
                                                "Use \"%s\" to open the selected files",
                                                launcher->n_files_to_process), g_app_info_get_name (applications->data));

      image = gtk_image_new_from_gicon (g_app_info_get_icon (applications->data), GTK_ICON_SIZE_MENU);
      menu_item = expidus_gtk_image_menu_item_new (label_text, tooltip_text, NULL, G_CALLBACK (lunar_launcher_menu_item_activated),
                                                G_OBJECT (launcher), image, menu);

      /* remember the default application for the "Open" action as quark */
      g_object_set_qdata_full (G_OBJECT (menu_item), lunar_launcher_appinfo_quark, applications->data, g_object_unref);
      g_free (tooltip_text);
      g_free (label_text);

      /* drop the default application from the list */
      applications = g_list_delete_link (applications, applications);
    }
  else
    {
      /* we can only show a generic "Open" action */
      label_text = g_strdup_printf (_("_Open With Default Applications"));
      tooltip_text = g_strdup_printf (ngettext ("Open the selected file with the default application",
                                                "Open the selected files with the default applications", launcher->n_files_to_process));
      expidus_gtk_menu_item_new (label_text, tooltip_text, NULL, G_CALLBACK (lunar_launcher_menu_item_activated), G_OBJECT (launcher), menu);
      g_free (tooltip_text);
      g_free (label_text);
    }

  if (launcher->n_files_to_process == launcher->n_directories_to_process && launcher->n_directories_to_process >= 1)
    {
      if (support_tabs)
        lunar_launcher_append_menu_item (launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_OPEN_IN_TAB, FALSE);
      lunar_launcher_append_menu_item (launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_OPEN_IN_WINDOW, FALSE);
    }

  if (G_LIKELY (applications != NULL))
    {
      menu_item = expidus_gtk_menu_item_new (_("Open With"),
                                          _("Choose another application with which to open the selected file"),
                                          NULL, NULL, NULL, menu);
      submenu = lunar_launcher_build_application_submenu (launcher, applications);
      gtk_menu_item_set_submenu (GTK_MENU_ITEM (menu_item), submenu);
    }
  else
    {
      if (launcher->n_files_to_process == 1)
        lunar_launcher_append_menu_item (launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_OPEN_WITH_OTHER, FALSE);
    }

  g_list_free_full (applications, g_object_unref);
  return TRUE;
}



/**
 * lunar_launcher_get_widget:
 * @launcher : a #LunarLauncher instance
 *
 * Will return the parent widget of this #LunarLauncher
 *
 * Return value: (transfer none): the parent widget of this #LunarLauncher
 **/
GtkWidget*
lunar_launcher_get_widget (LunarLauncher *launcher)
{
  _lunar_return_val_if_fail (LUNAR_IS_LAUNCHER (launcher), NULL);
  return launcher->widget;
}
