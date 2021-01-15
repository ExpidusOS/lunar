/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@expidus.org>
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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <gdk/gdkkeysyms.h>

#include <lunar/lunar-application.h>
#include <lunar/lunar-browser.h>
#include <lunar/lunar-clipboard-manager.h>
#include <lunar/lunar-compact-view.h>
#include <lunar/lunar-details-view.h>
#include <lunar/lunar-dialogs.h>
#include <lunar/lunar-shortcuts-pane.h>
#include <lunar/lunar-gio-extensions.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-gtk-extensions.h>
#include <lunar/lunar-history.h>
#include <lunar/lunar-icon-view.h>
#include <lunar/lunar-launcher.h>
#include <lunar/lunar-location-buttons.h>
#include <lunar/lunar-location-entry.h>
#include <lunar/lunar-marshal.h>
#include <lunar/lunar-menu.h>
#include <lunar/lunar-pango-extensions.h>
#include <lunar/lunar-preferences-dialog.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-util.h>
#include <lunar/lunar-statusbar.h>
#include <lunar/lunar-tree-pane.h>
#include <lunar/lunar-window.h>
#include <lunar/lunar-device-monitor.h>

#include <glib.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
  PROP_ZOOM_LEVEL,
  PROP_DIRECTORY_SPECIFIC_SETTINGS,
};

/* Signal identifiers */
enum
{
  BACK,
  RELOAD,
  TOGGLE_SIDEPANE,
  TOGGLE_MENUBAR,
  ZOOM_IN,
  ZOOM_OUT,
  ZOOM_RESET,
  TAB_CHANGE,
  LAST_SIGNAL,
};

struct _LunarBookmark
{
  GFile *g_file;
  gchar *name;
};
typedef struct _LunarBookmark LunarBookmark;



static void      lunar_window_screen_changed             (GtkWidget              *widget,
                                                           GdkScreen              *old_screen,
                                                           gpointer                userdata);
static void      lunar_window_dispose                    (GObject                *object);
static void      lunar_window_finalize                   (GObject                *object);
static gboolean  lunar_window_delete                     (GtkWidget              *widget,
                                                           GdkEvent               *event,
                                                           gpointer                data);
static void      lunar_window_get_property               (GObject                *object,
                                                           guint                   prop_id,
                                                           GValue                 *value,
                                                           GParamSpec             *pspec);
static void      lunar_window_set_property               (GObject                *object,
                                                           guint                   prop_id,
                                                           const GValue           *value,
                                                           GParamSpec             *pspec);
static gboolean  lunar_window_reload                     (LunarWindow           *window,
                                                           gboolean                reload_info);
static gboolean  lunar_window_toggle_sidepane            (LunarWindow           *window);
static gboolean  lunar_window_zoom_in                    (LunarWindow           *window);
static gboolean  lunar_window_zoom_out                   (LunarWindow           *window);
static gboolean  lunar_window_zoom_reset                 (LunarWindow           *window);
static gboolean  lunar_window_tab_change                 (LunarWindow           *window,
                                                           gint                    nth);
static void      lunar_window_realize                    (GtkWidget              *widget);
static void      lunar_window_unrealize                  (GtkWidget              *widget);
static gboolean  lunar_window_configure_event            (GtkWidget              *widget,
                                                           GdkEventConfigure      *event);
static void      lunar_window_notebook_switch_page       (GtkWidget              *notebook,
                                                           GtkWidget              *page,
                                                           guint                   page_num,
                                                           LunarWindow           *window);
static void      lunar_window_notebook_page_added        (GtkWidget              *notebook,
                                                           GtkWidget              *page,
                                                           guint                   page_num,
                                                           LunarWindow           *window);
static void      lunar_window_notebook_page_removed      (GtkWidget              *notebook,
                                                           GtkWidget              *page,
                                                           guint                   page_num,
                                                           LunarWindow           *window);
static gboolean  lunar_window_notebook_button_press_event(GtkWidget              *notebook,
                                                           GdkEventButton         *event,
                                                           LunarWindow           *window);
static gboolean  lunar_window_notebook_popup_menu        (GtkWidget              *notebook,
                                                           LunarWindow           *window);
static gpointer  lunar_window_notebook_create_window     (GtkWidget              *notebook,
                                                           GtkWidget              *page,
                                                           gint                    x,
                                                           gint                    y,
                                                           LunarWindow           *window);
static GtkWidget*lunar_window_notebook_insert_page       (LunarWindow           *window,
                                                           LunarFile             *directory,
                                                           GType                   view_type,
                                                           gint                    position,
                                                           LunarHistory          *history);
static void      lunar_window_notebook_select_current_page(LunarWindow           *window);

static GtkWidget*lunar_window_paned_notebooks_add        (LunarWindow           *window);
static void      lunar_window_paned_notebooks_switch     (LunarWindow           *window);
static gboolean  lunar_window_paned_notebooks_select     (GtkWidget              *notebook,
                                                           GtkDirectionType       *direction,
                                                           LunarWindow           *window);
static void      lunar_window_paned_notebooks_indicate_focus (LunarWindow       *window,
                                                           GtkWidget              *notebook);
static gboolean  lunar_window_split_view_is_active       (LunarWindow           *window);

static void      lunar_window_update_location_bar_visible(LunarWindow           *window);
static void      lunar_window_handle_reload_request      (LunarWindow           *window);
static void      lunar_window_install_sidepane           (LunarWindow           *window,
                                                           GType                   type);
static void      lunar_window_start_open_location        (LunarWindow           *window,
                                                           const gchar            *initial_text);
