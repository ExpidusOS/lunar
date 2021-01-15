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

#ifndef __LUNAR_STANDARD_VIEW_H__
#define __LUNAR_STANDARD_VIEW_H__

#include <lunar/lunar-clipboard-manager.h>
#include <lunar/lunar-history.h>
#include <lunar/lunar-icon-factory.h>
#include <lunar/lunar-list-model.h>
#include <lunar/lunar-preferences.h>
#include <lunar/lunar-view.h>

G_BEGIN_DECLS;

typedef struct _LunarStandardViewPrivate LunarStandardViewPrivate;
typedef struct _LunarStandardViewClass   LunarStandardViewClass;
typedef struct _LunarStandardView        LunarStandardView;

#define LUNAR_TYPE_STANDARD_VIEW             (lunar_standard_view_get_type ())
#define LUNAR_STANDARD_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_STANDARD_VIEW, LunarStandardView))
#define LUNAR_STANDARD_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_STANDARD_VIEW, LunarStandardViewClass))
#define LUNAR_IS_STANDARD_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_STANDARD_VIEW))
#define LUNAR_IS_STANDARD_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_STANDARD_VIEW))
#define LUNAR_STANDARD_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_STANDARD_VIEW, LunarStandardViewClass))

/* #ExpidusGtkActionEntrys provided by this widget */
typedef enum
{
  LUNAR_STANDARD_VIEW_ACTION_SELECT_ALL_FILES,
  LUNAR_STANDARD_VIEW_ACTION_SELECT_BY_PATTERN,
  LUNAR_STANDARD_VIEW_ACTION_INVERT_SELECTION,
  LUNAR_STANDARD_VIEW_ACTION_UNSELECT_ALL_FILES,
  LUNAR_STANDARD_VIEW_ACTION_ARRANGE_ITEMS_MENU,
  LUNAR_STANDARD_VIEW_ACTION_SORT_BY_NAME,
  LUNAR_STANDARD_VIEW_ACTION_SORT_BY_SIZE,
  LUNAR_STANDARD_VIEW_ACTION_SORT_BY_TYPE,
  LUNAR_STANDARD_VIEW_ACTION_SORT_BY_MTIME,
  LUNAR_STANDARD_VIEW_ACTION_SORT_ASCENDING,
  LUNAR_STANDARD_VIEW_ACTION_SORT_DESCENDING,
  LUNAR_STANDARD_VIEW_ACTION_SORT_ORDER_TOGGLE,
  
} LunarStandardViewAction;

struct _LunarStandardViewClass
{
  GtkScrolledWindowClass __parent__;

  /* Returns the list of currently selected GtkTreePath's, where
   * both the list and the items are owned by the caller. */
  GList       *(*get_selected_items)    (LunarStandardView *standard_view);

  /* Selects all items in the view */
  void         (*select_all)            (LunarStandardView *standard_view);

  /* Unselects all items in the view */
  void         (*unselect_all)          (LunarStandardView *standard_view);

  /* Invert selection in the view */
  void         (*selection_invert)      (LunarStandardView *standard_view);

  /* Selects the given item */
  void         (*select_path)           (LunarStandardView *standard_view,
                                         GtkTreePath        *path);

  /* Called by the LunarStandardView class to let derived class
   * place the cursor on the item/row referred to by path. If
   * start_editing is TRUE, the derived class should also start
   * editing that item/row.
   */
  void         (*set_cursor)            (LunarStandardView *standard_view,
                                         GtkTreePath        *path,
                                         gboolean            start_editing);

  /* Called by the LunarStandardView class to let derived class
   * scroll the view to the given path.
   */
  void         (*scroll_to_path)        (LunarStandardView *standard_view,
                                         GtkTreePath        *path,
                                         gboolean            use_align,
                                         gfloat              row_align,
                                         gfloat              col_align);

  /* Returns the path at the given position or NULL if no item/row
   * is located at that coordinates. The path is freed by the caller.
   */
  GtkTreePath *(*get_path_at_pos)       (LunarStandardView *standard_view,
                                         gint                x,
                                         gint                y);

  /* Returns the visible range */
  gboolean     (*get_visible_range)     (LunarStandardView *standard_view,
                                         GtkTreePath       **start_path,
                                         GtkTreePath       **end_path);

  /* Sets the item/row that is highlighted for feedback. NULL is
   * passed for path to disable the highlighting.
   */
  void         (*highlight_path)        (LunarStandardView  *standard_view,
                                         GtkTreePath         *path);

  /* external signals */
  void         (*start_open_location)   (LunarStandardView *standard_view,
                                         const gchar        *initial_text);

  /* Appends view-specific menu items to the given menu */
  void        (*append_menu_items)      (LunarStandardView *standard_view, GtkMenu *menu, GtkAccelGroup *accel_group);

  /* Connects view-specific accelerators to the given accelGroup */
  void        (*connect_accelerators)    (LunarStandardView *standard_view, GtkAccelGroup *accel_group);

  /* Disconnects view-specific accelerators to the given accelGroup */
  void        (*disconnect_accelerators)    (LunarStandardView *standard_view, GtkAccelGroup *accel_group);

  /* Internal action signals */
  gboolean     (*delete_selected_files) (LunarStandardView *standard_view);

  /* The name of the property in LunarPreferences, that determines
   * the last (and default) zoom-level for the view classes (i.e. in
   * case of LunarIconView, this is "last-icon-view-zoom-level").
   */
  const gchar *zoom_level_property_name;
};

struct _LunarStandardView
{
  GtkScrolledWindow __parent__;

  LunarPreferences         *preferences;

  LunarListModel           *model;

  LunarIconFactory         *icon_factory;
  GtkCellRenderer           *icon_renderer;
  GtkCellRenderer           *name_renderer;

  EndoBinding                *loading_binding;
  gboolean                   loading;
  GtkAccelGroup             *accel_group;

  LunarStandardViewPrivate *priv;
};

GType          lunar_standard_view_get_type              (void) G_GNUC_CONST;

void           lunar_standard_view_context_menu          (LunarStandardView       *standard_view);
void           lunar_standard_view_queue_popup           (LunarStandardView       *standard_view,
                                                           GdkEventButton           *event);
void           lunar_standard_view_selection_changed     (LunarStandardView       *standard_view);
void           lunar_standard_view_set_history           (LunarStandardView       *standard_view,
                                                           LunarHistory            *history);
LunarHistory *lunar_standard_view_get_history           (LunarStandardView       *standard_view);
LunarHistory *lunar_standard_view_copy_history          (LunarStandardView       *standard_view);
void           lunar_standard_view_append_menu_items     (LunarStandardView       *standard_view,
                                                           GtkMenu                  *menu,
                                                           GtkAccelGroup            *accel_group);
GtkWidget     *lunar_standard_view_append_menu_item      (LunarStandardView       *standard_view,
                                                           GtkMenu                  *menu,
                                                           LunarStandardViewAction  action);
void           _lunar_standard_view_open_on_middle_click (LunarStandardView       *standard_view,
                                                           GtkTreePath              *tree_path,
                                                           guint                     event_state);

G_END_DECLS;

#endif /* !__LUNAR_STANDARD_VIEW_H__ */
