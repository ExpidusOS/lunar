/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
 * Copyright (c) 2012      Nick Schermer <nick@expidus.org>
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

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif
#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif
#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include <glib.h>
#include <glib/gstdio.h>

#include <lunar/lunar-file.h>
#include <lunar/lunar-shortcuts-model.h>
#include <lunar/lunar-device-monitor.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-util.h>
#include <lunar/lunar-private.h>

#define SPINNER_CYCLE_DURATION 1000
#define SPINNER_NUM_STEPS      12



#define LUNAR_SHORTCUT(obj) ((LunarShortcut *) (obj))



typedef struct _LunarShortcut LunarShortcut;



enum
{
  PROP_0,
  PROP_HIDDEN_BOOKMARKS,
  PROP_FILE_SIZE_BINARY
};



static void               lunar_shortcuts_model_tree_model_init    (GtkTreeModelIface         *iface);
static void               lunar_shortcuts_model_drag_source_init   (GtkTreeDragSourceIface    *iface);
static void               lunar_shortcuts_model_get_property       (GObject                   *object,
                                                                     guint                      prop_id,
                                                                     GValue                    *value,
                                                                     GParamSpec                *pspec);
static void               lunar_shortcuts_model_set_property       (GObject                   *object,
                                                                     guint                      prop_id,
                                                                     const GValue              *value,
                                                                     GParamSpec                *pspec);
static void               lunar_shortcuts_model_finalize           (GObject                   *object);
static GtkTreeModelFlags  lunar_shortcuts_model_get_flags          (GtkTreeModel              *tree_model);
static gint               lunar_shortcuts_model_get_n_columns      (GtkTreeModel              *tree_model);
static GType              lunar_shortcuts_model_get_column_type    (GtkTreeModel              *tree_model,
                                                                     gint                       idx);