static void      lunar_window_action_open_new_tab        (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_open_new_window     (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_detach_tab          (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_close_all_windows   (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_close_tab           (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_close_window        (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_preferences         (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_reload              (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_action_toggle_split_view   (LunarWindow           *window);
static void      lunar_window_action_switch_next_tab     (LunarWindow           *window);
static void      lunar_window_action_switch_previous_tab (LunarWindow           *window);
static void      lunar_window_action_pathbar_changed     (LunarWindow           *window);
static void      lunar_window_action_toolbar_changed     (LunarWindow           *window);
static void      lunar_window_action_shortcuts_changed   (LunarWindow           *window);
static void      lunar_window_action_tree_changed        (LunarWindow           *window);
static void      lunar_window_action_statusbar_changed   (LunarWindow           *window);
static void      lunar_window_action_menubar_changed     (LunarWindow           *window);
static void      lunar_window_action_detailed_view       (LunarWindow           *window);
static void      lunar_window_action_icon_view           (LunarWindow           *window);
static void      lunar_window_action_compact_view        (LunarWindow           *window);
static void      lunar_window_replace_view               (LunarWindow           *window,
                                                           GtkWidget              *view,
                                                           GType                   view_type);
static void      lunar_window_action_view_changed        (LunarWindow           *window,
                                                           GType                   view_type);
static void      lunar_window_action_go_up               (LunarWindow           *window);
static void      lunar_window_action_back                (LunarWindow           *window);
static void      lunar_window_action_forward             (LunarWindow           *window);
static void      lunar_window_action_open_home           (LunarWindow           *window);
static void      lunar_window_action_open_desktop        (LunarWindow           *window);
static void      lunar_window_action_open_computer       (LunarWindow           *window);
static void      lunar_window_action_open_templates      (LunarWindow           *window);
static void      lunar_window_action_open_file_system    (LunarWindow           *window);
static void      lunar_window_action_open_trash          (LunarWindow           *window);
static void      lunar_window_action_open_network        (LunarWindow           *window);
static void      lunar_window_action_open_bookmark       (GFile                  *g_file);
static void      lunar_window_action_open_location       (LunarWindow           *window);
static void      lunar_window_action_contents            (LunarWindow           *window);
static void      lunar_window_action_about               (LunarWindow           *window);
static void      lunar_window_action_show_hidden         (LunarWindow           *window);
static gboolean  lunar_window_propagate_key_event        (GtkWindow              *window,
                                                           GdkEvent               *key_event,
                                                           gpointer                user_data);
static void      lunar_window_action_open_file_menu      (LunarWindow           *window);
static void      lunar_window_current_directory_changed  (LunarFile             *current_directory,
                                                           LunarWindow           *window);
static void      lunar_window_menu_item_selected         (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_menu_item_deselected       (LunarWindow           *window,
                                                           GtkWidget              *menu_item);
static void      lunar_window_notify_loading             (LunarView             *view,
                                                           GParamSpec             *pspec,
                                                           LunarWindow           *window);
static void      lunar_window_device_pre_unmount         (LunarDeviceMonitor    *device_monitor,
                                                           LunarDevice           *device,
                                                           GFile                  *root_file,
                                                           LunarWindow           *window);
static void      lunar_window_device_changed             (LunarDeviceMonitor    *device_monitor,
                                                           LunarDevice           *device,
                                                           LunarWindow           *window);
static gboolean  lunar_window_save_paned                 (LunarWindow           *window);
static gboolean  lunar_window_save_geometry_timer        (gpointer                user_data);
static void      lunar_window_save_geometry_timer_destroy(gpointer                user_data);
static void      lunar_window_set_zoom_level             (LunarWindow           *window,
                                                           LunarZoomLevel         zoom_level);
static void      lunar_window_update_window_icon         (LunarWindow           *window);
static void      lunar_window_create_menu                (LunarWindow           *window,
                                                           LunarWindowAction      action,
                                                           GCallback               cb_update_menu);
static void      lunar_window_update_file_menu           (LunarWindow           *window,
                                                           GtkWidget              *menu);
static void      lunar_window_update_edit_menu           (LunarWindow           *window,
                                                           GtkWidget              *menu);
static void      lunar_window_update_view_menu           (LunarWindow           *window,
                                                           GtkWidget              *menu);
static void      lunar_window_update_go_menu             (LunarWindow           *window,
                                                           GtkWidget              *menu);
static void      lunar_window_update_help_menu           (LunarWindow           *window,
                                                           GtkWidget              *menu);
static void      lunar_window_select_files               (LunarWindow           *window,
                                                           GList                  *path_list);
static void      lunar_window_binding_create             (LunarWindow           *window,
                                                           gpointer                src_object,
                                                           const gchar            *src_prop,
                                                           gpointer                dst_object,
                                                           const                   gchar *dst_prop,
                                                           GBindingFlags           flags);
static gboolean  lunar_window_history_clicked            (GtkWidget              *button,
                                                           GdkEventButton         *event,
                                                           GtkWidget              *window);
static gboolean  lunar_window_button_press_event         (GtkWidget              *view,
                                                           GdkEventButton         *event,
                                                           LunarWindow           *window);
static void      lunar_window_history_changed            (LunarWindow           *window);
static void      lunar_window_update_bookmarks           (LunarWindow           *window);
static void      lunar_window_free_bookmarks             (LunarWindow           *window);
static void      lunar_window_menu_add_bookmarks         (LunarWindow           *window,
                                                           GtkMenuShell           *view_menu);
static gboolean  lunar_window_check_uca_key_activation   (LunarWindow           *window,
                                                           GdkEventKey            *key_event,
                                                           gpointer                user_data);
static void      lunar_window_set_directory_specific_settings (LunarWindow      *window,
                                                                gboolean           directory_specific_settings);
static void      lunar_window_set_current_directory_gfile     (LunarWindow      *window,
                                                                GFile             *current_directory);
static GType     lunar_window_view_type_for_directory         (LunarWindow      *window,
                                                                LunarFile        *directory);
static void      lunar_window_action_clear_directory_specific_settings (LunarWindow  *window);



struct _LunarWindowClass
{
  GtkWindowClass __parent__;

  /* internal action signals */
  gboolean (*reload)          (LunarWindow *window,
                               gboolean      reload_info);
  gboolean (*zoom_in)         (LunarWindow *window);
  gboolean (*zoom_out)        (LunarWindow *window);
  gboolean (*zoom_reset)      (LunarWindow *window);
  gboolean (*tab_change)      (LunarWindow *window,
                               gint          idx);
};

struct _LunarWindow
{
  GtkWindow __parent__;

  /* support for custom preferences actions */
  LunarxProviderFactory *provider_factory;
  GList                  *lunarx_preferences_providers;

  GFile                  *bookmark_file;
  GList                  *bookmarks;
  GFileMonitor           *bookmark_monitor;

  LunarClipboardManager *clipboard;

  LunarPreferences      *preferences;

  LunarIconFactory      *icon_factory;

  /* to be able to change folder on "device-pre-unmount" if required */
  LunarDeviceMonitor    *device_monitor;

  GtkWidget              *grid;
  GtkWidget              *menubar;
  GtkWidget              *spinner;
  GtkWidget              *paned;
  GtkWidget              *sidepane;
  GtkWidget              *view_box;

  /* split view panes */
  GtkWidget              *paned_notebooks;
  GtkWidget              *notebook_selected;
  GtkWidget              *notebook_left;
  GtkWidget              *notebook_right;

  GtkWidget              *view;
  GtkWidget              *statusbar;

  GType                   view_type;
  GSList                 *view_bindings;

  /* support for two different styles of location bars */
  GtkWidget              *location_bar;
  GtkWidget              *location_toolbar;

  /* we need to maintain pointers to be able to toggle sensitivity */
  GtkWidget              *location_toolbar_item_back;
  GtkWidget              *location_toolbar_item_forward;
  GtkWidget              *location_toolbar_item_parent;

  LunarLauncher         *launcher;

  gulong                  signal_handler_id_history_changed;

  LunarFile             *current_directory;
  GtkAccelGroup          *accel_group;

  /* zoom-level support */
  LunarZoomLevel         zoom_level;

  gboolean                show_hidden;

  gboolean                directory_specific_settings;

  /* support to remember window geometry */
  guint                   save_geometry_timer_id;

  /* support to toggle side pane using F9,
   * see the toggle_sidepane() function.
   */
  GType                   toggle_sidepane_type;

  /* Takes care to select a file after e.g. rename/create */
  GClosure               *select_files_closure;
};



static ExpidusGtkActionEntry lunar_window_action_entries[] =
{
    { LUNAR_WINDOW_ACTION_FILE_MENU,                      "<Actions>/LunarWindow/file-menu",                       "",                     EXPIDUS_GTK_MENU_ITEM,       N_ ("_File"),                  NULL, NULL, NULL,},
    { LUNAR_WINDOW_ACTION_NEW_TAB,                        "<Actions>/LunarWindow/new-tab",                         "<Primary>t",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("New _Tab"),               N_ ("Open a new tab for the displayed location"),                                    "tab-new",                 G_CALLBACK (lunar_window_action_open_new_tab),       },
    { LUNAR_WINDOW_ACTION_NEW_WINDOW,                     "<Actions>/LunarWindow/new-window",                      "<Primary>n",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("New _Window"),            N_ ("Open a new Lunar window for the displayed location"),                          "window-new",              G_CALLBACK (lunar_window_action_open_new_window),    },
    { LUNAR_WINDOW_ACTION_DETACH_TAB,                     "<Actions>/LunarWindow/detach-tab",                      "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Detac_h Tab"),            N_ ("Open current folder in a new window"),                                          NULL,                      G_CALLBACK (lunar_window_action_detach_tab),         },
    { LUNAR_WINDOW_ACTION_CLOSE_TAB,                      "<Actions>/LunarWindow/close-tab",                       "<Primary>w",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("C_lose Tab"),             N_ ("Close this folder"),                                                            "window-close",            G_CALLBACK (lunar_window_action_close_tab),          },
    { LUNAR_WINDOW_ACTION_CLOSE_WINDOW,                   "<Actions>/LunarWindow/close-window",                    "<Primary>q",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Close Window"),          N_ ("Close this window"),                                                            "application-exit",        G_CALLBACK (lunar_window_action_close_window),       },
    { LUNAR_WINDOW_ACTION_CLOSE_ALL_WINDOWS,              "<Actions>/LunarWindow/close-all-windows",               "<Primary><Shift>w",    EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Close _All Windows"),     N_ ("Close all Lunar windows"),                                                     NULL,                      G_CALLBACK (lunar_window_action_close_all_windows),  },

    { LUNAR_WINDOW_ACTION_EDIT_MENU,                      "<Actions>/LunarWindow/edit-menu",                       "",                     EXPIDUS_GTK_MENU_ITEM,       N_ ("_Edit"),                  NULL,                                                                                NULL,                      NULL,                                                 },
    { LUNAR_WINDOW_ACTION_PREFERENCES,                    "<Actions>/LunarWindow/preferences",                     "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Pr_eferences..."),        N_ ("Edit Lunars Preferences"),                                                     "preferences-system",      G_CALLBACK (lunar_window_action_preferences),        },

    { LUNAR_WINDOW_ACTION_VIEW_MENU,                      "<Actions>/LunarWindow/view-menu",                       "",                     EXPIDUS_GTK_MENU_ITEM,       N_ ("_View"),                  NULL,                                                                                NULL,                      NULL,                                                 },
    { LUNAR_WINDOW_ACTION_RELOAD,                         "<Actions>/LunarWindow/reload",                          "<Primary>r",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Reload"),                N_ ("Reload the current folder"),                                                    "view-refresh-symbolic",   G_CALLBACK (lunar_window_action_reload),             },
    { LUNAR_WINDOW_ACTION_RELOAD_ALT,                     "<Actions>/LunarWindow/reload-alt",                      "F5",                   EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                          NULL,                                                                                NULL,                      G_CALLBACK (lunar_window_action_reload),             },
    { LUNAR_WINDOW_ACTION_VIEW_SPLIT,                     "<Actions>/LunarWindow/toggle-split-view",               "F3",                   EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("Spl_it View"),            N_ ("Open/Close Split View"),                                                        NULL,                      G_CALLBACK (lunar_window_action_toggle_split_view),  },
    { LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_MENU,    "<Actions>/LunarWindow/view-location-selector-menu",     "",                     EXPIDUS_GTK_MENU_ITEM,       N_ ("_Location Selector"),     NULL,                                                                                NULL,                      NULL,                                                 },
    { LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_PATHBAR, "<Actions>/LunarWindow/view-location-selector-pathbar",  "",                     EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("_Pathbar Style"),         N_ ("Modern approach with buttons that correspond to folders"),                      NULL,                      G_CALLBACK (lunar_window_action_pathbar_changed),    },
    { LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_TOOLBAR, "<Actions>/LunarWindow/view-location-selector-toolbar",  "",                     EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("_Toolbar Style"),         N_ ("Traditional approach with location bar and navigation buttons"),                NULL,                      G_CALLBACK (lunar_window_action_toolbar_changed),    },
    { LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_MENU,            "<Actions>/LunarWindow/view-side-pane-menu",             "",                     EXPIDUS_GTK_MENU_ITEM,       N_ ("_Side Pane"),             NULL,                                                                                NULL,                      NULL,                                                 },
    { LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_SHORTCUTS,       "<Actions>/LunarWindow/view-side-pane-shortcuts",        "<Primary>b",           EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("_Shortcuts"),             N_ ("Toggles the visibility of the shortcuts pane"),                                 NULL,                      G_CALLBACK (lunar_window_action_shortcuts_changed),  },
    { LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_TREE,            "<Actions>/LunarWindow/view-side-pane-tree",             "<Primary>e",           EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("_Tree"),                  N_ ("Toggles the visibility of the tree pane"),                                      NULL,                      G_CALLBACK (lunar_window_action_tree_changed),       },
    { LUNAR_WINDOW_ACTION_TOGGLE_SIDE_PANE,               "<Actions>/LunarWindow/toggle-side-pane",                "F9",                   EXPIDUS_GTK_MENU_ITEM,       NULL,                          NULL,                                                                                NULL,                      G_CALLBACK (lunar_window_toggle_sidepane),           },
    { LUNAR_WINDOW_ACTION_VIEW_STATUSBAR,                 "<Actions>/LunarWindow/view-statusbar",                  "",                     EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("St_atusbar"),             N_ ("Change the visibility of this window's statusbar"),                             NULL,                      G_CALLBACK (lunar_window_action_statusbar_changed),  },
    { LUNAR_WINDOW_ACTION_VIEW_MENUBAR,                   "<Actions>/LunarWindow/view-menubar",                    "<Primary>m",           EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("_Menubar"),               N_ ("Change the visibility of this window's menubar"),                               NULL,                      G_CALLBACK (lunar_window_action_menubar_changed),    },
    { LUNAR_WINDOW_ACTION_SHOW_HIDDEN,                    "<Actions>/LunarWindow/show-hidden",                     "<Primary>h",           EXPIDUS_GTK_CHECK_MENU_ITEM, N_ ("Show _Hidden Files"),     N_ ("Toggles the display of hidden files in the current window"),                    NULL,                      G_CALLBACK (lunar_window_action_show_hidden),        },
    { LUNAR_WINDOW_ACTION_ZOOM_IN,                        "<Actions>/LunarWindow/zoom-in",                         "<Primary>KP_Add",      EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Zoom I_n"),               N_ ("Show the contents in more detail"),                                             "zoom-in-symbolic",        G_CALLBACK (lunar_window_zoom_in),                   },
    { LUNAR_WINDOW_ACTION_ZOOM_OUT,                       "<Actions>/LunarWindow/zoom-out",                        "<Primary>KP_Subtract", EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Zoom _Out"),              N_ ("Show the contents in less detail"),                                             "zoom-out-symbolic",       G_CALLBACK (lunar_window_zoom_out),                  },
    { LUNAR_WINDOW_ACTION_ZOOM_RESET,                     "<Actions>/LunarWindow/zoom-reset",                      "<Primary>KP_0",        EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Normal Si_ze"),           N_ ("Show the contents at the normal size"),                                         "zoom-original-symbolic",  G_CALLBACK (lunar_window_zoom_reset),                },
    { LUNAR_WINDOW_ACTION_CLEAR_DIRECTORY_SPECIFIC_SETTINGS,"<Actions>/LunarWindow/clear-directory-specific-settings","",                  EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Clear Saved _Folder View Settings"), N_ ("Delete saved view settings for this folder"),                         NULL,                     G_CALLBACK (lunar_window_action_clear_directory_specific_settings), },
    { LUNAR_WINDOW_ACTION_VIEW_AS_ICONS,                  "<Actions>/LunarWindow/view-as-icons",                   "<Primary>1",           EXPIDUS_GTK_RADIO_MENU_ITEM, N_ ("_Icon View"),             N_ ("Display folder content in an icon view"),                                        NULL,                      G_CALLBACK (lunar_window_action_icon_view),          },
    { LUNAR_WINDOW_ACTION_VIEW_AS_DETAILED_LIST,          "<Actions>/LunarWindow/view-as-detailed-list",           "<Primary>2",           EXPIDUS_GTK_RADIO_MENU_ITEM, N_ ("_List View"),             N_ ("Display folder content in a detailed list view"),                                NULL,                      G_CALLBACK (lunar_window_action_detailed_view),      },
    { LUNAR_WINDOW_ACTION_VIEW_AS_COMPACT_LIST,           "<Actions>/LunarWindow/view-as-compact-list",            "<Primary>3",           EXPIDUS_GTK_RADIO_MENU_ITEM, N_ ("_Compact View"),          N_ ("Display folder content in a compact list view"),                                 NULL,                      G_CALLBACK (lunar_window_action_compact_view),       },

    { LUNAR_WINDOW_ACTION_GO_MENU,                        "<Actions>/LunarWindow/go-menu",                         "",                     EXPIDUS_GTK_MENU_ITEM,       N_ ("_Go"),                    NULL,                                                                                NULL,                      NULL                                                  },
    { LUNAR_WINDOW_ACTION_OPEN_FILE_SYSTEM,               "<Actions>/LunarWindow/open-file-system",                "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("File System"),            N_ ("Browse the file system"),                                                       "drive-harddisk",          G_CALLBACK (lunar_window_action_open_file_system),   },
    { LUNAR_WINDOW_ACTION_OPEN_COMPUTER,                  "<Actions>/LunarWindow/open-computer",                    "",                    EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Computer"),               N_ ("Go to the computer folder"),                                                    "computer",                G_CALLBACK (lunar_window_action_open_computer),      },
    { LUNAR_WINDOW_ACTION_OPEN_HOME,                      "<Actions>/LunarWindow/open-home",                       "<Alt>Home",            EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Home"),                  N_ ("Go to the home folder"),                                                        "go-home-symbolic",        G_CALLBACK (lunar_window_action_open_home),          },
    { LUNAR_WINDOW_ACTION_OPEN_DESKTOP,                   "<Actions>/LunarWindow/open-desktop",                    "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Desktop"),                N_ ("Go to the desktop folder"),                                                     "user-desktop",            G_CALLBACK (lunar_window_action_open_desktop),       },
    { LUNAR_WINDOW_ACTION_OPEN_COMPUTER,                  "<Actions>/LunarWindow/open-computer",                   "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Computer"),               N_ ("Browse all local and remote disks and folders accessible from this computer"),  "computer",                G_CALLBACK (lunar_window_action_open_computer),      },
    { LUNAR_WINDOW_ACTION_OPEN_TRASH,                     "<Actions>/LunarWindow/open-trash",                      "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("T_rash"),                 N_ ("Display the contents of the trash can"),                                        NULL,                      G_CALLBACK (lunar_window_action_open_trash),         },
    { LUNAR_WINDOW_ACTION_OPEN_PARENT,                    "<Actions>/LunarWindow/open-parent",                     "<Alt>Up",              EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Open _Parent"),           N_ ("Open the parent folder"),                                                       "go-up-symbolic",          G_CALLBACK (lunar_window_action_go_up),              },
    { LUNAR_WINDOW_ACTION_OPEN_LOCATION,                  "<Actions>/LunarWindow/open-location",                   "<Primary>l",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Open Location..."),      N_ ("Specify a location to open"),                                                   NULL,                      G_CALLBACK (lunar_window_action_open_location),      },
    { LUNAR_WINDOW_ACTION_OPEN_LOCATION_ALT,              "<Actions>/LunarWindow/open-location-alt",               "<Alt>d",               EXPIDUS_GTK_MENU_ITEM,       "open-location-alt",           NULL,                                                                                NULL,                      G_CALLBACK (lunar_window_action_open_location),      },
    { LUNAR_WINDOW_ACTION_OPEN_TEMPLATES,                 "<Actions>/LunarWindow/open-templates",                  "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_("T_emplates"),              N_ ("Go to the templates folder"),                                                   "text-x-generic-template", G_CALLBACK (lunar_window_action_open_templates),     },
    { LUNAR_WINDOW_ACTION_OPEN_NETWORK,                   "<Actions>/LunarWindow/open-network",                    "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_("B_rowse Network"),         N_ ("Browse local network connections"),                                             "network-workgroup",       G_CALLBACK (lunar_window_action_open_network),       },

    { LUNAR_WINDOW_ACTION_HELP_MENU,                      "<Actions>/LunarWindow/contents/help-menu",              "",                     EXPIDUS_GTK_MENU_ITEM      , N_ ("_Help"),                  NULL, NULL, NULL},
    { LUNAR_WINDOW_ACTION_CONTENTS,                       "<Actions>/LunarWindow/contents",                        "F1",                   EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Contents"),              N_ ("Display Lunar user manual"),                                                   "help-browser",            G_CALLBACK (lunar_window_action_contents),            },
    { LUNAR_WINDOW_ACTION_ABOUT,                          "<Actions>/LunarWindow/about",                           "",                     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_About"),                 N_ ("Display information about Lunar"),                                             "help-about",              G_CALLBACK (lunar_window_action_about),               },
    { LUNAR_WINDOW_ACTION_BACK,                           "<Actions>/LunarStandardView/back",                      "<Alt>Left",            EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Back"),                   N_ ("Go to the previous visited folder"),                                            "go-previous-symbolic",    G_CALLBACK (lunar_window_action_back),                },
    { LUNAR_WINDOW_ACTION_BACK_ALT,                       "<Actions>/LunarStandardView/back-alt",                  "BackSpace",            EXPIDUS_GTK_IMAGE_MENU_ITEM, NULL,                          NULL,                                                                                NULL,                      G_CALLBACK (lunar_window_action_back),                },
    { LUNAR_WINDOW_ACTION_FORWARD,                        "<Actions>/LunarStandardView/forward",                   "<Alt>Right",           EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("Forward"),                N_ ("Go to the next visited folder"),                                                "go-next-symbolic",        G_CALLBACK (lunar_window_action_forward),             },
    { LUNAR_WINDOW_ACTION_SWITCH_PREV_TAB,                "<Actions>/LunarWindow/switch-previous-tab",             "<Primary>Page_Up",     EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Previous Tab"),          N_ ("Switch to Previous Tab"),                                                       "go-previous",             G_CALLBACK (lunar_window_action_switch_previous_tab), },
    { LUNAR_WINDOW_ACTION_SWITCH_NEXT_TAB,                "<Actions>/LunarWindow/switch-next-tab",                 "<Primary>Page_Down",   EXPIDUS_GTK_IMAGE_MENU_ITEM, N_ ("_Next Tab"),              N_ ("Switch to Next Tab"),                                                           "go-next",                 G_CALLBACK (lunar_window_action_switch_next_tab),     },
    { 0,                                                   "<Actions>/LunarWindow/open-file-menu",                  "F10",                  0,                        NULL,                          NULL,                                                                                NULL,                      G_CALLBACK (lunar_window_action_open_file_menu),      },
};

#define get_action_entry(id) expidus_gtk_get_action_entry_by_id(lunar_window_action_entries,G_N_ELEMENTS(lunar_window_action_entries),id)



static guint window_signals[LAST_SIGNAL];



G_DEFINE_TYPE_WITH_CODE (LunarWindow, lunar_window, GTK_TYPE_WINDOW,
                         G_IMPLEMENT_INTERFACE (LUNAR_TYPE_BROWSER, NULL))



static void
lunar_window_class_init (LunarWindowClass *klass)
{
  GtkWidgetClass *gtkwidget_class;
  GtkBindingSet  *binding_set;
  GObjectClass   *gobject_class;
  guint           i;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = lunar_window_dispose;
  gobject_class->finalize = lunar_window_finalize;
  gobject_class->get_property = lunar_window_get_property;
  gobject_class->set_property = lunar_window_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->realize = lunar_window_realize;
  gtkwidget_class->unrealize = lunar_window_unrealize;
  gtkwidget_class->configure_event = lunar_window_configure_event;

  klass->reload = lunar_window_reload;
  klass->zoom_in = lunar_window_zoom_in;
  klass->zoom_out = lunar_window_zoom_out;
  klass->zoom_reset = lunar_window_zoom_reset;
  klass->tab_change = lunar_window_tab_change;

  expidus_gtk_translate_action_entries (lunar_window_action_entries, G_N_ELEMENTS (lunar_window_action_entries));

  /**
   * LunarWindow:current-directory:
   *
   * The directory currently displayed within this #LunarWindow
   * or %NULL.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_CURRENT_DIRECTORY,
                                   g_param_spec_object ("current-directory",
                                                        "current-directory",
                                                        "current-directory",
                                                        LUNAR_TYPE_FILE,
                                                        ENDO_PARAM_READWRITE));

  /**
   * LunarWindow:zoom-level:
   *
   * The #LunarZoomLevel applied to the #LunarView currently
   * shown within this window.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_ZOOM_LEVEL,
                                   g_param_spec_enum ("zoom-level",
                                                      "zoom-level",
                                                      "zoom-level",
                                                      LUNAR_TYPE_ZOOM_LEVEL,
                                                      LUNAR_ZOOM_LEVEL_100_PERCENT,
                                                      ENDO_PARAM_READWRITE));

  /**
   * LunarWindow:directory-specific-settings:
   *
   * Whether to use directory specific settings.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_DIRECTORY_SPECIFIC_SETTINGS,
                                   g_param_spec_boolean ("directory-specific-settings",
                                                         "directory-specific-settings",
                                                         "directory-specific-settings",
                                                         FALSE,
                                                         ENDO_PARAM_READWRITE));

  /**
   * LunarWindow::reload:
   * @window : a #LunarWindow instance.
   *
   * Emitted whenever the user requests to reload the contents
   * of the currently displayed folder.
   **/
  window_signals[RELOAD] =
    g_signal_new (I_("reload"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (LunarWindowClass, reload),
                  g_signal_accumulator_true_handled, NULL,
                  _lunar_marshal_BOOLEAN__BOOLEAN,
                  G_TYPE_BOOLEAN, 1,
                  G_TYPE_BOOLEAN);

  /**
   * LunarWindow::zoom-in:
   * @window : a #LunarWindow instance.
   *
   * Emitted whenever the user requests to zoom in. This
   * is an internal signal used to bind the action to keys.
   **/
  window_signals[ZOOM_IN] =
    g_signal_new (I_("zoom-in"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (LunarWindowClass, zoom_in),
                  g_signal_accumulator_true_handled, NULL,
                  _lunar_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  /**
   * LunarWindow::zoom-out:
   * @window : a #LunarWindow instance.
   *
   * Emitted whenever the user requests to zoom out. This
   * is an internal signal used to bind the action to keys.
   **/
  window_signals[ZOOM_OUT] =
    g_signal_new (I_("zoom-out"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (LunarWindowClass, zoom_out),
                  g_signal_accumulator_true_handled, NULL,
                  _lunar_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  /**
   * LunarWindow::zoom-reset:
   * @window : a #LunarWindow instance.
   *
   * Emitted whenever the user requests reset the zoom level.
   * This is an internal signal used to bind the action to keys.
   **/
  window_signals[ZOOM_RESET] =
    g_signal_new (I_("zoom-reset"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (LunarWindowClass, zoom_reset),
                  g_signal_accumulator_true_handled, NULL,
                  _lunar_marshal_BOOLEAN__VOID,
                  G_TYPE_BOOLEAN, 0);

  /**
   * LunarWindow::tab-change:
   * @window : a #LunarWindow instance.
   * @idx    : tab index,
   *
   * Emitted whenever the user uses a Alt+N combination to
   * switch tabs.
   **/
  window_signals[TAB_CHANGE] =
    g_signal_new (I_("tab-change"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                  G_STRUCT_OFFSET (LunarWindowClass, tab_change),
                  g_signal_accumulator_true_handled, NULL,
                  _lunar_marshal_BOOLEAN__INT,
                  G_TYPE_BOOLEAN, 1,
                  G_TYPE_INT);

  /* setup the key bindings for the windows */
  binding_set = gtk_binding_set_by_class (klass);

  /* setup the key bindings for Alt+N */
  for (i = 0; i < 10; i++)
    {
      gtk_binding_entry_add_signal (binding_set, GDK_KEY_0 + i, GDK_MOD1_MASK,
                                    "tab-change", 1, G_TYPE_UINT, i - 1);
    }
}



static void
lunar_window_paned_notebooks_destroy (GtkWidget    *paned_notebooks,
                                       GtkWidget    *widget,
                                       LunarWindow *window)
{
  if (window->notebook_left)
    {
      gtk_widget_destroy (window->notebook_left);
      window->notebook_left = NULL;
    }
  if (window->notebook_right)
    {
      gtk_widget_destroy (window->notebook_right);
      window->notebook_right = NULL;
    }
}



static void
lunar_window_init (LunarWindow *window)
{
  GtkWidget       *label;
  GtkWidget       *infobar;
  GtkWidget       *item;
  gboolean         last_menubar_visible;
  gchar           *last_location_bar;
  gchar           *last_side_pane;
  GType            type;
  gint             last_separator_position;
  gint             last_window_width;
  gint             last_window_height;
  gboolean         last_window_maximized;
  gboolean         last_statusbar_visible;
  GtkToolItem     *tool_item;
  gboolean         small_icons;
  GtkStyleContext *context;

  /* unset the view type */
  window->view_type = G_TYPE_NONE;

  /* grab a reference on the provider factory and load the providers*/
  window->provider_factory = lunarx_provider_factory_get_default ();
  window->lunarx_preferences_providers = lunarx_provider_factory_list_providers (window->provider_factory, LUNARX_TYPE_PREFERENCES_PROVIDER);

  /* grab a reference on the preferences */
  window->preferences = lunar_preferences_get ();

  window->accel_group = gtk_accel_group_new ();
  expidus_gtk_accel_map_add_entries (lunar_window_action_entries, G_N_ELEMENTS (lunar_window_action_entries));
  expidus_gtk_accel_group_connect_action_entries (window->accel_group,
                                               lunar_window_action_entries,
                                               G_N_ELEMENTS (lunar_window_action_entries),
                                               window);

  gtk_window_add_accel_group (GTK_WINDOW (window), window->accel_group);

  /* get all properties for init */
  g_object_get (G_OBJECT (window->preferences),
                "last-show-hidden", &window->show_hidden,
                "last-window-width", &last_window_width,
                "last-window-height", &last_window_height,
                "last-window-maximized", &last_window_maximized,
                "last-menubar-visible", &last_menubar_visible,
                "last-separator-position", &last_separator_position,
                "last-location-bar", &last_location_bar,
                "last-side-pane", &last_side_pane,
                "last-statusbar-visible", &last_statusbar_visible,
                "misc-small-toolbar-icons", &small_icons,
                NULL);

  /* update the visual on screen_changed events */
  g_signal_connect (window, "screen-changed", G_CALLBACK (lunar_window_screen_changed), NULL);

  /* invoke the lunar_window_screen_changed function to initially set the best possible visual.*/
  lunar_window_screen_changed (GTK_WIDGET (window), NULL, NULL);

  /* set up a handler to confirm exit when there are multiple tabs open  */
  g_signal_connect (window, "delete-event", G_CALLBACK (lunar_window_delete), NULL);

  /* connect to the volume monitor */
  window->device_monitor = lunar_device_monitor_get ();
  g_signal_connect (window->device_monitor, "device-pre-unmount", G_CALLBACK (lunar_window_device_pre_unmount), window);
  g_signal_connect (window->device_monitor, "device-removed", G_CALLBACK (lunar_window_device_changed), window);
  g_signal_connect (window->device_monitor, "device-changed", G_CALLBACK (lunar_window_device_changed), window);

  window->icon_factory = lunar_icon_factory_get_default ();

  /* Catch key events before accelerators get processed */
  g_signal_connect (window, "key-press-event", G_CALLBACK (lunar_window_propagate_key_event), NULL);
  g_signal_connect (window, "key-release-event", G_CALLBACK (lunar_window_propagate_key_event), NULL);


  window->select_files_closure = g_cclosure_new_swap (G_CALLBACK (lunar_window_select_files), window, NULL);
  g_closure_ref (window->select_files_closure);
  g_closure_sink (window->select_files_closure);
  window->launcher = g_object_new (LUNAR_TYPE_LAUNCHER, "widget", GTK_WIDGET (window),
                                  "select-files-closure",  window->select_files_closure, NULL);

  endo_binding_new (G_OBJECT (window), "current-directory", G_OBJECT (window->launcher), "current-directory");
  g_signal_connect_swapped (G_OBJECT (window->launcher), "change-directory", G_CALLBACK (lunar_window_set_current_directory), window);
  g_signal_connect_swapped (G_OBJECT (window->launcher), "open-new-tab", G_CALLBACK (lunar_window_notebook_open_new_tab), window);
  lunar_launcher_append_accelerators (window->launcher, window->accel_group);

  /* determine the default window size from the preferences */
  gtk_window_set_default_size (GTK_WINDOW (window), last_window_width, last_window_height);

  /* restore the maxized state of the window */
  if (G_UNLIKELY (last_window_maximized))
    gtk_window_maximize (GTK_WINDOW (window));

  /* add lunar style class for easier theming */
  context = gtk_widget_get_style_context (GTK_WIDGET (window));
  gtk_style_context_add_class (context, "lunar");

  window->grid = gtk_grid_new ();
  gtk_container_add (GTK_CONTAINER (window), window->grid);
  gtk_widget_show (window->grid);

  /* build the menubar */
  window->menubar = gtk_menu_bar_new ();
  lunar_window_create_menu (window, LUNAR_WINDOW_ACTION_FILE_MENU, G_CALLBACK (lunar_window_update_file_menu));
  lunar_window_create_menu (window, LUNAR_WINDOW_ACTION_EDIT_MENU, G_CALLBACK (lunar_window_update_edit_menu));
  lunar_window_create_menu (window, LUNAR_WINDOW_ACTION_VIEW_MENU, G_CALLBACK (lunar_window_update_view_menu));
  lunar_window_create_menu (window, LUNAR_WINDOW_ACTION_GO_MENU, G_CALLBACK (lunar_window_update_go_menu));
  lunar_window_create_menu (window, LUNAR_WINDOW_ACTION_HELP_MENU, G_CALLBACK (lunar_window_update_help_menu));
  gtk_widget_show_all (window->menubar);

  if (last_menubar_visible == FALSE)
    gtk_widget_hide (window->menubar);
  gtk_widget_set_hexpand (window->menubar, TRUE);
  gtk_grid_attach (GTK_GRID (window->grid), window->menubar, 0, 0, 1, 1);

  /* append the menu item for the spinner */
  item = gtk_menu_item_new ();
  gtk_widget_set_sensitive (GTK_WIDGET (item), FALSE);
  g_object_set (G_OBJECT (item), "right-justified", TRUE, NULL);
  gtk_menu_shell_append (GTK_MENU_SHELL (window->menubar), item);
  gtk_widget_show (item);

  /* place the spinner into the menu item */
  window->spinner = gtk_spinner_new ();
  gtk_container_add (GTK_CONTAINER (item), window->spinner);
  endo_binding_new (G_OBJECT (window->spinner), "active",
                   G_OBJECT (window->spinner), "visible");

  /* check if we need to add the root warning */
  if (G_UNLIKELY (geteuid () == 0))
    {
      /* add the bar for the root warning */
      infobar = gtk_info_bar_new ();
      gtk_info_bar_set_message_type (GTK_INFO_BAR (infobar), GTK_MESSAGE_WARNING);
      gtk_widget_set_hexpand (infobar, TRUE);
      gtk_grid_attach (GTK_GRID (window->grid), infobar, 0, 2, 1, 1);
      gtk_widget_show (infobar);

      /* add the label with the root warning */
      label = gtk_label_new (_("Warning: you are using the root account. You may harm your system."));
      gtk_container_add (GTK_CONTAINER (gtk_info_bar_get_content_area (GTK_INFO_BAR (infobar))), label);
      gtk_widget_show (label);
    }

  window->paned = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_container_set_border_width (GTK_CONTAINER (window->paned), 0);
  gtk_widget_set_hexpand (window->paned, TRUE);
  gtk_widget_set_vexpand (window->paned, TRUE);
  gtk_grid_attach (GTK_GRID (window->grid), window->paned, 0, 4, 1, 1);
  gtk_widget_show (window->paned);

  /* determine the last separator position and apply it to the paned view */
  gtk_paned_set_position (GTK_PANED (window->paned), last_separator_position);
  g_signal_connect_swapped (window->paned, "accept-position", G_CALLBACK (lunar_window_save_paned), window);
  g_signal_connect_swapped (window->paned, "button-release-event", G_CALLBACK (lunar_window_save_paned), window);

  window->view_box = gtk_grid_new ();
  gtk_paned_pack2 (GTK_PANED (window->paned), window->view_box, TRUE, FALSE);
  gtk_widget_show (window->view_box);

  /* split view: Create panes where the two notebooks */
  window->paned_notebooks = gtk_paned_new (GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_add2 (GTK_PANED (window->paned), window->view_box);
  gtk_widget_add_events (window->paned_notebooks, GDK_ENTER_NOTIFY_MASK | GDK_LEAVE_NOTIFY_MASK | GDK_BUTTON_PRESS_MASK);
  gtk_grid_attach (GTK_GRID (window->view_box), window->paned_notebooks, 0, 1, 1, 2);
  gtk_widget_show (window->paned_notebooks);

  /** close notebooks on window-remove signal because later on window property
   *  pointers are broken.
   **/
  g_signal_connect (G_OBJECT (window), "remove", G_CALLBACK (lunar_window_paned_notebooks_destroy), window);

  /* add first notebook and select it*/
  window->notebook_selected = lunar_window_paned_notebooks_add(window);

  /* allocate the new location bar widget */
  window->location_bar = lunar_location_bar_new ();
  g_object_bind_property (G_OBJECT (window), "current-directory", G_OBJECT (window->location_bar), "current-directory", G_BINDING_SYNC_CREATE);
  g_signal_connect_swapped (G_OBJECT (window->location_bar), "change-directory", G_CALLBACK (lunar_window_set_current_directory), window);
  g_signal_connect_swapped (G_OBJECT (window->location_bar), "open-new-tab", G_CALLBACK (lunar_window_notebook_open_new_tab), window);
  g_signal_connect_swapped (G_OBJECT (window->location_bar), "reload-requested", G_CALLBACK (lunar_window_handle_reload_request), window);
  g_signal_connect_swapped (G_OBJECT (window->location_bar), "entry-done", G_CALLBACK (lunar_window_update_location_bar_visible), window);

  /* setup the toolbar for the location bar */
  window->location_toolbar = gtk_toolbar_new ();
  gtk_toolbar_set_style (GTK_TOOLBAR (window->location_toolbar), GTK_TOOLBAR_ICONS);
  gtk_toolbar_set_icon_size (GTK_TOOLBAR (window->location_toolbar),
                              small_icons ? GTK_ICON_SIZE_SMALL_TOOLBAR : GTK_ICON_SIZE_LARGE_TOOLBAR);
  gtk_widget_set_hexpand (window->location_toolbar, TRUE);
  gtk_grid_attach (GTK_GRID (window->grid), window->location_toolbar, 0, 1, 1, 1);

  window->location_toolbar_item_back = expidus_gtk_tool_button_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_BACK), G_OBJECT (window), GTK_TOOLBAR (window->location_toolbar));
  window->location_toolbar_item_forward = expidus_gtk_tool_button_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_FORWARD), G_OBJECT (window), GTK_TOOLBAR (window->location_toolbar));
  window->location_toolbar_item_parent = expidus_gtk_tool_button_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_PARENT), G_OBJECT (window), GTK_TOOLBAR (window->location_toolbar));
  expidus_gtk_tool_button_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_HOME), G_OBJECT (window), GTK_TOOLBAR (window->location_toolbar));

  g_signal_connect (G_OBJECT (window->location_toolbar_item_back), "button-press-event", G_CALLBACK (lunar_window_history_clicked), G_OBJECT (window));
  g_signal_connect (G_OBJECT (window->location_toolbar_item_forward), "button-press-event", G_CALLBACK (lunar_window_history_clicked), G_OBJECT (window));
  g_signal_connect (G_OBJECT (window), "button-press-event", G_CALLBACK (lunar_window_button_press_event), G_OBJECT (window));
  window->signal_handler_id_history_changed = 0;

  /* The UCA shortcuts need to be checked 'by hand', since we dont want to permanently keep menu items for them */
  g_signal_connect (window, "key-press-event", G_CALLBACK (lunar_window_check_uca_key_activation), NULL);

  /* add the location bar to the toolbar */
  tool_item = gtk_tool_item_new ();
  gtk_tool_item_set_expand (tool_item, TRUE);
  gtk_toolbar_insert (GTK_TOOLBAR (window->location_toolbar), tool_item, -1);
  gtk_toolbar_set_show_arrow (GTK_TOOLBAR (window->location_toolbar), FALSE);

  /* add the location bar itself */
  gtk_container_add (GTK_CONTAINER (tool_item), window->location_bar);

  /* display the toolbar */
  gtk_widget_show_all (window->location_toolbar);

  g_free (last_location_bar);

  /* setup setting the location bar visibility on-demand */
  g_signal_connect_object (G_OBJECT (window->preferences), "notify::last-location-bar", G_CALLBACK (lunar_window_update_location_bar_visible), window, G_CONNECT_SWAPPED);
  lunar_window_update_location_bar_visible (window);

  /* update window icon whenever preferences change */
  g_signal_connect_object (G_OBJECT (window->preferences), "notify::misc-change-window-icon", G_CALLBACK (lunar_window_update_window_icon), window, G_CONNECT_SWAPPED);

  /* determine the selected side pane */
  if (endo_str_is_equal (last_side_pane, g_type_name (LUNAR_TYPE_SHORTCUTS_PANE)))
    type = LUNAR_TYPE_SHORTCUTS_PANE;
  else if (endo_str_is_equal (last_side_pane, g_type_name (LUNAR_TYPE_TREE_PANE)))
    type = LUNAR_TYPE_TREE_PANE;
  else
    type = G_TYPE_NONE;
  lunar_window_install_sidepane (window, type);
  g_free (last_side_pane);

  /* synchronise the "directory-specific-settings" property with the global "misc-directory-specific-settings" property */
  endo_binding_new (G_OBJECT (window->preferences), "misc-directory-specific-settings", G_OBJECT (window), "directory-specific-settings");

  /* setup a new statusbar */
  window->statusbar = lunar_statusbar_new ();
  gtk_widget_set_hexpand (window->statusbar, TRUE);
  gtk_grid_attach (GTK_GRID (window->view_box), window->statusbar, 0, 3, 1, 1);
  if (last_statusbar_visible)
    gtk_widget_show (window->statusbar);

  if (G_LIKELY (window->view != NULL))
    lunar_window_binding_create (window, window->view, "statusbar-text", window->statusbar, "text", G_BINDING_SYNC_CREATE);

  /* ensure that all the view types are registered */
  g_type_ensure (LUNAR_TYPE_ICON_VIEW);
  g_type_ensure (LUNAR_TYPE_DETAILS_VIEW);
  g_type_ensure (LUNAR_TYPE_COMPACT_VIEW);

  /* load the bookmarks file and monitor */
  window->bookmarks = NULL;
  window->bookmark_file = lunar_g_file_new_for_bookmarks ();
  window->bookmark_monitor = g_file_monitor_file (window->bookmark_file, G_FILE_MONITOR_NONE, NULL, NULL);
  if (G_LIKELY (window->bookmark_monitor != NULL))
      g_signal_connect_swapped (window->bookmark_monitor, "changed", G_CALLBACK (lunar_window_update_bookmarks), window);

  /* initial load of the bookmarks */
  lunar_window_update_bookmarks (window);
}


static void
lunar_window_screen_changed (GtkWidget *widget,
                              GdkScreen *old_screen,
                              gpointer   userdata)
{
  GdkScreen *screen = gdk_screen_get_default ();
  GdkVisual *visual = gdk_screen_get_rgba_visual (screen);

  if (visual == NULL || !gdk_screen_is_composited (screen))
    visual = gdk_screen_get_system_visual (screen);

  gtk_widget_set_visual (GTK_WIDGET (widget), visual);
}


/**
 * lunar_window_select_files:
 * @window            : a #LunarWindow instance.
 * @files_to_selected : a list of #GFile<!---->s
 *
 * Visually selects the files, given by the list
 **/
static void
lunar_window_select_files (LunarWindow *window,
                            GList        *files_to_selected)
{
  GList *lunarFiles = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  for (GList *lp = files_to_selected; lp != NULL; lp = lp->next)
      lunarFiles = g_list_append (lunarFiles, lunar_file_get (G_FILE (files_to_selected->data), NULL));
  lunar_view_set_selected_files (LUNAR_VIEW (window->view), lunarFiles);
  g_list_free_full (lunarFiles, g_object_unref);
}



static void
lunar_window_create_menu (LunarWindow       *window,
                           LunarWindowAction  action,
                           GCallback           cb_update_menu)
{
  GtkWidget *item;
  GtkWidget *submenu;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (action), G_OBJECT (window), GTK_MENU_SHELL (window->menubar));

  submenu = g_object_new (LUNAR_TYPE_MENU, "menu-type", LUNAR_MENU_TYPE_WINDOW, "launcher", window->launcher, NULL);
  gtk_menu_set_accel_group (GTK_MENU (submenu), window->accel_group);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), GTK_WIDGET (submenu));
  g_signal_connect_swapped (G_OBJECT (submenu), "show", G_CALLBACK (cb_update_menu), window);
}



static void
lunar_window_update_file_menu (LunarWindow *window,
                                GtkWidget    *menu)
{
  GtkWidget  *item;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  lunar_gtk_menu_clean (GTK_MENU (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_NEW_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_NEW_WINDOW), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  lunar_menu_add_sections (LUNAR_MENU (menu), LUNAR_MENU_SECTION_OPEN
                                              | LUNAR_MENU_SECTION_SENDTO
                                              | LUNAR_MENU_SECTION_CREATE_NEW_FILES
                                              | LUNAR_MENU_SECTION_EMPTY_TRASH
                                              | LUNAR_MENU_SECTION_CUSTOM_ACTIONS
                                              | LUNAR_MENU_SECTION_PROPERTIES);
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_DETACH_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  gtk_widget_set_sensitive (item, gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected)) > 1);
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_CLOSE_ALL_WINDOWS), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_CLOSE_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_CLOSE_WINDOW), G_OBJECT (window), GTK_MENU_SHELL (menu));

  gtk_widget_show_all (GTK_WIDGET (menu));
  lunar_window_redirect_menu_tooltips_to_statusbar (window, GTK_MENU (menu));
}



static void
lunar_window_update_edit_menu (LunarWindow *window,
                                GtkWidget    *menu)
{
  GtkWidget       *gtk_menu_item;
  GList           *lunarx_menu_items;
  GList           *pp, *lp;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  lunar_gtk_menu_clean (GTK_MENU (menu));
  lunar_menu_add_sections (LUNAR_MENU (menu), LUNAR_MENU_SECTION_CUT
                                              | LUNAR_MENU_SECTION_COPY_PASTE
                                              | LUNAR_MENU_SECTION_TRASH_DELETE);
  if (window->view != NULL)
    {
      lunar_standard_view_append_menu_item (LUNAR_STANDARD_VIEW (window->view),
                                             GTK_MENU (menu), LUNAR_STANDARD_VIEW_ACTION_SELECT_ALL_FILES);
      lunar_standard_view_append_menu_item (LUNAR_STANDARD_VIEW (window->view),
                                             GTK_MENU (menu), LUNAR_STANDARD_VIEW_ACTION_SELECT_BY_PATTERN);
      lunar_standard_view_append_menu_item (LUNAR_STANDARD_VIEW (window->view),
                                             GTK_MENU (menu), LUNAR_STANDARD_VIEW_ACTION_INVERT_SELECTION);
      lunar_standard_view_append_menu_item (LUNAR_STANDARD_VIEW (window->view),
                                             GTK_MENU (menu), LUNAR_STANDARD_VIEW_ACTION_UNSELECT_ALL_FILES);
    }
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  lunar_menu_add_sections (LUNAR_MENU (menu), LUNAR_MENU_SECTION_DUPLICATE
                                              | LUNAR_MENU_SECTION_MAKELINK
                                              | LUNAR_MENU_SECTION_RENAME
                                              | LUNAR_MENU_SECTION_RESTORE);

  /* determine the available preferences providers */
  if (G_LIKELY (window->lunarx_preferences_providers != NULL))
    {
      /* add menu items from all providers */
      for (pp = window->lunarx_preferences_providers; pp != NULL; pp = pp->next)
        {
          /* determine the available menu items for the provider */
          lunarx_menu_items = lunarx_preferences_provider_get_menu_items (LUNARX_PREFERENCES_PROVIDER (pp->data), GTK_WIDGET (window));
          for (lp = lunarx_menu_items; lp != NULL; lp = lp->next)
            {
                gtk_menu_item = lunar_gtk_menu_lunarx_menu_item_new (lp->data, GTK_MENU_SHELL (menu));

                /* Each lunarx_menu_item will be destroyed together with its related gtk_menu_item */
                g_signal_connect_swapped (G_OBJECT (gtk_menu_item), "destroy", G_CALLBACK (g_object_unref), lp->data);
            }

          /* release the list */
          g_list_free (lunarx_menu_items);
        }
    }
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_PREFERENCES), G_OBJECT (window), GTK_MENU_SHELL (menu));
  gtk_widget_show_all (GTK_WIDGET (menu));

  lunar_window_redirect_menu_tooltips_to_statusbar (window, GTK_MENU (menu));
}



static void
lunar_window_update_view_menu (LunarWindow *window,
                                GtkWidget    *menu)
{
  GtkWidget  *item;
  GtkWidget  *sub_items;
  gchar      *last_location_bar;
  gchar      *last_side_pane;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  lunar_gtk_menu_clean (GTK_MENU (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_RELOAD), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_SPLIT), G_OBJECT (window), lunar_window_split_view_is_active (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_MENU), G_OBJECT (window), GTK_MENU_SHELL (menu));
  sub_items =  gtk_menu_new();
  gtk_menu_set_accel_group (GTK_MENU (sub_items), window->accel_group);
  g_object_get (window->preferences, "last-location-bar", &last_location_bar, NULL);
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_PATHBAR), G_OBJECT (window),
                                                   endo_str_is_equal (last_location_bar, g_type_name (LUNAR_TYPE_LOCATION_ENTRY)), GTK_MENU_SHELL (sub_items));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_TOOLBAR), G_OBJECT (window),
                                                   endo_str_is_equal (last_location_bar, g_type_name (LUNAR_TYPE_LOCATION_BUTTONS)), GTK_MENU_SHELL (sub_items));
  g_free (last_location_bar);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), GTK_WIDGET (sub_items));
  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_MENU), G_OBJECT (window), GTK_MENU_SHELL (menu));
  sub_items =  gtk_menu_new();
  gtk_menu_set_accel_group (GTK_MENU (sub_items), window->accel_group);
  g_object_get (window->preferences, "last-side-pane", &last_side_pane, NULL);
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_SHORTCUTS), G_OBJECT (window),
                                                   endo_str_is_equal (last_side_pane, g_type_name (LUNAR_TYPE_SHORTCUTS_PANE)), GTK_MENU_SHELL (sub_items));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_TREE), G_OBJECT (window),
                                                   endo_str_is_equal (last_side_pane, g_type_name (LUNAR_TYPE_TREE_PANE)), GTK_MENU_SHELL (sub_items));
  g_free (last_side_pane);
  gtk_menu_item_set_submenu (GTK_MENU_ITEM (item), GTK_WIDGET (sub_items));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_STATUSBAR), G_OBJECT (window),
                                                   gtk_widget_get_visible (window->statusbar), GTK_MENU_SHELL (menu));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_MENUBAR), G_OBJECT (window),
                                                   gtk_widget_get_visible (window->menubar), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_SHOW_HIDDEN), G_OBJECT (window),
                                                   window->show_hidden, GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  if (window->view != NULL)
    lunar_standard_view_append_menu_items (LUNAR_STANDARD_VIEW (window->view), GTK_MENU (menu), window->accel_group);
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  lunar_window_append_menu_item (window, GTK_MENU_SHELL (menu), LUNAR_WINDOW_ACTION_ZOOM_IN);
  lunar_window_append_menu_item (window, GTK_MENU_SHELL (menu), LUNAR_WINDOW_ACTION_ZOOM_OUT);
  lunar_window_append_menu_item (window, GTK_MENU_SHELL (menu), LUNAR_WINDOW_ACTION_ZOOM_RESET);
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));

  if (window->directory_specific_settings)
    {
      item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_CLEAR_DIRECTORY_SPECIFIC_SETTINGS),
                                                       G_OBJECT (window), GTK_MENU_SHELL (menu));
      gtk_widget_set_sensitive (item, lunar_file_has_directory_specific_settings (window->current_directory));
      expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_AS_ICONS),
                                                 G_OBJECT (window), window->view_type == LUNAR_TYPE_ICON_VIEW, GTK_MENU_SHELL (menu));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_AS_DETAILED_LIST),
                                                 G_OBJECT (window), window->view_type == LUNAR_TYPE_DETAILS_VIEW, GTK_MENU_SHELL (menu));
  expidus_gtk_toggle_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_VIEW_AS_COMPACT_LIST),
                                                 G_OBJECT (window), window->view_type == LUNAR_TYPE_COMPACT_VIEW, GTK_MENU_SHELL (menu));

  gtk_widget_show_all (GTK_WIDGET (menu));

  lunar_window_redirect_menu_tooltips_to_statusbar (window, GTK_MENU (menu));
}



