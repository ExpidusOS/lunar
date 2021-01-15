/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2006      Matt McClinch <mattmcclinch@gmail.com>
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

#ifndef __LUNAR_COMPACT_VIEW_H__
#define __LUNAR_COMPACT_VIEW_H__

#include <lunar/lunar-abstract-icon-view.h>

G_BEGIN_DECLS;

typedef struct _LunarCompactViewClass LunarCompactViewClass;
typedef struct _LunarCompactView      LunarCompactView;

#define LUNAR_TYPE_COMPACT_VIEW             (lunar_compact_view_get_type ())
#define LUNAR_COMPACT_VIEW(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_ICON_VIEW, LunarCompactView))
#define LUNAR_COMPACT_VIEW_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_ICON_VIEW, LunarCompactViewClass))
#define LUNAR_IS_COMPACT_VIEW(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_ICON_VIEW))
#define LUNAR_IS_COMPACT_VIEW_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), LUNAR_TYPE_ICON_VIEW))
#define LUNAR_COMPACT_VIEW_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_ICON_VIEW, LunarCompactViewClass))

GType lunar_compact_view_get_type (void) G_GNUC_CONST;

G_END_DECLS;

#endif /* !__LUNAR_COMPACT_VIEW_H__ */
