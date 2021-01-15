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

#ifndef __LUNAR_SHORTCUTS_MODEL_H__
#define __LUNAR_SHORTCUTS_MODEL_H__

#include <lunar/lunar-file.h>
#include <lunar/lunar-device.h>

G_BEGIN_DECLS;

typedef struct _LunarShortcutsModelClass LunarShortcutsModelClass;
typedef struct _LunarShortcutsModel      LunarShortcutsModel;
typedef enum   _LunarShortcutGroup       LunarShortcutGroup;

#define LUNAR_TYPE_SHORTCUTS_MODEL            (lunar_shortcuts_model_get_type ())
#define LUNAR_SHORTCUTS_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_SHORTCUTS_MODEL, LunarShortcutsModel))
#define LUNAR_SHORTCUTS_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_SHORTCUTS_MODEL, LunarShortcutsModelClass))
#define LUNAR_IS_SHORTCUTS_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_SHORTCUTS_MODEL))
#define LUNAR_IS_SHORTCUTS_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_SHORTCUTS_MODEL))
#define LUNAR_SHORTCUTS_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_MODEL_SHORTCUTS_MODEL, LunarShortcutsModelClass))

typedef enum
{
  LUNAR_SHORTCUTS_MODEL_COLUMN_IS_HEADER,
  LUNAR_SHORTCUTS_MODEL_COLUMN_IS_ITEM,
  LUNAR_SHORTCUTS_MODEL_COLUMN_VISIBLE,
  LUNAR_SHORTCUTS_MODEL_COLUMN_NAME,
  LUNAR_SHORTCUTS_MODEL_COLUMN_TOOLTIP,
  LUNAR_SHORTCUTS_MODEL_COLUMN_FILE,
  LUNAR_SHORTCUTS_MODEL_COLUMN_LOCATION,
  LUNAR_SHORTCUTS_MODEL_COLUMN_GICON,
  LUNAR_SHORTCUTS_MODEL_COLUMN_DEVICE,
  LUNAR_SHORTCUTS_MODEL_COLUMN_MUTABLE,
  LUNAR_SHORTCUTS_MODEL_COLUMN_CAN_EJECT,
  LUNAR_SHORTCUTS_MODEL_COLUMN_GROUP,
  LUNAR_SHORTCUTS_MODEL_COLUMN_BUSY,
  LUNAR_SHORTCUTS_MODEL_COLUMN_BUSY_PULSE,
  LUNAR_SHORTCUTS_MODEL_COLUMN_HIDDEN,
  LUNAR_SHORTCUTS_MODEL_N_COLUMNS,
} LunarShortcutsModelColumn;

#define LUNAR_SHORTCUT_GROUP_DEVICES (LUNAR_SHORTCUT_GROUP_DEVICES_HEADER \
                                       | LUNAR_SHORTCUT_GROUP_DEVICES_FILESYSTEM \
                                       | LUNAR_SHORTCUT_GROUP_DEVICES_VOLUMES \
                                       | LUNAR_SHORTCUT_GROUP_DEVICES_MOUNTS)
#define LUNAR_SHORTCUT_GROUP_PLACES  (LUNAR_SHORTCUT_GROUP_PLACES_HEADER \
                                       | LUNAR_SHORTCUT_GROUP_PLACES_DEFAULT \
                                       | LUNAR_SHORTCUT_GROUP_PLACES_TRASH \
                                       | LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS \
                                       | LUNAR_SHORTCUT_GROUP_PLACES_COMPUTER )
#define LUNAR_SHORTCUT_GROUP_NETWORK (LUNAR_SHORTCUT_GROUP_NETWORK_HEADER \
                                       | LUNAR_SHORTCUT_GROUP_NETWORK_DEFAULT \
                                       | LUNAR_SHORTCUT_GROUP_NETWORK_MOUNTS)
#define LUNAR_SHORTCUT_GROUP_HEADER  (LUNAR_SHORTCUT_GROUP_DEVICES_HEADER \
                                       | LUNAR_SHORTCUT_GROUP_PLACES_HEADER \
                                       | LUNAR_SHORTCUT_GROUP_NETWORK_HEADER)

enum _LunarShortcutGroup
{
  /* LUNAR_SHORTCUT_GROUP_PLACES */
  LUNAR_SHORTCUT_GROUP_PLACES_HEADER      = (1 << 0),  /* places header */
  LUNAR_SHORTCUT_GROUP_PLACES_COMPUTER    = (1 << 1),  /* computer */
  LUNAR_SHORTCUT_GROUP_PLACES_DEFAULT     = (1 << 2),  /* home and desktop */
  LUNAR_SHORTCUT_GROUP_PLACES_TRASH       = (1 << 3),  /* trash */
  LUNAR_SHORTCUT_GROUP_PLACES_BOOKMARKS   = (1 << 4),  /* gtk-bookmarks */

  /* LUNAR_SHORTCUT_GROUP_DEVICES */
  LUNAR_SHORTCUT_GROUP_DEVICES_HEADER     = (1 << 5),  /* devices header */
  LUNAR_SHORTCUT_GROUP_DEVICES_FILESYSTEM = (1 << 6),  /* local filesystem */
  LUNAR_SHORTCUT_GROUP_DEVICES_VOLUMES    = (1 << 7),  /* local LunarDevices */
  LUNAR_SHORTCUT_GROUP_DEVICES_MOUNTS     = (1 << 8),  /* local mounts, like cameras and archives */

  /* LUNAR_SHORTCUT_GROUP_NETWORK */
  LUNAR_SHORTCUT_GROUP_NETWORK_HEADER     = (1 << 9),  /* network header */
  LUNAR_SHORTCUT_GROUP_NETWORK_DEFAULT    = (1 << 10), /* browse network */
  LUNAR_SHORTCUT_GROUP_NETWORK_MOUNTS     = (1 << 11), /* remote LunarDevices */
};



GType                  lunar_shortcuts_model_get_type      (void) G_GNUC_CONST;

LunarShortcutsModel  *lunar_shortcuts_model_get_default   (void);

gboolean               lunar_shortcuts_model_has_bookmark  (LunarShortcutsModel *model,
                                                             GFile                *file);

gboolean               lunar_shortcuts_model_iter_for_file (LunarShortcutsModel *model,
                                                             LunarFile           *file,
                                                             GtkTreeIter          *iter);

gboolean               lunar_shortcuts_model_drop_possible (LunarShortcutsModel *model,
                                                             GtkTreePath          *path);

void                   lunar_shortcuts_model_add           (LunarShortcutsModel *model,
                                                             GtkTreePath          *dst_path,
                                                             gpointer              file);
void                   lunar_shortcuts_model_move          (LunarShortcutsModel *model,
                                                             GtkTreePath          *src_path,
                                                             GtkTreePath          *dst_path);
void                   lunar_shortcuts_model_remove        (LunarShortcutsModel *model,
                                                             GtkTreePath          *path);
void                   lunar_shortcuts_model_rename        (LunarShortcutsModel *model,
                                                             GtkTreeIter          *iter,
                                                             const gchar          *name);
void                   lunar_shortcuts_model_set_busy      (LunarShortcutsModel *model,
                                                             LunarDevice         *device,
                                                             gboolean              busy);
void                   lunar_shortcuts_model_set_hidden    (LunarShortcutsModel *model,
                                                             GtkTreePath          *path,
                                                             gboolean              hidden);

G_END_DECLS;

#endif /* !__LUNAR_SHORTCUTS_MODEL_H__ */