static void
lunar_window_update_go_menu (LunarWindow *window,
                              GtkWidget    *menu)
{
  gchar                    *icon_name;
  const ExpidusGtkActionEntry *action_entry;
  LunarHistory            *history = NULL;
  GtkWidget                *item;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  if (window->view != NULL)
    history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));

  lunar_gtk_menu_clean (GTK_MENU (menu));
  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_PARENT), G_OBJECT (window), GTK_MENU_SHELL (menu));
  gtk_widget_set_sensitive (item, !lunar_g_file_is_root (lunar_file_get_file (window->current_directory)));
  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_BACK), G_OBJECT (window), GTK_MENU_SHELL (menu));
  if (history != NULL)
    gtk_widget_set_sensitive (item, lunar_history_has_back (history));
  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_FORWARD), G_OBJECT (window), GTK_MENU_SHELL (menu));
  if (history != NULL)
    gtk_widget_set_sensitive (item, lunar_history_has_forward (history));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_COMPUTER), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_HOME), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_DESKTOP), G_OBJECT (window), GTK_MENU_SHELL (menu));
  if (lunar_g_vfs_is_uri_scheme_supported ("trash"))
    {
      GFile      *gfile;
      LunarFile *trash_folder;

      /* try to connect to the trash bin */
      gfile = lunar_g_file_new_for_trash ();
      if (gfile != NULL)
        {
          trash_folder = lunar_file_get (gfile, NULL);
          if (trash_folder != NULL)
            {
              action_entry = get_action_entry (LUNAR_WINDOW_ACTION_OPEN_TRASH);
              if (action_entry != NULL)
                {
                  if (lunar_file_get_item_count (trash_folder) > 0)
                    icon_name = "user-trash-full";
                  else
                    icon_name = "user-trash";
                  expidus_gtk_image_menu_item_new_from_icon_name (action_entry->menu_item_label_text, action_entry->menu_item_tooltip_text,
                                                               action_entry->accel_path, action_entry->callback, G_OBJECT (window), icon_name, GTK_MENU_SHELL (menu));
                }
              g_object_unref (trash_folder);
            }
          g_object_unref (gfile);
        }
    }
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_TEMPLATES), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  lunar_window_menu_add_bookmarks (window, GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_FILE_SYSTEM), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_NETWORK), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_OPEN_LOCATION), G_OBJECT (window), GTK_MENU_SHELL (menu));
  gtk_widget_show_all (GTK_WIDGET (menu));

  lunar_window_redirect_menu_tooltips_to_statusbar (window, GTK_MENU (menu));
}



