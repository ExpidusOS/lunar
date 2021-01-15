/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2007 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_ENUM_TYPES_H__
#define __LUNAR_ENUM_TYPES_H__

#include <endo/endo.h>

G_BEGIN_DECLS;

#define LUNAR_TYPE_RENAMER_MODE (lunar_renamer_mode_get_type ())

/**
 * LunarRenamerMode:
 * @LUNAR_RENAMER_MODE_NAME   : only the name should be renamed.
 * @LUNAR_RENAMER_MODE_SUFFIX : only the suffix should be renamed.
 * @LUNAR_RENAMER_MODE_BOTH   : the name and the suffix should be renamed.
 *
 * The rename mode for a #LunarRenamerModel instance.
 **/
typedef enum
{
  LUNAR_RENAMER_MODE_NAME,
  LUNAR_RENAMER_MODE_SUFFIX,
  LUNAR_RENAMER_MODE_BOTH,
} LunarRenamerMode;

GType lunar_renamer_mode_get_type (void) G_GNUC_CONST;



#define LUNAR_TYPE_DATE_STYLE (lunar_date_style_get_type ())

/**
 * LunarDateStyle:
 * @LUNAR_DATE_STYLE_SIMPLE : display only the date.
 * @LUNAR_DATE_STYLE_SHORT  : display date and time in a short manner.
 * @LUNAR_DATE_STYLE_LONG   : display date and time in a long manner.
 * @LUNAR_DATE_STYLE_ISO    : display date and time in ISO standard form.
 *
 * The style used to display dates (e.g. modification dates) to the user.
 **/
typedef enum
{
  LUNAR_DATE_STYLE_SIMPLE,
  LUNAR_DATE_STYLE_SHORT,
  LUNAR_DATE_STYLE_LONG,
  LUNAR_DATE_STYLE_YYYYMMDD,
  LUNAR_DATE_STYLE_MMDDYYYY,
  LUNAR_DATE_STYLE_DDMMYYYY,
  LUNAR_DATE_STYLE_CUSTOM,
} LunarDateStyle;

GType lunar_date_style_get_type (void) G_GNUC_CONST;


#define LUNAR_TYPE_COLUMN (lunar_column_get_type ())

/**
 * LunarColumn:
 * @LUNAR_COLUMN_DATE_ACCESSED : last access time.
 * @LUNAR_COLUMN_DATE_MODIFIED : last modification time.
 * @LUNAR_COLUMN_GROUP         : group's name.
 * @LUNAR_COLUMN_MIME_TYPE     : mime type (e.g. "text/plain").
 * @LUNAR_COLUMN_NAME          : display name.
 * @LUNAR_COLUMN_OWNER         : owner's name.
 * @LUNAR_COLUMN_PERMISSIONS   : permissions bits.
 * @LUNAR_COLUMN_SIZE          : file size.
 * @LUNAR_COLUMN_SIZE_IN_BYTES : file size in bytes.
 * @LUNAR_COLUMN_TYPE          : file type (e.g. 'plain text document').
 * @LUNAR_COLUMN_FILE          : #LunarFile object.
 * @LUNAR_COLUMN_FILE_NAME     : real file name.
 *
 * Columns exported by #LunarListModel using the #GtkTreeModel
 * interface.
 **/
typedef enum
{
  /* visible columns */
  LUNAR_COLUMN_DATE_ACCESSED,
  LUNAR_COLUMN_DATE_MODIFIED,
  LUNAR_COLUMN_GROUP,
  LUNAR_COLUMN_MIME_TYPE,
  LUNAR_COLUMN_NAME,
  LUNAR_COLUMN_OWNER,
  LUNAR_COLUMN_PERMISSIONS,
  LUNAR_COLUMN_SIZE,
  LUNAR_COLUMN_SIZE_IN_BYTES,
  LUNAR_COLUMN_TYPE,

  /* special internal columns */
  LUNAR_COLUMN_FILE,
  LUNAR_COLUMN_FILE_NAME,

  /* number of columns */
  LUNAR_N_COLUMNS,

  /* number of visible columns */
  LUNAR_N_VISIBLE_COLUMNS = LUNAR_COLUMN_FILE,
} LunarColumn;

GType        lunar_column_get_type          (void)                      G_GNUC_CONST;
const gchar* lunar_column_string_from_value (LunarColumn  value);
gboolean     lunar_column_value_from_string (const gchar  *value_string,
                                              gint         *value);


#define LUNAR_TYPE_ICON_SIZE (lunar_icon_size_get_type ())

/**
 * LunarIconSize:
 * Icon sizes matching the various #LunarZoomLevel<!---->s.
 **/
