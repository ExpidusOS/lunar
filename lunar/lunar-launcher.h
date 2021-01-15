/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2020 Alexander Schwinn <alexxcons@expidus.org>
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

#ifndef __LUNAR_LAUNCHER_H__
#define __LUNAR_LAUNCHER_H__

#include <lunar/lunar-component.h>

G_BEGIN_DECLS;

typedef struct _LunarLauncherClass LunarLauncherClass;
typedef struct _LunarLauncher      LunarLauncher;

#define LUNAR_TYPE_LAUNCHER            (lunar_launcher_get_type ())
#define LUNAR_LAUNCHER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_LAUNCHER, LunarLauncher))
#define LUNAR_LAUNCHER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_LAUNCHER, LunarLauncherClass))
#define LUNAR_IS_LAUNCHER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_LAUNCHER))
#define LUNAR_IS_LAUNCHER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_LAUNCHER))
#define LUNAR_LAUNCHER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_LAUNCHER, LunarLauncherClass))

/* #ExpidusGtkActionEntrys provided by this widget */
typedef enum
{
  LUNAR_LAUNCHER_ACTION_OPEN,
  LUNAR_LAUNCHER_ACTION_EXECUTE,
  LUNAR_LAUNCHER_ACTION_OPEN_IN_TAB,
  LUNAR_LAUNCHER_ACTION_OPEN_IN_WINDOW,
  LUNAR_LAUNCHER_ACTION_OPEN_WITH_OTHER,
  LUNAR_LAUNCHER_ACTION_SENDTO_MENU,
  LUNAR_LAUNCHER_ACTION_SENDTO_SHORTCUTS,
  LUNAR_LAUNCHER_ACTION_SENDTO_DESKTOP,
  LUNAR_LAUNCHER_ACTION_PROPERTIES,
  LUNAR_LAUNCHER_ACTION_MAKE_LINK,
  LUNAR_LAUNCHER_ACTION_DUPLICATE,
  LUNAR_LAUNCHER_ACTION_RENAME,
  LUNAR_LAUNCHER_ACTION_EMPTY_TRASH,
  LUNAR_LAUNCHER_ACTION_CREATE_FOLDER,
  LUNAR_LAUNCHER_ACTION_CREATE_DOCUMENT,
  LUNAR_LAUNCHER_ACTION_RESTORE,
  LUNAR_LAUNCHER_ACTION_MOVE_TO_TRASH,
  LUNAR_LAUNCHER_ACTION_DELETE,
  LUNAR_LAUNCHER_ACTION_TRASH_DELETE,
  LUNAR_LAUNCHER_ACTION_PASTE,
  LUNAR_LAUNCHER_ACTION_PASTE_INTO_FOLDER,
  LUNAR_LAUNCHER_ACTION_COPY,
  LUNAR_LAUNCHER_ACTION_CUT,
  LUNAR_LAUNCHER_ACTION_MOUNT,
  LUNAR_LAUNCHER_ACTION_UNMOUNT,
  LUNAR_LAUNCHER_ACTION_EJECT,
} LunarLauncherAction;

typedef enum
{
  LUNAR_LAUNCHER_CHANGE_DIRECTORY,
  LUNAR_LAUNCHER_OPEN_AS_NEW_TAB,
  LUNAR_LAUNCHER_OPEN_AS_NEW_WINDOW,
  LUNAR_LAUNCHER_NO_ACTION,
} LunarLauncherFolderOpenAction;

GType           lunar_launcher_get_type                             (void) G_GNUC_CONST;
void            lunar_launcher_activate_selected_files              (LunarLauncher                 *launcher,
                                                                      LunarLauncherFolderOpenAction  action,
                                                                      GAppInfo                       *app_info);
void            lunar_launcher_open_selected_folders                (LunarLauncher                 *launcher,
                                                                      gboolean                        open_in_tabs);
void            lunar_launcher_set_widget                           (LunarLauncher                 *launcher,
                                                                      GtkWidget                      *widget);
GtkWidget      *lunar_launcher_get_widget                           (LunarLauncher                 *launcher);
void            lunar_launcher_append_accelerators                  (LunarLauncher                 *launcher,
                                                                      GtkAccelGroup                  *accel_group);
GtkWidget      *lunar_launcher_append_menu_item                     (LunarLauncher                 *launcher,
                                                                      GtkMenuShell                   *menu,
                                                                      LunarLauncherAction            action,
                                                                      gboolean                        force);
gboolean        lunar_launcher_append_open_section                  (LunarLauncher                 *launcher,
                                                                      GtkMenuShell                   *menu,
                                                                      gboolean                        support_tabs,
                                                                      gboolean                        support_change_directory,
                                                                      gboolean                        force);
gboolean        lunar_launcher_append_custom_actions                (LunarLauncher                 *launcher,
                                                                      GtkMenuShell                   *menu);
gboolean        lunar_launcher_check_uca_key_activation             (LunarLauncher                 *launcher,
                                                                      GdkEventKey                    *key_event);
void            lunar_launcher_action_mount                         (LunarLauncher                 *launcher);
void            lunar_launcher_action_unmount                       (LunarLauncher                 *launcher);
void            lunar_launcher_action_eject                         (LunarLauncher                 *launcher);


G_END_DECLS;

#endif /* !__LUNAR_LAUNCHER_H__ */
