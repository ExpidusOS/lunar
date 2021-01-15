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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib/gi18n-lib.h>

#include <lunarx/lunarx-private.h>
#include <lunarx/lunarx-menu-item.h>
#include <lunarx/lunarx-menu.h>



/**
 * SECTION: lunarx-menu
 * @short_description: The base class for submenus added to the context menus
 * @title: LunarxMenu
 * @include: lunarx/lunarx.h
 *
 * The class for submenus that can be added to Lunar's context menus. Extensions
 * can provide LunarxMenu objects by attaching them to LunarxMenuItem objects,
 * using lunarx_menu_item_set_menu().
 */



static void lunarx_menu_finalize (GObject      *object);



struct _LunarxMenuPrivate
{
  GList *items;
};



G_DEFINE_TYPE_WITH_PRIVATE (LunarxMenu, lunarx_menu, G_TYPE_OBJECT)



static void
lunarx_menu_class_init (LunarxMenuClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunarx_menu_finalize;
}



static void
lunarx_menu_init (LunarxMenu *menu)
{
  menu->priv = lunarx_menu_get_instance_private (menu);
  menu->priv->items = NULL;
}



static void
lunarx_menu_finalize (GObject *object)
{
  LunarxMenu *menu = LUNARX_MENU (object);

  if (menu->priv->items)
    g_list_free (menu->priv->items);

  (*G_OBJECT_CLASS (lunarx_menu_parent_class)->finalize) (object);
}



/**
 * lunarx_menu_new:
 *
 * Creates a new menu that can be added to the toolbar or to a contextual menu.
 *
 * Returns: a newly created #LunarxMenu
 */
LunarxMenu *
lunarx_menu_new (void)
{
  return g_object_new (LUNARX_TYPE_MENU, NULL);
}



/**
 * lunarx_menu_append_item:
 * @menu: a #LunarxMenu
 * @item: a #LunarxMenuItem
 */
void
lunarx_menu_append_item (LunarxMenu *menu, LunarxMenuItem *item)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (item != NULL);

  menu->priv->items = g_list_append (menu->priv->items, g_object_ref (item));
}



/**
 * lunarx_menu_prepend_item:
 * @menu: a #LunarxMenu
 * @item: a #LunarxMenuItem
 */
void
lunarx_menu_prepend_item (LunarxMenu *menu, LunarxMenuItem *item)
{
  g_return_if_fail (menu != NULL);
  g_return_if_fail (item != NULL);

  menu->priv->items = g_list_prepend (menu->priv->items, g_object_ref (item));
}



/**
 * lunarx_menu_get_items:
 * @menu: a #LunarxMenu
 *
 * Returns: (element-type LunarxMenuItem) (transfer full): the provided #LunarxMenuItem list
 * Must be freed with lunarx_menu_item_list_free() after usage
 */
GList *
lunarx_menu_get_items (LunarxMenu *menu)
{
  GList *items;

  g_return_val_if_fail (menu != NULL, NULL);

  items = g_list_copy (menu->priv->items);
  g_list_foreach (items, (GFunc) (void (*)(void)) g_object_ref, NULL);

  return items;
}
