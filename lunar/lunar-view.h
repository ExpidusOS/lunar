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

#ifndef __LUNAR_VIEW_H__
#define __LUNAR_VIEW_H__

#include <lunar/lunar-component.h>
#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-navigator.h>

G_BEGIN_DECLS;

typedef struct _LunarViewIface LunarViewIface;
typedef struct _LunarView      LunarView;

#define LUNAR_TYPE_VIEW            (lunar_view_get_type ())
#define LUNAR_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_VIEW, LunarView))
#define LUNAR_IS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_VIEW))
#define LUNAR_VIEW_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNAR_TYPE_VIEW, LunarViewIface))

struct _LunarViewIface
{
  GTypeInterface __parent__;

  /* virtual methods */
  gboolean        (*get_loading)        (LunarView     *view);
  const gchar    *(*get_statusbar_text) (LunarView     *view);

  gboolean        (*get_show_hidden)    (LunarView     *view);
  void            (*set_show_hidden)    (LunarView     *view,
                                         gboolean        show_hidden);

  LunarZoomLevel (*get_zoom_level)     (LunarView     *view);
  void            (*set_zoom_level)     (LunarView     *view,
                                         LunarZoomLevel zoom_level);
  void            (*reset_zoom_level)   (LunarView     *view);

  void            (*reload)             (LunarView     *view,
                                         gboolean        reload_info);

  gboolean        (*get_visible_range)  (LunarView     *view,
                                         LunarFile    **start_file,
                                         LunarFile    **end_file);

  void            (*scroll_to_file)     (LunarView     *view,
                                         LunarFile     *file,
                                         gboolean        select,
                                         gboolean        use_align,
                                         gfloat          row_align,
                                         gfloat          col_align);
  GList*          (*get_selected_files) (LunarView     *view);
  void            (*set_selected_files) (LunarView     *view,
                                         GList          *path_list);
};

GType           lunar_view_get_type            (void) G_GNUC_CONST;

gboolean        lunar_view_get_loading         (LunarView     *view);
const gchar    *lunar_view_get_statusbar_text  (LunarView     *view);

gboolean        lunar_view_get_show_hidden     (LunarView     *view);
void            lunar_view_set_show_hidden     (LunarView     *view,
                                                 gboolean        show_hidden);

LunarZoomLevel lunar_view_get_zoom_level      (LunarView     *view);
void            lunar_view_set_zoom_level      (LunarView     *view,
                                                 LunarZoomLevel zoom_level);
void            lunar_view_reset_zoom_level    (LunarView     *view);

void            lunar_view_reload              (LunarView     *view,
                                                 gboolean        reload_info);

gboolean        lunar_view_get_visible_range   (LunarView     *view,
                                                 LunarFile    **start_file,
                                                 LunarFile    **end_file);

void            lunar_view_scroll_to_file      (LunarView     *view,
                                                 LunarFile     *file,
                                                 gboolean        select_file,
                                                 gboolean        use_align,
                                                 gfloat          row_align,
                                                 gfloat          col_align);
GList*          lunar_view_get_selected_files  (LunarView     *view);
void            lunar_view_set_selected_files  (LunarView     *view,
                                                 GList          *path_list);

G_END_DECLS;

#endif /* !__LUNAR_VIEW_H__ */
