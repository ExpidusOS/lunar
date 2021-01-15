/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
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

#include <endo/endo.h>

#include <lunar/lunar-component.h>
#include <lunar/lunar-private.h>



static void lunar_component_class_init (gpointer klass);



GType
lunar_component_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("LunarComponent"),
                                            sizeof (LunarComponentIface),
                                            (GClassInitFunc) (void (*)(void)) lunar_component_class_init,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, LUNAR_TYPE_NAVIGATOR);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static void
lunar_component_class_init (gpointer klass)
{
  /**
   * LunarComponent:selected-files:
   *
   * The list of currently selected files for the #LunarWindow to
   * which this #LunarComponent belongs.
   *
   * The exact semantics of this property depend on the implementor
   * of this interface. For example, #LunarComponent<!---->s will update
   * the property depending on the users selection with the
   * #GtkTreeComponent or #EndoIconComponent. While other components in a window,
   * like the #LunarShortcutsPane, will not update this property on
   * their own, but rely on #LunarWindow to synchronize the selected
   * files list with the selected files list from the active #LunarComponent.
   *
   * This way all components can behave properly depending on the
   * set of selected files even though they don't have direct access
   * to the #LunarComponent.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boxed ("selected-files",
                                                           "selected-files",
                                                           "selected-files",
                                                           LUNARX_TYPE_FILE_INFO_LIST,
                                                           ENDO_PARAM_READWRITE));
}



/**
 * lunar_component_get_selected_files:
 * @component : a #LunarComponent instance.
 *
 * Returns the set of selected files. Check the description
 * of the :selected-files property for details.
 *
 * Return value: the set of selected files.
 **/
GList*
lunar_component_get_selected_files (LunarComponent *component)
{
  _lunar_return_val_if_fail (LUNAR_IS_COMPONENT (component), NULL);
  return (*LUNAR_COMPONENT_GET_IFACE (component)->get_selected_files) (component);
}



/**
 * lunar_component_set_selected_files:
 * @component      : a #LunarComponent instance.
 * @selected_files : a #GList of #LunarFile<!---->s.
 *
 * Sets the selected files for @component to @selected_files.
 * Check the description of the :selected-files property for
 * details.
 **/
void
lunar_component_set_selected_files (LunarComponent *component,
                                     GList           *selected_files)
{
  _lunar_return_if_fail (LUNAR_IS_COMPONENT (component));
  (*LUNAR_COMPONENT_GET_IFACE (component)->set_selected_files) (component, selected_files);
}



/**
 * lunar_component_restore_selection:
 * @component      : a #LunarComponent instance.
 *
 * Make sure that the @selected_files stay selected when a @component
 * updates. This may be necessary on row changes etc.
 **/
void
lunar_component_restore_selection (LunarComponent *component)
{
  GList           *selected_files;

  _lunar_return_if_fail (LUNAR_IS_COMPONENT (component));

  selected_files = lunar_g_file_list_copy (lunar_component_get_selected_files (component));
  lunar_component_set_selected_files (component, selected_files);
  lunar_g_file_list_free (selected_files);
}
