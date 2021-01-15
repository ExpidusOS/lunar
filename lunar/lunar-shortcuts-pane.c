/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-shortcuts-model.h>
#include <lunar/lunar-shortcuts-pane.h>
#include <lunar/lunar-shortcuts-view.h>
#include <lunar/lunar-side-pane.h>



enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
  PROP_SELECTED_FILES,
  PROP_SHOW_HIDDEN,
};



static void          lunar_shortcuts_pane_component_init        (LunarComponentIface     *iface);
static void          lunar_shortcuts_pane_navigator_init        (LunarNavigatorIface     *iface);
static void          lunar_shortcuts_pane_side_pane_init        (LunarSidePaneIface      *iface);
static void          lunar_shortcuts_pane_dispose               (GObject                  *object);
static void          lunar_shortcuts_pane_finalize              (GObject                  *object);
static void          lunar_shortcuts_pane_get_property          (GObject                  *object,
                                                                  guint                     prop_id,
                                                                  GValue                   *value,
                                                                  GParamSpec               *pspec);
static void          lunar_shortcuts_pane_set_property          (GObject                  *object,
                                                                  guint                     prop_id,
                                                                  const GValue             *value,
                                                                  GParamSpec               *pspec);
static LunarFile   *lunar_shortcuts_pane_get_current_directory (LunarNavigator          *navigator);
static void          lunar_shortcuts_pane_set_current_directory (LunarNavigator          *navigator,
                                                                  LunarFile               *current_directory);
static GList        *lunar_shortcuts_pane_get_selected_files    (LunarComponent          *component);
static void          lunar_shortcuts_pane_set_selected_files    (LunarComponent          *component,
                                                                  GList                    *selected_files);
static void          lunar_shortcuts_pane_show_shortcuts_view_padding (GtkWidget          *widget);
static void          lunar_shortcuts_pane_hide_shortcuts_view_padding (GtkWidget          *widget);



struct _LunarShortcutsPaneClass
{
  GtkScrolledWindowClass __parent__;
};

struct _LunarShortcutsPane
{
  GtkScrolledWindow __parent__;

  LunarFile       *current_directory;
  GList            *selected_files;

  GtkWidget        *view;

  guint             idle_select_directory;
};



G_DEFINE_TYPE_WITH_CODE (LunarShortcutsPane, lunar_shortcuts_pane, GTK_TYPE_SCROLLED_WINDOW,
    G_IMPLEMENT_INTERFACE (LUNAR_TYPE_NAVIGATOR, lunar_shortcuts_pane_navigator_init)
    G_IMPLEMENT_INTERFACE (LUNAR_TYPE_COMPONENT, lunar_shortcuts_pane_component_init)
    G_IMPLEMENT_INTERFACE (LUNAR_TYPE_SIDE_PANE, lunar_shortcuts_pane_side_pane_init))



static void
lunar_shortcuts_pane_class_init (LunarShortcutsPaneClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = lunar_shortcuts_pane_dispose;
  gobject_class->finalize = lunar_shortcuts_pane_finalize;
  gobject_class->get_property = lunar_shortcuts_pane_get_property;
  gobject_class->set_property = lunar_shortcuts_pane_set_property;

  /* override LunarNavigator's properties */
  g_object_class_override_property (gobject_class, PROP_CURRENT_DIRECTORY, "current-directory");

  /* override LunarComponent's properties */
  g_object_class_override_property (gobject_class, PROP_SELECTED_FILES, "selected-files");

  /* override LunarSidePane's properties */
  g_object_class_override_property (gobject_class, PROP_SHOW_HIDDEN, "show-hidden");
}



static void
lunar_shortcuts_pane_component_init (LunarComponentIface *iface)
{
  iface->get_selected_files = lunar_shortcuts_pane_get_selected_files;
  iface->set_selected_files = lunar_shortcuts_pane_set_selected_files;
}



static void
lunar_shortcuts_pane_navigator_init (LunarNavigatorIface *iface)
{
  iface->get_current_directory = lunar_shortcuts_pane_get_current_directory;
  iface->set_current_directory = lunar_shortcuts_pane_set_current_directory;
}



