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

#ifndef __LUNAR_SBR_REMOVE_RENAMER_H__
#define __LUNAR_SBR_REMOVE_RENAMER_H__

#include <lunar-sbr/lunar-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _LunarSbrRemoveRenamerClass LunarSbrRemoveRenamerClass;
typedef struct _LunarSbrRemoveRenamer      LunarSbrRemoveRenamer;

#define LUNAR_SBR_TYPE_REMOVE_RENAMER            (lunar_sbr_remove_renamer_get_type ())
#define LUNAR_SBR_REMOVE_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_SBR_TYPE_REMOVE_RENAMER, LunarSbrRemoveRenamer))
#define LUNAR_SBR_REMOVE_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_SBR_TYPE_REMOVE_RENAMER, LunarSbrRemoveRenamerClass))
#define LUNAR_SBR_IS_REMOVE_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_SBR_TYPE_REMOVE_RENAMER))
#define LUNAR_SBR_IS_REMOVE_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_SBR_TYPE_REMOVE_RENAMER))
#define LUNAR_SBR_REMOVE_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_SBR_TYPE_REMOVE_RENAMER, LunarSbrRemoveRenamerClass))

GType                   lunar_sbr_remove_renamer_get_type              (void) G_GNUC_CONST;
void                    lunar_sbr_remove_renamer_register_type         (LunarxProviderPlugin  *plugin);

LunarSbrRemoveRenamer *lunar_sbr_remove_renamer_new                   (void) G_GNUC_MALLOC;

guint                   lunar_sbr_remove_renamer_get_end_offset        (LunarSbrRemoveRenamer *remove_renamer);
void                    lunar_sbr_remove_renamer_set_end_offset        (LunarSbrRemoveRenamer *remove_renamer,
                                                                         guint                   end_offset);

LunarSbrOffsetMode     lunar_sbr_remove_renamer_get_end_offset_mode   (LunarSbrRemoveRenamer *remove_renamer);
void                    lunar_sbr_remove_renamer_set_end_offset_mode   (LunarSbrRemoveRenamer *remove_renamer,
                                                                         LunarSbrOffsetMode     end_offset_mode);

guint                   lunar_sbr_remove_renamer_get_start_offset      (LunarSbrRemoveRenamer *remove_renamer);
void                    lunar_sbr_remove_renamer_set_start_offset      (LunarSbrRemoveRenamer *remove_renamer,
                                                                         guint                   start_offset);

LunarSbrOffsetMode     lunar_sbr_remove_renamer_get_start_offset_mode (LunarSbrRemoveRenamer *remove_renamer);
void                    lunar_sbr_remove_renamer_set_start_offset_mode (LunarSbrRemoveRenamer *remove_renamer,
                                                                         LunarSbrOffsetMode     start_offset_mode);

G_END_DECLS;

#endif /* !__LUNAR_SBR_REMOVE_RENAMER_H__ */
