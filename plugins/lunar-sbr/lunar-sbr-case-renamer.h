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

#ifndef __LUNAR_SBR_CASE_RENAMER_H__
#define __LUNAR_SBR_CASE_RENAMER_H__

#include <lunar-sbr/lunar-sbr-enum-types.h>

G_BEGIN_DECLS;

typedef struct _LunarSbrCaseRenamerClass LunarSbrCaseRenamerClass;
typedef struct _LunarSbrCaseRenamer      LunarSbrCaseRenamer;

#define LUNAR_SBR_TYPE_CASE_RENAMER            (lunar_sbr_case_renamer_get_type ())
#define LUNAR_SBR_CASE_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_SBR_TYPE_CASE_RENAMER, LunarSbrCaseRenamer))
#define LUNAR_SBR_CASE_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_SBR_TYPE_CASE_RENAMER, LunarSbrCaseRenamerClass))
#define LUNAR_SBR_IS_CASE_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_SBR_TYPE_CASE_RENAMER))
#define LUNAR_SBR_IS_CASE_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_SBR_TYPE_CASE_RENAMER))
#define LUNAR_SBR_CASE_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_SBR_TYPE_CASE_RENAMER, LunarSbrCaseRenamerClass))

GType                     lunar_sbr_case_renamer_get_type      (void) G_GNUC_CONST;
void                      lunar_sbr_case_renamer_register_type (LunarxProviderPlugin   *plugin);

LunarSbrCaseRenamer     *lunar_sbr_case_renamer_new           (void) G_GNUC_MALLOC;

LunarSbrCaseRenamerMode  lunar_sbr_case_renamer_get_mode      (LunarSbrCaseRenamer    *case_renamer);
void                      lunar_sbr_case_renamer_set_mode      (LunarSbrCaseRenamer    *case_renamer,
                                                                 LunarSbrCaseRenamerMode mode);

G_END_DECLS;

#endif /* !__LUNAR_SBR_CASE_RENAMER_H__ */