static void
lunar_shortcuts_pane_side_pane_init (LunarSidePaneIface *iface)
{
  iface->get_show_hidden = (gpointer) endo_noop_false;
  iface->set_show_hidden = (gpointer) endo_noop;
}



static void
lunar_shortcuts_pane_init (LunarShortcutsPane *shortcuts_pane)
{
  GtkWidget *vscrollbar;

  /* configure the GtkScrolledWindow */
  gtk_scrolled_window_set_hadjustment (GTK_SCROLLED_WINDOW (shortcuts_pane), NULL);
  gtk_scrolled_window_set_vadjustment (GTK_SCROLLED_WINDOW (shortcuts_pane), NULL);
  gtk_scrolled_window_set_shadow_type (GTK_SCROLLED_WINDOW (shortcuts_pane), GTK_SHADOW_IN);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (shortcuts_pane), GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

  /* allocate the shortcuts view */
  shortcuts_pane->view = lunar_shortcuts_view_new ();
  gtk_container_add (GTK_CONTAINER (shortcuts_pane), shortcuts_pane->view);
  gtk_widget_show (shortcuts_pane->view);

  vscrollbar = gtk_scrolled_window_get_vscrollbar (GTK_SCROLLED_WINDOW (shortcuts_pane));
  g_signal_connect_swapped (G_OBJECT (vscrollbar), "map",
                            G_CALLBACK (lunar_shortcuts_pane_show_shortcuts_view_padding),
                            shortcuts_pane->view);
  g_signal_connect_swapped (G_OBJECT (vscrollbar), "unmap",
                            G_CALLBACK (lunar_shortcuts_pane_hide_shortcuts_view_padding),
                            shortcuts_pane->view);

  /* add widget to css class */
  gtk_style_context_add_class (gtk_widget_get_style_context (GTK_WIDGET (shortcuts_pane)), "shortcuts-pane");

  /* connect the "shortcut-activated" signal */
  g_signal_connect_swapped (G_OBJECT (shortcuts_pane->view), "shortcut-activated", G_CALLBACK (lunar_navigator_change_directory), shortcuts_pane);
  g_signal_connect_swapped (G_OBJECT (shortcuts_pane->view), "shortcut-activated-tab", G_CALLBACK (lunar_navigator_open_new_tab), shortcuts_pane);
}



static void
lunar_shortcuts_pane_dispose (GObject *object)
{
  LunarShortcutsPane *shortcuts_pane = LUNAR_SHORTCUTS_PANE (object);

  lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (shortcuts_pane), NULL);
  lunar_component_set_selected_files (LUNAR_COMPONENT (shortcuts_pane), NULL);

  (*G_OBJECT_CLASS (lunar_shortcuts_pane_parent_class)->dispose) (object);
}



static void
lunar_shortcuts_pane_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (lunar_shortcuts_pane_parent_class)->finalize) (object);
}



