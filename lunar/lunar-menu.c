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
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <lunar/lunar-menu.h>

#include <lunar/lunar-gtk-extensions.h>
#include <lunar/lunar-launcher.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-window.h>



/**
 * SECTION:lunar-menu
 * @Short_description: Wrapper of GtkMenu to simplify the creation commonly used menu-sections in lunar
 * @Title: LunarMenu
 *
 * #LunarMenu is a #GtkMenu which provides a unified menu-creation service for different lunar widgets.
 *
 * Based on the passed flags and selected sections, it fills itself with the requested menu-items
 * by creating them with #LunarLauncher.
 */



/* property identifiers */
enum
{
  PROP_0,
  PROP_MENU_TYPE,
  PROP_LAUNCHER,
  PROP_FORCE_SECTION_OPEN,
  PROP_TAB_SUPPORT_DISABLED,
  PROP_CHANGE_DIRECTORY_SUPPORT_DISABLED,
};

static void lunar_menu_finalize      (GObject                *object);
static void lunar_menu_get_property  (GObject                *object,
                                       guint                   prop_id,
                                       GValue                 *value,
                                       GParamSpec             *pspec);
static void lunar_menu_set_property  (GObject                *object,
                                       guint                   prop_uid,
                                       const GValue           *value,
                                       GParamSpec             *pspec);

struct _LunarMenuClass
{
  GtkMenuClass __parent__;
};

struct _LunarMenu
{
  GtkMenu __parent__;
  LunarLauncher  *launcher;

  /* true, if the 'open' section should be forced */
  gboolean         force_section_open;

  /* true, if 'open as new tab' should not be shown */
  gboolean         tab_support_disabled;

  /* true, if 'open' for folders, which would result in changing the directory, should not be shown */
  gboolean         change_directory_support_disabled;

  /* detailed type of the lunar menu */
  LunarMenuType   type;
};



static GQuark lunar_menu_handler_quark;

G_DEFINE_TYPE (LunarMenu, lunar_menu, GTK_TYPE_MENU)



