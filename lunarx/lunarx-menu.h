/*-
 * Copyright (c) 2017 Andre Miranda <andreldm@expidus.org>
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

#if !defined(LUNARX_INSIDE_LUNARX_H) && !defined(LUNARX_COMPILATION)
#error "Only <lunarx/lunarx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __LUNARX_MENU_H__
#define __LUNARX_MENU_H__

#include <glib-object.h>

G_BEGIN_DECLS

/* LunarxMenu types */
typedef struct _LunarxMenuPrivate LunarxMenuPrivate;
typedef struct _LunarxMenuClass   LunarxMenuClass;
typedef struct _LunarxMenu        LunarxMenu;

/* LunarxMenuItem types */
typedef struct _LunarxMenuItemPrivate LunarxMenuItemPrivate;
typedef struct _LunarxMenuItemClass   LunarxMenuItemClass;
typedef struct _LunarxMenuItem        LunarxMenuItem;




/* LunarxMenu defines */
#define LUNARX_TYPE_MENU            (lunarx_menu_get_type ())
#define LUNARX_MENU(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_MENU, LunarxMenu))
#define LUNARX_MENU_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNARX_TYPE_MENU, LunarxMenuClass))
#define LUNARX_IS_MENU(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_MENU))
#define LUNARX_IS_MENU_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), LUNARX_TYPE_MENU))
#define LUNARX_MENU_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), LUNARX_TYPE_MENU))

/* LunarxMenuItem defines */
#define LUNARX_TYPE_MENU_ITEM            (lunarx_menu_item_get_type ())
#define LUNARX_MENU_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_MENU_ITEM, LunarxMenuItem))
#define LUNARX_MENU_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNARX_TYPE_MENU_ITEM, LunarxMenuItemClass))
#define LUNARX_IS_MENU_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_MENU_ITEM))
#define LUNARX_IS_MENU_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((obj), LUNARX_TYPE_MENU_ITEM))
#define LUNARX_MENU_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS((obj), LUNARX_TYPE_MENU_ITEM))



/* LunarxMenu structs */
struct _LunarxMenuClass {
  GObjectClass __parent__;
};

struct _LunarxMenu {
  GObject __parent__;
  LunarxMenuPrivate *priv;
};

/* LunarxMenuItem structs */
struct _LunarxMenuItemClass {
  GObjectClass __parent__;
  void (*activate) (LunarxMenuItem *item);
};

struct _LunarxMenuItem {
  GObject __parent__;
  LunarxMenuItemPrivate *priv;
};



/* LunarxMenu methods */
GType         lunarx_menu_get_type       (void) G_GNUC_CONST;

LunarxMenu  *lunarx_menu_new            (void) G_GNUC_MALLOC;

void          lunarx_menu_append_item    (LunarxMenu     *menu,
                                           LunarxMenuItem *item);

void          lunarx_menu_prepend_item   (LunarxMenu     *menu,
                                           LunarxMenuItem *item);

GList*        lunarx_menu_get_items      (LunarxMenu     *menu);

/* LunarxMenuItem methods */
GType             lunarx_menu_item_get_type      (void) G_GNUC_CONST;

LunarxMenuItem  *lunarx_menu_item_new           (const gchar     *name,
                                                   const gchar     *label,
                                                   const gchar     *tooltip,
                                                   const gchar     *icon) G_GNUC_MALLOC;

void              lunarx_menu_item_activate      (LunarxMenuItem *item);

gboolean          lunarx_menu_item_get_sensitive (LunarxMenuItem *item);
void              lunarx_menu_item_set_sensitive (LunarxMenuItem *item,
                                                   gboolean         sensitive);

void              lunarx_menu_item_list_free     (GList           *items);

void              lunarx_menu_item_set_menu      (LunarxMenuItem *item,
                                                   LunarxMenu     *menu);

G_END_DECLS

#endif /* !__LUNARX_MENU_H__ */