static gboolean           lunar_shortcuts_model_get_iter           (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreePath               *path);
static GtkTreePath       *lunar_shortcuts_model_get_path           (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static void               lunar_shortcuts_model_get_value          (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     gint                       column,
                                                                     GValue                    *value);
static gboolean           lunar_shortcuts_model_iter_next          (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static gboolean           lunar_shortcuts_model_iter_children      (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreeIter               *parent);
static gboolean           lunar_shortcuts_model_iter_has_child     (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static gint               lunar_shortcuts_model_iter_n_children    (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter);
static gboolean           lunar_shortcuts_model_iter_nth_child     (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreeIter               *parent,
                                                                     gint                       n);
static gboolean           lunar_shortcuts_model_iter_parent        (GtkTreeModel              *tree_model,
                                                                     GtkTreeIter               *iter,
                                                                     GtkTreeIter               *child);
static gboolean           lunar_shortcuts_model_row_draggable      (GtkTreeDragSource         *source,
                                                                     GtkTreePath               *path);
static gboolean           lunar_shortcuts_model_drag_data_get      (GtkTreeDragSource         *source,
                                                                     GtkTreePath               *path,
                                                                     GtkSelectionData          *selection_data);
static gboolean           lunar_shortcuts_model_drag_data_delete   (GtkTreeDragSource         *source,
                                                                     GtkTreePath               *path);
static void               lunar_shortcuts_model_header_visibility  (LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_shortcut_devices   (LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_shortcut_places    (LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_shortcut_network   (LunarShortcutsModel      *model);
static gboolean           lunar_shortcuts_model_get_hidden         (LunarShortcutsModel      *model,
                                                                     LunarShortcut            *shortcut);
static void               lunar_shortcuts_model_add_shortcut       (LunarShortcutsModel      *model,
                                                                     LunarShortcut            *shortcut);
static void               lunar_shortcuts_model_remove_shortcut    (LunarShortcutsModel      *model,
                                                                     LunarShortcut            *shortcut);
static gboolean           lunar_shortcuts_model_load               (gpointer                   data);
static void               lunar_shortcuts_model_save               (LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_monitor            (GFileMonitor              *monitor,
                                                                     GFile                     *file,
                                                                     GFile                     *other_file,
                                                                     GFileMonitorEvent          event_type,
                                                                     gpointer                   user_data);
static void               lunar_shortcuts_model_file_changed       (LunarFile                *file,
                                                                     LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_file_destroy       (LunarFile                *file,
                                                                     LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_device_added       (LunarDeviceMonitor       *device_monitor,
                                                                     LunarDevice              *device,
                                                                     LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_device_removed     (LunarDeviceMonitor       *device_monitor,
                                                                     LunarDevice              *device,
                                                                     LunarShortcutsModel      *model);
static void               lunar_shortcuts_model_device_changed     (LunarDeviceMonitor       *device_monitor,
                                                                     LunarDevice              *device,
                                                                     LunarShortcutsModel      *model);

static void               lunar_shortcut_free                      (LunarShortcut            *shortcut,
                                                                     LunarShortcutsModel      *model);



struct _LunarShortcutsModelClass
{
  GObjectClass __parent__;
};

struct _LunarShortcutsModel
{
  GObject         __parent__;

  /* the model stamp is only used when debugging is
   * enabled, to make sure we don't accept iterators
   * generated by another model.
   */
#ifndef NDEBUG
  gint                  stamp;
#endif

  GList                *shortcuts;

  LunarPreferences    *preferences;
  gchar               **hidden_bookmarks;
  gboolean              file_size_binary;

  LunarDeviceMonitor  *device_monitor;

  gint64                bookmarks_time;
  GFile                *bookmarks_file;
  GFileMonitor         *bookmarks_monitor;
  guint                 bookmarks_idle_id;

  guint                 busy_timeout_id;
};

struct _LunarShortcut
{
  LunarShortcutGroup  group;

  gchar               *name;
  GIcon               *gicon;
  gchar               *tooltip;
  gint                 sort_id;

  guint                busy : 1;
  guint                busy_pulse;

  GFile               *location;
  LunarFile          *file;
  LunarDevice        *device;

  guint                hidden : 1;
};



G_DEFINE_TYPE_WITH_CODE (LunarShortcutsModel, lunar_shortcuts_model, G_TYPE_OBJECT,
    G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_MODEL, lunar_shortcuts_model_tree_model_init)
    G_IMPLEMENT_INTERFACE (GTK_TYPE_TREE_DRAG_SOURCE, lunar_shortcuts_model_drag_source_init))



static void
lunar_shortcuts_model_class_init (LunarShortcutsModelClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_shortcuts_model_finalize;
  gobject_class->get_property = lunar_shortcuts_model_get_property;
  gobject_class->set_property = lunar_shortcuts_model_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_HIDDEN_BOOKMARKS,
                                   g_param_spec_boxed ("hidden-bookmarks",
                                                       NULL,
                                                       NULL,
                                                       G_TYPE_STRV,
                                                       ENDO_PARAM_READWRITE));

  /**
   * LunarPropertiesDialog:file_size_binary:
   *
   * Whether the file size should be shown in binary or decimal.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE_SIZE_BINARY,
                                   g_param_spec_boolean ("file-size-binary",
                                                         "FileSizeBinary",
                                                         NULL,
                                                         TRUE,
                                                         ENDO_PARAM_READWRITE));
}



static void
lunar_shortcuts_model_tree_model_init (GtkTreeModelIface *iface)
{
  iface->get_flags = lunar_shortcuts_model_get_flags;
  iface->get_n_columns = lunar_shortcuts_model_get_n_columns;
  iface->get_column_type = lunar_shortcuts_model_get_column_type;
  iface->get_iter = lunar_shortcuts_model_get_iter;
  iface->get_path = lunar_shortcuts_model_get_path;
  iface->get_value = lunar_shortcuts_model_get_value;
  iface->iter_next = lunar_shortcuts_model_iter_next;
  iface->iter_children = lunar_shortcuts_model_iter_children;
  iface->iter_has_child = lunar_shortcuts_model_iter_has_child;
  iface->iter_n_children = lunar_shortcuts_model_iter_n_children;
  iface->iter_nth_child = lunar_shortcuts_model_iter_nth_child;
  iface->iter_parent = lunar_shortcuts_model_iter_parent;
}



static void
lunar_shortcuts_model_drag_source_init (GtkTreeDragSourceIface *iface)
{
  iface->row_draggable = lunar_shortcuts_model_row_draggable;
  iface->drag_data_get = lunar_shortcuts_model_drag_data_get;
  iface->drag_data_delete = lunar_shortcuts_model_drag_data_delete;
}



static void
lunar_shortcuts_model_init (LunarShortcutsModel *model)
{
#ifndef NDEBUG
  model->stamp = g_random_int ();
#endif

  /* hidden bookmarks */
  model->preferences = lunar_preferences_get ();
  endo_binding_new (G_OBJECT (model->preferences), "hidden-bookmarks",
                   G_OBJECT (model), "hidden-bookmarks");

  /* binary file size */
  endo_binding_new (G_OBJECT (model->preferences), "misc-file-size-binary",
                   G_OBJECT (model), "file-size-binary");

  /* load volumes */
  lunar_shortcuts_model_shortcut_devices (model);

  /* add network */
  lunar_shortcuts_model_shortcut_network (model);

  /* add bookmarks */
  lunar_shortcuts_model_shortcut_places (model);
}



static void
lunar_shortcuts_model_finalize (GObject *object)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (object);

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  /* stop the busy timeout */
  if (model->busy_timeout_id != 0)
    g_source_remove (model->busy_timeout_id);

  /* stop bookmark load idle */
  if (model->bookmarks_idle_id != 0)
    g_source_remove (model->bookmarks_idle_id);

  /* free all shortcuts */
  g_list_foreach (model->shortcuts, (GFunc) (void (*)(void)) lunar_shortcut_free, model);
  g_list_free (model->shortcuts);

  /* disconnect from the preferences */
  g_object_unref (model->preferences);

  /* free hidden list */
  g_strfreev (model->hidden_bookmarks);

  /* detach from the file monitor */
  if (model->bookmarks_monitor != NULL)
    {
      g_file_monitor_cancel (model->bookmarks_monitor);
      g_object_unref (model->bookmarks_monitor);
    }

  if (model->bookmarks_file != NULL)
    g_object_unref (model->bookmarks_file);

  /* unlink from the device monitor */
  g_signal_handlers_disconnect_matched (model->device_monitor, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, model);
  g_object_unref (model->device_monitor);

  (*G_OBJECT_CLASS (lunar_shortcuts_model_parent_class)->finalize) (object);
}



static void
lunar_shortcuts_model_get_property (GObject    *object,
                                     guint       prop_id,
                                     GValue     *value,
                                     GParamSpec *pspec)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (object);

  switch (prop_id)
    {
    case PROP_HIDDEN_BOOKMARKS:
      g_value_set_boxed (value, model->hidden_bookmarks);
      break;

    case PROP_FILE_SIZE_BINARY:
      g_value_set_boolean (value, model->file_size_binary);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_shortcuts_model_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (object);
  GList                *lp;
  LunarShortcut       *shortcut;
  gboolean              hidden;
  GtkTreeIter           iter;
  GtkTreePath          *path;
  guint                 idx;

  switch (prop_id)
    {
    case PROP_HIDDEN_BOOKMARKS:
      g_strfreev (model->hidden_bookmarks);
      model->hidden_bookmarks = g_value_dup_boxed (value);

      /* update shortcuts */
      for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
        {
          shortcut = lp->data;

          /* skip devices and headers*/
          if (shortcut->device != NULL
              || (shortcut->file == NULL && shortcut->location == NULL))
            continue;

          /* update state if required */
          hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
          if (shortcut->hidden != hidden)
            {
              shortcut->hidden = hidden;

              GTK_TREE_ITER_INIT (iter, model->stamp, lp);

              path = gtk_tree_path_new_from_indices (idx, -1);
              gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
              gtk_tree_path_free (path);
            }
        }

      /* update header visibility */
      lunar_shortcuts_model_header_visibility (model);
      break;

    case PROP_FILE_SIZE_BINARY:
      model->file_size_binary = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static GtkTreeModelFlags
lunar_shortcuts_model_get_flags (GtkTreeModel *tree_model)
{
  return GTK_TREE_MODEL_ITERS_PERSIST | GTK_TREE_MODEL_LIST_ONLY;
}



static gint
lunar_shortcuts_model_get_n_columns (GtkTreeModel *tree_model)
{
  return LUNAR_SHORTCUTS_MODEL_N_COLUMNS;
}



static GType
lunar_shortcuts_model_get_column_type (GtkTreeModel *tree_model,
                                        gint          idx)
{
  switch (idx)
    {
    case LUNAR_SHORTCUTS_MODEL_COLUMN_IS_HEADER:
      return G_TYPE_BOOLEAN;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_IS_ITEM:
      return G_TYPE_BOOLEAN;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_VISIBLE:
      return G_TYPE_BOOLEAN;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_NAME:
      return G_TYPE_STRING;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_TOOLTIP:
      return G_TYPE_STRING;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_FILE:
      return LUNAR_TYPE_FILE;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_LOCATION:
      return G_TYPE_FILE;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_GICON:
      return G_TYPE_ICON;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_DEVICE:
      return LUNAR_TYPE_DEVICE;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_MUTABLE:
      return G_TYPE_BOOLEAN;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_CAN_EJECT:
      return G_TYPE_BOOLEAN;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_GROUP:
      return G_TYPE_UINT;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_BUSY:
      return G_TYPE_BOOLEAN;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_BUSY_PULSE:
      return G_TYPE_UINT;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_HIDDEN:
      return G_TYPE_BOOLEAN;
    }

  _lunar_assert_not_reached ();
  return G_TYPE_INVALID;
}



static gboolean
lunar_shortcuts_model_get_iter (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter,
                                 GtkTreePath  *path)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (tree_model);
  GList                 *lp;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);
  _lunar_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

  /* determine the list item for the path */
  lp = g_list_nth (model->shortcuts, gtk_tree_path_get_indices (path)[0]);
  if (G_LIKELY (lp != NULL))
    {
      GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
      return TRUE;
    }

  return FALSE;
}



static GtkTreePath*
lunar_shortcuts_model_get_path (GtkTreeModel *tree_model,
                                 GtkTreeIter  *iter)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (tree_model);
  gint                  idx;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), NULL);
  _lunar_return_val_if_fail (iter->stamp == model->stamp, NULL);

  /* lookup the list item in the shortcuts list */
  idx = g_list_position (model->shortcuts, iter->user_data);
  if (G_LIKELY (idx >= 0))
    return gtk_tree_path_new_from_indices (idx, -1);

  return NULL;
}



static void
lunar_shortcuts_model_get_value (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter,
                                  gint          column,
                                  GValue       *value)
{
  LunarShortcut *shortcut;
  GFile          *file;
  gboolean        can_eject;
  gboolean        file_size_binary;
  gchar          *disk_usage;
  gchar          *device_name;
  gchar          *device_id;
  gchar          *location;
  gchar          *tooltip;
  guint32         trash_items;
  gchar          *parse_name;

  _lunar_return_if_fail (iter->stamp == LUNAR_SHORTCUTS_MODEL (tree_model)->stamp);
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (tree_model));

  /* determine the shortcut for the list item */
  shortcut = LUNAR_SHORTCUT (((GList *) iter->user_data)->data);
  if (shortcut == NULL)
    return;

  switch (column)
    {
    case LUNAR_SHORTCUTS_MODEL_COLUMN_IS_HEADER:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, (shortcut->group & LUNAR_SHORTCUT_GROUP_HEADER) != 0);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_IS_ITEM:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, (shortcut->group & LUNAR_SHORTCUT_GROUP_HEADER) == 0);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_VISIBLE:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, !shortcut->hidden);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_NAME:
      g_value_init (value, G_TYPE_STRING);
      if (G_UNLIKELY (shortcut->device != NULL))
        g_value_take_string (value, lunar_device_get_name (shortcut->device));
      else if (shortcut->name != NULL)
        g_value_set_static_string (value, shortcut->name);
      else if (shortcut->file != NULL)
        g_value_set_static_string (value, lunar_file_get_display_name (shortcut->file));
      else if (shortcut->location != NULL)
        g_value_take_string (value, lunar_g_file_get_display_name_remote (shortcut->location));
      else
        g_value_set_static_string (value, "");
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_TOOLTIP:
      g_value_init (value, G_TYPE_STRING);
      if ((shortcut->group & LUNAR_SHORTCUT_GROUP_DEVICES) != 0)
        {
          /* get Unix device path */
          if (shortcut->device != NULL)
            device_id = lunar_device_get_identifier_unix (shortcut->device);
          else
            device_id = NULL;

          /* get device root directory */
          if (shortcut->device != NULL)
            file = lunar_device_get_root (shortcut->device);
          else if (shortcut->file != NULL)
            file = g_object_ref (lunar_file_get_file (shortcut->file));
          else
            file = NULL;

          if (file != NULL)
            {
              if (g_file_has_uri_scheme (file, "file") && lunar_g_file_is_root (file))
                location = g_strdup (_("Browse the file system"));
              else if (!g_file_has_uri_scheme (file, "file"))
                location = g_file_get_uri (file);
              else
                location = lunar_g_file_get_location (file);

              if (device_id != NULL)
                {
                  gchar *tmp = g_strdup_printf ("%s (%s)", location, device_id);
                  g_free (location);
                  g_free (device_id);
                  location = tmp;
                }

              file_size_binary = LUNAR_SHORTCUTS_MODEL (tree_model)->file_size_binary;
              disk_usage = lunar_g_file_get_free_space_string (file, file_size_binary);

              if (disk_usage != NULL)
                tooltip = g_strdup_printf ("%s\n%s", location, disk_usage);
              else
                tooltip = g_strdup_printf ("%s", location);

              g_value_take_string (value, tooltip);

              g_free (location);
              g_free (disk_usage);

              g_object_unref (file);
            }
          else
            {
              if (device_id != NULL)
                {
                  device_name = lunar_device_get_name (shortcut->device);
                  tooltip = g_strdup_printf (_("Mount and open %s (%s)"), device_name, device_id);
                  g_value_take_string (value, tooltip);
                  g_free (device_name);
                  g_free (device_id);
                }
            }
          break;
        }
      else if ((shortcut->group & LUNAR_SHORTCUT_GROUP_PLACES_TRASH) != 0)
        {
          trash_items = lunar_file_get_item_count (shortcut->file);
          if (trash_items == 0)
            {
              g_value_set_static_string (value, _("Trash is empty"));
            }
          else
            {
              tooltip = g_strdup_printf (ngettext ("Trash contains %d file",
                                                   "Trash contains %d files",
                                                   trash_items), trash_items);
              g_value_take_string (value, tooltip);
            }
          break;
        }
      else if (shortcut->tooltip == NULL)
        {
          if (shortcut->device != NULL)
            {
              file = lunar_device_get_root (shortcut->device);
              if (G_LIKELY (file != NULL))
                {
                  shortcut->tooltip = g_file_get_uri (file);
                  g_object_unref (file);
                }
            }
          else
            {
              if (shortcut->file != NULL)
                file = lunar_file_get_file (shortcut->file);
              else if (shortcut->location != NULL)
                file = shortcut->location;
              else
                file = NULL;

              if (G_LIKELY (file != NULL))
                {
                  parse_name = g_file_get_parse_name (file);
                  shortcut->tooltip = g_markup_escape_text (parse_name, -1);
                  g_free (parse_name);
                }
            }
        }

      g_value_set_static_string (value, shortcut->tooltip);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_FILE:
      g_value_init (value, LUNAR_TYPE_FILE);
      g_value_set_object (value, shortcut->file);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_GICON:
      g_value_init (value, G_TYPE_ICON);
      g_value_set_object (value, shortcut->gicon);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_DEVICE:
      g_value_init (value, LUNAR_TYPE_DEVICE);
      g_value_set_object (value, shortcut->device);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_LOCATION:
      g_value_init (value, G_TYPE_FILE);
      if (shortcut->location != NULL)
        g_value_set_object (value, shortcut->location);
      else if (shortcut->file != NULL)
        g_value_set_object (value, lunar_file_get_file (shortcut->file));
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_MUTABLE:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value,
                           shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_CAN_EJECT:
      if (shortcut->device != NULL)
        can_eject = lunar_device_can_eject (shortcut->device);
      else
        can_eject = FALSE;

      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, can_eject);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_GROUP:
      g_value_init (value, G_TYPE_UINT);
      g_value_set_uint (value, shortcut->group);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_BUSY:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, shortcut->busy);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_BUSY_PULSE:
      g_value_init (value, G_TYPE_UINT);
      g_value_set_uint (value, shortcut->busy_pulse);
      break;

    case LUNAR_SHORTCUTS_MODEL_COLUMN_HIDDEN:
      g_value_init (value, G_TYPE_BOOLEAN);
      g_value_set_boolean (value, FALSE);
      break;

    default:
      _lunar_assert_not_reached ();
    }
}



