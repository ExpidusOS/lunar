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

#ifndef __LUNAR_SBR_NUMBER_RENAMER_H__
#define __LUNAR_SBR_NUMBER_RENAMER_H__

#include <lunar-sbr/lunar-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _LunarSbrNumberRenamerClass LunarSbrNumberRenamerClass;
typedef struct _LunarSbrNumberRenamer      LunarSbrNumberRenamer;

#define LUNAR_SBR_TYPE_NUMBER_RENAMER            (lunar_sbr_number_renamer_get_type ())
#define LUNAR_SBR_NUMBER_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_SBR_TYPE_NUMBER_RENAMER, LunarSbrNumberRenamer))
#define LUNAR_SBR_NUMBER_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_SBR_TYPE_NUMBER_RENAMER, LunarSbrNumberRenamerClass))
#define LUNAR_SBR_IS_NUMBER_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_SBR_TYPE_NUMBER_RENAMER))
#define LUNAR_SBR_IS_NUMBER_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_SBR_TYPE_NUMBER_RENAMER))
#define LUNAR_SBR_NUMBER_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_SBR_TYPE_NUMBER_RENAMER, LunarSbrNumberRenamerClass))

GType                   lunar_sbr_number_renamer_get_type        (void) G_GNUC_CONST;
void                    lunar_sbr_number_renamer_register_type   (LunarxProviderPlugin  *plugin);

LunarSbrNumberRenamer *lunar_sbr_number_renamer_new             (void) G_GNUC_MALLOC;

LunarSbrNumberMode     lunar_sbr_number_renamer_get_mode        (LunarSbrNumberRenamer *number_renamer);
void                    lunar_sbr_number_renamer_set_mode        (LunarSbrNumberRenamer *number_renamer,
                                                                   LunarSbrNumberMode     mode);

const gchar            *lunar_sbr_number_renamer_get_start        (LunarSbrNumberRenamer *number_renamer);
void                    lunar_sbr_number_renamer_set_start        (LunarSbrNumberRenamer *number_renamer,
                                                                   const gchar            *start);

const gchar            *lunar_sbr_number_renamer_get_text        (LunarSbrNumberRenamer *number_renamer);
void                    lunar_sbr_number_renamer_set_text        (LunarSbrNumberRenamer *number_renamer,
                                                                   const gchar            *text);

LunarSbrTextMode       lunar_sbr_number_renamer_get_text_mode   (LunarSbrNumberRenamer *number_renamer);
void                    lunar_sbr_number_renamer_set_text_mode   (LunarSbrNumberRenamer *number_renamer,
                                                                   LunarSbrTextMode       text_mode);

G_END_DECLS;

#endif /* !__LUNAR_SBR_NUMBER_RENAMER_H__ */
