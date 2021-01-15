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

#ifndef __LUNAR_SBR_REPLACE_RENAMER_H__
#define __LUNAR_SBR_REPLACE_RENAMER_H__

#include <lunarx/lunarx.h>

G_BEGIN_DECLS;

typedef struct _LunarSbrReplaceRenamerClass LunarSbrReplaceRenamerClass;
typedef struct _LunarSbrReplaceRenamer      LunarSbrReplaceRenamer;

#define LUNAR_SBR_TYPE_REPLACE_RENAMER            (lunar_sbr_replace_renamer_get_type ())
#define LUNAR_SBR_REPLACE_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_SBR_TYPE_REPLACE_RENAMER, LunarSbrReplaceRenamer))
#define LUNAR_SBR_REPLACE_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_SBR_TYPE_REPLACE_RENAMER, LunarSbrReplaceRenamerClass))
#define LUNAR_SBR_IS_REPLACE_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_SBR_TYPE_REPLACE_RENAMER))
#define LUNAR_SBR_IS_REPLACE_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_SBR_TYPE_REPLACE_RENAMER))
#define LUNAR_SBR_REPLACE_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_SBR_TYPE_REPLACE_RENAMER, LunarSbrReplaceRenamerClass))

GType                    lunar_sbr_replace_renamer_get_type            (void) G_GNUC_CONST;
void                     lunar_sbr_replace_renamer_register_type       (LunarxProviderPlugin   *plugin);

LunarSbrReplaceRenamer *lunar_sbr_replace_renamer_new                 (void) G_GNUC_MALLOC;

gboolean                 lunar_sbr_replace_renamer_get_case_sensitive  (LunarSbrReplaceRenamer *replace_renamer);
void                     lunar_sbr_replace_renamer_set_case_sensitive  (LunarSbrReplaceRenamer *replace_renamer,
                                                                         gboolean                 case_sensitive);

const gchar             *lunar_sbr_replace_renamer_get_pattern         (LunarSbrReplaceRenamer *replace_renamer);
void                     lunar_sbr_replace_renamer_set_pattern         (LunarSbrReplaceRenamer *replace_renamer,
                                                                         const gchar             *pattern);

gboolean                 lunar_sbr_replace_renamer_get_regexp          (LunarSbrReplaceRenamer *replace_renamer);
void                     lunar_sbr_replace_renamer_set_regexp          (LunarSbrReplaceRenamer *replace_renamer,
                                                                         gboolean                 regexp);

const gchar             *lunar_sbr_replace_renamer_get_replacement     (LunarSbrReplaceRenamer *replace_renamer);
void                     lunar_sbr_replace_renamer_set_replacement     (LunarSbrReplaceRenamer *replace_renamer,
                                                                         const gchar             *replacement);

G_END_DECLS;

#endif /* !__LUNAR_SBR_REPLACE_RENAMER_H__ */
