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

#include <gdk/gdkkeysyms.h>

#include <lunar/lunar-column-editor.h>
#include <lunar/lunar-details-view.h>
#include <lunar/lunar-launcher.h>
#include <lunar/lunar-gtk-extensions.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-window.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FIXED_COLUMNS,
};



static void         lunar_details_view_finalize                (GObject                *object);
static void         lunar_details_view_get_property            (GObject                *object,
                                                                 guint                   prop_id,
                                                                 GValue                 *value,
                                                                 GParamSpec             *pspec);
static void         lunar_details_view_set_property            (GObject                *object,
                                                                 guint                   prop_id,
                                                                 const GValue           *value,
                                                                 GParamSpec             *pspec);
static AtkObject   *lunar_details_view_get_accessible          (GtkWidget              *widget);
static GList       *lunar_details_view_get_selected_items      (LunarStandardView     *standard_view);
static void         lunar_details_view_select_all              (LunarStandardView     *standard_view);
static void         lunar_details_view_unselect_all            (LunarStandardView     *standard_view);
static void         lunar_details_view_selection_invert        (LunarStandardView     *standard_view);
static void         lunar_details_view_select_path             (LunarStandardView     *standard_view,
                                                                 GtkTreePath            *path);
static void         lunar_details_view_set_cursor              (LunarStandardView     *standard_view,
                                                                 GtkTreePath            *path,
                                                                 gboolean                start_editing);
static void         lunar_details_view_scroll_to_path          (LunarStandardView     *standard_view,
                                                                 GtkTreePath            *path,
                                                                 gboolean                use_align,
                                                                 gfloat                  row_align,
                                                                 gfloat                  col_align);
static GtkTreePath *lunar_details_view_get_path_at_pos         (LunarStandardView     *standard_view,
                                                                 gint                    x,
                                                                 gint                    y);
static gboolean     lunar_details_view_get_visible_range       (LunarStandardView     *standard_view,
                                                                 GtkTreePath           **start_path,
                                                                 GtkTreePath           **end_path);
static void         lunar_details_view_highlight_path          (LunarStandardView     *standard_view,
                                                                 GtkTreePath            *path);
static void         lunar_details_view_notify_model            (GtkTreeView            *tree_view,
                                                                 GParamSpec             *pspec,
                                                                 LunarDetailsView      *details_view);
static void         lunar_details_view_notify_width            (GtkTreeViewColumn      *tree_view_column,
                                                                 GParamSpec             *pspec,
                                                                 LunarDetailsView      *details_view);
static gboolean     lunar_details_view_button_press_event      (GtkTreeView            *tree_view,
                                                                 GdkEventButton         *event,
                                                                 LunarDetailsView      *details_view);
static gboolean     lunar_details_view_key_press_event         (GtkTreeView            *tree_view,
                                                                 GdkEventKey            *event,
                                                                 LunarDetailsView      *details_view);
static void         lunar_details_view_row_activated           (GtkTreeView            *tree_view,
                                                                 GtkTreePath            *path,
                                                                 GtkTreeViewColumn      *column,
                                                                 LunarDetailsView      *details_view);
static gboolean     lunar_details_view_select_cursor_row       (GtkTreeView            *tree_view,
                                                                 gboolean                editing,
                                                                 LunarDetailsView      *details_view);
static void         lunar_details_view_row_changed             (GtkTreeView            *tree_view,
                                                                 GtkTreePath            *path,
                                                                 GtkTreeViewColumn      *column,
                                                                 LunarDetailsView      *details_view);
static void         lunar_details_view_columns_changed         (LunarColumnModel      *column_model,
                                                                 LunarDetailsView      *details_view);
static void         lunar_details_view_zoom_level_changed      (LunarDetailsView      *details_view);
static gboolean     lunar_details_view_get_fixed_columns       (LunarDetailsView      *details_view);
static void         lunar_details_view_set_fixed_columns       (LunarDetailsView      *details_view,
                                                                 gboolean                fixed_columns);
static void         lunar_details_view_connect_accelerators    (LunarStandardView     *standard_view,
                                                                 GtkAccelGroup          *accel_group);
static void         lunar_details_view_disconnect_accelerators (LunarStandardView     *standard_view,
                                                                 GtkAccelGroup          *accel_group);
static void         lunar_details_view_append_menu_items       (LunarStandardView     *standard_view,
                                                                 GtkMenu                *menu,
                                                                 GtkAccelGroup          *accel_group);


struct _LunarDetailsViewClass
{
  LunarStandardViewClass __parent__;
};



