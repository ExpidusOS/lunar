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

#ifndef __LUNAR_WINDOW_H__
#define __LUNAR_WINDOW_H__

#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-folder.h>
#include <lunar/lunar-launcher.h>

#include <libexpidus1ui/libexpidus1ui.h>

G_BEGIN_DECLS;

typedef struct _LunarWindowClass LunarWindowClass;
typedef struct _LunarWindow      LunarWindow;

#define LUNAR_TYPE_WINDOW            (lunar_window_get_type ())
#define LUNAR_WINDOW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_WINDOW, LunarWindow))
#define LUNAR_WINDOW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_WINDOW, LunarWindowClass))
#define LUNAR_IS_WINDOW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_WINDOW))
#define LUNAR_IS_WINDOW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_WINDOW))
#define LUNAR_WINDOW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_WINDOW, LunarWindowClass))

/* #ExpidusGtkActionEntrys provided by this widget */
typedef enum
{
  LUNAR_WINDOW_ACTION_FILE_MENU,
  LUNAR_WINDOW_ACTION_NEW_TAB,
  LUNAR_WINDOW_ACTION_NEW_WINDOW,
  LUNAR_WINDOW_ACTION_DETACH_TAB,
  LUNAR_WINDOW_ACTION_CLOSE_TAB,
  LUNAR_WINDOW_ACTION_CLOSE_WINDOW,
  LUNAR_WINDOW_ACTION_CLOSE_ALL_WINDOWS,
  LUNAR_WINDOW_ACTION_EDIT_MENU,
  LUNAR_WINDOW_ACTION_PREFERENCES,
  LUNAR_WINDOW_ACTION_VIEW_MENU,
  LUNAR_WINDOW_ACTION_RELOAD,
  LUNAR_WINDOW_ACTION_RELOAD_ALT,
  LUNAR_WINDOW_ACTION_VIEW_SPLIT,
  LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_MENU,
  LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_PATHBAR,
  LUNAR_WINDOW_ACTION_VIEW_LOCATION_SELECTOR_TOOLBAR,
  LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_MENU,
  LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_SHORTCUTS,
  LUNAR_WINDOW_ACTION_VIEW_SIDE_PANE_TREE,
  LUNAR_WINDOW_ACTION_TOGGLE_SIDE_PANE,
  LUNAR_WINDOW_ACTION_VIEW_STATUSBAR,
  LUNAR_WINDOW_ACTION_VIEW_MENUBAR,
  LUNAR_WINDOW_ACTION_SHOW_HIDDEN,
  LUNAR_WINDOW_ACTION_ZOOM_IN,
  LUNAR_WINDOW_ACTION_ZOOM_IN_ALT,
  LUNAR_WINDOW_ACTION_ZOOM_OUT,
  LUNAR_WINDOW_ACTION_ZOOM_RESET,
  LUNAR_WINDOW_ACTION_CLEAR_DIRECTORY_SPECIFIC_SETTINGS,
  LUNAR_WINDOW_ACTION_VIEW_AS_ICONS,
  LUNAR_WINDOW_ACTION_VIEW_AS_DETAILED_LIST,
  LUNAR_WINDOW_ACTION_VIEW_AS_COMPACT_LIST,
  LUNAR_WINDOW_ACTION_GO_MENU,
  LUNAR_WINDOW_ACTION_OPEN_PARENT,
  LUNAR_WINDOW_ACTION_BACK,
  LUNAR_WINDOW_ACTION_BACK_ALT,
  LUNAR_WINDOW_ACTION_FORWARD,
  LUNAR_WINDOW_ACTION_OPEN_FILE_SYSTEM,
  LUNAR_WINDOW_ACTION_OPEN_HOME,
  LUNAR_WINDOW_ACTION_OPEN_DESKTOP,
  LUNAR_WINDOW_ACTION_OPEN_COMPUTER,
  LUNAR_WINDOW_ACTION_OPEN_TRASH,
  LUNAR_WINDOW_ACTION_OPEN_LOCATION,
  LUNAR_WINDOW_ACTION_OPEN_LOCATION_ALT,
  LUNAR_WINDOW_ACTION_OPEN_TEMPLATES,
  LUNAR_WINDOW_ACTION_OPEN_NETWORK,
  LUNAR_WINDOW_ACTION_HELP_MENU,
  LUNAR_WINDOW_ACTION_CONTENTS,
  LUNAR_WINDOW_ACTION_ABOUT,
  LUNAR_WINDOW_ACTION_SWITCH_PREV_TAB,
  LUNAR_WINDOW_ACTION_SWITCH_NEXT_TAB,

} LunarWindowAction;

GType                     lunar_window_get_type                            (void) G_GNUC_CONST;
LunarFile               *lunar_window_get_current_directory               (LunarWindow        *window);
void                      lunar_window_set_current_directory               (LunarWindow        *window,
                                                                             LunarFile          *current_directory);
void                      lunar_window_scroll_to_file                      (LunarWindow        *window,
                                                                             LunarFile          *file,
                                                                             gboolean             select,
                                                                             gboolean             use_align,
                                                                             gfloat               row_align,
                                                                             gfloat               col_align);
gchar                   **lunar_window_get_directories                     (LunarWindow        *window,
                                                                             gint                *active_page);
gboolean                  lunar_window_set_directories                     (LunarWindow        *window,
                                                                             gchar              **uris,
                                                                             gint                 active_page);
void                      lunar_window_update_directories                  (LunarWindow        *window,
                                                                             LunarFile          *old_directory,
                                                                             LunarFile          *new_directory);
void                      lunar_window_notebook_open_new_tab               (LunarWindow        *window,
                                                                             LunarFile          *directory);
gboolean                  lunar_window_has_shortcut_sidepane               (LunarWindow        *window);
GtkWidget*                lunar_window_get_sidepane                        (LunarWindow        *window);
void                      lunar_window_append_menu_item                    (LunarWindow        *window,
                                                                             GtkMenuShell        *menu,
                                                                             LunarWindowAction   action);
LunarLauncher*           lunar_window_get_launcher                        (LunarWindow        *window);
void                      lunar_window_redirect_menu_tooltips_to_statusbar (LunarWindow        *window,
                                                                             GtkMenu             *menu);
const ExpidusGtkActionEntry* lunar_window_get_action_entry                    (LunarWindow        *window,
                                                                             LunarWindowAction   action);
G_END_DECLS;

#endif /* !__LUNAR_WINDOW_H__ */