static void
lunar_window_update_help_menu (LunarWindow *window,
                                GtkWidget    *menu)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  lunar_gtk_menu_clean (GTK_MENU (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_CONTENTS), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_ABOUT), G_OBJECT (window), GTK_MENU_SHELL (menu));
  gtk_widget_show_all (GTK_WIDGET (menu));

  lunar_window_redirect_menu_tooltips_to_statusbar (window, GTK_MENU (menu));
}



static void
lunar_window_dispose (GObject *object)
{
  LunarWindow  *window = LUNAR_WINDOW (object);

  /* indicate that history items are out of use */
  window->location_toolbar_item_back = NULL;
  window->location_toolbar_item_forward = NULL;

  if (window->accel_group != NULL)
    {
      gtk_accel_group_disconnect (window->accel_group, NULL);
      gtk_window_remove_accel_group (GTK_WINDOW (window), window->accel_group);
      g_object_unref (window->accel_group);
      window->accel_group = NULL;
    }

  /* destroy the save geometry timer source */
  if (G_UNLIKELY (window->save_geometry_timer_id != 0))
    g_source_remove (window->save_geometry_timer_id);

  /* disconnect from the current-directory */
  lunar_window_set_current_directory (window, NULL);

  (*G_OBJECT_CLASS (lunar_window_parent_class)->dispose) (object);
}



static void
lunar_window_finalize (GObject *object)
{
  LunarWindow *window = LUNAR_WINDOW (object);

  /* disconnect from the volume monitor */
  g_signal_handlers_disconnect_matched (window->device_monitor, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, window);
  g_object_unref (window->device_monitor);

  g_object_unref (window->icon_factory);
  g_object_unref (window->launcher);

  if (window->bookmark_file != NULL)
    g_object_unref (window->bookmark_file);

  if (window->bookmark_monitor != NULL)
    {
      g_file_monitor_cancel (window->bookmark_monitor);
      g_object_unref (window->bookmark_monitor);
    }

  /* release our reference on the provider factory */
  g_object_unref (window->provider_factory);

  /* release the preferences reference */
  g_object_unref (window->preferences);

  g_closure_invalidate (window->select_files_closure);
  g_closure_unref (window->select_files_closure);

  (*G_OBJECT_CLASS (lunar_window_parent_class)->finalize) (object);
}



static gboolean lunar_window_delete (GtkWidget *widget,
                                      GdkEvent  *event,
                                      gpointer   data )
{
  gboolean      confirm_close_multiple_tabs, do_not_ask_again;
  gint          response, n_tabs = 0;
  LunarWindow *window = LUNAR_WINDOW (widget);

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (widget),FALSE);

  /* if we don't have muliple tabs in one of the notebooks then just exit */
  if (window->notebook_left)
    n_tabs += gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_left));
  if (window->notebook_right)
    n_tabs += gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_right));

  if (lunar_window_split_view_is_active (window))
    {
      if (n_tabs < 3)
        return FALSE;
    }
  else
    {
      if (n_tabs < 2)
        return FALSE;
    }

  /* check if the user has disabled confirmation of closing multiple tabs, and just exit if so */
  g_object_get (G_OBJECT (window->preferences),
                "misc-confirm-close-multiple-tabs", &confirm_close_multiple_tabs,
                NULL);
  if(!confirm_close_multiple_tabs)
    return FALSE;

  /* ask the user for confirmation */
  do_not_ask_again = FALSE;
  response = expidus_dialog_confirm_close_tabs (GTK_WINDOW (widget), n_tabs, TRUE, &do_not_ask_again);

  /* if the user requested not to be asked again, store this preference */
  if (response != GTK_RESPONSE_CANCEL && do_not_ask_again)
    g_object_set (G_OBJECT (window->preferences),
                  "misc-confirm-close-multiple-tabs", FALSE, NULL);

  if(response == GTK_RESPONSE_YES)
    return FALSE;

  /* close active tab in active notebook */
  if(response == GTK_RESPONSE_CLOSE)
    gtk_notebook_remove_page (GTK_NOTEBOOK (window->notebook_selected), gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected)));
  return TRUE;
}



static void
lunar_window_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  LunarWindow *window = LUNAR_WINDOW (object);

  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      g_value_set_object (value, lunar_window_get_current_directory (window));
      break;

    case PROP_ZOOM_LEVEL:
      g_value_set_enum (value, window->zoom_level);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_window_set_property (GObject            *object,
                            guint               prop_id,
                            const GValue       *value,
                            GParamSpec         *pspec)
{
  LunarWindow *window = LUNAR_WINDOW (object);

  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      lunar_window_set_current_directory (window, g_value_get_object (value));
      break;

    case PROP_ZOOM_LEVEL:
      lunar_window_set_zoom_level (window, g_value_get_enum (value));
      break;

    case PROP_DIRECTORY_SPECIFIC_SETTINGS:
      lunar_window_set_directory_specific_settings (window, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gboolean
lunar_window_reload (LunarWindow *window,
                      gboolean      reload_info)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* force the view to reload */
  if (G_LIKELY (window->view != NULL))
    {
      lunar_view_reload (LUNAR_VIEW (window->view), reload_info);
      return TRUE;
    }

  return FALSE;
}



/**
 * lunar_window_has_shortcut_sidepane:
 * @window : a #LunarWindow instance.
 *
 * Return value: True, if this window is running a shortcut sidepane
 **/
gboolean
lunar_window_has_shortcut_sidepane (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* check if a side pane is currently active */
  if (G_LIKELY (window->sidepane != NULL))
    {
      return G_OBJECT_TYPE (window->sidepane) == LUNAR_TYPE_SHORTCUTS_PANE;
    }
  return FALSE;
}



/**
 * lunar_window_get_sidepane:
 * @window : a #LunarWindow instance.
 *
 * Return value: (transfer none): The #LunarSidePane of this window, or NULL if not available
 **/
GtkWidget* lunar_window_get_sidepane (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);
  return GTK_WIDGET (window->sidepane);
}



static gboolean
lunar_window_toggle_sidepane (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* check if a side pane is currently active */
  if (G_LIKELY (window->sidepane != NULL))
    {
      /* determine the currently active side pane type */
      window->toggle_sidepane_type = G_OBJECT_TYPE (window->sidepane);
      lunar_window_install_sidepane (window, G_TYPE_NONE);
    }
  else
    {
      /* check if we have a previously remembered toggle type */
      if (window->toggle_sidepane_type == LUNAR_TYPE_TREE_PANE || window->toggle_sidepane_type == LUNAR_TYPE_SHORTCUTS_PANE)
          lunar_window_install_sidepane (window, window->toggle_sidepane_type);
    }

  return TRUE;
}



static gboolean
lunar_window_zoom_in (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* check if we can still zoom in */
  if (G_LIKELY (window->zoom_level < LUNAR_ZOOM_N_LEVELS - 1))
    {
      lunar_window_set_zoom_level (window, window->zoom_level + 1);
      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_window_zoom_out (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* check if we can still zoom out */
  if (G_LIKELY (window->zoom_level > 0))
    {
      lunar_window_set_zoom_level (window, window->zoom_level - 1);
      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_window_zoom_reset (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* tell the view to reset it's zoom level */
  if (G_LIKELY (window->view != NULL))
    {
      lunar_view_reset_zoom_level (LUNAR_VIEW (window->view));
      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_window_tab_change (LunarWindow *window,
                          gint          nth)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  /* Alt+0 is 10th tab */
  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook_selected),
                                 nth == -1 ? 9 : nth);

  return TRUE;
}



static void
lunar_window_action_switch_next_tab (LunarWindow *window)
{
  gint current_page;
  gint new_page;
  gint pages;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected));
  new_page = (current_page + 1) % pages;

  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook_selected), new_page);
}



static void
lunar_window_action_switch_previous_tab (LunarWindow *window)
{
  gint current_page;
  gint new_page;
  gint pages;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  current_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));
  pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected));
  new_page = (current_page - 1) % pages;

  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook_selected), new_page);
}



static void
lunar_window_realize (GtkWidget *widget)
{
  LunarWindow *window = LUNAR_WINDOW (widget);

  /* let the GtkWidget class perform the realize operation */
  (*GTK_WIDGET_CLASS (lunar_window_parent_class)->realize) (widget);

  /* connect to the clipboard manager of the new display and be sure to redraw the window
   * whenever the clipboard contents change to make sure we always display up2date state.
   */
  window->clipboard = lunar_clipboard_manager_get_for_display (gtk_widget_get_display (widget));
  g_signal_connect_swapped (G_OBJECT (window->clipboard), "changed",
                            G_CALLBACK (gtk_widget_queue_draw), widget);
}



static void
lunar_window_unrealize (GtkWidget *widget)
{
  LunarWindow *window = LUNAR_WINDOW (widget);

  /* disconnect from the clipboard manager */
  g_signal_handlers_disconnect_by_func (G_OBJECT (window->clipboard), gtk_widget_queue_draw, widget);

  /* let the GtkWidget class unrealize the window */
  (*GTK_WIDGET_CLASS (lunar_window_parent_class)->unrealize) (widget);

  /* drop the reference on the clipboard manager, we do this after letting the GtkWidget class
   * unrealise the window to prevent the clipboard being disposed during the unrealize  */
  g_object_unref (G_OBJECT (window->clipboard));
}



static gboolean
lunar_window_configure_event (GtkWidget         *widget,
                               GdkEventConfigure *event)
{
  LunarWindow *window = LUNAR_WINDOW (widget);
  GtkAllocation widget_allocation;

  gtk_widget_get_allocation (widget, &widget_allocation);

  /* check if we have a new dimension here */
  if (widget_allocation.width != event->width || widget_allocation.height != event->height)
    {
      /* drop any previous timer source */
      if (window->save_geometry_timer_id != 0)
        g_source_remove (window->save_geometry_timer_id);

      /* check if we should schedule another save timer */
      if (gtk_widget_get_visible (widget))
        {
          /* save the geometry one second after the last configure event */
          window->save_geometry_timer_id = g_timeout_add_seconds_full (G_PRIORITY_LOW, 1, lunar_window_save_geometry_timer,
                                                                       window, lunar_window_save_geometry_timer_destroy);
        }
    }

  /* let Gtk+ handle the configure event */
  return (*GTK_WIDGET_CLASS (lunar_window_parent_class)->configure_event) (widget, event);
}



static void
lunar_window_binding_destroyed (gpointer data,
                                 GObject  *binding)
{
  LunarWindow *window = LUNAR_WINDOW (data);

  if (window->view_bindings != NULL)
    window->view_bindings = g_slist_remove (window->view_bindings, binding);
}



static void
lunar_window_binding_create (LunarWindow *window,
                              gpointer src_object,
                              const gchar *src_prop,
                              gpointer dst_object,
                              const gchar *dst_prop,
                              GBindingFlags flags)
{
  GBinding *binding;

  _lunar_return_if_fail (G_IS_OBJECT (src_object));
  _lunar_return_if_fail (G_IS_OBJECT (dst_object));

  binding = g_object_bind_property (G_OBJECT (src_object), src_prop,
                                    G_OBJECT (dst_object), dst_prop,
                                    flags);

  g_object_weak_ref (G_OBJECT (binding), lunar_window_binding_destroyed, window);
  window->view_bindings = g_slist_prepend (window->view_bindings, binding);
}



static void
lunar_window_notebook_switch_page (GtkWidget    *notebook,
                                    GtkWidget    *page,
                                    guint         page_num,
                                    LunarWindow *window)
{
  GSList        *view_bindings;
  LunarFile    *current_directory;
  LunarHistory *history;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (GTK_IS_NOTEBOOK (notebook));
  _lunar_return_if_fail (LUNAR_IS_VIEW (page));

  /* leave if nothing changed */
  if (window->view == page)
    return;

  /* Use accelerators only on the current active tab */
  if (window->view != NULL)
    g_object_set (G_OBJECT (window->view), "accel-group", NULL, NULL);
  g_object_set (G_OBJECT (page), "accel-group", window->accel_group, NULL);

  if (G_LIKELY (window->view != NULL))
    {
      /* disconnect from previous history */
      if (window->signal_handler_id_history_changed != 0)
        {
          history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));
          g_signal_handler_disconnect (history, window->signal_handler_id_history_changed);
          window->signal_handler_id_history_changed = 0;
        }

      /* unset view during switch */
      window->view = NULL;
    }

  /* disconnect existing bindings */
  view_bindings = window->view_bindings;
  window->view_bindings = NULL;
  g_slist_free_full (view_bindings, g_object_unref);

  /* update the directory of the current window */
  current_directory = lunar_navigator_get_current_directory (LUNAR_NAVIGATOR (page));
  lunar_window_set_current_directory (window, current_directory);

  /* add stock bindings */
  lunar_window_binding_create (window, window, "current-directory", page, "current-directory", G_BINDING_DEFAULT);
  lunar_window_binding_create (window, page, "loading", window->spinner, "active", G_BINDING_SYNC_CREATE);
  lunar_window_binding_create (window, page, "selected-files", window->launcher, "selected-files", G_BINDING_SYNC_CREATE);
  lunar_window_binding_create (window, page, "zoom-level", window, "zoom-level", G_BINDING_SYNC_CREATE | G_BINDING_BIDIRECTIONAL);

  /* connect to the sidepane (if any) */
  if (G_LIKELY (window->sidepane != NULL))
    {
      lunar_window_binding_create (window, page, "selected-files",
                                    window->sidepane, "selected-files",
                                    G_BINDING_SYNC_CREATE);
    }

  /* connect to the statusbar (if any) */
  if (G_LIKELY (window->statusbar != NULL))
    {
      lunar_window_binding_create (window, page, "statusbar-text",
                                    window->statusbar, "text",
                                    G_BINDING_SYNC_CREATE);
    }

  /* activate new view */
  window->view = page;
  window->view_type = G_TYPE_FROM_INSTANCE (page);

  /* remember the last view type if directory specific settings are not enabled */
  if (!window->directory_specific_settings && window->view_type != G_TYPE_NONE)
    g_object_set (G_OBJECT (window->preferences), "last-view", g_type_name (window->view_type), NULL);

  /* connect to the new history */
  history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));
  if (history != NULL)
    {
      window->signal_handler_id_history_changed = g_signal_connect_swapped (G_OBJECT (history), "history-changed", G_CALLBACK (lunar_window_history_changed), window);
      lunar_window_history_changed (window);
    }

  /* update the selection */
  lunar_standard_view_selection_changed (LUNAR_STANDARD_VIEW (page));

  gtk_widget_grab_focus (page);
}



static void
lunar_window_notebook_show_tabs (LunarWindow *window)
{
  gboolean   always_show_tabs;
  gint       n_pages = 0;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (window->notebook_left || window->notebook_right);

  g_object_get (G_OBJECT (window->preferences), "misc-always-show-tabs", &always_show_tabs, NULL);

  /* check both notebooks, maybe not the selected one get clicked */
  if (window->notebook_left)
    n_pages += gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_left));
  if (window->notebook_right)
    n_pages += gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_right));

  if (lunar_window_split_view_is_active (window))
    {
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (window->notebook_left), n_pages > 2 || always_show_tabs);
      gtk_notebook_set_show_tabs (GTK_NOTEBOOK (window->notebook_right), n_pages > 2 || always_show_tabs);
    }
  else
    gtk_notebook_set_show_tabs (GTK_NOTEBOOK (window->notebook_selected), n_pages > 1 || always_show_tabs);
}



static void
lunar_window_history_changed (LunarWindow *window)
{
  LunarHistory *history;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  if (window->view == NULL)
    return;

  history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));
  if (history == NULL)
    return;

  if (window->location_toolbar_item_back != NULL)
    gtk_widget_set_sensitive (window->location_toolbar_item_back, lunar_history_has_back (history));

  if (window->location_toolbar_item_forward != NULL)
    gtk_widget_set_sensitive (window->location_toolbar_item_forward, lunar_history_has_forward (history));
}