struct _LunarDetailsView
{
  LunarStandardView __parent__;

  /* the column model */
  LunarColumnModel *column_model;

  /* the tree view columns */
  GtkTreeViewColumn *columns[LUNAR_N_VISIBLE_COLUMNS];

  /* whether to use fixed column widths */
  gboolean           fixed_columns;

  /* whether the most recent item activation used a mouse button press */
  gboolean           button_pressed;

  /* event source id for lunar_details_view_zoom_level_changed_reload_fixed_columns */
  guint idle_id;

};



static ExpidusGtkActionEntry lunar_details_view_action_entries[] =
{
    { LUNAR_DETAILS_VIEW_ACTION_CONFIGURE_COLUMNS,  "<Actions>/LunarStandardView/configure-columns", "", EXPIDUS_GTK_MENU_ITEM , N_ ("Configure _Columns..."), N_("Configure the columns in the detailed list view"), NULL, G_CALLBACK (lunar_show_column_editor), },
};

#define get_action_entry(id) expidus_gtk_get_action_entry_by_id(lunar_details_view_action_entries,G_N_ELEMENTS(lunar_details_view_action_entries),id)



G_DEFINE_TYPE (LunarDetailsView, lunar_details_view, LUNAR_TYPE_STANDARD_VIEW)



static void
lunar_details_view_class_init (LunarDetailsViewClass *klass)
{
  LunarStandardViewClass *lunarstandard_view_class;
  GtkWidgetClass          *gtkwidget_class;
  GObjectClass            *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_details_view_finalize;
  gobject_class->get_property = lunar_details_view_get_property;
  gobject_class->set_property = lunar_details_view_set_property;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->get_accessible = lunar_details_view_get_accessible;

  lunarstandard_view_class = LUNAR_STANDARD_VIEW_CLASS (klass);
  lunarstandard_view_class->get_selected_items = lunar_details_view_get_selected_items;
  lunarstandard_view_class->select_all = lunar_details_view_select_all;
  lunarstandard_view_class->unselect_all = lunar_details_view_unselect_all;
  lunarstandard_view_class->selection_invert = lunar_details_view_selection_invert;
  lunarstandard_view_class->select_path = lunar_details_view_select_path;
  lunarstandard_view_class->set_cursor = lunar_details_view_set_cursor;
  lunarstandard_view_class->scroll_to_path = lunar_details_view_scroll_to_path;
  lunarstandard_view_class->get_path_at_pos = lunar_details_view_get_path_at_pos;
  lunarstandard_view_class->get_visible_range = lunar_details_view_get_visible_range;
  lunarstandard_view_class->highlight_path = lunar_details_view_highlight_path;
  lunarstandard_view_class->append_menu_items = lunar_details_view_append_menu_items;
  lunarstandard_view_class->connect_accelerators = lunar_details_view_connect_accelerators;
  lunarstandard_view_class->disconnect_accelerators = lunar_details_view_disconnect_accelerators;
  lunarstandard_view_class->zoom_level_property_name = "last-details-view-zoom-level";

  expidus_gtk_translate_action_entries (lunar_details_view_action_entries, G_N_ELEMENTS (lunar_details_view_action_entries));

  /**
   * LunarDetailsView:fixed-columns:
   *
   * %TRUE to use fixed column widths.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FIXED_COLUMNS,
                                   g_param_spec_boolean ("fixed-columns",
                                                         "fixed-columns",
                                                         "fixed-columns",
                                                         FALSE,
                                                         ENDO_PARAM_READWRITE));
}



static void
lunar_details_view_init (LunarDetailsView *details_view)
{
  GtkTreeSelection *selection;
  GtkCellRenderer  *right_aligned_renderer;
  GtkCellRenderer  *left_aligned_renderer;
  GtkCellRenderer  *renderer;
  LunarColumn      column;
  GtkWidget        *tree_view;

  /* we need to force the GtkTreeView to recalculate column sizes
   * whenever the zoom-level changes, so we connect a handler here.
   */
  g_signal_connect (G_OBJECT (details_view), "notify::zoom-level", G_CALLBACK (lunar_details_view_zoom_level_changed), NULL);

  /* create the tree view to embed */
  tree_view = endo_tree_view_new ();
  g_signal_connect (G_OBJECT (tree_view), "notify::model",
                    G_CALLBACK (lunar_details_view_notify_model), details_view);
  g_signal_connect (G_OBJECT (tree_view), "button-press-event",
                    G_CALLBACK (lunar_details_view_button_press_event), details_view);
  g_signal_connect (G_OBJECT (tree_view), "key-press-event",
                    G_CALLBACK (lunar_details_view_key_press_event), details_view);
  g_signal_connect (G_OBJECT (tree_view), "row-activated",
                    G_CALLBACK (lunar_details_view_row_activated), details_view);
  g_signal_connect (G_OBJECT (tree_view), "select-cursor-row",
                    G_CALLBACK (lunar_details_view_select_cursor_row), details_view);
  gtk_container_add (GTK_CONTAINER (details_view), tree_view);
  gtk_widget_show (tree_view);

  /* configure general aspects of the details view */
  gtk_tree_view_set_enable_search (GTK_TREE_VIEW (tree_view), TRUE);

  /* enable rubberbanding (if supported) */
  gtk_tree_view_set_rubber_banding (GTK_TREE_VIEW (tree_view), TRUE);

  /* connect to the default column model */
  details_view->column_model = lunar_column_model_get_default ();
  g_signal_connect (G_OBJECT (details_view->column_model), "columns-changed", G_CALLBACK (lunar_details_view_columns_changed), details_view);
  g_signal_connect_after (G_OBJECT (LUNAR_STANDARD_VIEW (details_view)->model), "row-changed",
                          G_CALLBACK (lunar_details_view_row_changed), details_view);

  /* allocate the shared right-aligned text renderer */
  right_aligned_renderer = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "xalign", 1.0f, NULL);
  g_object_ref_sink (G_OBJECT (right_aligned_renderer));

  /* allocate the shared left-aligned text renderer */
  left_aligned_renderer = g_object_new (GTK_TYPE_CELL_RENDERER_TEXT, "xalign", 0.0f, NULL);
  g_object_ref_sink (G_OBJECT (left_aligned_renderer));

  /* allocate the tree view columns */
  for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
    {
      /* allocate the tree view column */
      details_view->columns[column] = gtk_tree_view_column_new ();
      g_object_ref_sink (G_OBJECT (details_view->columns[column]));
      gtk_tree_view_column_set_min_width (details_view->columns[column], 50);
      gtk_tree_view_column_set_resizable (details_view->columns[column], TRUE);
      gtk_tree_view_column_set_sort_column_id (details_view->columns[column], column);
      gtk_tree_view_column_set_title (details_view->columns[column], lunar_column_model_get_column_name (details_view->column_model, column));

      /* stay informed whenever the width of the column is changed */
      g_signal_connect (G_OBJECT (details_view->columns[column]), "notify::width", G_CALLBACK (lunar_details_view_notify_width), details_view);

      /* name column gets special renderers */
      if (G_UNLIKELY (column == LUNAR_COLUMN_NAME))
        {
          /* add the icon renderer */
          gtk_tree_view_column_pack_start (details_view->columns[column], LUNAR_STANDARD_VIEW (details_view)->icon_renderer, FALSE);
          gtk_tree_view_column_set_attributes (details_view->columns[column], LUNAR_STANDARD_VIEW (details_view)->icon_renderer,
                                               "file", LUNAR_COLUMN_FILE,
                                               NULL);

          /* add the name renderer */
          g_object_set (G_OBJECT (LUNAR_STANDARD_VIEW (details_view)->name_renderer),
                        "xalign", 0.0, "ellipsize", PANGO_ELLIPSIZE_END, "width-chars", 30, NULL);
          gtk_tree_view_column_pack_start (details_view->columns[column], LUNAR_STANDARD_VIEW (details_view)->name_renderer, TRUE);
          gtk_tree_view_column_set_attributes (details_view->columns[column], LUNAR_STANDARD_VIEW (details_view)->name_renderer,
                                               "text", LUNAR_COLUMN_NAME,
                                               NULL);

          /* add some spacing between the icon and the name */
          gtk_tree_view_column_set_spacing (details_view->columns[column], 2);
          gtk_tree_view_column_set_expand (details_view->columns[column], TRUE);
        }
      else
        {
          /* size is right aligned, everything else is left aligned */
          renderer = (column == LUNAR_COLUMN_SIZE || column == LUNAR_COLUMN_SIZE_IN_BYTES) ? right_aligned_renderer : left_aligned_renderer;

          /* add the renderer */
          gtk_tree_view_column_pack_start (details_view->columns[column], renderer, TRUE);
          gtk_tree_view_column_set_attributes (details_view->columns[column], renderer, "text", column, NULL);
        }

      /* append the tree view column to the tree view */
      gtk_tree_view_append_column (GTK_TREE_VIEW (tree_view), details_view->columns[column]);
    }

  /* configure the tree selection */
  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (tree_view));
  gtk_tree_selection_set_mode (selection, GTK_SELECTION_MULTIPLE);
  g_signal_connect_swapped (G_OBJECT (selection), "changed",
                            G_CALLBACK (lunar_standard_view_selection_changed), details_view);

  /* apply the initial column order and visibility from the column model */
  lunar_details_view_columns_changed (details_view->column_model, details_view);

  /* synchronize the "fixed-columns" property */
  endo_binding_new (G_OBJECT (LUNAR_STANDARD_VIEW (details_view)->preferences),
                   "last-details-view-fixed-columns",
                   G_OBJECT (details_view),
                   "fixed-columns");

  /* apply fixed column widths if we start in fixed column mode */
  if (G_UNLIKELY (details_view->fixed_columns))
    {
      /* apply to all columns */
      for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
        gtk_tree_view_column_set_fixed_width (details_view->columns[column], lunar_column_model_get_column_width (details_view->column_model, column));
    }

  /* release the shared text renderers */
  g_object_unref (G_OBJECT (right_aligned_renderer));
  g_object_unref (G_OBJECT (left_aligned_renderer));
}



