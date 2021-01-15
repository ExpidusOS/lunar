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

#ifndef __LUNAR_SIDE_PANE_H__
#define __LUNAR_SIDE_PANE_H__

#include <lunar/lunar-component.h>

G_BEGIN_DECLS;

typedef struct _LunarSidePaneIface LunarSidePaneIface;
typedef struct _LunarSidePane      LunarSidePane;

#define LUNAR_TYPE_SIDE_PANE           (lunar_side_pane_get_type ())
#define LUNAR_SIDE_PANE(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_SIDE_PANE, LunarSidePane))
#define LUNAR_IS_SIDE_PANE(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_SIDE_PANE))
#define LUNAR_SIDE_PANE_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNAR_TYPE_SIDE_PANE, LunarSidePaneIface))

struct _LunarSidePaneIface
{
  GTypeInterface __parent__;

  /* virtual methods */
  gboolean (*get_show_hidden) (LunarSidePane *side_pane);
  void     (*set_show_hidden) (LunarSidePane *side_pane,
                               gboolean        show_hidden);
};

GType    lunar_side_pane_get_type        (void) G_GNUC_CONST;

gboolean lunar_side_pane_get_show_hidden (LunarSidePane *side_pane);
void     lunar_side_pane_set_show_hidden (LunarSidePane *side_pane,
                                           gboolean        show_hidden);

G_END_DECLS;

#endif /* !__LUNAR_SIDE_PANE_H__ */
