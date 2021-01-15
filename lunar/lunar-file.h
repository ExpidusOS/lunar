/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
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

#ifndef __LUNAR_FILE_H__
#define __LUNAR_FILE_H__

#include <glib.h>

#include <lunarx/lunarx.h>

#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-gio-extensions.h>
#include <lunar/lunar-user.h>

G_BEGIN_DECLS;

typedef struct _LunarFileClass LunarFileClass;
typedef struct _LunarFile      LunarFile;

#define LUNAR_TYPE_FILE            (lunar_file_get_type ())
#define LUNAR_FILE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_FILE, LunarFile))
#define LUNAR_FILE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_FILE, LunarFileClass))
#define LUNAR_IS_FILE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_FILE))
#define LUNAR_IS_FILE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_FILE))
#define LUNAR_FILE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_FILE, LunarFileClass))

/**
 * LunarFileDateType:
 * @LUNAR_FILE_DATE_ACCESSED : date of last access to the file.
 * @LUNAR_FILE_DATE_CHANGED  : date of last change to the file meta data or the content.
 * @LUNAR_FILE_DATE_CREATED  : date of file creation.
 * @LUNAR_FILE_DATE_MODIFIED : date of last modification of the file's content.
 *
 * The various dates that can be queried about a #LunarFile. Note, that not all
 * #LunarFile implementations support all types listed above. See the documentation
 * of the lunar_file_get_date() method for details.
 **/
typedef enum
{
  LUNAR_FILE_DATE_ACCESSED,
  LUNAR_FILE_DATE_CHANGED,
  LUNAR_FILE_DATE_CREATED,
  LUNAR_FILE_DATE_MODIFIED,
} LunarFileDateType;

/**
 * LunarFileIconState:
 * @LUNAR_FILE_ICON_STATE_DEFAULT : the default icon for the file.
 * @LUNAR_FILE_ICON_STATE_DROP    : the drop accept icon for the file.
 * @LUNAR_FILE_ICON_STATE_OPEN    : the folder is expanded.
 *
 * The various file icon states that are used within the file manager
 * views.
 **/
typedef enum /*< enum >*/
{
  LUNAR_FILE_ICON_STATE_DEFAULT,
  LUNAR_FILE_ICON_STATE_DROP,
  LUNAR_FILE_ICON_STATE_OPEN,
} LunarFileIconState;

/**
 * LunarFileThumbState:
 * @LUNAR_FILE_THUMB_STATE_UNKNOWN : unknown whether there's a thumbnail.
 * @LUNAR_FILE_THUMB_STATE_NONE    : no thumbnail is available.
 * @LUNAR_FILE_THUMB_STATE_READY   : a thumbnail is available.
 * @LUNAR_FILE_THUMB_STATE_LOADING : a thumbnail is being generated.
 *
 * The state of the thumbnailing for a given #LunarFile.
 **/
typedef enum /*< flags >*/
{
  LUNAR_FILE_THUMB_STATE_UNKNOWN = 0,
  LUNAR_FILE_THUMB_STATE_NONE    = 1,
  LUNAR_FILE_THUMB_STATE_READY   = 2,
  LUNAR_FILE_THUMB_STATE_LOADING = 3,
} LunarFileThumbState;



#define LUNAR_FILE_EMBLEM_NAME_SYMBOLIC_LINK "emblem-symbolic-link"
#define LUNAR_FILE_EMBLEM_NAME_CANT_READ     "emblem-noread"
#define LUNAR_FILE_EMBLEM_NAME_CANT_WRITE    "emblem-nowrite"
#define LUNAR_FILE_EMBLEM_NAME_DESKTOP       "emblem-desktop"



/**
 * LunarFileGetFunc:
 *
 * Callback type for loading #LunarFile<!---->s asynchronously. If you
 * want to keep the #LunarFile, you need to ref it, else it will be
 * destroyed.
 **/
typedef void (*LunarFileGetFunc) (GFile      *location,
                                   LunarFile *file,
                                   GError     *error,
                                   gpointer    user_data);



GType             lunar_file_get_type                   (void) G_GNUC_CONST;

LunarFile       *lunar_file_get                        (GFile                  *file,
                                                          GError                **error);
LunarFile       *lunar_file_get_with_info              (GFile                  *file,
                                                          GFileInfo              *info,
                                                          gboolean                not_mounted);
LunarFile       *lunar_file_get_for_uri                (const gchar            *uri,
                                                          GError                **error);
void              lunar_file_get_async                  (GFile                  *location,
                                                          GCancellable           *cancellable,
                                                          LunarFileGetFunc       func,
                                                          gpointer                user_data);

GFile            *lunar_file_get_file                   (const LunarFile       *file) G_GNUC_PURE;

GFileInfo        *lunar_file_get_info                   (const LunarFile       *file) G_GNUC_PURE;