static void
lunar_details_view_get_property (GObject    *object,
                                  guint       prop_id,
                                  GValue     *value,
                                  GParamSpec *pspec)
{
  LunarDetailsView *details_view = LUNAR_DETAILS_VIEW (object);

  switch (prop_id)
    {
    case PROP_FIXED_COLUMNS:
      g_value_set_boolean (value, lunar_details_view_get_fixed_columns (details_view));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_details_view_set_property (GObject      *object,
                                  guint         prop_id,
                                  const GValue *value,
                                  GParamSpec   *pspec)
{
  LunarDetailsView *details_view = LUNAR_DETAILS_VIEW (object);

  switch (prop_id)
    {
    case PROP_FIXED_COLUMNS:
      lunar_details_view_set_fixed_columns (details_view, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_details_view_finalize (GObject *object)
{
  LunarDetailsView *details_view = LUNAR_DETAILS_VIEW (object);
  LunarColumn       column;

  /* release the tree view columns array */
  for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
    g_object_unref (G_OBJECT (details_view->columns[column]));

  /* disconnect from the default column model */
  g_signal_handlers_disconnect_by_func (G_OBJECT (details_view->column_model), lunar_details_view_columns_changed, details_view);
  g_object_unref (G_OBJECT (details_view->column_model));

  if (details_view->idle_id)
    g_source_remove (details_view->idle_id);

  (*G_OBJECT_CLASS (lunar_details_view_parent_class)->finalize) (object);
}



static AtkObject*
lunar_details_view_get_accessible (GtkWidget *widget)
{
  AtkObject *object;

  /* query the atk object for the tree view class */
  object = (*GTK_WIDGET_CLASS (lunar_details_view_parent_class)->get_accessible) (widget);

  /* set custom Atk properties for the details view */
  if (G_LIKELY (object != NULL))
    {
      atk_object_set_description (object, _("Detailed directory listing"));
      atk_object_set_name (object, _("Details view"));
      atk_object_set_role (object, ATK_ROLE_DIRECTORY_PANE);
    }

  return object;
}



static GList*
lunar_details_view_get_selected_items (LunarStandardView *standard_view)
{
  GtkTreeSelection *selection;

  _lunar_return_val_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view), NULL);

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
  return gtk_tree_selection_get_selected_rows (selection, NULL);
}



static void
lunar_details_view_select_all (LunarStandardView *standard_view)
{
  GtkTreeSelection *selection;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
  gtk_tree_selection_select_all (selection);
}



static void
lunar_details_view_unselect_all (LunarStandardView *standard_view)
{
  GtkTreeSelection *selection;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
  gtk_tree_selection_unselect_all (selection);
}



static void
lunar_details_view_selection_invert_foreach (GtkTreeModel *model,
                                              GtkTreePath  *path,
                                              GtkTreeIter  *iter,
                                              gpointer      data)
{
  GList      **list = data;

  *list = g_list_prepend (*list, gtk_tree_path_copy (path));
}



static void
lunar_details_view_selection_invert (LunarStandardView *standard_view)
{
  GtkTreeSelection *selection;
  GList            *selected_paths = NULL;
  GList            *lp;
  GtkTreePath      *path;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));

  /* block updates */
  g_signal_handlers_block_by_func (selection, lunar_standard_view_selection_changed, standard_view);

  /* get paths of selected files */
  gtk_tree_selection_selected_foreach (selection, lunar_details_view_selection_invert_foreach, &selected_paths);

  gtk_tree_selection_select_all (selection);

  for (lp = selected_paths; lp != NULL; lp = lp->next)
    {
      path = lp->data;
      gtk_tree_selection_unselect_path (selection, path);
      gtk_tree_path_free (path);
    }

  g_list_free (selected_paths);

  /* unblock updates */
  g_signal_handlers_unblock_by_func (selection, lunar_standard_view_selection_changed, standard_view);

  lunar_standard_view_selection_changed (LUNAR_STANDARD_VIEW (standard_view));
}



static void
lunar_details_view_select_path (LunarStandardView *standard_view,
                                 GtkTreePath        *path)
{
  GtkTreeSelection *selection;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));

  selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
  gtk_tree_selection_select_path (selection, path);
}



