/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2011 Jannis Pohlmann <jannis@expidus.org>
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

#ifndef __LUNAR_THUMBNAIL_CACHE_H__
#define __LUNAR_THUMBNAIL_CACHE_H__

#include <gio/gio.h>

G_BEGIN_DECLS

#define LUNAR_TYPE_THUMBNAIL_CACHE            (lunar_thumbnail_cache_get_type ())
#define LUNAR_THUMBNAIL_CACHE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_THUMBNAIL_CACHE, LunarThumbnailCache))
#define LUNAR_THUMBNAIL_CACHE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_THUMBNAIL_CACHE, LunarThumbnailCacheClass))
#define LUNAR_IS_THUMBNAIL_CACHE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_THUMBNAIL_CACHE))
#define LUNAR_IS_THUMBNAIL_CACHE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_THUMBNAIL_CACHE)
#define LUNAR_THUMBNAIL_CACHE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_THUMBNAIL_CACHE, LunarThumbnailCacheClass))

typedef struct _LunarThumbnailCachePrivate LunarThumbnailCachePrivate;
typedef struct _LunarThumbnailCacheClass   LunarThumbnailCacheClass;
typedef struct _LunarThumbnailCache        LunarThumbnailCache;

GType                 lunar_thumbnail_cache_get_type     (void) G_GNUC_CONST;

LunarThumbnailCache *lunar_thumbnail_cache_new          (void) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

void                  lunar_thumbnail_cache_move_file    (LunarThumbnailCache *cache,
                                                           GFile                *source_file,
                                                           GFile                *target_file);
void                  lunar_thumbnail_cache_copy_file    (LunarThumbnailCache *cache,
                                                           GFile                *source_file,
                                                           GFile                *target_file);
void                  lunar_thumbnail_cache_delete_file  (LunarThumbnailCache *cache,
                                                           GFile                *file);
void                  lunar_thumbnail_cache_cleanup_file (LunarThumbnailCache *cache,
                                                           GFile                *file);

G_END_DECLS

#endif /* !__LUNAR_THUMBNAIL_CACHE_H__ */