typedef enum
{
  LUNAR_ICON_SIZE_16   =  16,
  LUNAR_ICON_SIZE_24   =  24,
  LUNAR_ICON_SIZE_32   =  32,
  LUNAR_ICON_SIZE_48   =  48,
  LUNAR_ICON_SIZE_64   =  64,
  LUNAR_ICON_SIZE_96   =  96,
  LUNAR_ICON_SIZE_128  = 128,
  LUNAR_ICON_SIZE_160  = 160,
  LUNAR_ICON_SIZE_192  = 192,
  LUNAR_ICON_SIZE_256  = 256,
} LunarIconSize;

GType lunar_icon_size_get_type (void) G_GNUC_CONST;


#define LUNAR_TYPE_THUMBNAIL_MODE (lunar_thumbnail_mode_get_type ())

/**
 * LunarThumbnailMode:
 * @LUNAR_THUMBNAIL_MODE_NEVER      : never show thumbnails.
 * @LUNAR_THUMBNAIL_MODE_ONLY_LOCAL : only show thumbnails on local filesystems.
 * @LUNAR_THUMBNAIL_MODE_ALWAYS     : always show thumbnails (everywhere).
 **/
typedef enum
{
  LUNAR_THUMBNAIL_MODE_NEVER,
  LUNAR_THUMBNAIL_MODE_ONLY_LOCAL,
  LUNAR_THUMBNAIL_MODE_ALWAYS
} LunarThumbnailMode;

GType lunar_thumbnail_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_TYPE_THUMBNAIL_SIZE (lunar_thumbnail_size_get_type ())

/**
 * LunarThumbnailSize:
 * @LUNAR_THUMBNAIL_NORMAL      : max 128px x 128px
 * @LUNAR_THUMBNAIL_LARGE       : max 256px x 256px
 **/
typedef enum
{
  LUNAR_THUMBNAIL_SIZE_NORMAL,
  LUNAR_THUMBNAIL_SIZE_LARGE
} LunarThumbnailSize;

GType       lunar_thumbnail_size_get_type (void)                               G_GNUC_CONST;
const char* lunar_thumbnail_size_get_nick (LunarThumbnailSize thumbnail_size) G_GNUC_CONST;


#define LUNAR_TYPE_PARALLEL_COPY_MODE (lunar_parallel_copy_mode_get_type ())

/**
 * LunarParallelCopyMode:
 * @LUNAR_PARALLEL_COPY_MODE_NEVER                   : copies will be done consecutively, one after another.
 * @LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL              : only do parallel copies when source and destination are local files.
 * @LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL_SAME_DEVICES : same as only local, but only if source and destination devices are the same.
 * @LUNAR_PARALLEL_COPY_MODE_ALWAYS                  : all copies will be started immediately.
 **/
typedef enum
{
  LUNAR_PARALLEL_COPY_MODE_NEVER,
  LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL,
  LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL_SAME_DEVICES,
  LUNAR_PARALLEL_COPY_MODE_ALWAYS
} LunarParallelCopyMode;

GType lunar_parallel_copy_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_TYPE_RECURSIVE_PERMISSIONS (lunar_recursive_permissions_get_type ())

/**
 * LunarRecursivePermissionsMode:
 * @LUNAR_RECURSIVE_PERMISSIONS_ASK    : ask the user every time permissions are changed.
 * @LUNAR_RECURSIVE_PERMISSIONS_ALWAYS : always apply the change recursively.
 * @LUNAR_RECURSIVE_PERMISSIONS_NEVER  : never apply the change recursively.
 *
 * Modus operandi when changing permissions.
 **/
typedef enum
{
  LUNAR_RECURSIVE_PERMISSIONS_ASK,
  LUNAR_RECURSIVE_PERMISSIONS_ALWAYS,
  LUNAR_RECURSIVE_PERMISSIONS_NEVER,
} LunarRecursivePermissionsMode;

GType lunar_recursive_permissions_get_type (void) G_GNUC_CONST;


#define LUNAR_TYPE_ZOOM_LEVEL (lunar_zoom_level_get_type ())

/**
 * LunarZoomLevel:
 * Lists the various zoom levels supported by Lunar's
 * folder views.
 **/