static void
lunar_details_view_set_cursor (LunarStandardView *standard_view,
                                GtkTreePath        *path,
                                gboolean            start_editing)
{
  GtkCellRendererMode mode;
  GtkTreeViewColumn  *column;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));

  /* make sure the name renderer is editable */
  g_object_get ( G_OBJECT (standard_view->name_renderer), "mode", &mode, NULL);
  g_object_set ( G_OBJECT (standard_view->name_renderer), "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);

  /* tell the tree view to start editing the given row */
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), 0);
  gtk_tree_view_set_cursor_on_cell (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))),
                                    path, column, standard_view->name_renderer,
                                    start_editing);

  /* reset the name renderer mode */
  g_object_set (G_OBJECT (standard_view->name_renderer), "mode", mode, NULL);
}



static void
lunar_details_view_scroll_to_path (LunarStandardView *standard_view,
                                    GtkTreePath        *path,
                                    gboolean            use_align,
                                    gfloat              row_align,
                                    gfloat              col_align)
{
  GtkTreeViewColumn *column;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));

  /* tell the tree view to scroll to the given row */
  column = gtk_tree_view_get_column (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), 0);
  gtk_tree_view_scroll_to_cell (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), path, column, use_align, row_align, col_align);
}



static GtkTreePath*
lunar_details_view_get_path_at_pos (LunarStandardView *standard_view,
                                     gint                x,
                                     gint                y)
{
  GtkTreePath *path;

  _lunar_return_val_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view), NULL);

  if (gtk_tree_view_get_dest_row_at_pos (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), x, y, &path, NULL))
    return path;

  return NULL;
}



