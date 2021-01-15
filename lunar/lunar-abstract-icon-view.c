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

#include <lunar/lunar-abstract-icon-view.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-gtk-extensions.h>
#include <lunar/lunar-launcher.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-window.h>



static void         lunar_abstract_icon_view_style_set               (GtkWidget                    *widget,
                                                                       GtkStyle                     *previous_style);
static GList       *lunar_abstract_icon_view_get_selected_items      (LunarStandardView           *standard_view);
static void         lunar_abstract_icon_view_select_all              (LunarStandardView           *standard_view);
static void         lunar_abstract_icon_view_unselect_all            (LunarStandardView           *standard_view);
static void         lunar_abstract_icon_view_selection_invert        (LunarStandardView           *standard_view);
static void         lunar_abstract_icon_view_select_path             (LunarStandardView           *standard_view,
                                                                       GtkTreePath                  *path);
static void         lunar_abstract_icon_view_set_cursor              (LunarStandardView           *standard_view,
                                                                       GtkTreePath                  *path,
                                                                       gboolean                      start_editing);
static void         lunar_abstract_icon_view_scroll_to_path          (LunarStandardView           *standard_view,
                                                                       GtkTreePath                  *path,
                                                                       gboolean                      use_align,
                                                                       gfloat                        row_align,
                                                                       gfloat                        col_align);
static GtkTreePath *lunar_abstract_icon_view_get_path_at_pos         (LunarStandardView           *standard_view,
                                                                       gint                          x,
                                                                       gint                          y);
static gboolean     lunar_abstract_icon_view_get_visible_range       (LunarStandardView           *standard_view,
                                                                       GtkTreePath                 **start_path,
                                                                       GtkTreePath                 **end_path);
static void         lunar_abstract_icon_view_highlight_path          (LunarStandardView           *standard_view,
                                                                       GtkTreePath                  *path);
