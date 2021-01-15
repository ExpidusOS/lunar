/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_TREE_PANE_H__
#define __LUNAR_TREE_PANE_H__

#include <lunar/lunar-side-pane.h>

G_BEGIN_DECLS;

typedef struct _LunarTreePaneClass LunarTreePaneClass;
typedef struct _LunarTreePane      LunarTreePane;

#define LUNAR_TYPE_TREE_PANE             (lunar_tree_pane_get_type ())
#define LUNAR_TREE_PANE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_TREE_PANE, LunarTreePane))
#define LUNAR_TREE_PANE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_TREE_PANE, LunarTreePaneClass))
#define LUNAR_IS_TREE_PANE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_TREE_PANE))
#define LUNAR_IS_TREE_PANE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_TREE_PANE))
#define LUNAR_TREE_PANE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_TREE_PANE, LunarTreePaneClass))

GType      lunar_tree_pane_get_type (void) G_GNUC_CONST;

G_END_DECLS;

#endif /* !__LUNAR_TREE_PANE_H__ */