static gboolean
lunar_details_view_get_visible_range (LunarStandardView *standard_view,
                                       GtkTreePath       **start_path,
                                       GtkTreePath       **end_path)
{
  _lunar_return_val_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view), FALSE);

  return gtk_tree_view_get_visible_range (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), start_path, end_path);
}



static void
lunar_details_view_highlight_path (LunarStandardView *standard_view,
                                    GtkTreePath        *path)
{
  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (standard_view));
  gtk_tree_view_set_drag_dest_row (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), path, GTK_TREE_VIEW_DROP_INTO_OR_AFTER);
}



static void
lunar_details_view_notify_model (GtkTreeView       *tree_view,
                                  GParamSpec        *pspec,
                                  LunarDetailsView *details_view)
{
  /* We need to set the search column here, as GtkTreeView resets it
   * whenever a new model is set.
   */
  gtk_tree_view_set_search_column (tree_view, LUNAR_COLUMN_NAME);
}



static void
lunar_details_view_notify_width (GtkTreeViewColumn *tree_view_column,
                                  GParamSpec        *pspec,
                                  LunarDetailsView *details_view)
{
  LunarColumn column;

  _lunar_return_if_fail (GTK_IS_TREE_VIEW_COLUMN (tree_view_column));
  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  /* lookup the column no for the given tree view column */
  for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
    if (details_view->columns[column] == tree_view_column)
      {
        /* save the new width as default fixed width */
        lunar_column_model_set_column_width (details_view->column_model, column, gtk_tree_view_column_get_width (tree_view_column));
        break;
      }
}



