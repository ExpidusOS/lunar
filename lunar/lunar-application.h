/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2005      Jeff Franks <jcfranks@expidus.org>
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@expidus.org>
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

#ifndef __LUNAR_APPLICATION_H__
#define __LUNAR_APPLICATION_H__

#include <lunar/lunar-window.h>
#include <lunar/lunar-thumbnail-cache.h>

G_BEGIN_DECLS;

typedef struct _LunarApplicationClass LunarApplicationClass;
typedef struct _LunarApplication      LunarApplication;

#define LUNAR_TYPE_APPLICATION             (lunar_application_get_type ())
#define LUNAR_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_APPLICATION, LunarApplication))
#define LUNAR_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_APPLICATION, LunarApplicationClass))
#define LUNAR_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_APPLICATION))
#define LUNAR_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_APPLICATION))
#define LUNAR_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_APPLICATION, LunarApplicationClass))

GType                 lunar_application_get_type                   (void) G_GNUC_CONST;

LunarApplication    *lunar_application_get                        (void);

void                  lunar_application_quit                       (LunarApplication *application);

gboolean              lunar_application_get_daemon                 (LunarApplication *application);
void                  lunar_application_set_daemon                 (LunarApplication *application,
                                                                     gboolean           daemon);

GList                *lunar_application_get_windows                (LunarApplication *application);

gboolean              lunar_application_has_windows                (LunarApplication *application);
void                  lunar_application_close_all_windows          (LunarApplication *application);
void                  lunar_application_take_window                (LunarApplication *application,
                                                                     GtkWindow         *window);

GtkWidget            *lunar_application_open_window                (LunarApplication *application,
                                                                     LunarFile        *directory,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id,
                                                                     gboolean           force_new_window);

gboolean              lunar_application_bulk_rename                (LunarApplication *application,
                                                                     const gchar       *working_directory,
                                                                     gchar            **filenames,
                                                                     gboolean           standalone,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id,
                                                                     GError           **error);

gboolean              lunar_application_process_filenames          (LunarApplication *application,
                                                                     const gchar       *working_directory,
                                                                     gchar            **filenames,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id,
                                                                     GError           **error);

void                  lunar_application_rename_file                (LunarApplication *application,
                                                                     LunarFile        *file,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id);
void                  lunar_application_create_file                (LunarApplication *application,
                                                                     LunarFile        *parent_directory,
                                                                     const gchar       *content_type,
                                                                     GdkScreen         *screen,
                                                                     const gchar       *startup_id);
void                  lunar_application_create_file_from_template (LunarApplication *application,
                                                                    LunarFile        *parent_directory,
                                                                    LunarFile        *template_file,
                                                                    GdkScreen         *screen,
                                                                    const gchar       *startup_id);
void                  lunar_application_copy_to                   (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GList             *target_file_list,
                                                                    GClosure          *new_files_closure);

void                  lunar_application_copy_into                 (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GFile             *target_file,
                                                                    GClosure          *new_files_closure);

void                  lunar_application_link_into                 (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GFile             *target_file,
                                                                    GClosure          *new_files_closure);

void                  lunar_application_move_into                 (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *source_file_list,
                                                                    GFile             *target_file,
                                                                    GClosure          *new_files_closure);

void                  lunar_application_unlink_files              (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list,
                                                                    gboolean           permanently);

void                  lunar_application_trash                     (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list);

void                  lunar_application_creat                     (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list,
                                                                    GFile             *template_file,
                                                                    GClosure          *new_files_closure);

void                  lunar_application_mkdir                     (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *file_list,
                                                                    GClosure          *new_files_closure);

void                  lunar_application_empty_trash               (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    const gchar       *startup_id);

void                  lunar_application_restore_files             (LunarApplication *application,
                                                                    gpointer           parent,
                                                                    GList             *trash_file_list,
                                                                    GClosure          *new_files_closure);

LunarThumbnailCache *lunar_application_get_thumbnail_cache       (LunarApplication *application);

G_END_DECLS;

#endif /* !__LUNAR_APPLICATION_H__ */