static gboolean
lunar_shortcuts_model_iter_next (GtkTreeModel *tree_model,
                                  GtkTreeIter  *iter)
{
  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (tree_model), FALSE);
  _lunar_return_val_if_fail (iter->stamp == LUNAR_SHORTCUTS_MODEL (tree_model)->stamp, FALSE);

  iter->user_data = g_list_next (iter->user_data);
  return (iter->user_data != NULL);
}



static gboolean
lunar_shortcuts_model_iter_children (GtkTreeModel *tree_model,
                                      GtkTreeIter  *iter,
                                      GtkTreeIter  *parent)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (tree_model);

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);

  if (G_LIKELY (parent == NULL && model->shortcuts != NULL))
    {
      GTK_TREE_ITER_INIT (*iter, model->stamp, model->shortcuts);
      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_shortcuts_model_iter_has_child (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter)
{
  return FALSE;
}



static gint
lunar_shortcuts_model_iter_n_children (GtkTreeModel *tree_model,
                                        GtkTreeIter  *iter)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (tree_model);

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), 0);

  return (iter == NULL) ? g_list_length (model->shortcuts) : 0;
}



static gboolean
lunar_shortcuts_model_iter_nth_child (GtkTreeModel *tree_model,
                                       GtkTreeIter  *iter,
                                       GtkTreeIter  *parent,
                                       gint          n)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (tree_model);

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);

  if (G_LIKELY (parent == NULL))
    {
      GTK_TREE_ITER_INIT (*iter, model->stamp, g_list_nth (model->shortcuts, n));
      return (iter->user_data != NULL);
    }

  return FALSE;
}



static gboolean
lunar_shortcuts_model_iter_parent (GtkTreeModel *tree_model,
                                    GtkTreeIter  *iter,
                                    GtkTreeIter  *child)
{
  return FALSE;
}



static gboolean
lunar_shortcuts_model_row_draggable (GtkTreeDragSource *source,
                                      GtkTreePath       *path)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (source);
  LunarShortcut       *shortcut;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);
  _lunar_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

  /* lookup the LunarShortcut for the path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);

  /* special shortcuts cannot be reordered */
  return (shortcut != NULL && shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS);
}



static gboolean
lunar_shortcuts_model_drag_data_get (GtkTreeDragSource *source,
                                      GtkTreePath       *path,
                                      GtkSelectionData  *selection_data)
{
  /* we simply return FALSE here, as the drag handling is done in
   * the LunarShortcutsView class.
   */
  return FALSE;
}



static gboolean
lunar_shortcuts_model_drag_data_delete (GtkTreeDragSource *source,
                                         GtkTreePath       *path)
{
  /* we simply return FALSE here, as this function can only be
   * called if the user is re-arranging shortcuts within the
   * model, which will be handle by the exchange method.
   */
  return FALSE;
}



static void
lunar_shortcuts_model_header_visibility (LunarShortcutsModel *model)
{
  GList          *lp;
  LunarShortcut *shortcut;
  LunarShortcut *header;
  guint           n_children = 0;
  GList          *lp_header = NULL;
  gint            idx_header = 0;
  gint            i;
  GtkTreePath    *path;
  GtkTreeIter     iter;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  for (lp = model->shortcuts, i = 0; lp != NULL; lp = lp->next, i++)
    {
      shortcut = lp->data;

      if ((shortcut->group & LUNAR_SHORTCUT_GROUP_HEADER) != 0)
        {
          if (lp_header != NULL)
            {
              update_header:

              header = lp_header->data;
              if (header->hidden != (n_children == 0))
                {
                  /* set new visibility */
                  header->hidden = (n_children == 0);

                  /* notify view */
                  path = gtk_tree_path_new_from_indices (idx_header, -1);
                  GTK_TREE_ITER_INIT (iter, model->stamp, lp_header);
                  gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
                  gtk_tree_path_free (path);
                }

              if (lp == NULL)
                return;
            }

          /* reset for new */
          idx_header = i;
          lp_header = lp;
          n_children = 0;
        }
      else if (!shortcut->hidden)
        {
          n_children++;
        }
    }

  _lunar_assert (lp == NULL);
  if (lp_header != NULL)
    goto update_header;
}