static void
lunar_window_notebook_page_added (GtkWidget    *notebook,
                                   GtkWidget    *page,
                                   guint         page_num,
                                   LunarWindow *window)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (GTK_IS_NOTEBOOK (notebook));
  _lunar_return_if_fail (LUNAR_IS_VIEW (page));
  _lunar_return_if_fail (window->notebook_selected == notebook);

  /* connect signals */
  g_signal_connect (G_OBJECT (page), "notify::loading", G_CALLBACK (lunar_window_notify_loading), window);
  g_signal_connect_swapped (G_OBJECT (page), "start-open-location", G_CALLBACK (lunar_window_start_open_location), window);
  g_signal_connect_swapped (G_OBJECT (page), "change-directory", G_CALLBACK (lunar_window_set_current_directory), window);
  g_signal_connect_swapped (G_OBJECT (page), "open-new-tab", G_CALLBACK (lunar_window_notebook_open_new_tab), window);

  /* update tab visibility */
  lunar_window_notebook_show_tabs (window);

  /* set default type if not set yet */
  if (window->view_type == G_TYPE_NONE)
    window->view_type = G_OBJECT_TYPE (page);
}



static void
lunar_window_notebook_page_removed (GtkWidget    *notebook,
                                     GtkWidget    *page,
                                     guint         page_num,
                                     LunarWindow *window)
{
  gint       n_pages;
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (GTK_IS_NOTEBOOK (notebook));
  _lunar_return_if_fail (LUNAR_IS_VIEW (page));
  _lunar_return_if_fail (window->notebook_left == notebook || window->notebook_right == notebook);
  
  /* drop connected signals */
  g_signal_handlers_disconnect_matched (page, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, window);

  n_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook));
  if (n_pages == 0)
    {
      if (lunar_window_split_view_is_active (window))
        {
          /* select the other notebook if the current gets closed */
          if (notebook == window->notebook_selected)
            lunar_window_paned_notebooks_switch (window);

          lunar_window_action_toggle_split_view (window);
        }
      else
        {
          /* destroy the window */
          gtk_widget_destroy (GTK_WIDGET (window));
        }
    }
  else
    {
      /* page from the other notebook was removed */
      if (notebook != window->notebook_selected)
        lunar_window_paned_notebooks_switch (window);
      else
        /* this page removed -> select next page */
        lunar_window_notebook_select_current_page (window);

      /* update tab visibility */
      lunar_window_notebook_show_tabs (window);
    }
}



static gboolean
lunar_window_notebook_button_press_event (GtkWidget      *notebook,
                                           GdkEventButton *event,
                                           LunarWindow   *window)
{
  gint           page_num = 0;
  GtkWidget     *page;
  GtkWidget     *label_box;
  GtkAllocation  alloc;
  gint           x, y;
  gboolean       close_tab;

  if ((event->button == 2 || event->button == 3)
      && event->type == GDK_BUTTON_PRESS)
    {
      /* get real window coordinates */
      gdk_window_get_position (event->window, &x, &y);
      x += event->x;
      y += event->y;

      /* lookup the clicked tab */
      while ((page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (notebook), page_num)) != NULL)
        {
          label_box = gtk_notebook_get_tab_label (GTK_NOTEBOOK (notebook), page);
          gtk_widget_get_allocation (label_box, &alloc);

          if (x >= alloc.x && x < alloc.x + alloc.width
              && y >= alloc.y && y < alloc.y + alloc.height)
            break;

          page_num++;
        }

      /* leave if no tab could be found */
      if (page == NULL)
        return FALSE;

      if (event->button == 2)
        {
          /* check if we should close the tab */
          g_object_get (window->preferences, "misc-tab-close-middle-click", &close_tab, NULL);
          if (close_tab)
            gtk_widget_destroy (page);
        }
      else if (event->button == 3)
        {
          /* update the current tab before we show the menu */
          gtk_notebook_set_current_page (GTK_NOTEBOOK (notebook), page_num);

          /* show the tab menu */
          lunar_window_notebook_popup_menu (notebook, window);
        }

      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_window_notebook_popup_menu (GtkWidget    *notebook,
                                   LunarWindow *window)
{
  GtkWidget *menu;

  menu = gtk_menu_new ();
  gtk_menu_set_accel_group (GTK_MENU (menu), window->accel_group);
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_NEW_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_DETACH_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_SWITCH_PREV_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_SWITCH_NEXT_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_WINDOW_ACTION_CLOSE_TAB), G_OBJECT (window), GTK_MENU_SHELL (menu));
  gtk_widget_show_all (menu);
  lunar_gtk_menu_run (GTK_MENU (menu));
  return TRUE;
}



static gpointer
lunar_window_notebook_create_window (GtkWidget    *notebook,
                                      GtkWidget    *page,
                                      gint          x,
                                      gint          y,
                                      LunarWindow *window)
{
  GtkWidget         *new_window;
  LunarApplication *application;
  gint               width, height;
  GdkMonitor        *monitor;
  GdkScreen         *screen;
  GdkRectangle       geo;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), NULL);
  _lunar_return_val_if_fail (GTK_IS_NOTEBOOK (notebook), NULL);
  _lunar_return_val_if_fail (window->notebook_selected == notebook, NULL);
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (page), NULL);

  /* do nothing if this window has only 1 tab */
  if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (notebook)) < 2)
    return NULL;

  /* create new window */
  application = lunar_application_get ();
  screen = gtk_window_get_screen (GTK_WINDOW (window));
  new_window = lunar_application_open_window (application, NULL, screen, NULL, TRUE);
  g_object_unref (application);

  /* make sure the new window has the same size */
  gtk_window_get_size (GTK_WINDOW (window), &width, &height);
  gtk_window_resize (GTK_WINDOW (new_window), width, height);

  /* move the window to the drop position */
  if (x >= 0 && y >= 0)
    {
      /* get the monitor geometry */
      monitor = gdk_display_get_monitor_at_point (gdk_display_get_default (), x, y);
      gdk_monitor_get_geometry (monitor, &geo);

      /* calculate window position, but keep it on the current monitor */
      x = CLAMP (x - width / 2, geo.x, geo.x + geo.width - width);
      y = CLAMP (y - height / 2, geo.y, geo.y + geo.height - height);

      /* move the window */
      gtk_window_move (GTK_WINDOW (new_window), MAX (0, x), MAX (0, y));
    }

  /* insert page in new notebook */
  return LUNAR_WINDOW (new_window)->notebook_selected;
}



static GtkWidget*
lunar_window_notebook_insert_page (LunarWindow  *window,
                                    LunarFile    *directory,
                                    GType          view_type,
                                    gint           position,
                                    LunarHistory *history)
{
  GtkWidget      *view;
  GtkWidget      *label;
  GtkWidget      *label_box;
  GtkWidget      *button;
  GtkWidget      *icon;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), NULL);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (directory), NULL);
  _lunar_return_val_if_fail (view_type != G_TYPE_NONE, NULL);
  _lunar_return_val_if_fail (history == NULL || LUNAR_IS_HISTORY (history), NULL);

  /* allocate and setup a new view */
  view = g_object_new (view_type, "current-directory", directory, NULL);
  lunar_view_set_show_hidden (LUNAR_VIEW (view), window->show_hidden);
  gtk_widget_show (view);

  /* set the history of the view if a history is provided */
  if (history != NULL)
    lunar_standard_view_set_history (LUNAR_STANDARD_VIEW (view), history);

  label_box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 0);

  label = gtk_label_new (NULL);
  endo_binding_new (G_OBJECT (view), "display-name", G_OBJECT (label), "label");
  endo_binding_new (G_OBJECT (view), "tooltip-text", G_OBJECT (label), "tooltip-text");
  gtk_widget_set_has_tooltip (label, TRUE);
  gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
  gtk_widget_set_margin_start (GTK_WIDGET(label), 3);
  gtk_widget_set_margin_end (GTK_WIDGET(label), 3);
  gtk_widget_set_margin_top (GTK_WIDGET(label), 3);
  gtk_widget_set_margin_bottom (GTK_WIDGET(label), 3);
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_END);
  gtk_label_set_single_line_mode (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (label_box), label, TRUE, TRUE, 0);
  gtk_widget_show (label);

  button = gtk_button_new ();
  gtk_box_pack_start (GTK_BOX (label_box), button, FALSE, FALSE, 0);
  gtk_widget_set_can_default (button, FALSE);
  gtk_widget_set_focus_on_click (button, FALSE);
  gtk_button_set_relief (GTK_BUTTON (button), GTK_RELIEF_NONE);
  gtk_widget_set_tooltip_text (button, _("Close tab"));
  g_signal_connect_swapped (G_OBJECT (button), "clicked", G_CALLBACK (gtk_widget_destroy), view);
  gtk_widget_show (button);

  icon = gtk_image_new_from_icon_name ("window-close", GTK_ICON_SIZE_MENU);
  gtk_container_add (GTK_CONTAINER (button), icon);
  gtk_widget_show (icon);

  /* insert the new page */
  gtk_notebook_insert_page (GTK_NOTEBOOK (window->notebook_selected), view, label_box, position);

  /* set tab child properties */
  gtk_container_child_set (GTK_CONTAINER (window->notebook_selected), view, "tab-expand", TRUE, NULL);
  gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (window->notebook_selected), view, TRUE);
  gtk_notebook_set_tab_detachable (GTK_NOTEBOOK (window->notebook_selected), view, TRUE);

  /* only gets clicks on the notebook(page) it self */
  g_signal_connect (G_OBJECT (gtk_bin_get_child (GTK_BIN (view))), "focus-in-event", G_CALLBACK (lunar_window_paned_notebooks_select), window);

  return view;
}



static void
lunar_window_notebook_select_current_page (LunarWindow *window)
{
  gint       current_page_n;
  GtkWidget *current_page;

  _lunar_return_if_fail (window->notebook_selected != NULL);

  current_page_n = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));
  current_page = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook_selected), current_page_n);
  lunar_window_notebook_switch_page (window->notebook_selected, current_page, current_page_n, window);
}



static GtkWidget*
lunar_window_paned_notebooks_add (LunarWindow *window)
{
  GtkWidget *notebook;
  _lunar_return_val_if_fail(LUNAR_IS_WINDOW (window), NULL);
  _lunar_return_val_if_fail (!lunar_window_split_view_is_active (window), NULL);

  notebook = gtk_notebook_new ();
  gtk_widget_set_hexpand (notebook, TRUE);
  gtk_widget_set_vexpand (notebook, TRUE);
  g_signal_connect (G_OBJECT (notebook), "switch-page", G_CALLBACK (lunar_window_notebook_switch_page), window);
  g_signal_connect (G_OBJECT (notebook), "page-added", G_CALLBACK (lunar_window_notebook_page_added), window);
  g_signal_connect (G_OBJECT (notebook), "page-removed", G_CALLBACK (lunar_window_notebook_page_removed), window);
  g_signal_connect_after (G_OBJECT (notebook), "button-press-event", G_CALLBACK (lunar_window_notebook_button_press_event), window);
  g_signal_connect (G_OBJECT (notebook), "popup-menu", G_CALLBACK (lunar_window_notebook_popup_menu), window);
  g_signal_connect (G_OBJECT (notebook), "create-window", G_CALLBACK (lunar_window_notebook_create_window), window);

  /* only gets clicks on tabs */
  g_signal_connect (G_OBJECT (GTK_CONTAINER (notebook)), "focus-in-event", G_CALLBACK (lunar_window_paned_notebooks_select), window);

  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), FALSE);
  gtk_notebook_set_scrollable (GTK_NOTEBOOK (notebook), TRUE);
  gtk_container_set_border_width (GTK_CONTAINER (notebook), 0);
  gtk_notebook_set_group_name (GTK_NOTEBOOK (notebook), "lunar-tabs");
  gtk_widget_show (notebook);
  if (window->notebook_left == NULL)
    {
      gtk_paned_pack1 (GTK_PANED (window->paned_notebooks), notebook, TRUE, FALSE);
      window->notebook_left = notebook;
    }
  else if (window->notebook_right == NULL)
    {
      gtk_paned_pack2 (GTK_PANED (window->paned_notebooks), notebook, TRUE, FALSE);
      window->notebook_right = notebook;
    }
  return notebook;
}



static void
lunar_window_paned_notebooks_switch (LunarWindow *window)
{
  GtkWidget *new_curr_notebook = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (lunar_window_split_view_is_active (window));

  if (window->notebook_selected == window->notebook_left)
    new_curr_notebook = window->notebook_right;
  else if (window->notebook_selected == window->notebook_right)
    new_curr_notebook = window->notebook_left;

  if (new_curr_notebook)
    {
      lunar_window_paned_notebooks_indicate_focus (window, new_curr_notebook);

      /* select and activate selected notebook */
      window->notebook_selected = new_curr_notebook;
      lunar_window_notebook_select_current_page (window);
    }
}



static gboolean
lunar_window_paned_notebooks_select (GtkWidget         *view,
                                      GtkDirectionType  *direction,
                                      LunarWindow      *window)
{
  GtkWidget  *selected_notebook;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);
  _lunar_return_val_if_fail (window->notebook_left != NULL || window->notebook_right != NULL, FALSE);

  if (!lunar_window_split_view_is_active (window))
    return FALSE;

  selected_notebook = gtk_widget_get_ancestor (view, GTK_TYPE_NOTEBOOK);
  if (selected_notebook == window->notebook_selected)
    return FALSE;

  lunar_window_paned_notebooks_switch (window);
  return FALSE;
}



static void
lunar_window_paned_notebooks_indicate_focus (LunarWindow *window,
                                              GtkWidget    *notebook)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (GTK_IS_NOTEBOOK (notebook));
  _lunar_return_if_fail (lunar_window_split_view_is_active (window));

  gtk_notebook_set_show_border (GTK_NOTEBOOK (notebook), TRUE);
  if (notebook == window->notebook_left)
    gtk_notebook_set_show_border (GTK_NOTEBOOK (window->notebook_right), FALSE);

  if (notebook == window->notebook_right)
    gtk_notebook_set_show_border (GTK_NOTEBOOK (window->notebook_left), FALSE);
}



static gboolean
lunar_window_split_view_is_active (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);
  return (window->notebook_left && window->notebook_right);
}



void lunar_window_notebook_open_new_tab (LunarWindow *window,
                                          LunarFile   *directory)
{
  LunarHistory *history = NULL;
  GtkWidget     *view;
  gint           page_num;
  GType          view_type;

  /* save the history of the current view */
  if (LUNAR_IS_STANDARD_VIEW (window->view))
    history = lunar_standard_view_copy_history (LUNAR_STANDARD_VIEW (window->view));

  /* find the correct view type */
  view_type = lunar_window_view_type_for_directory (window, directory);

  /* insert the new view */
  page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));
  view = lunar_window_notebook_insert_page (window, directory, view_type, page_num + 1, history);

  /* switch to the new view */
  page_num = gtk_notebook_page_num (GTK_NOTEBOOK (window->notebook_selected), view);
  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook_selected), page_num);

  /* take focus on the new view */
  gtk_widget_grab_focus (view);
}



void
lunar_window_update_directories (LunarWindow *window,
                                  LunarFile   *old_directory,
                                  LunarFile   *new_directory)
{
  GtkWidget  *view;
  LunarFile *directory;
  gint        n;
  gint        n_pages;
  gint        active_page;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (LUNAR_IS_FILE (old_directory));
  _lunar_return_if_fail (LUNAR_IS_FILE (new_directory));

  n_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected));
  if (G_UNLIKELY (n_pages == 0))
    return;

  active_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));

  for (n = 0; n < n_pages; n++)
    {
      /* get the view */
      view = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook_selected), n);
      if (! LUNAR_IS_NAVIGATOR (view))
        continue;

      /* get the directory of the view */
      directory = lunar_navigator_get_current_directory (LUNAR_NAVIGATOR (view));
      if (! LUNAR_IS_FILE (directory))
        continue;

      /* if it matches the old directory, change to the new one */
      if (directory == old_directory)
        {
          if (n == active_page)
            lunar_navigator_change_directory (LUNAR_NAVIGATOR (view), new_directory);
          else
            lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (view), new_directory);
        }
    }
}



static void
lunar_window_update_location_bar_visible (LunarWindow *window)
{
  gchar *last_location_bar = NULL;

  g_object_get (window->preferences, "last-location-bar", &last_location_bar, NULL);

  if (endo_str_is_equal (last_location_bar, g_type_name (G_TYPE_NONE)))
    {
      gtk_widget_hide (window->location_toolbar);
      gtk_widget_grab_focus (window->view);
    }
  else
    gtk_widget_show (window->location_toolbar);

  g_free (last_location_bar);
}



static void
lunar_window_update_window_icon (LunarWindow *window)
{
  gboolean      change_window_icon;
  GtkIconTheme *icon_theme;
  const gchar  *icon_name = "folder";

  g_object_get (window->preferences, "misc-change-window-icon", &change_window_icon, NULL);

  if (change_window_icon)
    {
      icon_theme = gtk_icon_theme_get_for_screen (gtk_window_get_screen (GTK_WINDOW (window)));
      icon_name = lunar_file_get_icon_name (window->current_directory,
                                             LUNAR_FILE_ICON_STATE_DEFAULT,
                                             icon_theme);
    }

  gtk_window_set_icon_name (GTK_WINDOW (window), icon_name);
}



static void
lunar_window_handle_reload_request (LunarWindow *window)
{
  gboolean result;

  /* force the view to reload */
  g_signal_emit (G_OBJECT (window), window_signals[RELOAD], 0, TRUE, &result);
}



static void
lunar_window_install_sidepane (LunarWindow *window,
                                GType         type)
{
  GtkStyleContext *context;

  _lunar_return_if_fail (type == G_TYPE_NONE || g_type_is_a (type, LUNAR_TYPE_SIDE_PANE));
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* drop the previous side pane (if any) */
  if (G_UNLIKELY (window->sidepane != NULL))
    {
      gtk_widget_destroy (window->sidepane);
      window->sidepane = NULL;
    }

  /* check if we have a new sidepane widget */
  if (G_LIKELY (type != G_TYPE_NONE))
    {
      /* allocate the new side pane widget */
      window->sidepane = g_object_new (type, NULL);
      gtk_widget_set_size_request (window->sidepane, 0, -1);
      endo_binding_new (G_OBJECT (window), "current-directory", G_OBJECT (window->sidepane), "current-directory");
      g_signal_connect_swapped (G_OBJECT (window->sidepane), "change-directory", G_CALLBACK (lunar_window_set_current_directory), window);
      g_signal_connect_swapped (G_OBJECT (window->sidepane), "open-new-tab", G_CALLBACK (lunar_window_notebook_open_new_tab), window);
      context = gtk_widget_get_style_context (window->sidepane);
      gtk_style_context_add_class (context, "sidebar");
      gtk_paned_pack1 (GTK_PANED (window->paned), window->sidepane, FALSE, FALSE);
      gtk_widget_show (window->sidepane);

      /* connect the side pane widget to the view (if any) */
      if (G_LIKELY (window->view != NULL))
        lunar_window_binding_create (window, window->view, "selected-files", window->sidepane, "selected-files", G_BINDING_SYNC_CREATE);

      /* apply show_hidden config to tree pane */
      if (type == LUNAR_TYPE_TREE_PANE)
        lunar_side_pane_set_show_hidden (LUNAR_SIDE_PANE (window->sidepane), window->show_hidden);
    }

  /* remember the setting */
  if (gtk_widget_get_visible (GTK_WIDGET (window)))
    g_object_set (G_OBJECT (window->preferences), "last-side-pane", g_type_name (type), NULL);
}



