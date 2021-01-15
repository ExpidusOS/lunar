/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@expidus.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __LUNAR_THUMBNAILER_H__
#define __LUNAR_THUMBNAILER_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS

typedef struct _LunarThumbnailerClass LunarThumbnailerClass;
typedef struct _LunarThumbnailer      LunarThumbnailer;

#define LUNAR_TYPE_THUMBNAILER            (lunar_thumbnailer_get_type ())
#define LUNAR_THUMBNAILER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_THUMBNAILER, LunarThumbnailer))
#define LUNAR_THUMBNAILER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_THUMBNAILER, LunarThumbnailerClass))
#define LUNAR_IS_THUMBNAILER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_THUMBNAILER))
#define LUNAR_IS_THUMBNAILER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_THUMBNAILER))
#define LUNAR_THUMBNAILER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_THUMBNAILER, LunarThumbnailerClass))

GType              lunar_thumbnailer_get_type        (void) G_GNUC_CONST;

LunarThumbnailer *lunar_thumbnailer_get             (void) G_GNUC_MALLOC;

gboolean           lunar_thumbnailer_queue_file      (LunarThumbnailer        *thumbnailer,
                                                       LunarFile               *file,
                                                       guint                    *request);
gboolean           lunar_thumbnailer_queue_files     (LunarThumbnailer        *thumbnailer,
                                                       gboolean                  lazy_checks,
                                                       GList                    *files,
                                                       guint                    *request);
void               lunar_thumbnailer_dequeue         (LunarThumbnailer        *thumbnailer,
                                                       guint                     request);

G_END_DECLS

#endif /* !__LUNAR_THUMBNAILER_H__ */