static gboolean
lunar_details_view_button_press_event (GtkTreeView       *tree_view,
                                        GdkEventButton    *event,
                                        LunarDetailsView *details_view)
{
  GtkTreeSelection  *selection;
  GtkTreePath       *path = NULL;
  GtkTreeViewColumn *column;
  GtkTreeViewColumn *name_column;

  /* check if the event is for the bin window */
  if (G_UNLIKELY (event->window != gtk_tree_view_get_bin_window (tree_view)))
    return FALSE;

  /* get the current selection */
  selection = gtk_tree_view_get_selection (tree_view);

  /* get the column showing the filenames */
  name_column = details_view->columns[LUNAR_COLUMN_NAME];

  /* unselect all selected items if the user clicks on an empty area
   * of the treeview and no modifier key is active */
  if ((event->state & gtk_accelerator_get_default_mod_mask ()) == 0
      && !gtk_tree_view_get_path_at_pos (tree_view, event->x, event->y, &path, &column, NULL, NULL))
      gtk_tree_selection_unselect_all (selection);

  /* make sure that rubber banding is enabled */
  gtk_tree_view_set_rubber_banding (tree_view, TRUE);

  /* if the user clicked on a row with the left button */
  if (path != NULL && event->type == GDK_BUTTON_PRESS && event->button == 1)
    {
      GtkTreePath       *cursor_path;

      details_view->button_pressed = TRUE;

      /* grab the tree view */
      gtk_widget_grab_focus (GTK_WIDGET (tree_view));

      gtk_tree_view_get_cursor (tree_view, &cursor_path, NULL);
      if (cursor_path != NULL)
        {
          gtk_tree_path_free (cursor_path);

          if (column != name_column)
            gtk_tree_selection_unselect_all (selection);

          /* do not start rubber banding from the name column */
          if (!gtk_tree_selection_path_is_selected (selection, path)
              && column == name_column)
            {
              /* the clicked row does not have the focus and is not
               * selected, so make it the new selection (start) */
              gtk_tree_selection_unselect_all (selection);
              gtk_tree_selection_select_path (selection, path);

              gtk_tree_path_free (path);

              /* return FALSE to not abort dragging */
              return FALSE;
            }
          gtk_tree_path_free (path);
        }
    }

  /* open the context menu on right clicks */
  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      if (path == NULL)
        {
          /* open the context menu */
          lunar_standard_view_context_menu (LUNAR_STANDARD_VIEW (details_view));
        }
      else
        {
          /* select the clicked path if necessary */
          if (!gtk_tree_selection_path_is_selected (selection, path))
            {
              gtk_tree_selection_unselect_all (selection);
              gtk_tree_selection_select_path (selection, path);
            }

          /* show the context menu */
          lunar_standard_view_queue_popup (LUNAR_STANDARD_VIEW (details_view), event);
          gtk_tree_path_free (path);
        }

      return TRUE;
    }
  else if (event->type == GDK_BUTTON_PRESS && event->button == 2)
    {
      /* determine the path to the item that was middle-clicked */
      if (gtk_tree_view_get_path_at_pos (tree_view, event->x, event->y, &path, NULL, NULL, NULL))
        {
          /* select only the path to the item on which the user clicked */
          gtk_tree_selection_unselect_all (selection);
          gtk_tree_selection_select_path (selection, path);

          /* try to open the path as new window/tab, if possible */
          _lunar_standard_view_open_on_middle_click (LUNAR_STANDARD_VIEW (details_view), path, event->state);

          /* cleanup */
          gtk_tree_path_free (path);
        }

      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_details_view_key_press_event (GtkTreeView       *tree_view,
                                     GdkEventKey       *event,
                                     LunarDetailsView *details_view)
{
  details_view->button_pressed = FALSE;

  /* popup context menu if "Menu" or "<Shift>F10" is pressed */
  if (event->keyval == GDK_KEY_Menu || ((event->state & GDK_SHIFT_MASK) != 0 && event->keyval == GDK_KEY_F10))
    {
      lunar_standard_view_context_menu (LUNAR_STANDARD_VIEW (details_view));
      return TRUE;
    }

  return FALSE;
}



static void
lunar_details_view_row_activated (GtkTreeView       *tree_view,
                                   GtkTreePath       *path,
                                   GtkTreeViewColumn *column,
                                   LunarDetailsView *details_view)
{
  GtkTreeSelection *selection;
  LunarLauncher   *launcher;
  GtkWidget        *window;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  /* be sure to have only the clicked item selected */
  if (details_view->button_pressed)
    {
      selection = gtk_tree_view_get_selection (tree_view);
      gtk_tree_selection_unselect_all (selection);
      gtk_tree_selection_select_path (selection, path);
    }

  window = gtk_widget_get_toplevel (GTK_WIDGET (details_view));
  launcher = lunar_window_get_launcher (LUNAR_WINDOW (window));
  lunar_launcher_activate_selected_files (launcher, LUNAR_LAUNCHER_CHANGE_DIRECTORY, NULL);
}



static gboolean
lunar_details_view_select_cursor_row (GtkTreeView            *tree_view,
                                       gboolean                editing,
                                       LunarDetailsView      *details_view)
{
  /* This function is a work-around to fix bug #2487. The default gtk handler for
   * the "select-cursor-row" signal changes the selection to just the cursor row,
   * which prevents multiple file selections being opened. Thus we bypass the gtk
   * signal handler with g_signal_stop_emission_by_name, and emit the "open" action
   * directly. A better long-term solution would be to fix endo to avoid using the
   * default gtk signal handler there.
   */

  LunarLauncher *launcher;
  GtkWidget      *window;

  _lunar_return_val_if_fail (LUNAR_IS_DETAILS_VIEW (details_view), FALSE);

  g_signal_stop_emission_by_name(tree_view,"select-cursor-row");

  window = gtk_widget_get_toplevel (GTK_WIDGET (details_view));
  launcher = lunar_window_get_launcher (LUNAR_WINDOW (window));
  lunar_launcher_activate_selected_files (launcher, LUNAR_LAUNCHER_CHANGE_DIRECTORY, NULL);

  return TRUE;
}



static void
lunar_details_view_row_changed (GtkTreeView       *tree_view,
                                 GtkTreePath       *path,
                                 GtkTreeViewColumn *column,
                                 LunarDetailsView *details_view)
{
  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  gtk_widget_queue_draw (GTK_WIDGET (details_view));
}



static void
lunar_details_view_columns_changed (LunarColumnModel *column_model,
                                     LunarDetailsView *details_view)
{
  const LunarColumn *column_order;
  LunarColumn        column;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));
  _lunar_return_if_fail (LUNAR_IS_COLUMN_MODEL (column_model));
  _lunar_return_if_fail (details_view->column_model == column_model);

  /* determine the new column order */
  column_order = lunar_column_model_get_column_order (column_model);

  /* apply new order and visibility */
  for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
    {
      /* apply the new visibility for the tree view column */
      gtk_tree_view_column_set_visible (details_view->columns[column], lunar_column_model_get_column_visible (column_model, column));

      /* change the order of the column relative to its predecessor */
      if (G_LIKELY (column > 0))
        {
          gtk_tree_view_move_column_after (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (details_view))),
                                           details_view->columns[column_order[column]],
                                           details_view->columns[column_order[column - 1]]);
        }
    }
}