static void         lunar_abstract_icon_view_notify_model            (EndoIconView                  *view,
                                                                       GParamSpec                   *pspec,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static gboolean     lunar_abstract_icon_view_button_press_event      (EndoIconView                  *view,
                                                                       GdkEventButton               *event,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static gboolean     lunar_abstract_icon_view_button_release_event    (EndoIconView                  *view,
                                                                       GdkEventButton               *event,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static gboolean     lunar_abstract_icon_view_draw                    (EndoIconView                  *view,
                                                                       cairo_t                      *cr,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static gboolean     lunar_abstract_icon_view_key_press_event         (EndoIconView                  *view,
                                                                       GdkEventKey                  *event,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static gboolean     lunar_abstract_icon_view_motion_notify_event     (EndoIconView                  *view,
                                                                       GdkEventMotion               *event,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static void         lunar_abstract_icon_view_item_activated          (EndoIconView                  *view,
                                                                       GtkTreePath                  *path,
                                                                       LunarAbstractIconView       *abstract_icon_view);
static void         lunar_abstract_icon_view_zoom_level_changed      (LunarAbstractIconView       *abstract_icon_view);



struct _LunarAbstractIconViewPrivate
{
  /* mouse gesture support */
  gint   gesture_start_x;
  gint   gesture_start_y;
  gint   gesture_current_x;
  gint   gesture_current_y;
  gulong gesture_expose_id;
  gulong gesture_motion_id;
  gulong gesture_release_id;

  gboolean button_pressed;
};


G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (LunarAbstractIconView, lunar_abstract_icon_view, LUNAR_TYPE_STANDARD_VIEW)



static void
lunar_abstract_icon_view_class_init (LunarAbstractIconViewClass *klass)
{
  LunarStandardViewClass *lunarstandard_view_class;
  GtkWidgetClass          *gtkwidget_class;

  gtkwidget_class = GTK_WIDGET_CLASS (klass);
  gtkwidget_class->style_set = lunar_abstract_icon_view_style_set;

  lunarstandard_view_class = LUNAR_STANDARD_VIEW_CLASS (klass);
  lunarstandard_view_class->get_selected_items = lunar_abstract_icon_view_get_selected_items;
  lunarstandard_view_class->select_all = lunar_abstract_icon_view_select_all;
  lunarstandard_view_class->unselect_all = lunar_abstract_icon_view_unselect_all;
  lunarstandard_view_class->selection_invert = lunar_abstract_icon_view_selection_invert;
  lunarstandard_view_class->select_path = lunar_abstract_icon_view_select_path;
  lunarstandard_view_class->set_cursor = lunar_abstract_icon_view_set_cursor;
  lunarstandard_view_class->scroll_to_path = lunar_abstract_icon_view_scroll_to_path;
  lunarstandard_view_class->get_path_at_pos = lunar_abstract_icon_view_get_path_at_pos;
  lunarstandard_view_class->get_visible_range = lunar_abstract_icon_view_get_visible_range;
  lunarstandard_view_class->highlight_path = lunar_abstract_icon_view_highlight_path;

  /**
   * LunarAbstractIconView:column-spacing:
   *
   * The additional space inserted between columns in the
   * icon views.
   **/
  gtk_widget_class_install_style_property (gtkwidget_class,
                                           g_param_spec_int ("column-spacing",
                                                             "column-spacing",
                                                             "column-spacing",
                                                             0, G_MAXINT, 6,
                                                             ENDO_PARAM_READABLE));

  /**
   * LunarAbstractIconView:row-spacing:
   *
   * The additional space inserted between rows in the
   * icon views.
   **/
  gtk_widget_class_install_style_property (gtkwidget_class,
                                           g_param_spec_int ("row-spacing",
                                                             "row-spacing",
                                                             "row-spacing",
                                                             0, G_MAXINT, 6,
                                                             ENDO_PARAM_READABLE));
}



static void
lunar_abstract_icon_view_init (LunarAbstractIconView *abstract_icon_view)
{
  GtkWidget *view;

  /* connect private instance data */
  abstract_icon_view->priv = lunar_abstract_icon_view_get_instance_private (abstract_icon_view);

  /* stay informed about zoom-level changes, so we can force a re-layout on the abstract_icon view */
  g_signal_connect (G_OBJECT (abstract_icon_view), "notify::zoom-level", G_CALLBACK (lunar_abstract_icon_view_zoom_level_changed), NULL);

  /* create the real view */
  view = endo_icon_view_new ();
  g_signal_connect (G_OBJECT (view), "notify::model", G_CALLBACK (lunar_abstract_icon_view_notify_model), abstract_icon_view);
  g_signal_connect (G_OBJECT (view), "button-press-event", G_CALLBACK (lunar_abstract_icon_view_button_press_event), abstract_icon_view);
  g_signal_connect (G_OBJECT (view), "key-press-event", G_CALLBACK (lunar_abstract_icon_view_key_press_event), abstract_icon_view);
  g_signal_connect (G_OBJECT (view), "item-activated", G_CALLBACK (lunar_abstract_icon_view_item_activated), abstract_icon_view);
  g_signal_connect_swapped (G_OBJECT (view), "selection-changed", G_CALLBACK (lunar_standard_view_selection_changed), abstract_icon_view);
  gtk_container_add (GTK_CONTAINER (abstract_icon_view), view);
  gtk_widget_show (view);

  /* initialize the abstract icon view properties */
  endo_icon_view_set_enable_search (ENDO_ICON_VIEW (view), TRUE);
  endo_icon_view_set_selection_mode (ENDO_ICON_VIEW (view), GTK_SELECTION_MULTIPLE);

  /* add the abstract icon renderer */
  g_object_set (G_OBJECT (LUNAR_STANDARD_VIEW (abstract_icon_view)->icon_renderer), "follow-state", TRUE, NULL);
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (view), LUNAR_STANDARD_VIEW (abstract_icon_view)->icon_renderer, FALSE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (view), LUNAR_STANDARD_VIEW (abstract_icon_view)->icon_renderer,
                                 "file", LUNAR_COLUMN_FILE);

  /* add the name renderer */
  /*FIXME text prelit*/
  /*g_object_set (G_OBJECT (LUNAR_STANDARD_VIEW (abstract_icon_view)->name_renderer), "follow-state", TRUE, NULL);*/
  gtk_cell_layout_pack_start (GTK_CELL_LAYOUT (view), LUNAR_STANDARD_VIEW (abstract_icon_view)->name_renderer, TRUE);
  gtk_cell_layout_add_attribute (GTK_CELL_LAYOUT (view), LUNAR_STANDARD_VIEW (abstract_icon_view)->name_renderer,
                                 "text", LUNAR_COLUMN_NAME);

  /* update the icon view on size-allocate events */
  /* TODO: issue not reproducible anymore as of gtk 3.24.18
   * we can probably remove this in the future. */
  g_signal_connect_swapped (G_OBJECT (abstract_icon_view), "size-allocate",
                            G_CALLBACK (gtk_widget_queue_resize), view);
}



static void
lunar_abstract_icon_view_style_set (GtkWidget *widget,
                                     GtkStyle  *previous_style)
{
  gint column_spacing;
  gint row_spacing;

  /* determine the column/row spacing from the style */
  gtk_widget_style_get (widget, "column-spacing", &column_spacing, "row-spacing", &row_spacing, NULL);

  /* apply the column/row spacing to the icon view */
  endo_icon_view_set_column_spacing (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (widget))), column_spacing);
  endo_icon_view_set_row_spacing (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (widget))), row_spacing);

  /* call the parent handler */
  (*GTK_WIDGET_CLASS (lunar_abstract_icon_view_parent_class)->style_set) (widget, previous_style);
}



static GList*
lunar_abstract_icon_view_get_selected_items (LunarStandardView *standard_view)
{
  return endo_icon_view_get_selected_items (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
}



static void
lunar_abstract_icon_view_select_all (LunarStandardView *standard_view)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));
  endo_icon_view_select_all (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
}