static void
lunar_menu_class_init (LunarMenuClass *klass)
{
  GObjectClass *gobject_class;

  /* determine the "lunar-menu-handler" quark */
  lunar_menu_handler_quark = g_quark_from_static_string ("lunar-menu-handler");

  gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->finalize = lunar_menu_finalize;
  gobject_class->get_property = lunar_menu_get_property;
  gobject_class->set_property = lunar_menu_set_property;

  g_object_class_install_property (gobject_class,
                                   PROP_MENU_TYPE,
                                   g_param_spec_int ("menu-type",
                                                     "menu-type",
                                                     "menu-type",
                                                     0, N_LUNAR_MENU_TYPE - 1, 0, // min, max, default
                                                     G_PARAM_WRITABLE
                                                     | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (gobject_class,
                                   PROP_LAUNCHER,
                                   g_param_spec_object ("launcher",
                                                        "launcher",
                                                        "launcher",
                                                        LUNAR_TYPE_LAUNCHER,
                                                          G_PARAM_WRITABLE
                                                        | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (gobject_class,
                                   PROP_FORCE_SECTION_OPEN,
                                   g_param_spec_boolean ("force-section-open",
                                                         "force-section-open",
                                                         "force-section-open",
                                                         FALSE,
                                                           G_PARAM_WRITABLE
                                                         | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (gobject_class,
                                   PROP_TAB_SUPPORT_DISABLED,
                                   g_param_spec_boolean ("tab-support-disabled",
                                                         "tab-support-disabled",
                                                         "tab-support-disabled",
                                                         FALSE,
                                                           G_PARAM_WRITABLE
                                                         | G_PARAM_CONSTRUCT_ONLY));

  g_object_class_install_property (gobject_class,
                                   PROP_CHANGE_DIRECTORY_SUPPORT_DISABLED,
                                   g_param_spec_boolean ("change_directory-support-disabled",
                                                         "change_directory-support-disabled",
                                                         "change_directory-support-disabled",
                                                         FALSE,
                                                           G_PARAM_WRITABLE
                                                         | G_PARAM_CONSTRUCT_ONLY));
}



static void
lunar_menu_init (LunarMenu *menu)
{
  menu->force_section_open = FALSE;
  menu->type = FALSE;
  menu->tab_support_disabled = FALSE;
  menu->change_directory_support_disabled = FALSE;
}



static void
lunar_menu_finalize (GObject *object)
{
  LunarMenu *menu = LUNAR_MENU (object);

  g_object_unref (menu->launcher);

  (*G_OBJECT_CLASS (lunar_menu_parent_class)->finalize) (object);
}



static void
lunar_menu_get_property (GObject    *object,
                          guint       prop_id,
                          GValue     *value,
                          GParamSpec *pspec)
{
  switch (prop_id)
    {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_menu_set_property (GObject      *object,
                          guint         prop_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  LunarMenu *menu = LUNAR_MENU (object);

  switch (prop_id)
    {
    case PROP_MENU_TYPE:
      menu->type = g_value_get_int (value);
      break;

    case PROP_LAUNCHER:
      menu->launcher = g_value_dup_object (value);
      g_object_ref (G_OBJECT (menu->launcher));
     break;

    case PROP_FORCE_SECTION_OPEN:
      menu->force_section_open = g_value_get_boolean (value);
      break;

    case PROP_TAB_SUPPORT_DISABLED:
      menu->tab_support_disabled = g_value_get_boolean (value);
      break;

    case PROP_CHANGE_DIRECTORY_SUPPORT_DISABLED:
      menu->change_directory_support_disabled = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



/**
 * lunar_menu_add_sections:
 * @menu : a #LunarMenu instance
 * @menu_sections : bit enumeration of #LunarMenuSections which should be added to the #LunarMenu
 *
 * Method to add different sections of #GtkMenuItems to the #LunarMenu,
 * according to the selected #LunarMenuSections
 *
 * Return value: TRUE if any #GtkMenuItem was added
 **/
gboolean
lunar_menu_add_sections (LunarMenu         *menu,
                          LunarMenuSections  menu_sections)
{
  GtkWidget *window;
  gboolean   item_added;
  gboolean   force = menu->type == LUNAR_MENU_TYPE_WINDOW || menu->type == LUNAR_MENU_TYPE_CONTEXT_TREE_VIEW;

  _lunar_return_val_if_fail (LUNAR_IS_MENU (menu), FALSE);

  if (menu_sections & LUNAR_MENU_SECTION_CREATE_NEW_FILES)
    {
      item_added = FALSE;
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_CREATE_FOLDER, force) != NULL);

      /* No document creation for tree-view */
      if (menu->type != LUNAR_MENU_TYPE_CONTEXT_TREE_VIEW)
        item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_CREATE_DOCUMENT, force) != NULL);
      if (item_added)
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  if (menu_sections & LUNAR_MENU_SECTION_OPEN)
    {
      if (lunar_launcher_append_open_section (menu->launcher, GTK_MENU_SHELL (menu), !menu->tab_support_disabled, !menu->change_directory_support_disabled, menu->force_section_open))
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  if (menu_sections & LUNAR_MENU_SECTION_SENDTO)
    {
      if (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_SENDTO_MENU, FALSE) != NULL)
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  item_added = FALSE;
  if (menu_sections & LUNAR_MENU_SECTION_CUT)
    item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_CUT, force) != NULL);
  if (menu_sections & LUNAR_MENU_SECTION_COPY_PASTE)
    {
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_COPY, force) != NULL);
      if (menu->type == LUNAR_MENU_TYPE_CONTEXT_LOCATION_BUTTONS)
        item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_PASTE_INTO_FOLDER, force) != NULL);
      else
        item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_PASTE, force) != NULL);
    }
  if (item_added)
     expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));

  if (menu_sections & LUNAR_MENU_SECTION_TRASH_DELETE)
    {
      item_added = FALSE;
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_MOVE_TO_TRASH, force) != NULL);
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_DELETE, force) != NULL);
      if (item_added)
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }
  if (menu_sections & LUNAR_MENU_SECTION_EMPTY_TRASH)
    {
      if (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_EMPTY_TRASH, FALSE) != NULL )
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }
  if (menu_sections & LUNAR_MENU_SECTION_RESTORE)
    {
      if (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_RESTORE, FALSE) != NULL)
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  item_added = FALSE;
  if (menu_sections & LUNAR_MENU_SECTION_DUPLICATE)
    item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_DUPLICATE, force) != NULL);
  if (menu_sections & LUNAR_MENU_SECTION_MAKELINK)
    item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_MAKE_LINK, force) != NULL);
  if (menu_sections & LUNAR_MENU_SECTION_RENAME)
    item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_RENAME, force) != NULL);
  if (item_added)
     expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));

  if (menu_sections & LUNAR_MENU_SECTION_CUSTOM_ACTIONS)
    {
      if (lunar_launcher_append_custom_actions (menu->launcher, GTK_MENU_SHELL (menu)))
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  if (menu_sections & LUNAR_MENU_SECTION_MOUNTABLE)
    {
      item_added = FALSE;
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_MOUNT, FALSE) != NULL);
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_UNMOUNT, FALSE) != NULL);
      item_added |= (lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_EJECT, FALSE) != NULL);
      if (item_added)
         expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
    }

  if (menu_sections & LUNAR_MENU_SECTION_ZOOM)
    {
      window = lunar_launcher_get_widget (menu->launcher);
      if (LUNAR_IS_WINDOW (window))
        {
          lunar_window_append_menu_item (LUNAR_WINDOW (window), GTK_MENU_SHELL (menu), LUNAR_WINDOW_ACTION_ZOOM_IN);
          lunar_window_append_menu_item (LUNAR_WINDOW (window), GTK_MENU_SHELL (menu), LUNAR_WINDOW_ACTION_ZOOM_OUT);
          lunar_window_append_menu_item (LUNAR_WINDOW (window), GTK_MENU_SHELL (menu), LUNAR_WINDOW_ACTION_ZOOM_RESET);
          expidus_gtk_menu_append_seperator (GTK_MENU_SHELL (menu));
        }
    }

  if (menu_sections & LUNAR_MENU_SECTION_PROPERTIES)
      lunar_launcher_append_menu_item (menu->launcher, GTK_MENU_SHELL (menu), LUNAR_LAUNCHER_ACTION_PROPERTIES, FALSE);

  return TRUE;
}



/**
 * lunar_menu_get_launcher:
 * @menu : a #LunarMenu instance
 *
 * Return value: (transfer none): The launcher of this #LunarMenu instance
 **/
GtkWidget*
lunar_menu_get_launcher (LunarMenu *menu)
{
  _lunar_return_val_if_fail (LUNAR_IS_MENU (menu), NULL);
  return GTK_WIDGET (menu->launcher);
}



/**
 * lunar_menu_hide_accel_labels:
 * @menu : a #LunarMenu instance
 *
 * Will hide the accel_labels of all menu items of this menu
 **/
void
lunar_menu_hide_accel_labels (LunarMenu *menu)
{
  GList *children, *lp;

  _lunar_return_if_fail (LUNAR_IS_MENU (menu));

  children = gtk_container_get_children (GTK_CONTAINER (menu));
  for (lp = children; lp != NULL; lp = lp->next)
    expidus_gtk_menu_item_set_accel_label (lp->data, NULL);
  g_list_free (children);
}