static gboolean
lunar_details_view_zoom_level_changed_reload_fixed_columns (gpointer data)
{
  LunarDetailsView *details_view = data;
  _lunar_return_val_if_fail (LUNAR_IS_DETAILS_VIEW (details_view), FALSE);

  lunar_details_view_set_fixed_columns (details_view, TRUE);
  details_view->idle_id = 0;
  return FALSE;
}



static void
lunar_details_view_zoom_level_changed (LunarDetailsView *details_view)
{
  LunarColumn column;
  gboolean     fixed_columns_used = FALSE;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  if (details_view->fixed_columns == TRUE)
    fixed_columns_used = TRUE;

  /* Disable fixed column mode during resize, since it can generate graphical glitches */
  if (fixed_columns_used)
      lunar_details_view_set_fixed_columns (details_view, FALSE);

  /* determine the list of tree view columns */
  for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
    {
      /* just queue a resize on this column */
      gtk_tree_view_column_queue_resize (details_view->columns[column]);
    }

  if (fixed_columns_used)
    {
      /* Call when idle to ensure that gtk_tree_view_column_queue_resize got finished */
      details_view->idle_id = gdk_threads_add_idle (lunar_details_view_zoom_level_changed_reload_fixed_columns, details_view);
    }
}



/**
 * lunar_details_view_connect_accelerators:
 * @standard_view : a #LunarStandardView.
 * @accel_group   : a #GtkAccelGroup to be used used for new menu items
 *
 * Connects all accelerators and corresponding default keys of this widget to the global accelerator list
 * The concrete implementation depends on the concrete widget which is implementing this view
 **/
static void
lunar_details_view_connect_accelerators (LunarStandardView *standard_view,
                                          GtkAccelGroup      *accel_group)
{
  LunarDetailsView *details_view = LUNAR_DETAILS_VIEW (standard_view);

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  expidus_gtk_accel_map_add_entries (lunar_details_view_action_entries, G_N_ELEMENTS (lunar_details_view_action_entries));
  expidus_gtk_accel_group_connect_action_entries (accel_group,
                                               lunar_details_view_action_entries,
                                               G_N_ELEMENTS (lunar_details_view_action_entries),
                                               standard_view);
}