static void
lunar_abstract_icon_view_unselect_all (LunarStandardView *standard_view)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));
  endo_icon_view_unselect_all (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
}



static void
lunar_abstract_icon_view_selection_invert (LunarStandardView *standard_view)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));
  endo_icon_view_selection_invert (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))));
}



static void
lunar_abstract_icon_view_select_path (LunarStandardView *standard_view,
                                       GtkTreePath        *path)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));
  endo_icon_view_select_path (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), path);
}



static void
lunar_abstract_icon_view_set_cursor (LunarStandardView *standard_view,
                                      GtkTreePath        *path,
                                      gboolean            start_editing)
{
  GtkCellRendererMode mode;

  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));

  /* make sure the name renderer is editable */
  g_object_get ( G_OBJECT (standard_view->name_renderer), "mode", &mode, NULL);
  g_object_set ( G_OBJECT (standard_view->name_renderer), "mode", GTK_CELL_RENDERER_MODE_EDITABLE, NULL);

  /* tell the abstract_icon view to start editing the given item */
  endo_icon_view_set_cursor (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), path, standard_view->name_renderer, start_editing);

  /* reset the name renderer mode */
  g_object_set (G_OBJECT (standard_view->name_renderer), "mode", mode, NULL);
}



static void
lunar_abstract_icon_view_scroll_to_path (LunarStandardView *standard_view,
                                          GtkTreePath        *path,
                                          gboolean            use_align,
                                          gfloat              row_align,
                                          gfloat              col_align)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));
  endo_icon_view_scroll_to_path (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), path, use_align, row_align, col_align);
}



static GtkTreePath*
lunar_abstract_icon_view_get_path_at_pos (LunarStandardView *standard_view,
                                           gint                x,
                                           gint                y)
{
  _lunar_return_val_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view), NULL);
  return endo_icon_view_get_path_at_pos (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), x, y);
}



static gboolean
lunar_abstract_icon_view_get_visible_range (LunarStandardView *standard_view,
                                             GtkTreePath       **start_path,
                                             GtkTreePath       **end_path)
{
  _lunar_return_val_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view), FALSE);
  return endo_icon_view_get_visible_range (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), start_path, end_path);
}



static void
lunar_abstract_icon_view_highlight_path (LunarStandardView *standard_view,
                                          GtkTreePath        *path)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (standard_view));
  endo_icon_view_set_drag_dest_item (ENDO_ICON_VIEW (gtk_bin_get_child (GTK_BIN (standard_view))), path, ENDO_ICON_VIEW_DROP_INTO);
}



