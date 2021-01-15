/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_RENAMER_PAIR_H__
#define __LUNAR_RENAMER_PAIR_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarRenamerPair LunarRenamerPair;

#define LUNAR_TYPE_RENAMER_PAIR (lunar_renamer_pair_get_type ())

struct _LunarRenamerPair
{
  LunarFile *file;
  gchar      *name;
};

GType              lunar_renamer_pair_get_type   (void) G_GNUC_CONST;

LunarRenamerPair *lunar_renamer_pair_new        (LunarFile        *file,
                                                   const gchar       *name) G_GNUC_MALLOC;

void               lunar_renamer_pair_free       (gpointer           data);

GList             *lunar_renamer_pair_list_copy  (GList             *renamer_pair_list) G_GNUC_MALLOC;
void               lunar_renamer_pair_list_free  (GList             *renamer_pair_list);

G_END_DECLS;

#endif /* !__LUNAR_RENAMER_PAIR_H__ */
