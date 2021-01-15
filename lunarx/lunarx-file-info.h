/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
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

#if !defined(LUNARX_INSIDE_LUNARX_H) && !defined(LUNARX_COMPILATION)
#error "Only <lunarx/lunarx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __LUNARX_FILE_INFO_H__
#define __LUNARX_FILE_INFO_H__

#include <gio/gio.h>

#include <lunarx/lunarx-config.h>

G_BEGIN_DECLS

/*
 * File information namespaces available in the #GFileInfo returned by
 * lunarx_file_info_get_file_info().
 */
#define LUNARX_FILE_INFO_NAMESPACE \
  "access::*," \
  "id::filesystem," \
  "mountable::can-mount,standard::target-uri," \
  "preview::*," \
  "standard::type,standard::is-hidden,standard::is-backup," \
  "standard::is-symlink,standard::name,standard::display-name," \
  "standard::size,standard::symlink-target," \
  "time::*," \
  "trash::*," \
  "unix::gid,unix::uid,unix::mode," \
  "metadata::emblems," \
  "metadata::lunar-view-type," \
  "metadata::lunar-sort-column,metadata::lunar-sort-order"



/*
 * Filesystem information namespaces available in the #GFileInfo
 * returned by lunarx_file_info_get_filesystem_info().
 */
#define LUNARX_FILESYSTEM_INFO_NAMESPACE \
  "filesystem::*"

typedef struct _LunarxFileInfoIface LunarxFileInfoIface;
typedef struct _LunarxFileInfo      LunarxFileInfo;

#define LUNARX_TYPE_FILE_INFO            (lunarx_file_info_get_type ())
#define LUNARX_FILE_INFO(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_FILE_INFO, LunarxFileInfo))
#define LUNARX_IS_FILE_INFO(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_FILE_INFO))
#define LUNARX_FILE_INFO_GET_IFACE(obj)  (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNARX_TYPE_FILE_INFO, LunarxFileInfoIface))

/**
 * LunarxFileInfoIface:
 * @get_name: See lunarx_file_info_get_name().
 * @get_uri: See lunarx_file_info_get_uri().
 * @get_parent_uri: See lunarx_file_info_get_parent_uri().
 * @get_uri_scheme: See lunarx_file_info_get_uri_scheme().
 * @get_mime_type: See lunarx_file_info_get_mime_type().
 * @has_mime_type: See lunarx_file_info_has_mime_type().
 * @is_directory: See lunarx_file_info_is_directory().
 * @get_file_info: See lunarx_file_info_get_file_info().
 * @get_filesystem_info: See lunarx_filesystem_info_get_filesystem_info().
 * @get_location: See lunarx_location_get_location().
 * @changed: See lunarx_file_info_changed().
 * @renamed: See lunarx_file_info_renamed().
 *
 * Interface with virtual methods implemented by the file manager and accessible
 * from the extensions.
 */

struct _LunarxFileInfoIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/

  /* virtual methods */
  gchar     *(*get_name)            (LunarxFileInfo *file_info);

  gchar     *(*get_uri)             (LunarxFileInfo *file_info);
  gchar     *(*get_parent_uri)      (LunarxFileInfo *file_info);
  gchar     *(*get_uri_scheme)      (LunarxFileInfo *file_info);

  gchar     *(*get_mime_type)       (LunarxFileInfo *file_info);
  gboolean   (*has_mime_type)       (LunarxFileInfo *file_info,
                                     const gchar     *mime_type);

  gboolean   (*is_directory)        (LunarxFileInfo *file_info);

  GFileInfo *(*get_file_info)       (LunarxFileInfo *file_info);
  GFileInfo *(*get_filesystem_info) (LunarxFileInfo *file_info);
  GFile     *(*get_location)        (LunarxFileInfo *file_info);

  /*< private >*/
  void (*reserved0) (void);
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
  void (*reserved5) (void);
  void (*reserved6) (void);

  /*< public >*/

  /* signals */
  void (*changed) (LunarxFileInfo *file_info);
  void (*renamed) (LunarxFileInfo *file_info);

  /*< private >*/
  void (*reserved7) (void);
  void (*reserved8) (void);
  void (*reserved9) (void);
};

GType      lunarx_file_info_get_type            (void) G_GNUC_CONST;

gchar     *lunarx_file_info_get_name            (LunarxFileInfo *file_info);
gchar     *lunarx_file_info_get_uri             (LunarxFileInfo *file_info);
gchar     *lunarx_file_info_get_parent_uri      (LunarxFileInfo *file_info);
gchar     *lunarx_file_info_get_uri_scheme      (LunarxFileInfo *file_info);

gchar     *lunarx_file_info_get_mime_type       (LunarxFileInfo *file_info);
gboolean   lunarx_file_info_has_mime_type       (LunarxFileInfo *file_info,
                                                  const gchar     *mime_type);

gboolean   lunarx_file_info_is_directory        (LunarxFileInfo *file_info);

GFileInfo *lunarx_file_info_get_file_info       (LunarxFileInfo *file_info);
GFileInfo *lunarx_file_info_get_filesystem_info (LunarxFileInfo *file_info);
GFile     *lunarx_file_info_get_location        (LunarxFileInfo *file_info);

void       lunarx_file_info_changed             (LunarxFileInfo *file_info);
void       lunarx_file_info_renamed             (LunarxFileInfo *file_info);


#define LUNARX_TYPE_FILE_INFO_LIST (lunarx_file_info_list_get_type ())

GType      lunarx_file_info_list_get_type       (void) G_GNUC_CONST;

GList     *lunarx_file_info_list_copy           (GList           *file_infos);
void       lunarx_file_info_list_free           (GList           *file_infos);

G_END_DECLS

#endif /* !__LUNARX_FILE_INFO_H__ */
