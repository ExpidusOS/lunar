/*-
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
#ifndef __LUNAR_MENU_H__
#define __LUNAR_MENU_H__

#include <gtk/gtk.h>

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarMenuClass LunarMenuClass;
typedef struct _LunarMenu      LunarMenu;

#define LUNAR_TYPE_MENU             (lunar_menu_get_type ())
#define LUNAR_MENU(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_MENU, LunarMenu))
#define LUNAR_MENU_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass),  LUNAR_TYPE_MENU, LunarMenuClass))
#define LUNAR_IS_MENU(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_MENU))
#define LUNAR_IS_MENU_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass),  LUNAR_TYPE_MENU))
#define LUNAR_MENU_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj),  LUNAR_TYPE_MENU, LunarMenu))

/* For window menu, some items are shown insensitive, instead of hidden */
typedef enum
{
  LUNAR_MENU_TYPE_WINDOW,
  LUNAR_MENU_TYPE_CONTEXT_STANDARD_VIEW,
  LUNAR_MENU_TYPE_CONTEXT_LOCATION_BUTTONS,
  LUNAR_MENU_TYPE_CONTEXT_RENAMER,
  LUNAR_MENU_TYPE_CONTEXT_TREE_VIEW,
  N_LUNAR_MENU_TYPE,
} LunarMenuType;

/* Bundles of #GtkMenuItems, which can be created by this widget */
typedef enum
{
  LUNAR_MENU_SECTION_OPEN             = 1 << 0,
  LUNAR_MENU_SECTION_SENDTO           = 1 << 1,
  LUNAR_MENU_SECTION_CREATE_NEW_FILES = 1 << 2,
  LUNAR_MENU_SECTION_CUT              = 1 << 3,
  LUNAR_MENU_SECTION_COPY_PASTE       = 1 << 4,
  LUNAR_MENU_SECTION_TRASH_DELETE     = 1 << 5,
  LUNAR_MENU_SECTION_EMPTY_TRASH      = 1 << 6,
  LUNAR_MENU_SECTION_RESTORE          = 1 << 7,
  LUNAR_MENU_SECTION_DUPLICATE        = 1 << 8,
  LUNAR_MENU_SECTION_MAKELINK         = 1 << 9,
  LUNAR_MENU_SECTION_RENAME           = 1 << 10,
  LUNAR_MENU_SECTION_CUSTOM_ACTIONS   = 1 << 11,
  LUNAR_MENU_SECTION_ZOOM             = 1 << 12,
  LUNAR_MENU_SECTION_PROPERTIES       = 1 << 13,
  LUNAR_MENU_SECTION_MOUNTABLE        = 1 << 14,

} LunarMenuSections;


GType      lunar_menu_get_type          (void) G_GNUC_CONST;

gboolean   lunar_menu_add_sections      (LunarMenu         *menu,
                                          LunarMenuSections  menu_sections);
GtkWidget* lunar_menu_get_launcher      (LunarMenu         *menu);
void       lunar_menu_hide_accel_labels (LunarMenu         *menu);

G_END_DECLS;

#endif /* !__LUNAR_CONTEXT_MENU_H__ */