LunarFile       *lunar_file_get_parent                 (const LunarFile       *file,
                                                          GError                **error);

gboolean          lunar_file_check_loaded               (LunarFile             *file);

gboolean          lunar_file_execute                    (LunarFile             *file,
                                                          GFile                  *working_directory,
                                                          gpointer                parent,
                                                          GList                  *path_list,
                                                          const gchar            *startup_id,
                                                          GError                **error);

gboolean          lunar_file_launch                     (LunarFile             *file,
                                                          gpointer                parent,
                                                          const gchar            *startup_id,
                                                          GError                **error);

gboolean          lunar_file_rename                     (LunarFile             *file,
                                                          const gchar            *name,
                                                          GCancellable           *cancellable,
                                                          gboolean                called_from_job,
                                                          GError                **error);

GdkDragAction     lunar_file_accepts_drop               (LunarFile             *file,
                                                          GList                  *path_list,
                                                          GdkDragContext         *context,
                                                          GdkDragAction          *suggested_action_return);

guint64           lunar_file_get_date                   (const LunarFile       *file,
                                                          LunarFileDateType      date_type) G_GNUC_PURE;

gchar            *lunar_file_get_date_string            (const LunarFile       *file,
                                                          LunarFileDateType      date_type,
                                                          LunarDateStyle         date_style,
                                                          const gchar            *date_custom_style) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *lunar_file_get_mode_string            (const LunarFile       *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *lunar_file_get_size_string            (const LunarFile       *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *lunar_file_get_size_in_bytes_string   (const LunarFile       *file) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
gchar            *lunar_file_get_size_string_formatted  (const LunarFile       *file,
                                                          const gboolean          file_size_binary);
gchar            *lunar_file_get_size_string_long       (const LunarFile       *file,
                                                          const gboolean          file_size_binary);

GVolume          *lunar_file_get_volume                 (const LunarFile       *file);

LunarGroup      *lunar_file_get_group                  (const LunarFile       *file);
LunarUser       *lunar_file_get_user                   (const LunarFile       *file);

const gchar      *lunar_file_get_content_type           (LunarFile             *file);
gboolean          lunar_file_load_content_type          (LunarFile             *file);
const gchar      *lunar_file_get_symlink_target         (const LunarFile       *file);
const gchar      *lunar_file_get_basename               (const LunarFile       *file) G_GNUC_CONST;
gboolean          lunar_file_is_symlink                 (const LunarFile       *file);
guint64           lunar_file_get_size                   (const LunarFile       *file);
GAppInfo         *lunar_file_get_default_handler        (const LunarFile       *file);
GFileType         lunar_file_get_kind                   (const LunarFile       *file) G_GNUC_PURE;
GFile            *lunar_file_get_target_location        (const LunarFile       *file);
LunarFileMode    lunar_file_get_mode                   (const LunarFile       *file);
gboolean          lunar_file_is_mounted                 (const LunarFile       *file);
gboolean          lunar_file_exists                     (const LunarFile       *file);
gboolean          lunar_file_is_directory               (const LunarFile       *file) G_GNUC_PURE;
gboolean          lunar_file_is_shortcut                (const LunarFile       *file) G_GNUC_PURE;
gboolean          lunar_file_is_mountable               (const LunarFile       *file) G_GNUC_PURE;
gboolean          lunar_file_is_local                   (const LunarFile       *file);
gboolean          lunar_file_is_parent                  (const LunarFile       *file,
                                                          const LunarFile       *child);
gboolean          lunar_file_is_gfile_ancestor          (const LunarFile       *file,
                                                          GFile                  *ancestor);
gboolean          lunar_file_is_ancestor                (const LunarFile       *file,
                                                          const LunarFile       *ancestor);
gboolean          lunar_file_is_executable              (const LunarFile       *file);
gboolean          lunar_file_is_writable                (const LunarFile       *file);
gboolean          lunar_file_is_hidden                  (const LunarFile       *file);
gboolean          lunar_file_is_home                    (const LunarFile       *file);
gboolean          lunar_file_is_regular                 (const LunarFile       *file) G_GNUC_PURE;
gboolean          lunar_file_is_trashed                 (const LunarFile       *file);
gboolean          lunar_file_is_desktop_file            (const LunarFile       *file,
                                                          gboolean               *is_secure);
const gchar      *lunar_file_get_display_name           (const LunarFile       *file) G_GNUC_CONST;

gchar            *lunar_file_get_deletion_date          (const LunarFile       *file,
                                                          LunarDateStyle         date_style,
                                                          const gchar            *date_custom_style) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;
const gchar      *lunar_file_get_original_path          (const LunarFile       *file);
guint32           lunar_file_get_item_count             (const LunarFile       *file);

gboolean          lunar_file_is_chmodable               (const LunarFile       *file);
gboolean          lunar_file_is_renameable              (const LunarFile       *file);
gboolean          lunar_file_can_be_trashed             (const LunarFile       *file);

GList            *lunar_file_get_emblem_names           (LunarFile              *file);
void              lunar_file_set_emblem_names           (LunarFile              *file,
                                                          GList                   *emblem_names);

const gchar      *lunar_file_get_custom_icon            (const LunarFile        *file);
gboolean          lunar_file_set_custom_icon            (LunarFile              *file,
                                                          const gchar             *custom_icon,
                                                          GError                 **error);

const gchar     *lunar_file_get_thumbnail_path          (LunarFile              *file,
                                                          LunarThumbnailSize      thumbnail_size);
LunarFileThumbState lunar_file_get_thumb_state         (const LunarFile        *file);
void             lunar_file_set_thumb_state             (LunarFile              *file,
                                                          LunarFileThumbState     state);
GIcon            *lunar_file_get_preview_icon           (const LunarFile        *file);
GFilesystemPreviewType lunar_file_get_preview_type      (const LunarFile *file);
const gchar      *lunar_file_get_icon_name              (LunarFile              *file,
                                                          LunarFileIconState      icon_state,
                                                          GtkIconTheme            *icon_theme);

void              lunar_file_watch                      (LunarFile              *file);
void              lunar_file_unwatch                    (LunarFile              *file);

gboolean          lunar_file_reload                     (LunarFile              *file);
void              lunar_file_reload_idle                (LunarFile              *file);
void              lunar_file_reload_idle_unref          (LunarFile              *file);
void              lunar_file_reload_parent              (LunarFile              *file);

void              lunar_file_destroy                    (LunarFile              *file);

gint              lunar_file_compare_by_type            (LunarFile              *file_a,
                                                          LunarFile              *file_b);
gint              lunar_file_compare_by_name            (const LunarFile        *file_a,
                                                          const LunarFile        *file_b,
                                                          gboolean                 case_sensitive) G_GNUC_PURE;

LunarFile       *lunar_file_cache_lookup               (const GFile             *file);
gchar            *lunar_file_cached_display_name        (const GFile             *file);


GList            *lunar_file_list_get_applications      (GList                  *file_list);
GList            *lunar_file_list_to_lunar_g_file_list (GList                  *file_list);

gboolean          lunar_file_is_desktop                 (const LunarFile *file);

const gchar*      lunar_file_get_metadata_setting       (LunarFile             *file,
                                                          const gchar            *setting_name);
void              lunar_file_set_metadata_setting       (LunarFile             *file,
                                                          const gchar            *setting_name,
                                                          const gchar            *setting_value);
void              lunar_file_clear_directory_specific_settings (LunarFile      *file);
gboolean          lunar_file_has_directory_specific_settings   (LunarFile      *file);

/**
 * lunar_file_is_root:
 * @file : a #LunarFile.
 *
 * Checks whether @file refers to the root directory.
 *
 * Return value: %TRUE if @file is the root directory.
 **/
#define lunar_file_is_root(file) (lunar_g_file_is_root (lunar_file_get_file (file)))

/**
 * lunar_file_has_parent:
 * @file : a #LunarFile instance.
 *
 * Checks whether it is possible to determine the parent #LunarFile
 * for @file.
 *
 * Return value: whether @file has a parent.
 **/
#define lunar_file_has_parent(file) (!lunar_file_is_root (LUNAR_FILE ((file))))

/**
 * lunar_file_dup_uri:
 * @file : a #LunarFile instance.
 *
 * Returns the URI for the @file. The caller is responsible
 * to free the returned string when no longer needed.
 *
 * Return value: the URI for @file.
 **/
#define lunar_file_dup_uri(file) (g_file_get_uri (lunar_file_get_file (file)))

/**
 * lunar_file_has_uri_scheme:
 * @file       : a #LunarFile instance.
 * @uri_scheme : a URI scheme string.
 *
 * Checks whether the URI scheme of the file matches @uri_scheme.
 *
 * Return value: TRUE, if the schemes match, FALSE otherwise.
 **/
#define lunar_file_has_uri_scheme(file, uri_scheme) (g_file_has_uri_scheme (lunar_file_get_file (file), (uri_scheme)))

/**
 * lunar_file_changed:
 * @file : a #LunarFile instance.
 *
 * Emits the ::changed signal on @file. This function is meant to be called
 * by derived classes whenever they notice changes to the @file.
 **/
#define lunar_file_changed(file)                         \
G_STMT_START{                                             \
  lunarx_file_info_changed (LUNARX_FILE_INFO ((file))); \
}G_STMT_END


G_END_DECLS;

#endif /* !__LUNAR_FILE_H__ */