static gchar*
lunar_window_bookmark_get_accel_path (GFile *bookmark_file)
{
  GChecksum    *checksum;
  gchar        *uri;
  gchar        *accel_path;
  const gchar  *unique_name;

  _lunar_return_val_if_fail (G_IS_FILE (bookmark_file), NULL);

  /* create unique id based on the uri */
  uri = g_file_get_uri (bookmark_file);
  checksum = g_checksum_new (G_CHECKSUM_MD5);
  g_checksum_update (checksum, (const guchar *) uri, strlen (uri));
  unique_name = g_checksum_get_string (checksum);
  accel_path = g_strconcat("<Actions>/LunarBookmarks/", unique_name, NULL);

  g_free (uri);
  g_checksum_free (checksum);
  return accel_path;
}



static void
lunar_window_menu_add_bookmarks (LunarWindow *window,
                                  GtkMenuShell *view_menu)
{
  GList          *lp;
  LunarBookmark *bookmark;
  LunarFile     *lunar_file;
  gchar          *parse_name;
  gchar          *accel_path;
  gchar          *tooltip;
  const gchar    *name;
  gchar          *remote_name;
  GtkIconTheme   *icon_theme;
  const gchar    *icon_name;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  for (lp = window->bookmarks; lp != NULL; lp = lp->next)
    {
      bookmark = lp->data;
      accel_path = lunar_window_bookmark_get_accel_path (bookmark->g_file);
      parse_name = g_file_get_parse_name (bookmark->g_file);
      tooltip = g_strdup_printf (_("Open the location \"%s\""), parse_name);
      g_free (parse_name);

      if (g_file_has_uri_scheme (bookmark->g_file, "file"))
        {
          /* try to open the file corresponding to the uri */
          lunar_file = lunar_file_get (bookmark->g_file, NULL);
          if (G_LIKELY (lunar_file != NULL))
            {
              /* make sure the file refers to a directory */
              if (G_UNLIKELY (lunar_file_is_directory (lunar_file)))
                {
                  name = bookmark->name;
                  if (bookmark->name == NULL)
                    name = lunar_file_get_display_name (lunar_file);

                  icon_theme = gtk_icon_theme_get_for_screen (gtk_window_get_screen (GTK_WINDOW (window)));
                  icon_name = lunar_file_get_icon_name (lunar_file, LUNAR_FILE_ICON_STATE_DEFAULT, icon_theme);
                  expidus_gtk_image_menu_item_new_from_icon_name (name, tooltip, accel_path, G_CALLBACK (lunar_window_action_open_bookmark), G_OBJECT (bookmark->g_file), icon_name, view_menu);
               }
            g_object_unref (lunar_file);
          }
        }
      else
        {
          if (bookmark->name == NULL)
            remote_name = lunar_g_file_get_display_name_remote (bookmark->g_file);
          else
            remote_name = g_strdup (bookmark->name);
          expidus_gtk_image_menu_item_new_from_icon_name (remote_name, tooltip, accel_path, G_CALLBACK (lunar_window_action_open_bookmark),  G_OBJECT (bookmark->g_file), "folder-remote", view_menu);
          g_free (remote_name);
        }

      g_free (tooltip);
      g_free (accel_path);
    }
}



static LunarBookmark *
lunar_window_bookmark_add (LunarWindow *window,
                            GFile        *g_file,
                            const gchar  *name)
{
  LunarBookmark *bookmark;

  bookmark = g_slice_new0 (LunarBookmark);
  bookmark->g_file = g_object_ref (g_file);
  bookmark->name = g_strdup (name);

  window->bookmarks = g_list_append (window->bookmarks, bookmark);
  return bookmark;
}



static void
lunar_window_free_bookmarks (LunarWindow *window)
{
  GList          *lp;
  LunarBookmark *bookmark;

  for (lp = window->bookmarks; lp != NULL; lp = lp->next)
    {
      bookmark = lp->data;
      g_object_unref (bookmark->g_file);
      g_free (bookmark->name);
      g_slice_free (LunarBookmark, lp->data);
    }
  window->bookmarks = NULL;
}



static void
lunar_window_update_bookmark (GFile       *g_file,
                               const gchar *name,
                               gint         line_num,
                               gpointer     user_data)
{
  LunarWindow      *window = LUNAR_WINDOW (user_data);
  gchar             *accel_path;
  ExpidusGtkActionEntry entry[1];

  _lunar_return_if_fail (G_IS_FILE (g_file));
  _lunar_return_if_fail (name == NULL || g_utf8_validate (name, -1, NULL));
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* Add the bookmark to our internal list of bookmarks */
  lunar_window_bookmark_add (window, g_file, name);
  
  /* Add ref to window to each g_file, will be needed in callback */
  g_object_set_data_full (G_OBJECT (g_file), I_("lunar-window"), window, NULL);

  /* Build a minimal ExpidusGtkActionEntry in order to be able to use the methods below */
  accel_path = lunar_window_bookmark_get_accel_path (g_file);
  entry[0].accel_path = accel_path;
  entry[0].callback = G_CALLBACK (lunar_window_action_open_bookmark);
  entry[0].default_accelerator = "";

  /* Add entry, so that the bookmark can loaded/saved to acceels.scm (will be skipped if already available)*/
  expidus_gtk_accel_map_add_entries (entry, G_N_ELEMENTS (entry));

  /* Link action with callback */
  expidus_gtk_accel_group_disconnect_action_entries (window->accel_group, entry, G_N_ELEMENTS (entry));
  expidus_gtk_accel_group_connect_action_entries (window->accel_group, entry, G_N_ELEMENTS (entry), g_file);
  g_free (accel_path);
}



static void
lunar_window_update_bookmarks (LunarWindow *window)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  lunar_window_free_bookmarks (window);

  /* re-create our internal bookmarks according to the bookmark file */
  lunar_util_load_bookmarks (window->bookmark_file,
                              lunar_window_update_bookmark,
                              window);
}



static void
lunar_window_open_or_launch (LunarWindow *window,
                              LunarFile   *file)
{
  GError *error = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  if (lunar_file_is_directory (file))
    {
      /* open the new directory */
      lunar_window_set_current_directory (window, file);
    }
  else
    {
      /* try to launch the selected file */
      if (!lunar_file_launch (file, window, NULL, &error))
        {
          lunar_dialogs_show_error (window, error, _("Failed to launch \"%s\""),
                                     lunar_file_get_display_name (file));
          g_error_free (error);
        }
    }
}



static void
lunar_window_poke_file_finish (LunarBrowser *browser,
                                LunarFile    *file,
                                LunarFile    *target_file,
                                GError        *error,
                                gpointer       ignored)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  if (error == NULL)
    {
      lunar_window_open_or_launch (LUNAR_WINDOW (browser), target_file);
    }
  else
    {
      lunar_dialogs_show_error (GTK_WIDGET (browser), error,
                                 _("Failed to open \"%s\""),
                                 lunar_file_get_display_name (file));
    }
}



static void
lunar_window_start_open_location (LunarWindow *window,
                                   const gchar  *initial_text)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* temporary show the location toolbar, even if it is normally hidden */
  gtk_widget_show (window->location_toolbar);
  lunar_location_bar_request_entry (LUNAR_LOCATION_BAR (window->location_bar), initial_text);
}



static void
lunar_window_action_open_new_tab (LunarWindow *window,
                                   GtkWidget    *menu_item)
{
  /* open new tab with current directory as default */
  lunar_window_notebook_open_new_tab (window, lunar_window_get_current_directory (window));
}



static void
lunar_window_action_open_new_window (LunarWindow *window,
                                      GtkWidget    *menu_item)
{
  LunarApplication *application;
  LunarHistory     *history;
  LunarWindow      *new_window;
  LunarFile        *start_file;

  /* popup a new window */
  application = lunar_application_get ();
  new_window = LUNAR_WINDOW (lunar_application_open_window (application, window->current_directory,
                                                              gtk_widget_get_screen (GTK_WIDGET (window)), NULL, TRUE));
  g_object_unref (G_OBJECT (application));

  /* if we have no origin view we are done */
  if (window->view == NULL)
    return;

  /* let the view of the new window inherit the history of the origin view */
  history = lunar_standard_view_copy_history (LUNAR_STANDARD_VIEW (window->view));
  if (history != NULL)
    {
      lunar_standard_view_set_history (LUNAR_STANDARD_VIEW (new_window->view), history);
      lunar_window_history_changed (new_window);

      /* connect to the new history */
      window->signal_handler_id_history_changed = g_signal_connect_swapped (G_OBJECT (history), "history-changed", G_CALLBACK (lunar_window_history_changed), new_window);
    }

  /* determine the first visible file in the current window */
  if (lunar_view_get_visible_range (LUNAR_VIEW (window->view), &start_file, NULL))
    {
      /* scroll the new window to the same file */
      lunar_window_scroll_to_file (new_window, start_file, FALSE, TRUE, 0.1f, 0.1f);

      /* release the file reference */
      g_object_unref (G_OBJECT (start_file));
    }
}



static void
lunar_window_action_detach_tab (LunarWindow *window,
                                 GtkWidget    *menu_item)
{
  GtkWidget *notebook;
  GtkWidget *label;
  GtkWidget *view = window->view;

  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* create a new window */
  notebook = lunar_window_notebook_create_window (window->notebook_selected, view, -1, -1, window);
  if (notebook == NULL)
    return;

  /* get the current label */
  label = gtk_notebook_get_tab_label (GTK_NOTEBOOK (window->notebook_selected), view);
  _lunar_return_if_fail (GTK_IS_WIDGET (label));

  /* ref object so they don't destroy when removed from the container */
  g_object_ref (label);
  g_object_ref (view);

  /* remove view from the current notebook */
  gtk_container_remove (GTK_CONTAINER (window->notebook_selected), view);

  /* insert in the new notebook */
  gtk_notebook_insert_page (GTK_NOTEBOOK (notebook), view, label, 0);

  /* set tab child properties */
  gtk_container_child_set (GTK_CONTAINER (notebook), view, "tab-expand", TRUE, NULL);
  gtk_notebook_set_tab_reorderable (GTK_NOTEBOOK (notebook), view, TRUE);
  gtk_notebook_set_tab_detachable (GTK_NOTEBOOK (notebook), view, TRUE);

  /* release */
  g_object_unref (label);
  g_object_unref (view);
}



static void
lunar_window_action_close_all_windows (LunarWindow *window,
                                        GtkWidget    *menu_item)
{
  LunarApplication *application;
  GList             *windows;

  /* query the list of currently open windows */
  application = lunar_application_get ();
  windows = lunar_application_get_windows (application);
  g_object_unref (G_OBJECT (application));

  /* destroy all open windows */
  g_list_free_full (windows, (GDestroyNotify) gtk_widget_destroy);
}



static void
lunar_window_action_close_tab (LunarWindow *window,
                                GtkWidget    *menu_item)
{
  if (lunar_window_split_view_is_active (window))
    {
      if (window->view != NULL)
         gtk_widget_destroy (window->view);
    }
  else
    {
      if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected)) == 1)
        gtk_widget_destroy (GTK_WIDGET (window));
      else if (window->view != NULL)
        gtk_widget_destroy (window->view);
    }
}



static void
lunar_window_action_close_window (LunarWindow *window,
                                   GtkWidget    *menu_item)
{
  gtk_widget_destroy (GTK_WIDGET (window));
}



static void
lunar_window_action_preferences (LunarWindow *window,
                                  GtkWidget    *menu_item)
{
  GtkWidget         *dialog;
  LunarApplication *application;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* allocate and display a preferences dialog */;
  dialog = lunar_preferences_dialog_new (GTK_WINDOW (window));
  gtk_widget_show (dialog);

  /* ...and let the application take care of it */
  application = lunar_application_get ();
  lunar_application_take_window (application, GTK_WINDOW (dialog));
  g_object_unref (G_OBJECT (application));
}



static void
lunar_window_action_reload (LunarWindow *window,
                             GtkWidget    *menu_item)
{
  gboolean result;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* force the view to reload */
  g_signal_emit (G_OBJECT (window), window_signals[RELOAD], 0, TRUE, &result);

  /* update the location bar to show the current directory */
  if (window->location_bar != NULL)
    g_object_notify (G_OBJECT (window->location_bar), "current-directory");
}



static void
lunar_window_action_toggle_split_view (LunarWindow *window)
{

  LunarFile    *directory;
  LunarHistory *history = NULL;
  gint           page_num;
  GType          view_type;
  GtkAllocation  allocation; 

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (window->view_type != G_TYPE_NONE);

  if (lunar_window_split_view_is_active (window))
    {
      if (window->notebook_selected == window->notebook_left)
        {
          /** without split view, no VISUAL selection indicator needed:
           *  so missuse indicate_focus() to remove VISUAL selection indicator of
           *  the remaining notebook.
           **/
          lunar_window_paned_notebooks_indicate_focus (window, window->notebook_right);
          gtk_widget_destroy (window->notebook_right);
          window->notebook_right = NULL;
        }
      else if (window->notebook_selected == window->notebook_right)
        {
          lunar_window_paned_notebooks_indicate_focus (window, window->notebook_left);
          gtk_widget_destroy (window->notebook_left);
          window->notebook_left = NULL;
        }
    }
  else
    {
      window->notebook_selected = lunar_window_paned_notebooks_add (window);
      lunar_window_paned_notebooks_indicate_focus (window, window->notebook_selected);
      directory = lunar_window_get_current_directory (window);

      /* save the history of the current view */
      if (LUNAR_IS_STANDARD_VIEW (window->view))
        history = lunar_standard_view_copy_history (LUNAR_STANDARD_VIEW (window->view));

      /* find the correct view type */
      view_type = lunar_window_view_type_for_directory (window, directory);

      /* insert the new view */
      page_num = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));
      lunar_window_notebook_insert_page (window, directory, view_type, page_num+1, history);

      /* Prevent notebook expand on tab creation */
      gtk_widget_get_allocation (GTK_WIDGET (window->paned_notebooks), &allocation);
      gtk_paned_set_position (GTK_PANED (window->paned_notebooks), (gint)(allocation.width/2));
    }
}



static void
lunar_window_action_pathbar_changed (LunarWindow *window)
{
  gchar    *last_location_bar;
  gboolean  pathbar_checked;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  g_object_get (window->preferences, "last-location-bar", &last_location_bar, NULL);
  pathbar_checked = endo_str_is_equal (last_location_bar, g_type_name (LUNAR_TYPE_LOCATION_ENTRY));
  g_free (last_location_bar);

  if (pathbar_checked)
    g_object_set (window->preferences, "last-location-bar", g_type_name (G_TYPE_NONE), NULL);
  else
    g_object_set (window->preferences, "last-location-bar", g_type_name (LUNAR_TYPE_LOCATION_ENTRY), NULL);
}



static void
lunar_window_action_toolbar_changed (LunarWindow *window)
{
  gchar    *last_location_bar;
  gboolean  toolbar_checked;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  g_object_get (window->preferences, "last-location-bar", &last_location_bar, NULL);
  toolbar_checked = endo_str_is_equal (last_location_bar, g_type_name (LUNAR_TYPE_LOCATION_BUTTONS));
  g_free (last_location_bar);

  if (toolbar_checked)
    g_object_set (window->preferences, "last-location-bar", g_type_name (G_TYPE_NONE), NULL);
  else
    g_object_set (window->preferences, "last-location-bar", g_type_name (LUNAR_TYPE_LOCATION_BUTTONS), NULL);
}



static void
lunar_window_action_shortcuts_changed (LunarWindow *window)
{
  gchar    *last_side_pane;
  gboolean  shortcuts_checked;
  GType     type = G_TYPE_NONE;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  g_object_get (window->preferences, "last-side-pane", &last_side_pane, NULL);
  shortcuts_checked = endo_str_is_equal (last_side_pane, g_type_name (LUNAR_TYPE_SHORTCUTS_PANE));
  g_free (last_side_pane);

  if (shortcuts_checked)
    type = G_TYPE_NONE;
  else
    type = LUNAR_TYPE_SHORTCUTS_PANE;

  lunar_window_install_sidepane (window, type);
}



static void
lunar_window_action_tree_changed (LunarWindow *window)
{
  gchar    *last_side_pane;
  gboolean  tree_view_checked;
  GType     type = G_TYPE_NONE;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  g_object_get (window->preferences, "last-side-pane", &last_side_pane, NULL);
  tree_view_checked = endo_str_is_equal (last_side_pane, g_type_name (LUNAR_TYPE_TREE_PANE));
  g_free (last_side_pane);

  if (tree_view_checked)
    type = G_TYPE_NONE;
  else
    type = LUNAR_TYPE_TREE_PANE;

  lunar_window_install_sidepane (window, type);
}



static void
lunar_window_action_statusbar_changed (LunarWindow *window)
{
  gboolean last_statusbar_visible;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  g_object_get (window->preferences, "last-statusbar-visible", &last_statusbar_visible, NULL);

  gtk_widget_set_visible (window->statusbar, !last_statusbar_visible);

  g_object_set (G_OBJECT (window->preferences), "last-statusbar-visible", !last_statusbar_visible, NULL);
}



static void
lunar_window_action_menubar_changed (LunarWindow *window)
{
  gboolean last_menubar_visible;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  g_object_get (window->preferences, "last-menubar-visible", &last_menubar_visible, NULL);

  gtk_widget_set_visible (window->menubar, !last_menubar_visible);

  g_object_set (G_OBJECT (window->preferences), "last-menubar-visible", !last_menubar_visible, NULL);
}



static void
lunar_window_action_clear_directory_specific_settings (LunarWindow *window)
{
  GType       view_type;
  gboolean    result;

  /* clear the settings */
  lunar_file_clear_directory_specific_settings (window->current_directory);

  /* get the correct view type for the current directory */
  view_type = lunar_window_view_type_for_directory (window, window->current_directory);

  /* force the view to reload so that any changes to the settings are applied */
  g_signal_emit (G_OBJECT (window), window_signals[RELOAD], 0, TRUE, &result);

  /* replace the active view with a new one of the correct type */
  lunar_window_replace_view (window, window->view, view_type);
}



static void
lunar_window_action_detailed_view (LunarWindow *window)
{
  lunar_window_action_view_changed (window, LUNAR_TYPE_DETAILS_VIEW);
}



static void
lunar_window_action_icon_view (LunarWindow *window)
{
  lunar_window_action_view_changed (window, LUNAR_TYPE_ICON_VIEW);
}



static void
lunar_window_action_compact_view (LunarWindow *window)
{
  lunar_window_action_view_changed (window, LUNAR_TYPE_COMPACT_VIEW);
}