static const ExpidusGtkActionEntry*
lunar_abstract_icon_view_gesture_action (LunarAbstractIconView *abstract_icon_view, GtkTextDirection direction)
{
  GtkWidget *window;

  window = gtk_widget_get_toplevel (GTK_WIDGET (abstract_icon_view));
  if (abstract_icon_view->priv->gesture_start_y - abstract_icon_view->priv->gesture_current_y > 40
      && ABS (abstract_icon_view->priv->gesture_start_x - abstract_icon_view->priv->gesture_current_x) < 40)
    {
      return lunar_window_get_action_entry (LUNAR_WINDOW (window), LUNAR_WINDOW_ACTION_OPEN_PARENT);
    }
  else if (abstract_icon_view->priv->gesture_start_x - abstract_icon_view->priv->gesture_current_x > 40
      && ABS (abstract_icon_view->priv->gesture_start_y - abstract_icon_view->priv->gesture_current_y) < 40)
    {
      if (direction == GTK_TEXT_DIR_LTR || direction == GTK_TEXT_DIR_NONE)
        return lunar_window_get_action_entry (LUNAR_WINDOW (window), LUNAR_WINDOW_ACTION_BACK);
      else
        return lunar_window_get_action_entry (LUNAR_WINDOW (window), LUNAR_WINDOW_ACTION_FORWARD);
    }
  else if (abstract_icon_view->priv->gesture_current_x - abstract_icon_view->priv->gesture_start_x > 40
      && ABS (abstract_icon_view->priv->gesture_start_y - abstract_icon_view->priv->gesture_current_y) < 40)
    {
      if (direction == GTK_TEXT_DIR_LTR || direction == GTK_TEXT_DIR_NONE)
        return lunar_window_get_action_entry (LUNAR_WINDOW (window), LUNAR_WINDOW_ACTION_FORWARD);
      else
        return lunar_window_get_action_entry (LUNAR_WINDOW (window), LUNAR_WINDOW_ACTION_BACK);
    }
  else if (abstract_icon_view->priv->gesture_current_y - abstract_icon_view->priv->gesture_start_y > 40
      && ABS (abstract_icon_view->priv->gesture_start_x - abstract_icon_view->priv->gesture_current_x) < 40)
    {
      return lunar_window_get_action_entry (LUNAR_WINDOW (window), LUNAR_WINDOW_ACTION_RELOAD);
    }
  return NULL;
}



static void
lunar_abstract_icon_view_notify_model (EndoIconView            *view,
                                        GParamSpec             *pspec,
                                        LunarAbstractIconView *abstract_icon_view)
{
  /* We need to set the search column here, as EndoIconView resets it
   * whenever a new model is set.
   */
  endo_icon_view_set_search_column (view, LUNAR_COLUMN_NAME);
}



