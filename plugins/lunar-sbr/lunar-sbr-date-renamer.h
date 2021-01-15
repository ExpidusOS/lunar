/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2007 Nick Schermer <nick@expidus.org>
 * Copyright (c) 2007 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_SBR_DATE_RENAMER_H__
#define __LUNAR_SBR_DATE_RENAMER_H__

#include <lunar-sbr/lunar-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _LunarSbrDateRenamerClass LunarSbrDateRenamerClass;
typedef struct _LunarSbrDateRenamer      LunarSbrDateRenamer;

#define LUNAR_SBR_TYPE_DATE_RENAMER            (lunar_sbr_date_renamer_get_type ())
#define LUNAR_SBR_DATE_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_SBR_TYPE_DATE_RENAMER, LunarSbrDateRenamer))
#define LUNAR_SBR_DATE_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_SBR_TYPE_DATE_RENAMER, LunarSbrDateRenamerClass))
#define LUNAR_SBR_IS_DATE_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_SBR_TYPE_DATE_RENAMER))
#define LUNAR_SBR_IS_DATE_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_SBR_TYPE_DATE_RENAMER))
#define LUNAR_SBR_DATE_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_SBR_TYPE_DATE_RENAMER, LunarSbrDateRenamerClass))

GType                 lunar_sbr_date_renamer_get_type        (void) G_GNUC_CONST;

void                  lunar_sbr_date_renamer_register_type   (LunarxProviderPlugin  *plugin);

LunarSbrDateRenamer *lunar_sbr_date_renamer_new             (void) G_GNUC_MALLOC;

LunarSbrDateMode     lunar_sbr_date_renamer_get_mode        (LunarSbrDateRenamer *date_renamer);
void                  lunar_sbr_date_renamer_set_mode        (LunarSbrDateRenamer *date_renamer,
                                                               LunarSbrDateMode     mode);

const gchar          *lunar_sbr_date_renamer_get_format      (LunarSbrDateRenamer *date_renamer);
void                  lunar_sbr_date_renamer_set_format      (LunarSbrDateRenamer *date_renamer,
                                                               const gchar          *format);

guint                 lunar_sbr_date_renamer_get_offset      (LunarSbrDateRenamer *date_renamer);
void                  lunar_sbr_date_renamer_set_offset      (LunarSbrDateRenamer *date_renamer,
                                                               guint                 offset);

LunarSbrOffsetMode   lunar_sbr_date_renamer_get_offset_mode (LunarSbrDateRenamer *date_renamer);
void                  lunar_sbr_date_renamer_set_offset_mode (LunarSbrDateRenamer *date_renamer,
                                                               LunarSbrOffsetMode   offset_mode);

G_END_DECLS;

#endif /* !__LUNAR_SBR_DATE_RENAMER_H__ */
