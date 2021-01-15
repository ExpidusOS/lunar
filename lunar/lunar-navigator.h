/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_NAVIGATOR_H__
#define __LUNAR_NAVIGATOR_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarNavigatorIface LunarNavigatorIface;
typedef struct _LunarNavigator      LunarNavigator;

#define LUNAR_TYPE_NAVIGATOR           (lunar_navigator_get_type ())
#define LUNAR_NAVIGATOR(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_NAVIGATOR, LunarNavigator))
#define LUNAR_IS_NAVIGATOR(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_NAVIGATOR))
#define LUNAR_NAVIGATOR_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNAR_TYPE_NAVIGATOR, LunarNavigatorIface))

struct _LunarNavigatorIface
{
  GTypeInterface __parent__;

  /* methods */
  LunarFile *(*get_current_directory) (LunarNavigator *navigator);
  void        (*set_current_directory) (LunarNavigator *navigator,
                                        LunarFile      *current_directory);

  /* signals */
  void        (*change_directory)      (LunarNavigator *navigator,
                                        LunarFile      *directory);
  void        (*open_new_tab)          (LunarNavigator *navigator,
                                        LunarFile      *directory);
};

GType       lunar_navigator_get_type              (void) G_GNUC_CONST;

LunarFile *lunar_navigator_get_current_directory (LunarNavigator *navigator);
void        lunar_navigator_set_current_directory (LunarNavigator *navigator,
                                                    LunarFile      *current_directory);

void        lunar_navigator_change_directory      (LunarNavigator *navigator,
                                                    LunarFile      *directory);

void        lunar_navigator_open_new_tab          (LunarNavigator *navigator,
                                                    LunarFile      *directory);

G_END_DECLS;

#endif /* !__LUNAR_NAVIGATOR_H__ */