/**
 * lunar_details_view_disconnect_accelerators:
 * @standard_view : a #LunarStandardView.
 * @accel_group   : a #GtkAccelGroup to be disconnected
 *
 * Dont listen to the accel keys defined by the action entries any more
 **/
static void
lunar_details_view_disconnect_accelerators (LunarStandardView *standard_view,
                                             GtkAccelGroup      *accel_group)
{
  /* Dont listen to the accel keys defined by the action entries any more */
  expidus_gtk_accel_group_disconnect_action_entries (accel_group,
                                                  lunar_details_view_action_entries,
                                                  G_N_ELEMENTS (lunar_details_view_action_entries));
}



/**
 * lunar_details_view_append_menu_items:
 * @standard_view : a #LunarStandardView.
 * @menu          : the #GtkMenu to add the menu items.
 * @accel_group   : a #GtkAccelGroup to be used used for new menu items
 *
 * Appends widget-specific menu items to a #GtkMenu and connects them to the passed #GtkAccelGroup
 * Implements method 'append_menu_items' of #LunarStandardView
 **/
static void
lunar_details_view_append_menu_items (LunarStandardView *standard_view,
                                       GtkMenu            *menu,
                                       GtkAccelGroup      *accel_group)
{
  LunarDetailsView *details_view = LUNAR_DETAILS_VIEW (standard_view);

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  expidus_gtk_menu_item_new_from_action_entry (get_action_entry (LUNAR_DETAILS_VIEW_ACTION_CONFIGURE_COLUMNS), G_OBJECT (details_view), GTK_MENU_SHELL (menu));
}



/**
 * lunar_details_view_get_fixed_columns:
 * @details_view : a #LunarDetailsView.
 *
 * Returns %TRUE if @details_view uses fixed column widths.
 *
 * Return value: %TRUE if @details_view uses fixed column widths.
 **/
static gboolean
lunar_details_view_get_fixed_columns (LunarDetailsView *details_view)
{
  _lunar_return_val_if_fail (LUNAR_IS_DETAILS_VIEW (details_view), FALSE);
  return details_view->fixed_columns;
}



/**
 * lunar_details_view_set_fixed_columns:
 * @details_view  : a #LunarDetailsView.
 * @fixed_columns : %TRUE to use fixed column widths.
 *
 * Applies @fixed_columns to @details_view.
 **/
static void
lunar_details_view_set_fixed_columns (LunarDetailsView *details_view,
                                       gboolean           fixed_columns)
{
  LunarColumn column;
  gint         width;

  _lunar_return_if_fail (LUNAR_IS_DETAILS_VIEW (details_view));

  /* normalize the value */
  fixed_columns = !!fixed_columns;

  /* check if we have a new value */
  if (G_LIKELY (details_view->fixed_columns != fixed_columns))
    {
      /* apply the new value */
      details_view->fixed_columns = fixed_columns;

      /* disable in reverse order, otherwise graphical glitches can appear*/
      if (!fixed_columns)
        gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (details_view))), FALSE);

      /* apply the new setting to all columns */
      for (column = 0; column < LUNAR_N_VISIBLE_COLUMNS; ++column)
        {
          /* apply the new column mode */
          if (G_LIKELY (fixed_columns))
            {
              /* apply "width" as "fixed-width" for fixed columns mode */
              width = gtk_tree_view_column_get_width (details_view->columns[column]);
              if (G_UNLIKELY (width <= 0))
                width = lunar_column_model_get_column_width (details_view->column_model, column);
              gtk_tree_view_column_set_fixed_width (details_view->columns[column], MAX (width, 1));

              /* set column to fixed width */
              gtk_tree_view_column_set_sizing (details_view->columns[column], GTK_TREE_VIEW_COLUMN_FIXED);
            }
          else
            {
              /* reset column to grow-only mode */
              gtk_tree_view_column_set_sizing (details_view->columns[column], GTK_TREE_VIEW_COLUMN_GROW_ONLY);
            }
        }

      /* for fixed columns mode, we can enable the fixed height
       * mode to improve the performance of the GtkTreeVeiw. */
      if (fixed_columns)
        gtk_tree_view_set_fixed_height_mode (GTK_TREE_VIEW (gtk_bin_get_child (GTK_BIN (details_view))), TRUE);

      /* notify listeners */
      g_object_notify (G_OBJECT (details_view), "fixed-columns");
    }
}