static void
lunar_shortcuts_model_shortcut_devices (LunarShortcutsModel *model)
{
  LunarShortcut *shortcut;
  GList          *devices;
  GList          *lp;

  /* add the devices heading */
  shortcut = g_slice_new0 (LunarShortcut);
  shortcut->group = LUNAR_SHORTCUT_GROUP_DEVICES_HEADER;
  shortcut->name = g_strdup (_("Devices"));
  lunar_shortcuts_model_add_shortcut (model, shortcut);

  /* the filesystem entry */
  shortcut = g_slice_new0 (LunarShortcut);
  shortcut->group = LUNAR_SHORTCUT_GROUP_DEVICES_FILESYSTEM;
  shortcut->name = g_strdup (_("File System"));
  shortcut->tooltip = g_strdup (_("Browse the file system"));
  shortcut->file = lunar_file_get_for_uri ("file:///", NULL);
  shortcut->gicon = g_themed_icon_new ("drive-harddisk");
  shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
  lunar_shortcuts_model_add_shortcut (model, shortcut);

  /* connect to the device monitor */
  model->device_monitor = lunar_device_monitor_get ();

  /* get a list of all devices available */
  devices = lunar_device_monitor_get_devices (model->device_monitor);
  for (lp = devices; lp != NULL; lp = lp->next)
    {
      lunar_shortcuts_model_device_added (NULL, lp->data, model);
      g_object_unref (G_OBJECT (lp->data));
    }
  g_list_free (devices);

  /* monitor for changes */
  g_signal_connect (model->device_monitor, "device-added", G_CALLBACK (lunar_shortcuts_model_device_added), model);
  g_signal_connect (model->device_monitor, "device-removed", G_CALLBACK (lunar_shortcuts_model_device_removed), model);
  g_signal_connect (model->device_monitor, "device-changed", G_CALLBACK (lunar_shortcuts_model_device_changed), model);

  lunar_shortcuts_model_header_visibility (model);
}



static void
lunar_shortcuts_model_shortcut_network (LunarShortcutsModel *model)
{
  LunarShortcut *shortcut;

  /* add the network heading */
  shortcut = g_slice_new0 (LunarShortcut);
  shortcut->group = LUNAR_SHORTCUT_GROUP_NETWORK_HEADER;
  shortcut->name = g_strdup (_("Network"));
  lunar_shortcuts_model_add_shortcut (model, shortcut);

  /* append the browse network entry if it is supported */
  if (lunar_g_vfs_is_uri_scheme_supported ("network"))
    {
      shortcut = g_slice_new0 (LunarShortcut);
      shortcut->group = LUNAR_SHORTCUT_GROUP_NETWORK_DEFAULT;
      shortcut->name = g_strdup (_("Browse Network"));
      shortcut->tooltip = g_strdup (_("Browse local network connections"));
      shortcut->location = g_file_new_for_uri ("network://");
      shortcut->gicon = g_themed_icon_new ("network-workgroup");
      shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
      lunar_shortcuts_model_add_shortcut (model, shortcut);
    }
}



static void
lunar_shortcuts_model_shortcut_places (LunarShortcutsModel *model)
{
  LunarShortcut *shortcut;
  GFile          *home;
  GFile          *desktop;
  GFile          *trash;
  LunarFile     *file;

  /* add the places heading */
  shortcut = g_slice_new0 (LunarShortcut);
  shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_HEADER;
  shortcut->name = g_strdup (_("Places"));
  lunar_shortcuts_model_add_shortcut (model, shortcut);

  /* get home path */
  home = lunar_g_file_new_for_home ();

  /* add home entry */
  file = lunar_file_get (home, NULL);
  if (file != NULL)
    {
      shortcut = g_slice_new0 (LunarShortcut);
      shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_DEFAULT;
      shortcut->tooltip = g_strdup (_("Open the home folder"));
      shortcut->file = file;
      shortcut->gicon = g_themed_icon_new ("go-home");
      shortcut->sort_id = 0;
      shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
      lunar_shortcuts_model_add_shortcut (model, shortcut);
    }

  /* add desktop entry */
  desktop = lunar_g_file_new_for_desktop ();
  if (!g_file_equal (desktop, home))
    {
      file = lunar_file_get (desktop, NULL);
      if (file != NULL)
        {
          shortcut = g_slice_new0 (LunarShortcut);
          shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_DEFAULT;
          shortcut->tooltip = g_strdup (_("Open the desktop folder"));
          shortcut->file = file;
          shortcut->sort_id =  1;
          shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
          lunar_shortcuts_model_add_shortcut (model, shortcut);
        }
    }
  g_object_unref (desktop);
  g_object_unref (home);

  /* append the trash icon if the trash is supported */
  if (lunar_g_vfs_is_uri_scheme_supported ("trash"))
    {
      trash = lunar_g_file_new_for_trash ();
      file = lunar_file_get (trash, NULL);
      g_object_unref (trash);

      if (file != NULL)
        {
          shortcut = g_slice_new0 (LunarShortcut);
          shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_TRASH;
          shortcut->name = g_strdup (_("Trash"));
          shortcut->file = file;
          shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
          lunar_shortcuts_model_add_shortcut (model, shortcut);
        }
    }

  /* determine the URI to the Gtk+ bookmarks file */
  model->bookmarks_file = lunar_g_file_new_for_bookmarks ();

  /* register with the alteration monitor for the bookmarks file */
  model->bookmarks_monitor = g_file_monitor_file (model->bookmarks_file, G_FILE_MONITOR_NONE, NULL, NULL);
  if (G_LIKELY (model->bookmarks_monitor != NULL))
    {
      g_signal_connect (model->bookmarks_monitor, "changed",
        G_CALLBACK (lunar_shortcuts_model_monitor), model);
    }

  /* read the Gtk+ bookmarks file */
  model->bookmarks_idle_id = g_idle_add_full (G_PRIORITY_DEFAULT, lunar_shortcuts_model_load, model, NULL);

  /* append the computer icon if browsing the computer is supported */
  if (lunar_g_vfs_is_uri_scheme_supported ("computer"))
    {
      shortcut = g_slice_new0 (LunarShortcut);
      shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_COMPUTER;
      shortcut->name = g_strdup (_("Computer"));
      shortcut->tooltip = g_strdup (_("Browse the computer"));
      shortcut->location = g_file_new_for_uri ("computer://");
      shortcut->gicon = g_themed_icon_new ("computer");
      shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
      lunar_shortcuts_model_add_shortcut (model, shortcut);
    }
}



static gboolean
lunar_shortcuts_model_get_hidden (LunarShortcutsModel *model,
                                   LunarShortcut       *shortcut)
{
  gchar    *uri;
  guint     n;
  gboolean  hidden = FALSE;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);
  _lunar_return_val_if_fail (shortcut->device == NULL, FALSE);

  if (model->hidden_bookmarks == NULL)
    return FALSE;

  /* get the uri */
  if (shortcut->file != NULL)
    uri = lunar_file_dup_uri (shortcut->file);
  else if (shortcut->location != NULL)
    uri = g_file_get_uri (shortcut->location);
  else
    _lunar_assert_not_reached ();

  if (uri == NULL)
    return FALSE;

  for (n = 0; !hidden && model->hidden_bookmarks[n] != NULL; n++)
    hidden = (g_strcmp0 (model->hidden_bookmarks[n], uri) == 0);

  g_free (uri);

  return hidden;
}



static gint
lunar_shortcuts_model_sort_func (gconstpointer shortcut_a,
                                  gconstpointer shortcut_b)
{
  const LunarShortcut *a = shortcut_a;
  const LunarShortcut *b = shortcut_b;

  /* sort groups */
  if (a->group != b->group)
    return a->group - b->group;

  /* use sort order */
  if (a->sort_id != b->sort_id)
    return a->sort_id > b->sort_id ? 1 : -1;

  /* properly sort devices by timestamp */
  if (a->device != NULL && b->device != NULL)
    return lunar_device_sort (a->device, b->device);

  return g_strcmp0 (a->name, b->name);
}



