/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
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

#ifndef __LUNAR_GIO_EXTENSIONS_H__
#define __LUNAR_GIO_EXTENSIONS_H__

#include <gio/gio.h>

G_BEGIN_DECLS

GFile    *lunar_g_file_new_for_home             (void);
GFile    *lunar_g_file_new_for_root             (void);
GFile    *lunar_g_file_new_for_trash            (void);
GFile    *lunar_g_file_new_for_desktop          (void);
GFile    *lunar_g_file_new_for_computer         (void);
GFile    *lunar_g_file_new_for_bookmarks        (void);

gboolean  lunar_g_file_is_root                  (GFile                *file);
gboolean  lunar_g_file_is_trashed               (GFile                *file);
gboolean  lunar_g_file_is_home                  (GFile                *file);
gboolean  lunar_g_file_is_trash                 (GFile                *file);
gboolean  lunar_g_file_is_computer              (GFile                *file);
gboolean  lunar_g_file_is_network               (GFile                *file);

GKeyFile *lunar_g_file_query_key_file           (GFile                *file,
                                                  GCancellable         *cancellable,
                                                  GError              **error);
gboolean  lunar_g_file_write_key_file           (GFile                *file,
                                                  GKeyFile             *key_file,
                                                  GCancellable         *cancellable,
                                                  GError              **error);

gchar    *lunar_g_file_get_location             (GFile                *file);

gchar    *lunar_g_file_get_display_name         (GFile                *file);

gchar    *lunar_g_file_get_display_name_remote  (GFile                *file);

gboolean  lunar_g_vfs_is_uri_scheme_supported   (const gchar          *scheme);

gboolean  lunar_g_file_get_free_space           (GFile                *file,
                                                  guint64              *fs_free_return,
                                                  guint64              *fs_size_return);

gchar    *lunar_g_file_get_free_space_string    (GFile                *file,
                                                  gboolean              file_size_binary);

/**
 * LUNAR_TYPE_G_FILE_LIST:
 *
 * Returns the type ID for #GList<!---->s of #GFile<!---->s which is a
 * boxed type.
 **/
#define LUNAR_TYPE_G_FILE_LIST (lunar_g_file_list_get_type ())

GType     lunar_g_file_list_get_type               (void);

GList    *lunar_g_file_list_new_from_string        (const gchar       *string);
gchar   **lunar_g_file_list_to_stringv             (GList             *list);
GList    *lunar_g_file_list_get_parents            (GList             *list);

/* deep copy jobs for GLists */
#define   lunar_g_file_list_append(list,object)    g_list_append (list, g_object_ref (G_OBJECT (object)))
#define   lunar_g_file_list_prepend(list,object)   g_list_prepend (list, g_object_ref (G_OBJECT (object)))
#define   lunar_g_file_list_copy                   lunarx_file_info_list_copy
#define   lunar_g_file_list_free                   lunarx_file_info_list_free

gboolean  lunar_g_app_info_launch                  (GAppInfo          *info,
                                                     GFile             *working_directory,
                                                     GList             *path_list,
                                                     GAppLaunchContext *context,
                                                     GError           **error);

gboolean  lunar_g_app_info_should_show             (GAppInfo          *info);

gboolean  lunar_g_vfs_metadata_is_supported        (void);


G_END_DECLS

#endif /* !__LUNAR_GIO_EXTENSIONS_H__ */
