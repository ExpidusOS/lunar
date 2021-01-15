/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNARX_MENU_PROVIDER_H__
#define __LUNARX_MENU_PROVIDER_H__

#include <gtk/gtk.h>

#include <lunarx/lunarx-file-info.h>

G_BEGIN_DECLS

typedef struct _LunarxMenuProviderIface LunarxMenuProviderIface;
typedef struct _LunarxMenuProvider      LunarxMenuProvider;

#define LUNARX_TYPE_MENU_PROVIDER           (lunarx_menu_provider_get_type ())
#define LUNARX_MENU_PROVIDER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_MENU_PROVIDER, LunarxMenuProvider))
#define LUNARX_IS_MENU_PROVIDER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_MENU_PROVIDER))
#define LUNARX_MENU_PROVIDER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNARX_TYPE_MENU_PROVIDER, LunarxMenuProviderIface))

/**
 * LunarxMenuProviderIface:
 * @get_file_menu_items: See lunarx_menu_provider_get_file_menu_items().
 * @get_folder_menu_items: See lunarx_menu_provider_get_folder_menu_items().
 * @get_dnd_menu_items: See lunarx_menu_provider_get_dnd_menu_items().
 *
 * Interface with virtual methods implemented by extensions that provide
 * additional menu items for the file manager's context menus.
 * Providers don't need to implement all of the virtual methods listed in
 * the interface.
 */

struct _LunarxMenuProviderIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/
  GList *(*get_file_menu_items)    (LunarxMenuProvider *provider,
                                    GtkWidget           *window,
                                    GList               *files);

  GList *(*get_folder_menu_items)  (LunarxMenuProvider *provider,
                                    GtkWidget           *window,
                                    LunarxFileInfo     *folder);

  GList *(*get_dnd_menu_items)     (LunarxMenuProvider *provider,
                                    GtkWidget           *window,
                                    LunarxFileInfo     *folder,
                                    GList               *files);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
};

GType  lunarx_menu_provider_get_type           (void) G_GNUC_CONST;

GList *lunarx_menu_provider_get_file_menu_items   (LunarxMenuProvider *provider,
                                                    GtkWidget           *window,
                                                    GList               *files) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GList *lunarx_menu_provider_get_folder_menu_items (LunarxMenuProvider *provider,
                                                    GtkWidget           *window,
                                                    LunarxFileInfo     *folder) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

GList *lunarx_menu_provider_get_dnd_menu_items    (LunarxMenuProvider *provider,
                                                    GtkWidget           *window,
                                                    LunarxFileInfo     *folder,
                                                    GList               *files) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS

#endif /* !__LUNARX_MENU_PROVIDER_H__ */