static void
lunar_shortcuts_model_add_shortcut_with_path (LunarShortcutsModel *model,
                                               LunarShortcut       *shortcut,
                                               GtkTreePath          *path)
{
  GtkTreeIter  iter;
  GtkTreePath *sorted_path = NULL;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (shortcut->file == NULL || LUNAR_IS_FILE (shortcut->file));

  /* we want to stay informed about changes to the file */
  if (G_LIKELY (shortcut->file != NULL))
    {
      /* watch the file for changes */
      lunar_file_watch (shortcut->file);

      /* connect appropriate signals */
      g_signal_connect (G_OBJECT (shortcut->file), "changed",
                        G_CALLBACK (lunar_shortcuts_model_file_changed), model);
      g_signal_connect (G_OBJECT (shortcut->file), "destroy",
                        G_CALLBACK (lunar_shortcuts_model_file_destroy), model);
    }

  if (path == NULL)
    {
      /* insert the new shortcut to the shortcuts list */
      model->shortcuts = g_list_insert_sorted (model->shortcuts, shortcut, lunar_shortcuts_model_sort_func);
      sorted_path = gtk_tree_path_new_from_indices (g_list_index (model->shortcuts, shortcut), -1);
      path = sorted_path;
    }
  else
    {
      model->shortcuts = g_list_insert (model->shortcuts, shortcut, gtk_tree_path_get_indices (path)[0]);
    }

  /* tell everybody that we have a new shortcut */
  gtk_tree_model_get_iter (GTK_TREE_MODEL (model), &iter, path);
  gtk_tree_model_row_inserted (GTK_TREE_MODEL (model), path, &iter);

  if (sorted_path)
    gtk_tree_path_free (sorted_path);
}



static void
lunar_shortcuts_model_add_shortcut (LunarShortcutsModel *model,
                                     LunarShortcut       *shortcut)
{
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (shortcut->file == NULL || LUNAR_IS_FILE (shortcut->file));

  lunar_shortcuts_model_add_shortcut_with_path (model, shortcut, NULL);
}



static void
lunar_shortcuts_model_remove_shortcut (LunarShortcutsModel *model,
                                        LunarShortcut       *shortcut)
{
  GtkTreePath *path;
  gint         idx;
  gboolean     needs_save;

  /* determine the index of the shortcut */
  idx = g_list_index (model->shortcuts, shortcut);
  if (G_LIKELY (idx >= 0))
    {
      /* unlink the shortcut from the model */
      model->shortcuts = g_list_remove (model->shortcuts, shortcut);

      /* tell everybody that we have lost a shortcut */
      path = gtk_tree_path_new_from_indices (idx, -1);
      gtk_tree_model_row_deleted (GTK_TREE_MODEL (model), path);
      gtk_tree_path_free (path);

      /* check if we need to save */
      needs_save = (shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS);

      /* actually free the shortcut */
      lunar_shortcut_free (shortcut, model);

      /* the shortcuts list was changed, so write the gtk bookmarks file */
      if (needs_save)
        lunar_shortcuts_model_save (model);

      /* update header visibility */
      lunar_shortcuts_model_header_visibility (model);
    }
}



static gboolean
lunar_shortcuts_model_local_file (GFile *gfile)
{
  _lunar_return_val_if_fail (G_IS_FILE (gfile), FALSE);

  /* schemes we'd like to have as LunarFiles in the model */
  if (g_file_has_uri_scheme (gfile, "file")
      || g_file_has_uri_scheme (gfile, "computer")
      || g_file_has_uri_scheme (gfile, "recent"))
    return TRUE;

  return FALSE;
}



static void
lunar_shortcuts_model_load_line (GFile       *file_path,
                                  const gchar *name,
                                  gint         row_num,
                                  gpointer     user_data)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (user_data);
  LunarShortcut       *shortcut;
  LunarFile           *file;

  _lunar_return_if_fail (G_IS_FILE (file_path));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (name == NULL || g_utf8_validate (name, -1, NULL));

  /* handle local and remove files differently */
  if (lunar_shortcuts_model_local_file (file_path))
    {
      /* try to open the file corresponding to the uri */
      file = lunar_file_get (file_path, NULL);
      if (G_UNLIKELY (file == NULL))
        return;

      /* make sure the file refers to a directory */
      if (G_UNLIKELY (lunar_file_is_directory (file)))
        {
          /* create the shortcut entry */
          shortcut = g_slice_new0 (LunarShortcut);
          shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS;
          shortcut->file = file;
          shortcut->sort_id = row_num;
          shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
          shortcut->name = g_strdup (name);

          /* append the shortcut to the list */
          lunar_shortcuts_model_add_shortcut (model, shortcut);
        }
      else
        {
          g_object_unref (file);
        }
    }
  else
    {
      /* create the shortcut entry */
      shortcut = g_slice_new0 (LunarShortcut);
      shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS;
      shortcut->gicon = g_themed_icon_new ("folder-remote");
      shortcut->location = g_object_ref (file_path);
      shortcut->sort_id = row_num;
      shortcut->hidden = lunar_shortcuts_model_get_hidden (model, shortcut);
      shortcut->name = g_strdup (name);

      /* append the shortcut to the list */
      lunar_shortcuts_model_add_shortcut (model, shortcut);
    }
}



static gboolean
lunar_shortcuts_model_load (gpointer data)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (data);

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);

LUNAR_THREADS_ENTER

  /* parse the bookmarks */
  lunar_util_load_bookmarks (model->bookmarks_file,
                              lunar_shortcuts_model_load_line,
                              model);

  /* update the visibility */
  lunar_shortcuts_model_header_visibility (model);

LUNAR_THREADS_LEAVE

  model->bookmarks_idle_id = 0;

  return FALSE;
}



static gboolean
lunar_shortcuts_model_reload (gpointer data)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (data);
  LunarShortcut       *shortcut;
  GtkTreePath          *path;
  GList                *lp;
  gint                  idx;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);

LUNAR_THREADS_ENTER

  /* drop all existing user-defined shortcuts from the model */
  for (idx = 0, lp = model->shortcuts; lp != NULL; )
    {
      /* grab the shortcut */
      shortcut = LUNAR_SHORTCUT (lp->data);

      /* advance to the next list item */
      lp = g_list_next (lp);

      /* drop the shortcut if it is user-defined */
      if (shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS)
        {
          /* unlink the shortcut from the model */
          model->shortcuts = g_list_remove (model->shortcuts, shortcut);

          /* tell everybody that we have lost a shortcut */
          path = gtk_tree_path_new_from_indices (idx, -1);
          gtk_tree_model_row_deleted (GTK_TREE_MODEL (model), path);
          gtk_tree_path_free (path);

          /* actually free the shortcut */
          lunar_shortcut_free (shortcut, model);
        }
      else
        {
          ++idx;
        }
    }

LUNAR_THREADS_LEAVE

  /* load new bookmarks */
  return lunar_shortcuts_model_load (data);
}



static void
lunar_shortcuts_model_monitor (GFileMonitor     *monitor,
                                GFile            *file,
                                GFile            *other_file,
                                GFileMonitorEvent event_type,
                                gpointer          user_data)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (user_data);

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (model->bookmarks_monitor == monitor);

  /* leave if we saved less than 2 seconds ago */
  if (model->bookmarks_time + 2 * G_USEC_PER_SEC > g_get_real_time ())
    return;

  /* reload the shortcuts model */
  if (model->bookmarks_idle_id == 0)
    model->bookmarks_idle_id = g_idle_add (lunar_shortcuts_model_reload, model);
}



static void
lunar_shortcuts_model_save (LunarShortcutsModel *model)
{
  GString        *contents;
  LunarShortcut *shortcut;
  gchar          *bookmarks_path;
  gchar          *uri;
  GList          *lp;
  GError         *err = NULL;
  GFile          *parent = NULL;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  contents = g_string_new (NULL);

  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = LUNAR_SHORTCUT (lp->data);
      if (shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS)
        {
          if (shortcut->file != NULL)
            uri = lunar_file_dup_uri (shortcut->file);
          else if (shortcut->location != NULL)
            uri = g_file_get_uri (shortcut->location);
          else
            continue;

          if (G_LIKELY (shortcut->name != NULL))
            g_string_append_printf (contents, "%s %s\n", uri, shortcut->name);
          else
            g_string_append_printf (contents, "%s\n", uri);
          g_free (uri);
        }
    }

  /* create folder if it does not exist */
  parent = g_file_get_parent (model->bookmarks_file);
  if (!g_file_make_directory_with_parents (parent, NULL, &err))
    {
       if (g_error_matches (err, G_IO_ERROR, G_IO_ERROR_EXISTS))
         {
           g_clear_error (&err);
         }
       else
         {
           g_warning ("Failed to create bookmarks folder: %s", err->message);
           g_error_free (err);
         }
    }
  g_clear_object (&parent);

  /* write data to the disk */
  bookmarks_path = g_file_get_path (model->bookmarks_file);
  if (!g_file_set_contents (bookmarks_path, contents->str, contents->len, &err))
    {
      g_warning ("Failed to write \"%s\": %s", bookmarks_path, err->message);
      g_error_free (err);
    }
  g_free (bookmarks_path);
  g_string_free (contents, TRUE);

  /* store the save time */
  model->bookmarks_time = g_get_real_time ();
}



