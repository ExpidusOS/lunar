/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_UCA_CONTEXT_H__
#define __LUNAR_UCA_CONTEXT_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _LunarUcaContext LunarUcaContext;

LunarUcaContext *lunar_uca_context_new        (GtkWidget              *window,
                                                 GList                  *files);

LunarUcaContext *lunar_uca_context_ref        (LunarUcaContext       *context);
void              lunar_uca_context_unref      (LunarUcaContext       *context);

GList            *lunar_uca_context_get_files  (const LunarUcaContext *context);
GtkWidget        *lunar_uca_context_get_window (const LunarUcaContext *context);

G_END_DECLS;

#endif /* !__LUNAR_UCA_CONTEXT_H__ */