static void
lunar_shortcuts_pane_get_property (GObject    *object,
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

    case PROP_SHOW_HIDDEN:
      g_value_set_boolean (value, lunar_side_pane_get_show_hidden (LUNAR_SIDE_PANE (object)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_shortcuts_pane_set_property (GObject      *object,
                                    guint         prop_id,
                                    const GValue *value,
                                    GParamSpec   *pspec)
{
  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (object), g_value_get_object (value));
      break;

    case PROP_SELECTED_FILES:
      lunar_component_set_selected_files (LUNAR_COMPONENT (object), g_value_get_boxed (value));
      break;

    case PROP_SHOW_HIDDEN:
      lunar_side_pane_set_show_hidden (LUNAR_SIDE_PANE (object), g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static LunarFile*
lunar_shortcuts_pane_get_current_directory (LunarNavigator *navigator)
{
  return LUNAR_SHORTCUTS_PANE (navigator)->current_directory;
}



static gboolean
lunar_shortcuts_pane_set_current_directory_idle (gpointer data)
{
  LunarShortcutsPane *shortcuts_pane = LUNAR_SHORTCUTS_PANE (data);

  if (shortcuts_pane->current_directory != NULL)
    {
      lunar_shortcuts_view_select_by_file (LUNAR_SHORTCUTS_VIEW (shortcuts_pane->view),
                                            shortcuts_pane->current_directory);
    }

  /* unset id */
  shortcuts_pane->idle_select_directory = 0;

  return FALSE;
}



static void
lunar_shortcuts_pane_set_current_directory (LunarNavigator *navigator,
                                             LunarFile      *current_directory)
{
  LunarShortcutsPane *shortcuts_pane = LUNAR_SHORTCUTS_PANE (navigator);

  /* disconnect from the previously set current directory */
  if (G_LIKELY (shortcuts_pane->current_directory != NULL))
    g_object_unref (G_OBJECT (shortcuts_pane->current_directory));

  /* remove pending timeout */
  if (shortcuts_pane->idle_select_directory != 0)
    {
      g_source_remove (shortcuts_pane->idle_select_directory);
      shortcuts_pane->idle_select_directory = 0;
    }

  /* activate the new directory */
  shortcuts_pane->current_directory = current_directory;

  /* connect to the new directory */
  if (G_LIKELY (current_directory != NULL))
    {
      /* take a reference on the new directory */
      g_object_ref (G_OBJECT (current_directory));

      /* start idle to select item in sidepane (this to also make
       * the selection work when the bookmarks are loaded idle) */
      shortcuts_pane->idle_select_directory =
        g_idle_add_full (G_PRIORITY_LOW, lunar_shortcuts_pane_set_current_directory_idle,
                         shortcuts_pane, NULL);
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (shortcuts_pane), "current-directory");
}



static GList*
lunar_shortcuts_pane_get_selected_files (LunarComponent *component)
{
  return LUNAR_SHORTCUTS_PANE (component)->selected_files;
}



static void
lunar_shortcuts_pane_set_selected_files (LunarComponent *component,
                                          GList           *selected_files)
{
  LunarShortcutsPane *shortcuts_pane = LUNAR_SHORTCUTS_PANE (component);

  /* disconnect from the previously selected files... */
  lunar_g_file_list_free (shortcuts_pane->selected_files);

  /* ...and take a copy of the newly selected files */
  shortcuts_pane->selected_files = lunar_g_file_list_copy (selected_files);

  /* notify listeners */
  g_object_notify (G_OBJECT (shortcuts_pane), "selected-files");
}



/**
 * lunar_shortcuts_pane_add_shortcut:
 * @shortcuts_pane : Instance of a #LunarShortcutsPane
 * @file           : #LunarFile for which a shortcut should be added
 *
 * Adds a shortcut for the passed #LunarFile to the shortcuts_pane.
 * Only folders will be considered.
 **/
void
lunar_shortcuts_pane_add_shortcut (LunarShortcutsPane *shortcuts_pane,
                                    LunarFile          *file)
{
  GtkTreeModel *model;
  GtkTreeModel *child_model;

  _lunar_return_if_fail (LUNAR_IS_SHORTCUTS_PANE (shortcuts_pane));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  /* determine the shortcuts model for the view */
  model = gtk_tree_view_get_model (GTK_TREE_VIEW (shortcuts_pane->view));
  if (G_LIKELY (model != NULL))
    {
      child_model = gtk_tree_model_filter_get_model (GTK_TREE_MODEL_FILTER (model));
      if (G_LIKELY (lunar_file_is_directory (file)))
        {
          /* append the folder to the shortcuts model */
          lunar_shortcuts_model_add (LUNAR_SHORTCUTS_MODEL (child_model), NULL, file);
        }
    }
}



static void
lunar_shortcuts_pane_show_shortcuts_view_padding (GtkWidget *widget)
{
  lunar_shortcuts_view_toggle_padding (LUNAR_SHORTCUTS_VIEW (widget), TRUE);
}



static void
lunar_shortcuts_pane_hide_shortcuts_view_padding (GtkWidget *widget)
{
  lunar_shortcuts_view_toggle_padding (LUNAR_SHORTCUTS_VIEW (widget), FALSE);
}