static void
lunar_shortcuts_model_file_changed (LunarFile           *file,
                                     LunarShortcutsModel *model)
{
  LunarShortcut *shortcut;
  GtkTreePath    *path;
  GtkTreeIter     iter;
  GList          *lp;
  gint            idx;

  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  /* check if the file still refers to a directory or a not mounted URI,
   * otherwise we cannot keep it on the shortcuts list, and so we'll treat
   * it like the file was destroyed (and thereby removed) */

  if (G_UNLIKELY (!lunar_file_is_directory (file)))
    {
      lunar_shortcuts_model_file_destroy (file, model);
      return;
    }

  for (idx = 0, lp = model->shortcuts; lp != NULL; ++idx, lp = lp->next)
    {
      shortcut = LUNAR_SHORTCUT (lp->data);
      if (shortcut->file == file)
        {
          GTK_TREE_ITER_INIT (iter, model->stamp, lp);

          path = gtk_tree_path_new_from_indices (idx, -1);
          gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
          gtk_tree_path_free (path);
          break;
        }
    }
}



static void
lunar_shortcuts_model_file_destroy (LunarFile           *file,
                                     LunarShortcutsModel *model)
{
  LunarShortcut *shortcut = NULL;
  GList          *lp;

  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  /* lookup the shortcut matching the file */
  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = LUNAR_SHORTCUT (lp->data);
      if (shortcut->file == file)
        break;
    }

  /* verify that we actually found a shortcut */
  _lunar_assert (lp != NULL);
  _lunar_assert (LUNAR_IS_FILE (shortcut->file));

  /* drop the shortcut from the model */
  lunar_shortcuts_model_remove_shortcut (model, shortcut);
}



static void
lunar_shortcuts_model_device_added (LunarDeviceMonitor  *device_monitor,
                                     LunarDevice         *device,
                                     LunarShortcutsModel *model)
{
  LunarShortcut *shortcut;

  _lunar_return_if_fail (device_monitor == NULL || LUNAR_DEVICE_MONITOR (device_monitor));
  _lunar_return_if_fail (device_monitor == NULL || model->device_monitor == device_monitor);
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  /* allocate a new shortcut */
  shortcut = g_slice_new0 (LunarShortcut);
  shortcut->device = g_object_ref (device);
  shortcut->hidden = lunar_device_get_hidden (device);

  switch (lunar_device_get_kind (device))
    {
    case LUNAR_DEVICE_KIND_VOLUME:
      shortcut->group = LUNAR_SHORTCUT_GROUP_DEVICES_VOLUMES;
      break;

    case LUNAR_DEVICE_KIND_MOUNT_LOCAL:
      shortcut->group = LUNAR_SHORTCUT_GROUP_DEVICES_MOUNTS;
      break;

    case LUNAR_DEVICE_KIND_MOUNT_REMOTE:
      shortcut->group = LUNAR_SHORTCUT_GROUP_NETWORK_MOUNTS;
      break;
    }

  /* insert in the model */
  lunar_shortcuts_model_add_shortcut (model, shortcut);

  /* header visibility if call is from monitor */
  if (device_monitor != NULL
      && !shortcut->hidden)
    lunar_shortcuts_model_header_visibility (model);
}



static void
lunar_shortcuts_model_device_removed (LunarDeviceMonitor  *device_monitor,
                                       LunarDevice         *device,
                                       LunarShortcutsModel *model)
{
  GList *lp;

  _lunar_return_if_fail (LUNAR_DEVICE_MONITOR (device_monitor));
  _lunar_return_if_fail (model->device_monitor == device_monitor);
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

  /* find the shortcut */
  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    if (LUNAR_SHORTCUT (lp->data)->device == device)
      break;

  /* something is broken if we don't have a shortcut here */
  _lunar_assert (lp != NULL);
  _lunar_assert (LUNAR_SHORTCUT (lp->data)->device == device);

  /* drop the shortcut from the model */
  if (G_LIKELY (lp != NULL))
    lunar_shortcuts_model_remove_shortcut (model, lp->data);
}



static void
lunar_shortcuts_model_device_changed (LunarDeviceMonitor  *device_monitor,
                                       LunarDevice         *device,
                                       LunarShortcutsModel *model)
{
  GtkTreeIter     iter;
  GList          *lp;
  gint            idx;
  GtkTreePath    *path;
  LunarShortcut *shortcut;
  gboolean        update_header = FALSE;

  _lunar_return_if_fail (LUNAR_DEVICE_MONITOR (device_monitor));
  _lunar_return_if_fail (model->device_monitor == device_monitor);
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));

    /* find the shortcut */
  for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
    if (LUNAR_SHORTCUT (lp->data)->device == device)
      break;

  /* something is broken if we don't have a shortcut here */
  _lunar_assert (lp != NULL);
  _lunar_assert (LUNAR_SHORTCUT (lp->data)->device == device);

  if (G_LIKELY (lp != NULL))
    {
      shortcut = lp->data;

      /* cleanup tooltip */
      g_free (shortcut->tooltip);
      shortcut->tooltip = NULL;

      /* hidden state */
      if (shortcut->hidden != lunar_device_get_hidden (device))
        {
          shortcut->hidden = lunar_device_get_hidden (device);
          update_header = TRUE;
        }

      /* generate an iterator for the path */
      GTK_TREE_ITER_INIT (iter, model->stamp, lp);

      /* tell the view that the volume has changed in some way */
      path = gtk_tree_path_new_from_indices (idx, -1);
      gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
      gtk_tree_path_free (path);
    }

  /* header visibility */
  if (update_header)
    lunar_shortcuts_model_header_visibility (model);
}



static void
lunar_shortcut_free (LunarShortcut       *shortcut,
                      LunarShortcutsModel *model)
{
  if (G_LIKELY (shortcut->file != NULL))
    {
      /* drop the file watch */
      lunar_file_unwatch (shortcut->file);

      /* unregister from the file */
      g_signal_handlers_disconnect_matched (shortcut->file,
                                            G_SIGNAL_MATCH_DATA, 0,
                                            0, NULL, NULL, model);
      g_object_unref (shortcut->file);
    }

  if (G_LIKELY (shortcut->device != NULL))
    g_object_unref (shortcut->device);

  if (G_LIKELY (shortcut->gicon != NULL))
    g_object_unref (shortcut->gicon);

  if (G_LIKELY (shortcut->location != NULL))
    g_object_unref (shortcut->location);

  g_free (shortcut->name);
  g_free (shortcut->tooltip);

  /* release the shortcut itself */
  g_slice_free (LunarShortcut, shortcut);
}



static gboolean
lunar_shortcuts_model_busy_timeout (gpointer data)
{
  LunarShortcutsModel *model = LUNAR_SHORTCUTS_MODEL (data);
  gboolean              keep_running = FALSE;
  LunarShortcut       *shortcut;
  GtkTreePath          *path;
  guint                 idx;
  GtkTreeIter           iter;
  GList                *lp;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (data), FALSE);

  for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
    {
      shortcut = lp->data;
      if (!shortcut->busy)
        continue;

      /* loop the pulse of the shortcut */
      shortcut->busy_pulse++;
      if (shortcut->busy_pulse >= SPINNER_NUM_STEPS)
        shortcut->busy_pulse = 0;

      /* generate an iterator for the path */
      GTK_TREE_ITER_INIT (iter, model->stamp, lp);

      /* notify the views about the change */
      path = gtk_tree_path_new_from_indices (idx, -1);
      gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
      gtk_tree_path_free (path);

      /* keep the timeout running */
      keep_running = TRUE;
    }

  return keep_running;
}