typedef enum
{
  LUNAR_ZOOM_LEVEL_25_PERCENT,
  LUNAR_ZOOM_LEVEL_38_PERCENT,
  LUNAR_ZOOM_LEVEL_50_PERCENT,
  LUNAR_ZOOM_LEVEL_75_PERCENT,
  LUNAR_ZOOM_LEVEL_100_PERCENT,
  LUNAR_ZOOM_LEVEL_150_PERCENT,
  LUNAR_ZOOM_LEVEL_200_PERCENT,
  LUNAR_ZOOM_LEVEL_250_PERCENT,
  LUNAR_ZOOM_LEVEL_300_PERCENT,
  LUNAR_ZOOM_LEVEL_400_PERCENT,

  /*< private >*/
  LUNAR_ZOOM_N_LEVELS,
} LunarZoomLevel;

GType               lunar_zoom_level_get_type            (void)                       G_GNUC_CONST;
LunarThumbnailSize lunar_zoom_level_to_thumbnail_size   (LunarZoomLevel zoom_level) G_GNUC_CONST;


#define LUNAR_TYPE_JOB_RESPONSE (lunar_job_response_get_type ())

/**
 * LunarJobResponse:
 * @LUNAR_JOB_RESPONSE_YES         :
 * @LUNAR_JOB_RESPONSE_YES_ALL     :
 * @LUNAR_JOB_RESPONSE_NO          :
 * @LUNAR_JOB_RESPONSE_NO_ALL      :
 * @LUNAR_JOB_RESPONSE_CANCEL      :
 * @LUNAR_JOB_RESPONSE_RETRY       :
 * @LUNAR_JOB_RESPONSE_FORCE       :
 * @LUNAR_JOB_RESPONSE_REPLACE     :
 * @LUNAR_JOB_RESPONSE_REPLACE_ALL :
 * @LUNAR_JOB_RESPONSE_SKIP        :
 * @LUNAR_JOB_RESPONSE_SKIP_ALL    :
 * @LUNAR_JOB_RESPONSE_RENAME      :
 * @LUNAR_JOB_RESPONSE_RENAME_ALL  :
 *
 * Possible responses for the LunarJob::ask signal.
 **/
typedef enum /*< flags >*/
{
  LUNAR_JOB_RESPONSE_YES         = 1 << 0,
  LUNAR_JOB_RESPONSE_YES_ALL     = 1 << 1,
  LUNAR_JOB_RESPONSE_NO          = 1 << 2,
  LUNAR_JOB_RESPONSE_CANCEL      = 1 << 3,
  LUNAR_JOB_RESPONSE_NO_ALL      = 1 << 4,
  LUNAR_JOB_RESPONSE_RETRY       = 1 << 5,
  LUNAR_JOB_RESPONSE_FORCE       = 1 << 6,
  LUNAR_JOB_RESPONSE_REPLACE     = 1 << 7,
  LUNAR_JOB_RESPONSE_REPLACE_ALL = 1 << 8,
  LUNAR_JOB_RESPONSE_SKIP        = 1 << 9,
  LUNAR_JOB_RESPONSE_SKIP_ALL    = 1 << 10,
  LUNAR_JOB_RESPONSE_RENAME      = 1 << 11,
  LUNAR_JOB_RESPONSE_RENAME_ALL  = 1 << 12,
} LunarJobResponse;
#define LUNAR_JOB_RESPONSE_MAX_INT 12

GType lunar_job_response_get_type (void) G_GNUC_CONST;


#define LUNAR_TYPE_FILE_MODE (lunar_file_mode_get_type ())

/**
 * LunarFileMode:
 *
 * Special flags and permissions of a filesystem entity.
 **/
typedef enum /*< flags >*/
{
  LUNAR_FILE_MODE_SUID       = 04000,
  LUNAR_FILE_MODE_SGID       = 02000,
  LUNAR_FILE_MODE_STICKY     = 01000,
  LUNAR_FILE_MODE_USR_ALL    = 00700,
  LUNAR_FILE_MODE_USR_READ   = 00400,
  LUNAR_FILE_MODE_USR_WRITE  = 00200,
  LUNAR_FILE_MODE_USR_EXEC   = 00100,
  LUNAR_FILE_MODE_GRP_ALL    = 00070,
  LUNAR_FILE_MODE_GRP_READ   = 00040,
  LUNAR_FILE_MODE_GRP_WRITE  = 00020,
  LUNAR_FILE_MODE_GRP_EXEC   = 00010,
  LUNAR_FILE_MODE_OTH_ALL    = 00007,
  LUNAR_FILE_MODE_OTH_READ   = 00004,
  LUNAR_FILE_MODE_OTH_WRITE  = 00002,
  LUNAR_FILE_MODE_OTH_EXEC   = 00001,
} LunarFileMode;

GType lunar_file_mode_get_type (void) G_GNUC_CONST;

G_END_DECLS;

#endif /* !__LUNAR_ENUM_TYPES_H__ */
