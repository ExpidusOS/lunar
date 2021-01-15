/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __LUNAR_SBR_INSERT_RENAMER_H__
#define __LUNAR_SBR_INSERT_RENAMER_H__

#include <lunar-sbr/lunar-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _LunarSbrInsertRenamerClass LunarSbrInsertRenamerClass;
typedef struct _LunarSbrInsertRenamer      LunarSbrInsertRenamer;

#define LUNAR_SBR_TYPE_INSERT_RENAMER            (lunar_sbr_insert_renamer_get_type ())
#define LUNAR_SBR_INSERT_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_SBR_TYPE_INSERT_RENAMER, LunarSbrInsertRenamer))
#define LUNAR_SBR_INSERT_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_SBR_TYPE_INSERT_RENAMER, LunarSbrInsertRenamerClass))
#define LUNAR_SBR_IS_INSERT_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_SBR_TYPE_INSERT_RENAMER))
#define LUNAR_SBR_IS_INSERT_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_SBR_TYPE_INSERT_RENAMER))
#define LUNAR_SBR_INSERT_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_SBR_TYPE_INSERT_RENAMER, LunarSbrInsertRenamerClass))

GType                   lunar_sbr_insert_renamer_get_type        (void) G_GNUC_CONST;
void                    lunar_sbr_insert_renamer_register_type   (LunarxProviderPlugin  *plugin);

LunarSbrInsertRenamer *lunar_sbr_insert_renamer_new             (void) G_GNUC_MALLOC;

LunarSbrInsertMode     lunar_sbr_insert_renamer_get_mode        (LunarSbrInsertRenamer *insert_renamer);
void                    lunar_sbr_insert_renamer_set_mode        (LunarSbrInsertRenamer *insert_renamer,
                                                                   LunarSbrInsertMode     mode);

guint                   lunar_sbr_insert_renamer_get_offset      (LunarSbrInsertRenamer *insert_renamer);
void                    lunar_sbr_insert_renamer_set_offset      (LunarSbrInsertRenamer *insert_renamer,
                                                                   guint                   offset);

LunarSbrOffsetMode     lunar_sbr_insert_renamer_get_offset_mode (LunarSbrInsertRenamer *insert_renamer);
void                    lunar_sbr_insert_renamer_set_offset_mode (LunarSbrInsertRenamer *insert_renamer,
                                                                   LunarSbrOffsetMode     offset_mode);

const gchar            *lunar_sbr_insert_renamer_get_text        (LunarSbrInsertRenamer *insert_renamer);
void                    lunar_sbr_insert_renamer_set_text        (LunarSbrInsertRenamer *insert_renamer,
                                                                   const gchar            *text);

G_END_DECLS;

#endif /* !__LUNAR_SBR_INSERT_RENAMER_H__ */
