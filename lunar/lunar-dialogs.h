/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_DIALOGS_H__
#define __LUNAR_DIALOGS_H__

#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-file.h>
#include <lunar/lunar-job.h>

G_BEGIN_DECLS;

gchar             *lunar_dialogs_show_create           (gpointer              parent,
                                                         const gchar          *content_type,
                                                         const gchar          *filename,
                                                         const gchar          *title);
LunarJob         *lunar_dialogs_show_rename_file      (gpointer              parent,
                                                         LunarFile           *file);
void               lunar_dialogs_show_about            (GtkWindow            *parent,
                                                         const gchar          *title,
                                                         const gchar          *format,
                                                         ...) G_GNUC_PRINTF (3, 4);
void               lunar_dialogs_show_error            (gpointer              parent,
                                                         const GError         *error,
                                                         const gchar          *format,
                                                         ...) G_GNUC_PRINTF (3, 4);
LunarJobResponse  lunar_dialogs_show_job_ask          (GtkWindow            *parent,
                                                         const gchar          *question,
                                                         LunarJobResponse     choices);
LunarJobResponse  lunar_dialogs_show_job_ask_replace  (GtkWindow            *parent,
                                                         LunarFile           *src_file,
                                                         LunarFile           *dst_file);
void               lunar_dialogs_show_job_error        (GtkWindow            *parent,
                                                         GError               *error);
gboolean           lunar_dialogs_show_insecure_program (gpointer              parent,
                                                         const gchar          *title,
                                                         LunarFile           *file,
                                                         const gchar          *command);

G_END_DECLS;

#endif /* !__LUNAR_DIALOGS_H__ */