static void
lunar_window_replace_view (LunarWindow *window,
                            GtkWidget    *view,
                            GType         view_type)
{
  LunarFile     *file = NULL;
  LunarFile     *current_directory = NULL;
  GtkWidget      *new_view;
  LunarHistory  *history = NULL;
  GList          *selected_files = NULL;
  gint            page_num;
  gboolean        is_current_view;

  _lunar_return_if_fail (view_type != G_TYPE_NONE);

  /* if the view already has the correct type then just return */
  if (view != NULL && G_TYPE_FROM_INSTANCE (view) == view_type)
    return;

  /* is the view we are replacing the active view?
   * (note that this will be true if both view and window->view are NULL) */
  is_current_view = (view == window->view);

  /* save some settings from the old view for the new view */
  if (view != NULL)
    {
      /* disconnect from previous history if the old view is the active view */
      if (is_current_view && window->signal_handler_id_history_changed != 0)
        {
          history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (view));
          g_signal_handler_disconnect (history, window->signal_handler_id_history_changed);
          window->signal_handler_id_history_changed = 0;
        }

      /* get first visible file in the old view */
      if (!lunar_view_get_visible_range (LUNAR_VIEW (view), &file, NULL))
        file = NULL;

      /* store the active directory from the old view */
      current_directory = lunar_navigator_get_current_directory (LUNAR_NAVIGATOR (view));
      if (current_directory != NULL)
        g_object_ref (current_directory);

      /* remember the file selection from the old view */
      selected_files = lunar_g_file_list_copy (lunar_component_get_selected_files (LUNAR_COMPONENT (view)));

      /* save the history of the current view */
      history = NULL;
      if (LUNAR_IS_STANDARD_VIEW (view))
        history = lunar_standard_view_copy_history (LUNAR_STANDARD_VIEW (view));
    }

  if (is_current_view)
    window->view_type = view_type;

  /* if we have not got a current directory from the old view, use the window's current directory */
  if (current_directory == NULL && window->current_directory != NULL)
    current_directory = g_object_ref (window->current_directory);

  _lunar_assert (current_directory != NULL);

  /* find where to insert the new view */
  if (view != NULL)
    page_num = gtk_notebook_page_num (GTK_NOTEBOOK (window->notebook_selected), view);
  else
    page_num = -1;

  /* insert the new view */
  new_view = lunar_window_notebook_insert_page (window, current_directory, view_type, page_num + 1, history);

  /* if we are replacing the active view, make the new view the active view */
  if (is_current_view)
    {
      /* switch to the new view */
      page_num = gtk_notebook_page_num (GTK_NOTEBOOK (window->notebook_selected), new_view);
      gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook_selected), page_num);

      /* take focus on the new view */
      gtk_widget_grab_focus (new_view);
    }

  /* scroll to the previously visible file in the old view */
  if (G_UNLIKELY (file != NULL))
    lunar_view_scroll_to_file (LUNAR_VIEW (new_view), file, FALSE, TRUE, 0.0f, 0.0f);

  /* destroy the old view */
  if (view != NULL)
    gtk_widget_destroy (view);

  /* restore the file selection */
  lunar_component_set_selected_files (LUNAR_COMPONENT (new_view), selected_files);
  lunar_g_file_list_free (selected_files);

  /* remember the last view type if this is the active view and directory specific settings are not enabled */
  if (is_current_view && !window->directory_specific_settings && gtk_widget_get_visible (GTK_WIDGET (window)) && view_type != G_TYPE_NONE)
    g_object_set (G_OBJECT (window->preferences), "last-view", g_type_name (view_type), NULL);

  /* release the file references */
  if (G_UNLIKELY (file != NULL))
    g_object_unref (G_OBJECT (file));
  if (G_UNLIKELY (current_directory != NULL))
    g_object_unref (G_OBJECT (current_directory));

  /* connect to the new history if this is the active view */
  if (is_current_view)
    {
      history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (new_view));
      window->signal_handler_id_history_changed = g_signal_connect_swapped (G_OBJECT (history),
                                                                            "history-changed",
                                                                            G_CALLBACK (lunar_window_history_changed),
                                                                            window);
    }
}



static void
lunar_window_action_view_changed (LunarWindow *window,
                                   GType         view_type)
{
  lunar_window_replace_view (window, window->view, view_type);

  /* if directory specific settings are enabled, save the view type for this directory */
  if (window->directory_specific_settings)
    lunar_file_set_metadata_setting (window->current_directory, "view-type", g_type_name (view_type));
}



static void
lunar_window_action_go_up (LunarWindow *window)
{
  LunarFile *parent;
  GError     *error = NULL;

  parent = lunar_file_get_parent (window->current_directory, &error);
  if (G_LIKELY (parent != NULL))
    {
      lunar_window_set_current_directory (window, parent);
      g_object_unref (G_OBJECT (parent));
    }
  else
    {
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to open parent folder"));
      g_error_free (error);
    }
}



static void
lunar_window_action_back (LunarWindow *window)
{
  LunarHistory *history;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));
  lunar_history_action_back (history);
}



static void
lunar_window_action_forward (LunarWindow *window)
{
  LunarHistory *history;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));
  lunar_history_action_forward (history);
}



static void
lunar_window_action_open_home (LunarWindow *window)
{
  GFile         *home;
  LunarFile    *home_file;
  GError        *error = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* determine the path to the home directory */
  home = lunar_g_file_new_for_home ();

  /* determine the file for the home directory */
  home_file = lunar_file_get (home, &error);
  if (G_UNLIKELY (home_file == NULL))
    {
      /* display an error to the user */
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to open the home folder"));
      g_error_free (error);
    }
  else
    {
      /* open the home folder */
      lunar_window_set_current_directory (window, home_file);
      g_object_unref (G_OBJECT (home_file));
    }

  /* release our reference on the home path */
  g_object_unref (home);
}



static gboolean
lunar_window_open_user_folder (LunarWindow   *window,
                                GUserDirectory  lunar_user_dir,
                                const gchar    *default_name)
{
  LunarFile  *user_file = NULL;
  gboolean     result = FALSE;
  GError      *error = NULL;
  GFile       *home_dir;
  GFile       *user_dir;
  const gchar *path;
  gint         response;
  GtkWidget   *dialog;
  gchar       *parse_name;

  path = g_get_user_special_dir (lunar_user_dir);
  home_dir = lunar_g_file_new_for_home ();

  /* check if there is an entry in user-dirs.dirs */
  path = g_get_user_special_dir (lunar_user_dir);
  if (G_LIKELY (path != NULL))
    {
      user_dir = g_file_new_for_path (path);

      /* if equal to home, leave */
      if (g_file_equal (user_dir, home_dir))
        goto is_homedir;
    }
  else
    {
      /* build a name */
      user_dir = g_file_resolve_relative_path (home_dir, default_name);
    }

  /* try to load the user dir */
  user_file = lunar_file_get (user_dir, NULL);

  /* check if the directory exists */
  if (G_UNLIKELY (user_file == NULL || !lunar_file_exists (user_file)))
    {
      /* release the instance if it does not exist */
      if (user_file != NULL)
        {
          g_object_unref (user_file);
          user_file = NULL;
        }

      /* ask the user to create the directory */
      parse_name = g_file_get_parse_name (user_dir);
      dialog = gtk_message_dialog_new (GTK_WINDOW (window),
                                       GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                       GTK_MESSAGE_QUESTION,
                                       GTK_BUTTONS_YES_NO,
                                       _("The directory \"%s\" does not exist. Do you want to create it?"),
                                       parse_name);
      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_YES);
      response = gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
      g_free (parse_name);

      if (response == GTK_RESPONSE_YES
          && g_file_make_directory_with_parents (user_dir, NULL, &error))
        {
          /* try again */
          user_file = lunar_file_get (user_dir, &error);
        }
    }

  if (G_LIKELY (user_file != NULL))
    {
      /* open the folder */
      lunar_window_set_current_directory (window, user_file);
      g_object_unref (G_OBJECT (user_file));
      result = TRUE;
    }
  else if (error != NULL)
    {
      parse_name = g_file_get_parse_name (user_dir);
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to open directory \"%s\""), parse_name);
      g_free (parse_name);
      g_error_free (error);
    }

  is_homedir:

  g_object_unref (user_dir);
  g_object_unref (home_dir);

  return result;
}



static void
lunar_window_action_open_desktop (LunarWindow *window)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  lunar_window_open_user_folder (window, G_USER_DIRECTORY_DESKTOP, "Desktop");
}



static void
lunar_window_action_open_computer (LunarWindow *window)
{
  GFile         *computer;
  LunarFile    *computer_file;
  GError        *error = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* determine the computer location */
  computer = g_file_new_for_uri ("computer://");

  /* determine the file for this location */
  computer_file = lunar_file_get (computer, &error);
  if (G_UNLIKELY (computer_file == NULL))
    {
      /* display an error to the user */
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to browse the computer"));
      g_error_free (error);
    }
  else
    {
      /* open the computer location */
      lunar_window_set_current_directory (window, computer_file);
      g_object_unref (G_OBJECT (computer_file));
    }

  /* release our reference on the location itself */
  g_object_unref (computer);
}



static void
lunar_window_action_open_templates (LunarWindow *window)
{
  GtkWidget     *dialog;
  GtkWidget     *button;
  GtkWidget     *label;
  GtkWidget     *image;
  GtkWidget     *hbox;
  GtkWidget     *vbox;
  gboolean       show_about_templates;
  gboolean       success;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  success = lunar_window_open_user_folder (window, G_USER_DIRECTORY_TEMPLATES, "Templates");

  /* check whether we should display the "About Templates" dialog */
  g_object_get (G_OBJECT (window->preferences),
                "misc-show-about-templates", &show_about_templates,
                NULL);

  if (G_UNLIKELY(show_about_templates && success))
    {
      /* display the "About Templates" dialog */
      dialog = gtk_dialog_new_with_buttons (_("About Templates"), GTK_WINDOW (window),
                                            GTK_DIALOG_DESTROY_WITH_PARENT,
                                            _("_OK"), GTK_RESPONSE_OK,
                                            NULL);

      gtk_dialog_set_default_response (GTK_DIALOG (dialog), GTK_RESPONSE_OK);

      hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
      gtk_container_set_border_width (GTK_CONTAINER (hbox), 8);
      gtk_box_pack_start (GTK_BOX (gtk_dialog_get_content_area (GTK_DIALOG (dialog))), hbox, TRUE, TRUE, 0);
      gtk_widget_show (hbox);

      image = gtk_image_new_from_icon_name ("dialog-information", GTK_ICON_SIZE_DIALOG);
      gtk_widget_set_halign (image, GTK_ALIGN_CENTER);
      gtk_widget_set_valign (image, GTK_ALIGN_START);
      gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, FALSE, 0);
      gtk_widget_show (image);

      vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 18);
      gtk_box_pack_start (GTK_BOX (hbox), vbox, TRUE, TRUE, 0);
      gtk_widget_show (vbox);

      label = gtk_label_new (_("All files in this folder will appear in the \"Create Document\" menu."));
      gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
      gtk_label_set_attributes (GTK_LABEL (label), lunar_pango_attr_list_big_bold ());
      gtk_label_set_line_wrap (GTK_LABEL (label), FALSE);
      gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
      gtk_widget_show (label);

      label = gtk_label_new (_("If you frequently create certain kinds "
                             " of documents, make a copy of one and put it in this "
                             "folder. Lunar will add an entry for this document in the"
                             " \"Create Document\" menu.\n\n"
                             "You can then select the entry from the \"Create Document\" "
                             "menu and a copy of the document will be created in the "
                             "directory you are viewing."));
      gtk_label_set_xalign (GTK_LABEL (label), 0.0f);
      gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
      gtk_box_pack_start (GTK_BOX (vbox), label, TRUE, TRUE, 0);
      gtk_widget_show (label);

      button = gtk_check_button_new_with_mnemonic (_("Do _not display this message again"));
      endo_mutual_binding_new_with_negation (G_OBJECT (window->preferences), "misc-show-about-templates", G_OBJECT (button), "active");
      gtk_box_pack_start (GTK_BOX (vbox), button, FALSE, FALSE, 0);
      gtk_widget_show (button);

      gtk_window_set_default_size (GTK_WINDOW (dialog), 600, -1);

      gtk_dialog_run (GTK_DIALOG (dialog));
      gtk_widget_destroy (dialog);
    }
}



static void
lunar_window_action_open_file_system (LunarWindow *window)
{
  GFile         *root;
  LunarFile    *root_file;
  GError        *error = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* determine the path to the root directory */
  root = lunar_g_file_new_for_root ();

  /* determine the file for the root directory */
  root_file = lunar_file_get (root, &error);
  if (G_UNLIKELY (root_file == NULL))
    {
      /* display an error to the user */
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to open the file system root folder"));
      g_error_free (error);
    }
  else
    {
      /* open the root folder */
      lunar_window_set_current_directory (window, root_file);
      g_object_unref (G_OBJECT (root_file));
    }

  /* release our reference on the home path */
  g_object_unref (root);
}



static void
lunar_window_action_open_trash (LunarWindow *window)
{
  GFile      *trash_bin;
  LunarFile *trash_bin_file;
  GError     *error = NULL;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* determine the path to the trash bin */
  trash_bin = lunar_g_file_new_for_trash ();

  /* determine the file for the trash bin */
  trash_bin_file = lunar_file_get (trash_bin, &error);
  if (G_UNLIKELY (trash_bin_file == NULL))
    {
      /* display an error to the user */
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to display the contents of the trash can"));
      g_error_free (error);
    }
  else
    {
      /* open the trash folder */
      lunar_window_set_current_directory (window, trash_bin_file);
      g_object_unref (G_OBJECT (trash_bin_file));
    }

  /* release our reference on the trash bin path */
  g_object_unref (trash_bin);
}



static void
lunar_window_action_open_network (LunarWindow *window)
{
  LunarFile *network_file;
  GError     *error = NULL;
  GFile      *network;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* determine the network root location */
  network = g_file_new_for_uri ("network://");

  /* determine the file for this location */
  network_file = lunar_file_get (network, &error);
  if (G_UNLIKELY (network_file == NULL))
    {
      /* display an error to the user */
      lunar_dialogs_show_error (GTK_WIDGET (window), error, _("Failed to browse the network"));
      g_error_free (error);
    }
  else
    {
      /* open the network root location */
      lunar_window_set_current_directory (window, network_file);
      g_object_unref (G_OBJECT (network_file));
    }

  /* release our reference on the location itself */
  g_object_unref (network);
}



static gboolean
lunar_window_check_uca_key_activation (LunarWindow *window,
                                        GdkEventKey  *key_event,
                                        gpointer      user_data)
{
  if (lunar_launcher_check_uca_key_activation (window->launcher, key_event))
    return GDK_EVENT_STOP;
  return GDK_EVENT_PROPAGATE;
}



static gboolean
lunar_window_propagate_key_event (GtkWindow* window,
                                   GdkEvent  *key_event,
                                   gpointer   user_data)
{
  GtkWidget* focused_widget;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), GDK_EVENT_PROPAGATE);

  focused_widget = gtk_window_get_focus (window);

  /* Turn the accelerator priority around globally,
   * so that the focused widget always gets the accels first.
   * Implementing this cleanly while maintaining some wanted accels
   * (like Ctrl+N and endo accels) is a lot of work. So we resort to
   * only priorize GtkEditable, because that is the easiest way to
   * fix the right-ahead problem. */
  if (focused_widget != NULL && GTK_IS_EDITABLE (focused_widget))
    return gtk_window_propagate_key_event (window, (GdkEventKey *) key_event);

  return GDK_EVENT_PROPAGATE;
}



static void
lunar_window_poke_location_finish (LunarBrowser *browser,
                                    GFile         *location,
                                    LunarFile    *file,
                                    LunarFile    *target_file,
                                    GError        *error,
                                    gpointer       ignored)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  lunar_window_poke_file_finish (browser, file, target_file, error, ignored);
}



static void
lunar_window_action_open_bookmark (GFile *g_file)
{
  GtkWindow *window;

  window = g_object_get_data (G_OBJECT (g_file), I_("lunar-window"));
 
  lunar_window_set_current_directory_gfile (LUNAR_WINDOW (window), g_file);
}



static void
lunar_window_action_open_location (LunarWindow *window)
{
  /* just use the "start-open-location" callback */
  lunar_window_start_open_location (window, NULL);
}



static void
lunar_window_action_contents (LunarWindow *window)
{
  /* display the documentation index */
  expidus_dialog_show_help (GTK_WINDOW (window), "lunar", NULL, NULL);
}



static void
lunar_window_action_about (LunarWindow *window)
{
  /* just popup the about dialog */
  lunar_dialogs_show_about (GTK_WINDOW (window), PACKAGE_NAME,
                             _("Lunar is a fast and easy to use file manager\n"
                               "for the Expidus Desktop Environment."));
}



static void
lunar_window_action_show_hidden (LunarWindow *window)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  window->show_hidden = !window->show_hidden;
  gtk_container_foreach (GTK_CONTAINER (window->notebook_selected), (GtkCallback) (void (*)(void)) lunar_view_set_show_hidden, GINT_TO_POINTER (window->show_hidden));

  if (G_LIKELY (window->sidepane != NULL))
    lunar_side_pane_set_show_hidden (LUNAR_SIDE_PANE (window->sidepane), window->show_hidden);

  g_object_set (G_OBJECT (window->preferences), "last-show-hidden", window->show_hidden, NULL);
}



static void
lunar_window_action_open_file_menu (LunarWindow *window)
{
  gboolean  ret;
  GList    *children;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  children = gtk_container_get_children (GTK_CONTAINER (window->menubar));
  g_signal_emit_by_name (children->data, "button-press-event", NULL, &ret);
  g_list_free (children);
  gtk_menu_shell_select_first (GTK_MENU_SHELL (window->menubar), TRUE);
}



static void
lunar_window_current_directory_changed (LunarFile   *current_directory,
                                         LunarWindow *window)
{
  gboolean      show_full_path;
  gchar        *parse_name = NULL;
  const gchar  *name;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (LUNAR_IS_FILE (current_directory));
  _lunar_return_if_fail (window->current_directory == current_directory);

  /* get name of directory or full path */
  g_object_get (G_OBJECT (window->preferences), "misc-full-path-in-title", &show_full_path, NULL);
  if (G_UNLIKELY (show_full_path))
    name = parse_name = g_file_get_parse_name (lunar_file_get_file (current_directory));
  else
    name = lunar_file_get_display_name (current_directory);

  /* set window title */
  gtk_window_set_title (GTK_WINDOW (window), name);
  g_free (parse_name);

  /* set window icon */
  lunar_window_update_window_icon (window);
}



static void
lunar_window_menu_item_selected (LunarWindow *window,
                                  GtkWidget    *menu_item)
{
  gchar *tooltip;
  gint   id;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* we can only display tooltips if we have a statusbar */
  if (G_LIKELY (window->statusbar != NULL))
    {
      tooltip = gtk_widget_get_tooltip_text (menu_item);
      if (G_LIKELY (tooltip != NULL))
        {
          /* push to the statusbar */
          id = gtk_statusbar_get_context_id (GTK_STATUSBAR (window->statusbar), "Menu tooltip");
          gtk_statusbar_push (GTK_STATUSBAR (window->statusbar), id, tooltip);
          g_free (tooltip);
        }
    }
}



static void
lunar_window_menu_item_deselected (LunarWindow *window,
                                    GtkWidget    *menu_item)
{
  gint id;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* we can only undisplay tooltips if we have a statusbar */
  if (G_LIKELY (window->statusbar != NULL))
    {
      /* drop the last tooltip from the statusbar */
      id = gtk_statusbar_get_context_id (GTK_STATUSBAR (window->statusbar), "Menu tooltip");
      gtk_statusbar_pop (GTK_STATUSBAR (window->statusbar), id);
    }
}



static void
lunar_window_notify_loading (LunarView   *view,
                              GParamSpec   *pspec,
                              LunarWindow *window)
{
  GdkCursor *cursor;

  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  if (gtk_widget_get_realized (GTK_WIDGET (window))
      && window->view == GTK_WIDGET (view))
    {
      /* setup the proper cursor */
      if (lunar_view_get_loading (view))
        {
          cursor = gdk_cursor_new_for_display (gtk_widget_get_display (GTK_WIDGET (view)), GDK_WATCH);
          gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (window)), cursor);
          g_object_unref (cursor);
        }
      else
        {
          gdk_window_set_cursor (gtk_widget_get_window (GTK_WIDGET (window)), NULL);
        }
    }
}