static void
lunar_shortcuts_model_busy_timeout_destroyed (gpointer data)
{
  LUNAR_SHORTCUTS_MODEL (data)->busy_timeout_id = 0;
}



/**
 * lunar_shortcuts_model_get_default:
 *
 * Returns the default #LunarShortcutsModel instance shared by
 * all #LunarShortcutsView instances.
 *
 * Call #g_object_unref() on the returned object when you
 * don't need it any longer.
 *
 * Return value: the default #LunarShortcutsModel instance.
 **/
LunarShortcutsModel*
lunar_shortcuts_model_get_default (void)
{
  static LunarShortcutsModel *model = NULL;

  if (G_UNLIKELY (model == NULL))
    {
      model = g_object_new (LUNAR_TYPE_SHORTCUTS_MODEL, NULL);
      g_object_add_weak_pointer (G_OBJECT (model), (gpointer) &model);
    }
  else
    {
      g_object_ref (G_OBJECT (model));
    }

  return model;
}



/**
 * lunar_shortcuts_model_has_bookmark:
 * @model : a #LunarShortcutsModel instance.
 * @file  : a #ThuanrFile instance.
 *
 * Returns %TRUE if there is a bookmark (not a mount or volume) with
 * @file as destination.
 *
 * Return value: %TRUE if @file was found, else %FALSE.
 **/
gboolean
lunar_shortcuts_model_has_bookmark (LunarShortcutsModel *model,
                                     GFile                *file)
{
  GList          *lp;
  LunarShortcut *shortcut;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);
  _lunar_return_val_if_fail (G_IS_FILE (file), FALSE);

  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = lp->data;

      /* only check bookmarks */
      if (shortcut->group != LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS)
        continue;

      if (shortcut->file != NULL
          && g_file_equal (lunar_file_get_file (shortcut->file), file))
        return TRUE;

      if (shortcut->location != NULL
          && g_file_equal (shortcut->location, file))
        return TRUE;
    }

  return FALSE;
}



/**
 * lunar_shortcuts_model_iter_for_file:
 * @model : a #LunarShortcutsModel instance.
 * @file  : a #ThuanrFile instance.
 * @iter  : pointer to a #GtkTreeIter.
 *
 * Tries to lookup the #GtkTreeIter, that belongs to a shortcut, which
 * refers to @file and stores it to @iter. If no such #GtkTreeIter was
 * found, %FALSE will be returned and @iter won't be changed. Else
 * %TRUE will be returned and @iter will be set appropriately.
 *
 * Return value: %TRUE if @file was found, else %FALSE.
 **/
gboolean
lunar_shortcuts_model_iter_for_file (LunarShortcutsModel *model,
                                      LunarFile           *file,
                                      GtkTreeIter          *iter)
{
  GFile          *mount_point;
  GList          *lp;
  LunarShortcut *shortcut;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (file), FALSE);
  _lunar_return_val_if_fail (iter != NULL, FALSE);

  for (lp = model->shortcuts; lp != NULL; lp = lp->next)
    {
      shortcut = lp->data;

      /* check if we have a location that matches */
      if (shortcut->file == file)
        {
          GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
          return TRUE;
        }

      if (shortcut->location != NULL
          && g_file_equal (shortcut->location, lunar_file_get_file (file)))
        {
          GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
          return TRUE;
        }

      /* but maybe we have a mounted(!) volume with a matching mount point */
      if (shortcut->device != NULL
          && !lunar_device_get_hidden (shortcut->device))
        {
          mount_point = lunar_device_get_root (shortcut->device);
          if (mount_point != NULL)
            {
              if (g_file_equal (mount_point, lunar_file_get_file (file)))
                {
                  GTK_TREE_ITER_INIT (*iter, model->stamp, lp);
                  g_object_unref (mount_point);
                  return TRUE;
                }

              g_object_unref (mount_point);
            }
        }
    }

  return FALSE;
}



/**
 * lunar_shortcuts_model_drop_possible:
 * @model : a #LunarShortcutstModel.
 * @path  : a #GtkTreePath.
 *
 * Determines whether a drop is possible before the given @path, at the same depth
 * as @path. I.e., can we drop data at that location. @path does not have to exist;
 * the return value will almost certainly be FALSE if the parent of @path doesn't
 * exist, though.
 *
 * Return value: %TRUE if it's possible to drop data before @path, else %FALSE.
 **/
gboolean
lunar_shortcuts_model_drop_possible (LunarShortcutsModel *model,
                                      GtkTreePath          *path)
{
  LunarShortcut *shortcut;

  _lunar_return_val_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model), FALSE);
  _lunar_return_val_if_fail (gtk_tree_path_get_depth (path) > 0, FALSE);

  /* determine the list item for the path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);

  /* append to the list is not possible */
  if (G_LIKELY (shortcut == NULL))
    return FALSE;

  /* cannot drop before special shortcuts! */
  if (shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS)
    return TRUE;

  /* we can drop at the end of the bookmarks (before devices header) */
  if (shortcut->group == LUNAR_SHORTCUT_GROUP_DEVICES_HEADER)
    return TRUE;

  return FALSE;
}



/**
 * lunar_shortcuts_model_add:
 * @model    : a #LunarShortcutsModel.
 * @dst_path : the destination path.
 * @file     : the #LunarFile that should be added to the shortcuts list.
 *
 * Adds the shortcut @file to the @model at @dst_path, unless @file is
 * already present in @model in which case no action is performed.
 **/
void
lunar_shortcuts_model_add (LunarShortcutsModel *model,
                            GtkTreePath          *dst_path,
                            gpointer              file)
{
  LunarShortcut *shortcut;
  GFile          *location;
  GList          *lp;
  guint           position = 0;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (dst_path == NULL || gtk_tree_path_get_depth (dst_path) > 0);
  _lunar_return_if_fail (dst_path == NULL || gtk_tree_path_get_indices (dst_path)[0] >= 0);
  _lunar_return_if_fail (dst_path == NULL || gtk_tree_path_get_indices (dst_path)[0] <= (gint) g_list_length (model->shortcuts));
  _lunar_return_if_fail (LUNAR_IS_FILE (file) || G_IS_FILE (file));

  location = G_IS_FILE (file) ? file : lunar_file_get_file (file);

  /* verify that the file is not already in use as shortcut */
  if (lunar_shortcuts_model_has_bookmark (model, location))
    return;

  /* create the new shortcut that will be inserted */
  shortcut = g_slice_new0 (LunarShortcut);
  shortcut->group = LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS;

  if (lunar_shortcuts_model_local_file (location))
    {
      shortcut->file = lunar_file_get (location, NULL);
    }
  else
    {
      shortcut->location = G_FILE (g_object_ref (G_OBJECT (location)));
      shortcut->gicon = g_themed_icon_new ("folder-remote");
    }

  /* if no position was given, place the shortcut at the bottom */
  if (dst_path == NULL)
    {
      for (lp = model->shortcuts; lp != NULL; lp = lp->next)
        if (LUNAR_SHORTCUT (lp->data)->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS)
          position++;

      shortcut->sort_id = ++position;
    }

  /* add the shortcut to the list at the given position */
  lunar_shortcuts_model_add_shortcut_with_path (model, shortcut, dst_path);

  /* the shortcuts list was changed, so write the gtk bookmarks file */
  lunar_shortcuts_model_save (model);
}



/**
 * lunar_shortcuts_model_move:
 * @model    : a #LunarShortcutsModel.
 * @src_path : the source path.
 * @dst_path : the destination path.
 *
 * Moves the shortcut at @src_path to @dst_path, adjusting other
 * shortcut's positions as required.
 **/