static gboolean
lunar_abstract_icon_view_button_press_event (EndoIconView            *view,
                                              GdkEventButton         *event,
                                              LunarAbstractIconView *abstract_icon_view)
{
  GtkTreePath       *path;

  abstract_icon_view->priv->button_pressed = TRUE;

  if (event->type == GDK_BUTTON_PRESS && event->button == 3)
    {
      /* open the context menu on right clicks */
      if (endo_icon_view_get_item_at_pos (view, event->x, event->y, &path, NULL))
        {
          /* select the path on which the user clicked if not selected yet */
          if (!endo_icon_view_path_is_selected (view, path))
            {
              /* we don't unselect all other items if Control is active */
              if ((event->state & GDK_CONTROL_MASK) == 0)
                endo_icon_view_unselect_all (view);
              endo_icon_view_select_path (view, path);
            }
          gtk_tree_path_free (path);

          /* queue the menu popup */
          lunar_standard_view_queue_popup (LUNAR_STANDARD_VIEW (abstract_icon_view), event);
        }
      else if ((event->state & gtk_accelerator_get_default_mod_mask ()) == 0)
        {
          /* user clicked on an empty area, so we unselect everything
           * to make sure that the folder context menu is opened.
           */
          endo_icon_view_unselect_all (view);

          /* open the context menu */
          lunar_standard_view_context_menu (LUNAR_STANDARD_VIEW (abstract_icon_view));
        }

      return TRUE;
    }
  else if (event->type == GDK_BUTTON_PRESS && event->button == 2)
    {
      /* unselect all currently selected items */
      endo_icon_view_unselect_all (view);

      /* determine the path to the item that was middle-clicked */
      if (endo_icon_view_get_item_at_pos (view, event->x, event->y, &path, NULL))
        {
          /* select only the path to the item on which the user clicked */
          endo_icon_view_select_path (view, path);

          /* try to open the path as new window/tab, if possible */
          _lunar_standard_view_open_on_middle_click (LUNAR_STANDARD_VIEW (abstract_icon_view), path, event->state);

          /* cleanup */
          gtk_tree_path_free (path);
        }
      else if (event->type == GDK_BUTTON_PRESS)
        {
          abstract_icon_view->priv->gesture_start_x = abstract_icon_view->priv->gesture_current_x = event->x;
          abstract_icon_view->priv->gesture_start_y = abstract_icon_view->priv->gesture_current_y = event->y;
          abstract_icon_view->priv->gesture_expose_id = g_signal_connect_after (G_OBJECT (view), "draw",
                                                                                G_CALLBACK (lunar_abstract_icon_view_draw),
                                                                                G_OBJECT (abstract_icon_view));
          abstract_icon_view->priv->gesture_motion_id = g_signal_connect (G_OBJECT (view), "motion-notify-event",
                                                                          G_CALLBACK (lunar_abstract_icon_view_motion_notify_event),
                                                                          G_OBJECT (abstract_icon_view));
          abstract_icon_view->priv->gesture_release_id = g_signal_connect (G_OBJECT (view), "button-release-event",
                                                                           G_CALLBACK (lunar_abstract_icon_view_button_release_event),
                                                                           G_OBJECT (abstract_icon_view));
        }

      /* don't run the default handler here */
      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_abstract_icon_view_button_release_event (EndoIconView            *view,
                                                GdkEventButton         *event,
                                                LunarAbstractIconView *abstract_icon_view)
{
  const ExpidusGtkActionEntry *action_entry;
  GtkWidget                *window;

  _lunar_return_val_if_fail (ENDO_IS_ICON_VIEW (view), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (abstract_icon_view), FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_expose_id > 0, FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_motion_id > 0, FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_release_id > 0, FALSE);

  window = gtk_widget_get_toplevel (GTK_WIDGET (abstract_icon_view));

  /* unregister the "expose-event" handler */
  g_signal_handler_disconnect (G_OBJECT (view), abstract_icon_view->priv->gesture_expose_id);
  abstract_icon_view->priv->gesture_expose_id = 0;

  /* unregister the "motion-notify-event" handler */
  g_signal_handler_disconnect (G_OBJECT (view), abstract_icon_view->priv->gesture_motion_id);
  abstract_icon_view->priv->gesture_motion_id = 0;

  /* unregister the "button-release-event" handler */
  g_signal_handler_disconnect (G_OBJECT (view), abstract_icon_view->priv->gesture_release_id);
  abstract_icon_view->priv->gesture_release_id = 0;

  /* execute the related callback  (if any) */
  action_entry = lunar_abstract_icon_view_gesture_action (abstract_icon_view, gtk_widget_get_direction (window));
  if (G_LIKELY (action_entry != NULL))
    ((void(*)(GtkWindow*))action_entry->callback)(GTK_WINDOW (window));

  /* redraw the abstract_icon view */
  gtk_widget_queue_draw (GTK_WIDGET (view));

  return FALSE;
}



static gboolean
lunar_abstract_icon_view_draw (EndoIconView            *view,
                                cairo_t                *cr,
                                LunarAbstractIconView *abstract_icon_view)
{
  const ExpidusGtkActionEntry *action_entry = NULL;
  GdkPixbuf                *gesture_icon = NULL;
  gint                      x, y;

  _lunar_return_val_if_fail (ENDO_IS_ICON_VIEW (view), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (abstract_icon_view), FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_expose_id > 0, FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_motion_id > 0, FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_release_id > 0, FALSE);

  /* shade the abstract_icon view content while performing mouse gestures */
  cairo_set_source_rgba (cr, 1, 1, 1, 0.7);
  cairo_paint (cr);

  /* determine the gesture action.
   * GtkTextDirection needs to be fixed, so that e.g. the left arrow will always be shown on the left.
   * Even if the window uses GTK_TEXT_DIR_RTL.
   * */
  action_entry = lunar_abstract_icon_view_gesture_action (abstract_icon_view, GTK_TEXT_DIR_NONE);
  if (G_LIKELY (action_entry != NULL))
    {
      gesture_icon = gtk_icon_theme_load_icon (gtk_icon_theme_get_default(),
                                               action_entry->menu_item_icon_name,
                                               32,
                                               GTK_ICON_LOOKUP_FORCE_SIZE,
                                               NULL);
      /* draw the rendered icon */
      if (G_LIKELY (gesture_icon != NULL))
        {
          /* x/y position of the icon */
          x = abstract_icon_view->priv->gesture_start_x - gdk_pixbuf_get_width (gesture_icon) / 2;
          y = abstract_icon_view->priv->gesture_start_y - gdk_pixbuf_get_height (gesture_icon) / 2;

          /* render the icon into the abstract_icon view window */
          gdk_cairo_set_source_pixbuf (cr, gesture_icon, x, y);
          cairo_rectangle (cr, x, y,
                           gdk_pixbuf_get_width (gesture_icon),
                           gdk_pixbuf_get_height (gesture_icon));
          cairo_fill (cr);

          /* release the stock abstract_icon */
          g_object_unref (G_OBJECT (gesture_icon));
        }
    }

  return FALSE;
}



static gboolean
lunar_abstract_icon_view_key_press_event (EndoIconView            *view,
                                           GdkEventKey            *event,
                                           LunarAbstractIconView *abstract_icon_view)
{
  abstract_icon_view->priv->button_pressed = FALSE;

  /* popup context menu if "Menu" or "<Shift>F10" is pressed */
  if (event->keyval == GDK_KEY_Menu || ((event->state & GDK_SHIFT_MASK) != 0 && event->keyval == GDK_KEY_F10))
    {
      lunar_standard_view_context_menu (LUNAR_STANDARD_VIEW (abstract_icon_view));
      return TRUE;
    }

  return FALSE;
}



static gboolean
lunar_abstract_icon_view_motion_notify_event (EndoIconView            *view,
                                               GdkEventMotion         *event,
                                               LunarAbstractIconView *abstract_icon_view)
{
  GdkRectangle area;

  _lunar_return_val_if_fail (ENDO_IS_ICON_VIEW (view), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (abstract_icon_view), FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_expose_id > 0, FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_motion_id > 0, FALSE);
  _lunar_return_val_if_fail (abstract_icon_view->priv->gesture_release_id > 0, FALSE);

  /* schedule a complete redraw on the first motion event */
  if (abstract_icon_view->priv->gesture_current_x == abstract_icon_view->priv->gesture_start_x
      && abstract_icon_view->priv->gesture_current_y == abstract_icon_view->priv->gesture_start_y)
    {
      gtk_widget_queue_draw (GTK_WIDGET (view));
    }
  else
    {
      /* otherwise, just redraw the action abstract_icon area */
      gtk_icon_size_lookup (GTK_ICON_SIZE_DND, &area.width, &area.height);
      area.x = abstract_icon_view->priv->gesture_start_x - area.width / 2;
      area.y = abstract_icon_view->priv->gesture_start_y - area.height / 2;
      gdk_window_invalidate_rect (event->window, &area, TRUE);
    }

  /* update the current gesture position */
  abstract_icon_view->priv->gesture_current_x = event->x;
  abstract_icon_view->priv->gesture_current_y = event->y;

  /* don't execute the default motion notify handler */
  return TRUE;
}



static void
lunar_abstract_icon_view_item_activated (EndoIconView            *view,
                                          GtkTreePath            *path,
                                          LunarAbstractIconView *abstract_icon_view)
{
  GtkWidget *window;

  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (abstract_icon_view));

  /* be sure to have only the clicked item selected */
  if (abstract_icon_view->priv->button_pressed)
    {
      endo_icon_view_unselect_all (view);
      endo_icon_view_select_path (view, path);
    }

  window = gtk_widget_get_toplevel (GTK_WIDGET (abstract_icon_view));
  lunar_launcher_activate_selected_files (lunar_window_get_launcher (LUNAR_WINDOW (window)), LUNAR_LAUNCHER_CHANGE_DIRECTORY, NULL);
}



static void
lunar_abstract_icon_view_zoom_level_changed (LunarAbstractIconView *abstract_icon_view)
{
  _lunar_return_if_fail (LUNAR_IS_ABSTRACT_ICON_VIEW (abstract_icon_view));

  /* we use the same trick as with LunarDetailsView here, simply because its simple :-) */
  gtk_cell_layout_set_cell_data_func (GTK_CELL_LAYOUT (gtk_bin_get_child (GTK_BIN (abstract_icon_view))),
                                      LUNAR_STANDARD_VIEW (abstract_icon_view)->icon_renderer,
                                      NULL, NULL, NULL);
}
