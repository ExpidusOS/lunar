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

#ifndef __LUNAR_EMBLEM_CHOOSER_H__
#define __LUNAR_EMBLEM_CHOOSER_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarEmblemChooserClass LunarEmblemChooserClass;
typedef struct _LunarEmblemChooser      LunarEmblemChooser;

#define LUNAR_TYPE_EMBLEM_CHOOSER            (lunar_emblem_chooser_get_type ())
#define LUNAR_EMBLEM_CHOOSER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_EMBLEM_CHOOSER, LunarEmblemChooser))
#define LUNAR_EMBLEM_CHOOSER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_EMBLEM_CHOOSER, LunarEmblemChooserClass))
#define LUNAR_IS_EMBLEM_CHOOSER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_EMBLEM_CHOOSER))
#define LUNAR_IS_EMBLEM_CHOOSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_EMBLEM_CHOOSER))
#define LUNAR_EMBLEM_CHOOSER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_EMBLEM_CHOOSER, LunarEmblemChooserClass))

GType       lunar_emblem_chooser_get_type  (void) G_GNUC_CONST;

GtkWidget  *lunar_emblem_chooser_new       (void) G_GNUC_MALLOC;

G_END_DECLS;

#endif /* !__LUNAR_EMBLEM_CHOOSER_H__ */