void
lunar_shortcuts_model_move (LunarShortcutsModel *model,
                             GtkTreePath          *src_path,
                             GtkTreePath          *dst_path)
{
  LunarShortcut *shortcut;
  GtkTreePath    *path;
  GList          *lp;
  gint           *order;
  gint            index_src;
  gint            index_dst;
  gint            idx;
  gint            n_shortcuts;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (gtk_tree_path_get_depth (src_path) > 0);
  _lunar_return_if_fail (gtk_tree_path_get_depth (dst_path) > 0);
  _lunar_return_if_fail (gtk_tree_path_get_indices (src_path)[0] >= 0);
  _lunar_return_if_fail (gtk_tree_path_get_indices (src_path)[0] < (gint) g_list_length (model->shortcuts));
  _lunar_return_if_fail (gtk_tree_path_get_indices (dst_path)[0] > 0);

  index_src = gtk_tree_path_get_indices (src_path)[0];
  index_dst = gtk_tree_path_get_indices (dst_path)[0];

  if (G_UNLIKELY (index_src == index_dst))
    return;

  /* generate the order for the rows prior the dst/src rows */
  n_shortcuts = g_list_length (model->shortcuts);
  order = g_newa (gint, n_shortcuts);
  for (idx = 0, lp = model->shortcuts; idx < index_src && idx < index_dst; ++idx, lp = lp->next)
    order[idx] = idx;

  if (idx == index_src)
    {
      shortcut = LUNAR_SHORTCUT (lp->data);

      for (; idx < index_dst; ++idx, lp = lp->next)
        {
          if (lp->next == NULL)
            break;
          lp->data = lp->next->data;
          order[idx] = idx + 1;
        }

      lp->data = shortcut;
      order[idx++] = index_src;
    }
  else
    {
      for (; idx < index_src; ++idx, lp = lp->next)
        ;

      _lunar_assert (idx == index_src);

      shortcut = LUNAR_SHORTCUT (lp->data);

      for (; idx > index_dst; --idx, lp = lp->prev)
        {
          lp->data = lp->prev->data;
          order[idx] = idx - 1;
        }

      _lunar_assert (idx == index_dst);

      lp->data = shortcut;
      order[idx] = index_src;
      idx = index_src + 1;
    }

  /* generate the remaining order */
  for (; idx < n_shortcuts; ++idx)
    order[idx] = idx;

  /* tell all listeners about the reordering just performed */
  path = gtk_tree_path_new_first ();
  gtk_tree_model_rows_reordered (GTK_TREE_MODEL (model), path, NULL, order);
  gtk_tree_path_free (path);

  /* the shortcuts list was changed, so write the gtk bookmarks file */
  lunar_shortcuts_model_save (model);
}



/**
 * lunar_shortcuts_model_remove:
 * @model : a #LunarShortcutsModel.
 * @path  : the #GtkTreePath of the shortcut to remove.
 *
 * Removes the shortcut at @path from the @model and syncs to
 * on-disk storage. @path must refer to a valid, user-defined
 * shortcut, as you cannot remove system-defined entities (they
 * are managed internally).
 **/
void
lunar_shortcuts_model_remove (LunarShortcutsModel *model,
                               GtkTreePath          *path)
{
  LunarShortcut *shortcut;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (gtk_tree_path_get_depth (path) > 0);
  _lunar_return_if_fail (gtk_tree_path_get_indices (path)[0] >= 0);
  _lunar_return_if_fail (gtk_tree_path_get_indices (path)[0] < (gint) g_list_length (model->shortcuts));

  /* lookup the shortcut for the given path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);

  /* verify that the shortcut is removable */
  _lunar_assert (shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS);

  /* remove the shortcut (using the file destroy handler) */
  lunar_shortcuts_model_remove_shortcut (model, shortcut);
}



/**
 * lunar_shortcuts_model_rename:
 * @model : a #LunarShortcutsModel.
 * @iter  : the #GtkTreeIter which refers to the shortcut that
 *          should be renamed to @name.
 * @name  : the new name for the shortcut at @path or %NULL to
 *          return to the default name.
 *
 * Renames the shortcut at @iter to the new @name in @model.
 *
 * @name may be %NULL or an empty to reset the shortcut to
 * its default name.
 **/
void
lunar_shortcuts_model_rename (LunarShortcutsModel *model,
                               GtkTreeIter          *iter,
                               const gchar          *name)
{
  LunarShortcut *shortcut;
  GtkTreePath    *path;

  _lunar_return_if_fail (name == NULL || g_utf8_validate (name, -1, NULL));
  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (iter->stamp == model->stamp);
  _lunar_return_if_fail (iter->user_data != NULL);

  /* lookup the shortcut for the given path */
  shortcut = LUNAR_SHORTCUT (((GList *) iter->user_data)->data);

  /* verify the shortcut */
  _lunar_assert (shortcut->group == LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS);

  /* perform the rename */
  if (G_UNLIKELY (shortcut->name != NULL))
    g_free (shortcut->name);
  if (G_UNLIKELY (name == NULL || *name == '\0'))
    shortcut->name = NULL;
  else
    shortcut->name = g_strdup (name);

  /* notify the views about the change */
  path = gtk_tree_model_get_path (GTK_TREE_MODEL (model), iter);
  gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, iter);
  gtk_tree_path_free (path);

  /* save the changes to the model */
  lunar_shortcuts_model_save (model);
}



void
lunar_shortcuts_model_set_busy (LunarShortcutsModel *model,
                                 LunarDevice         *device,
                                 gboolean              busy)
{
  LunarShortcut *shortcut;
  GList          *lp;
  guint           idx;
  GtkTreeIter     iter;
  GtkTreePath    *path;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));

  /* get the device */
  for (lp = model->shortcuts, idx = 0; lp != NULL; lp = lp->next, idx++)
    if (LUNAR_SHORTCUT (lp->data)->device == device)
      break;

  if (lp == NULL)
    return;

  shortcut = lp->data;
  _lunar_assert (shortcut->device == device);

  if (G_LIKELY (shortcut->busy != busy))
    {
      shortcut->busy = busy;

      if (busy && model->busy_timeout_id == 0)
        {
          /* start the global cycle timeout */
          model->busy_timeout_id =
            gdk_threads_add_timeout_full (G_PRIORITY_DEFAULT,
                                          SPINNER_CYCLE_DURATION / SPINNER_NUM_STEPS,
                                          lunar_shortcuts_model_busy_timeout, model,
                                          lunar_shortcuts_model_busy_timeout_destroyed);
        }
      else if (!busy)
        {
          /* generate an iterator for the path */
          GTK_TREE_ITER_INIT (iter, model->stamp, lp);

          /* notify the views about the change */
          path = gtk_tree_path_new_from_indices (idx, -1);
          gtk_tree_model_row_changed (GTK_TREE_MODEL (model), path, &iter);
          gtk_tree_path_free (path);
        }
    }
}




void
lunar_shortcuts_model_set_hidden (LunarShortcutsModel *model,
                                   GtkTreePath          *path,
                                   gboolean              hidden)
{
  LunarShortcut  *shortcut;
  guint            length;
  gchar          **bookmarks;
  guint            pos;
  gchar           *uri;
  guint            n;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_MODEL (model));
  _lunar_return_if_fail (gtk_tree_path_get_depth (path) > 0);
  _lunar_return_if_fail (gtk_tree_path_get_indices (path)[0] >= 0);
  _lunar_return_if_fail (gtk_tree_path_get_indices (path)[0] < (gint) g_list_length (model->shortcuts));

  /* lookup the shortcut for the given path */
  shortcut = g_list_nth_data (model->shortcuts, gtk_tree_path_get_indices (path)[0]);
  _lunar_return_if_fail (shortcut != NULL);

  if (shortcut->device != NULL)
    {
      /* if this is a device, store in the device monitor */
      lunar_device_monitor_set_hidden (model->device_monitor, shortcut->device, hidden);
      return;
    }

  /* get the uri */
  if (shortcut->file != NULL)
    uri = lunar_file_dup_uri (shortcut->file);
  else if (shortcut->location != NULL)
    uri = g_file_get_uri (shortcut->location);
  else
    _lunar_assert_not_reached ();

  /* prepare array */
  length = model->hidden_bookmarks != NULL ? g_strv_length (model->hidden_bookmarks) : 0;
  bookmarks = g_new0 (gchar *, length + 2);
  pos = 0;

  /* copy other uuid in the new list */
  if (model->hidden_bookmarks != NULL)
    {
      for (n = 0; model->hidden_bookmarks[n] != NULL; n++)
        if (g_strcmp0 (model->hidden_bookmarks[n], uri) != 0)
          bookmarks[pos++] = g_strdup (model->hidden_bookmarks[n]);
    }

  /* add the new uri if it should hide */
  if (hidden)
    bookmarks[pos++] = uri;
  else
    g_free (uri);

  /* store new list */
  g_object_set (G_OBJECT (model->preferences), "hidden-bookmarks", bookmarks, NULL);
  g_strfreev (bookmarks);

  /* header visibility */
  lunar_shortcuts_model_header_visibility (model);
}