static void
lunar_window_device_pre_unmount (LunarDeviceMonitor *device_monitor,
                                  LunarDevice        *device,
                                  GFile               *root_file,
                                  LunarWindow        *window)
{
  _lunar_return_if_fail (LUNAR_IS_DEVICE_MONITOR (device_monitor));
  _lunar_return_if_fail (window->device_monitor == device_monitor);
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (G_IS_FILE (root_file));
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* nothing to do if we don't have a current directory */
  if (G_UNLIKELY (window->current_directory == NULL))
    return;

  /* check if the file is the current directory or an ancestor of the current directory */
  if (g_file_equal (lunar_file_get_file (window->current_directory), root_file)
      || lunar_file_is_gfile_ancestor (window->current_directory, root_file))
    {
      /* change to the home folder */
      lunar_window_action_open_home (window);
    }
}



static void
lunar_window_device_changed (LunarDeviceMonitor *device_monitor,
                              LunarDevice        *device,
                              LunarWindow        *window)
{
  GFile *root_file;

  _lunar_return_if_fail (LUNAR_IS_DEVICE_MONITOR (device_monitor));
  _lunar_return_if_fail (window->device_monitor == device_monitor);
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  if (lunar_device_is_mounted (device))
    return;

  root_file = lunar_device_get_root (device);
  if (root_file != NULL)
    {
      lunar_window_device_pre_unmount (device_monitor, device, root_file, window);
      g_object_unref (root_file);
    }
}



static gboolean
lunar_window_save_paned (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  g_object_set (G_OBJECT (window->preferences), "last-separator-position",
                gtk_paned_get_position (GTK_PANED (window->paned)), NULL);

  /* for button release event */
  return FALSE;
}



static gboolean
lunar_window_save_geometry_timer (gpointer user_data)
{
  GdkWindowState state;
  LunarWindow  *window = LUNAR_WINDOW (user_data);
  gboolean       remember_geometry;
  gint           width;
  gint           height;

LUNAR_THREADS_ENTER

  /* check if we should remember the window geometry */
  g_object_get (G_OBJECT (window->preferences), "misc-remember-geometry", &remember_geometry, NULL);
  if (G_LIKELY (remember_geometry))
    {
      /* check if the window is still visible */
      if (gtk_widget_get_visible (GTK_WIDGET (window)))
        {
          /* determine the current state of the window */
          state = gdk_window_get_state (gtk_widget_get_window (GTK_WIDGET (window)));

          /* don't save geometry for maximized or fullscreen windows */
          if ((state & (GDK_WINDOW_STATE_MAXIMIZED | GDK_WINDOW_STATE_FULLSCREEN)) == 0)
            {
              /* determine the current width/height of the window... */
              gtk_window_get_size (GTK_WINDOW (window), &width, &height);

              /* ...and remember them as default for new windows */
              g_object_set (G_OBJECT (window->preferences), "last-window-width", width, "last-window-height", height,
                            "last-window-maximized", FALSE, NULL);
            }
          else
            {
              /* only store that the window is full screen */
              g_object_set (G_OBJECT (window->preferences), "last-window-maximized", TRUE, NULL);
            }
        }
    }

LUNAR_THREADS_LEAVE

  return FALSE;
}



static void
lunar_window_save_geometry_timer_destroy (gpointer user_data)
{
  LUNAR_WINDOW (user_data)->save_geometry_timer_id = 0;
}



/**
 * lunar_window_set_zoom_level:
 * @window     : a #LunarWindow instance.
 * @zoom_level : the new zoom level for @window.
 *
 * Sets the zoom level for @window to @zoom_level.
 **/
void
lunar_window_set_zoom_level (LunarWindow   *window,
                              LunarZoomLevel zoom_level)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (zoom_level < LUNAR_ZOOM_N_LEVELS);

  /* check if we have a new zoom level */
  if (G_LIKELY (window->zoom_level != zoom_level))
    {
      /* remember the new zoom level */
      window->zoom_level = zoom_level;

      /* notify listeners */
      g_object_notify (G_OBJECT (window), "zoom-level");
    }
}



/**
 * lunar_window_set_directory_specific_settings:
 * @window                      : a #LunarWindow instance.
 * @directory_specific_settings : whether to use directory specific settings in @window.
 *
 * Toggles the use of directory specific settings in @window according to @directory_specific_settings.
 **/
void
lunar_window_set_directory_specific_settings (LunarWindow *window,
                                               gboolean      directory_specific_settings)
{
  GList      *tabs, *lp;
  LunarFile *directory;
  GType       view_type;
  gchar      *type_name;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  /* reset to the default view type if we are turning directory specific settings off */
  if (!directory_specific_settings && window->directory_specific_settings)
    {
      /* determine the default view type */
      g_object_get (G_OBJECT (window->preferences), "default-view", &type_name, NULL);
      view_type = g_type_from_name (type_name);
      g_free (type_name);

      /* set the last view type */
      if (!g_type_is_a (view_type, G_TYPE_NONE) && !g_type_is_a (view_type, G_TYPE_INVALID))
        g_object_set (G_OBJECT (window->preferences), "last-view", g_type_name (view_type), NULL);
    }

  /* save the setting */
  window->directory_specific_settings = directory_specific_settings;

  /* get all of the window's tabs */
  tabs = gtk_container_get_children (GTK_CONTAINER (window->notebook_selected));

  /* replace each tab with a tab of the correct view type */
  for (lp = tabs; lp != NULL; lp = lp->next)
    {
      if (!LUNAR_IS_STANDARD_VIEW (lp->data))
        continue;

      directory = lunar_navigator_get_current_directory (lp->data);

      if (!LUNAR_IS_FILE (directory))
        continue;

      /* find the correct view type for the new view */
      view_type = lunar_window_view_type_for_directory (window, directory);

      /* replace the old view with a new one */
      lunar_window_replace_view (window, lp->data, view_type);
    }

  g_list_free (tabs);
}



/**
 * lunar_window_get_current_directory:
 * @window : a #LunarWindow instance.
 *
 * Queries the #LunarFile instance, which represents the directory
 * currently displayed within @window. %NULL is returned if @window
 * is not currently associated with any directory.
 *
 * Return value: the directory currently displayed within @window or %NULL.
 **/
LunarFile*
lunar_window_get_current_directory (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), NULL);
  return window->current_directory;
}



/**
 * lunar_window_set_current_directory:
 * @window            : a #LunarWindow instance.
 * @current_directory : the new directory or %NULL.
 **/
void
lunar_window_set_current_directory (LunarWindow *window,
                                     LunarFile   *current_directory)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (current_directory == NULL || LUNAR_IS_FILE (current_directory));

  /* check if we already display the requested directory */
  if (G_UNLIKELY (window->current_directory == current_directory))
    return;

  /* disconnect from the previously active directory */
  if (G_LIKELY (window->current_directory != NULL))
    {
      /* disconnect signals and release reference */
      g_signal_handlers_disconnect_by_func (G_OBJECT (window->current_directory), lunar_window_current_directory_changed, window);
      g_object_unref (G_OBJECT (window->current_directory));
    }

  /* connect to the new directory */
  if (G_LIKELY (current_directory != NULL))
    {
      GType  type;
      gchar *type_name;
      gint   num_pages;

      /* take a reference on the file */
      g_object_ref (G_OBJECT (current_directory));

      num_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected));

      /* if the window is new, get the default-view type and set it as the last-view type (if it is a valid type)
       * so that it will be used as the initial view type for directories with no saved directory specific view type */
      if (num_pages == 0)
        {
          /* determine the default view type */
          g_object_get (G_OBJECT (window->preferences), "default-view", &type_name, NULL);
          type = g_type_from_name (type_name);
          g_free (type_name);

          /* set the last view type to the default view type if there is a default view type */
          if (!g_type_is_a (type, G_TYPE_NONE) && !g_type_is_a (type, G_TYPE_INVALID))
            g_object_set (G_OBJECT (window->preferences), "last-view", g_type_name (type), NULL);
        }

      type = lunar_window_view_type_for_directory (window, current_directory);

      if (num_pages == 0) /* create a new view if the window is new */
        {
          window->current_directory = current_directory;
          lunar_window_replace_view (window, window->view, type);
        }
      else /* change the view type if necessary, and set the current directory */
        {
          if (window->view != NULL && window->view_type != type)
            lunar_window_replace_view (window, window->view, type);

          window->current_directory = current_directory;
        }

      /* connect the "changed"/"destroy" signals */
      g_signal_connect (G_OBJECT (current_directory), "changed", G_CALLBACK (lunar_window_current_directory_changed), window);

      /* update window icon and title */
      lunar_window_current_directory_changed (current_directory, window);

      if (G_LIKELY (window->view != NULL))
        {
          /* grab the focus to the main view */
          gtk_widget_grab_focus (window->view);
        }

      lunar_window_history_changed (window);
      gtk_widget_set_sensitive (window->location_toolbar_item_parent, !lunar_g_file_is_root (lunar_file_get_file (current_directory)));
    }

  /* tell everybody that we have a new "current-directory",
   * we do this first so other widgets display the new
   * state already while the folder view is loading.
   */
  g_object_notify (G_OBJECT (window), "current-directory");
}



static void
lunar_window_set_current_directory_gfile (LunarWindow *window,
                                           GFile        *current_directory)
{
  LunarFile *lunar_file;

  /* remote files possibly need to be poked first */
  if (g_file_has_uri_scheme (current_directory, "file"))
    {
      lunar_file = lunar_file_get (current_directory, NULL);
      lunar_window_set_current_directory (LUNAR_WINDOW (window), lunar_file);
      g_object_unref (lunar_file);
    }
  else
    {
      lunar_browser_poke_location (LUNAR_BROWSER (window), current_directory, LUNAR_WINDOW (window),
                                    lunar_window_poke_location_finish, NULL);
    }
}



/**
 * lunar_window_scroll_to_file:
 * @window      : a #LunarWindow instance.
 * @file        : a #LunarFile.
 * @select_file : if %TRUE the @file will also be selected.
 * @use_align   : %TRUE to use the alignment arguments.
 * @row_align   : the vertical alignment.
 * @col_align   : the horizontal alignment.
 *
 * Tells the @window to scroll to the @file
 * in the current view.
 **/
void
lunar_window_scroll_to_file (LunarWindow *window,
                              LunarFile   *file,
                              gboolean      select_file,
                              gboolean      use_align,
                              gfloat        row_align,
                              gfloat        col_align)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  /* verify that we have a valid view */
  if (G_LIKELY (window->view != NULL))
    lunar_view_scroll_to_file (LUNAR_VIEW (window->view), file, select_file, use_align, row_align, col_align);
}



gchar **
lunar_window_get_directories (LunarWindow *window,
                               gint         *active_page)
{
  gint         n;
  gint         n_pages;
  gchar      **uris;
  GtkWidget   *view;
  LunarFile  *directory;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), NULL);

  n_pages = gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected));
  if (G_UNLIKELY (n_pages == 0))
    return NULL;

  /* create array of uris */
  uris = g_new0 (gchar *, n_pages + 1);
  for (n = 0; n < n_pages; n++)
    {
      /* get the view */
      view = gtk_notebook_get_nth_page (GTK_NOTEBOOK (window->notebook_selected), n);
      _lunar_return_val_if_fail (LUNAR_IS_NAVIGATOR (view), FALSE);

      /* get the directory of the view */
      directory = lunar_navigator_get_current_directory (LUNAR_NAVIGATOR (view));
      _lunar_return_val_if_fail (LUNAR_IS_FILE (directory), FALSE);

      /* add to array */
      uris[n] = lunar_file_dup_uri (directory);
    }

  /* selected tab */
  if (active_page != NULL)
    *active_page = gtk_notebook_get_current_page (GTK_NOTEBOOK (window->notebook_selected));

  return uris;
}



gboolean
lunar_window_set_directories (LunarWindow   *window,
                               gchar         **uris,
                               gint            active_page)
{
  LunarFile *directory;
  guint       n;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);
  _lunar_return_val_if_fail (uris != NULL, FALSE);

  for (n = 0; uris[n] != NULL; n++)
    {
      /* check if the string looks like an uri */
      if (!endo_str_looks_like_an_uri (uris[n]))
        continue;

      /* get the file for the uri */
      directory = lunar_file_get_for_uri (uris[n], NULL);
      if (G_UNLIKELY (directory == NULL))
        continue;

      /* open the directory in a new notebook */
      if (lunar_file_is_directory (directory))
        {
          if (gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected)) == 0)
            lunar_window_set_current_directory (window, directory);
          else
            lunar_window_notebook_open_new_tab (window, directory);
        }

      g_object_unref (G_OBJECT (directory));
    }

  /* select the page */
  gtk_notebook_set_current_page (GTK_NOTEBOOK (window->notebook_selected), active_page);

  /* we succeeded if new pages have been opened */
  return gtk_notebook_get_n_pages (GTK_NOTEBOOK (window->notebook_selected)) > 0;
}



/**
 * lunar_window_get_action_entry:
 * @window  : Instance of a  #LunarWindow
 * @action  : #LunarWindowAction for which the #ExpidusGtkActionEntry is requested
 *
 * returns a reference to the requested #ExpidusGtkActionEntry
 *
 * Return value: (transfer none): The reference to the #ExpidusGtkActionEntry
 **/
const ExpidusGtkActionEntry*
lunar_window_get_action_entry (LunarWindow       *window,
                                LunarWindowAction  action)
{
  return get_action_entry (action);
}



/**
 * lunar_window_append_menu_item:
 * @window  : Instance of a  #LunarWindow
 * @menu    : #GtkMenuShell to which the item should be added
 * @action  : #LunarWindowAction to select which item should be added
 *
 * Adds the selected, widget specific #GtkMenuItem to the passed #GtkMenuShell
 **/
void
lunar_window_append_menu_item (LunarWindow       *window,
                                GtkMenuShell       *menu,
                                LunarWindowAction  action)
{
  GtkWidget *item;

  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));

  item = expidus_gtk_menu_item_new_from_action_entry (get_action_entry (action), G_OBJECT (window), menu);

  if (action == LUNAR_WINDOW_ACTION_ZOOM_IN)
    gtk_widget_set_sensitive (item, G_LIKELY (window->zoom_level < LUNAR_ZOOM_N_LEVELS - 1));
  if (action == LUNAR_WINDOW_ACTION_ZOOM_OUT)
    gtk_widget_set_sensitive (item, G_LIKELY (window->zoom_level > 0));
}



/**
 * lunar_window_get_launcher:
 * @window : a #LunarWindow instance.
 *
 * Return value: (transfer none): The single #LunarLauncher of this #LunarWindow
 **/
LunarLauncher*
lunar_window_get_launcher (LunarWindow *window)
{
  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), NULL);

  return window->launcher;
}



static void
lunar_window_redirect_menu_tooltips_to_statusbar_recursive (GtkWidget    *menu_item,
                                                             LunarWindow *window)
{
  GtkWidget  *submenu;

  if (GTK_IS_MENU_ITEM (menu_item))
    {
      submenu = gtk_menu_item_get_submenu (GTK_MENU_ITEM (menu_item));
      if (submenu != NULL)
          gtk_container_foreach (GTK_CONTAINER (submenu), (GtkCallback) (void (*)(void)) lunar_window_redirect_menu_tooltips_to_statusbar_recursive, window);

      /* this disables to show the tooltip on hover */
      gtk_widget_set_has_tooltip (menu_item, FALSE);

      /* These method will put the tooltip on the statusbar */
      g_signal_connect_swapped (G_OBJECT (menu_item), "select", G_CALLBACK (lunar_window_menu_item_selected), window);
      g_signal_connect_swapped (G_OBJECT (menu_item), "deselect", G_CALLBACK (lunar_window_menu_item_deselected), window);
    }
}



/**
 * lunar_window_redirect_menu_tooltips_to_statusbar:
 * @window : a #LunarWindow instance.
 * @menu   : #GtkMenu for which all tooltips should be shown in the statusbar
 *
 * All tooltips of the provided #GtkMenu and any submenu will not be shown directly any more.
 * Instead they will be shown in the status bar of the passed #LunarWindow
 **/
void
lunar_window_redirect_menu_tooltips_to_statusbar (LunarWindow *window, GtkMenu *menu)
{
  _lunar_return_if_fail (LUNAR_IS_WINDOW (window));
  _lunar_return_if_fail (GTK_IS_MENU (menu));

  gtk_container_foreach (GTK_CONTAINER (menu), (GtkCallback) (void (*)(void)) lunar_window_redirect_menu_tooltips_to_statusbar_recursive, window);
}



static gboolean
lunar_window_button_press_event (GtkWidget      *view,
                                  GdkEventButton *event,
                                  LunarWindow   *window)
{
  const ExpidusGtkActionEntry* action_entry;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), FALSE);

  if (event->type == GDK_BUTTON_PRESS)
    {
      if (G_UNLIKELY (event->button == 8))
        {
          action_entry = get_action_entry (LUNAR_WINDOW_ACTION_BACK);
          ((void(*)(GtkWindow*))action_entry->callback)(GTK_WINDOW (window));
          return GDK_EVENT_STOP;
        }
      if (G_UNLIKELY (event->button == 9))
        {
          action_entry = get_action_entry (LUNAR_WINDOW_ACTION_FORWARD);
          ((void(*)(GtkWindow*))action_entry->callback)(GTK_WINDOW (window));
          return GDK_EVENT_STOP;
        }
    }

  return GDK_EVENT_PROPAGATE;
}



static gboolean
lunar_window_history_clicked (GtkWidget      *button,
                               GdkEventButton *event,
                               GtkWidget      *data)
{
  LunarHistory *history;
  LunarWindow  *window;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (data), FALSE);

  window = LUNAR_WINDOW (data);

  if (event->button == 3)
    {
      history = lunar_standard_view_get_history (LUNAR_STANDARD_VIEW (window->view));

      if (button == window->location_toolbar_item_back)
        lunar_history_show_menu (history, LUNAR_HISTORY_MENU_BACK, button);
      else if (button == window->location_toolbar_item_forward)
        lunar_history_show_menu (history, LUNAR_HISTORY_MENU_FORWARD, button);
      else
        g_warning ("This button is not able to spawn a history menu");
    }

  return FALSE;
}



/**
 * lunar_window_view_type_for_directory:
 * @window      : a #LunarWindow instance.
 * @directory   : #LunarFile representing the directory
 *
 * Return value: the #GType representing the view type which
 * @window would use to display @directory.
 **/
GType
lunar_window_view_type_for_directory (LunarWindow *window,
                                       LunarFile   *directory)
{
  GType  type = G_TYPE_NONE;
  gchar *type_name;

  _lunar_return_val_if_fail (LUNAR_IS_WINDOW (window), G_TYPE_NONE);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (directory), G_TYPE_NONE);

  /* if the  directory has a saved view type and directory specific view types are enabled, we use it */
  if (window->directory_specific_settings)
    {
      const gchar *dir_spec_type_name;

      dir_spec_type_name = lunar_file_get_metadata_setting (directory, "view-type");
      if (dir_spec_type_name != NULL)
        type = g_type_from_name (dir_spec_type_name);
    }

  /* if there is no saved view type for the directory or directory specific view types are not enabled,
   * we use the last view type */
  if (g_type_is_a (type, G_TYPE_NONE) || g_type_is_a (type, G_TYPE_INVALID))
    {
      /* determine the last view type */
      g_object_get (G_OBJECT (window->preferences), "last-view", &type_name, NULL);
      type = g_type_from_name (type_name);
      g_free (type_name);
    }

  /* fallback view type, in case nothing was set */
  if (g_type_is_a (type, G_TYPE_NONE) || g_type_is_a (type, G_TYPE_INVALID))
    type = LUNAR_TYPE_ICON_VIEW;

  return type;
}
