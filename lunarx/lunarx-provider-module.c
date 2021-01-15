/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gmodule.h>

#include <lunarx/lunarx-private.h>
#include <lunarx/lunarx-provider-module.h>
#include <lunarx/lunarx-provider-plugin.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_RESIDENT,
};



static void     lunarx_provider_module_plugin_init   (LunarxProviderPluginIface  *iface);
static void     lunarx_provider_module_get_property  (GObject                     *object,
                                                       guint                        prop_id,
                                                       GValue                      *value,
                                                       GParamSpec                  *pspec);
static void     lunarx_provider_module_set_property  (GObject                     *object,
                                                       guint                        prop_id,
                                                       const GValue                *value,
                                                       GParamSpec                  *pspec);
static gboolean lunarx_provider_module_load          (GTypeModule                 *type_module);
static void     lunarx_provider_module_unload        (GTypeModule                 *type_module);
static gboolean lunarx_provider_module_get_resident  (const LunarxProviderPlugin *plugin);
static void     lunarx_provider_module_set_resident  (LunarxProviderPlugin       *plugin,
                                                       gboolean                     resident);



struct _LunarxProviderModuleClass
{
  GTypeModuleClass __parent__;
};

struct _LunarxProviderModule
{
  GTypeModule __parent__;

  GModule *library;
  gboolean resident;

  void (*initialize) (LunarxProviderModule *module);
  void (*shutdown)   (void);
  void (*list_types) (const GType **types,
                      gint         *n_types);
};



G_DEFINE_TYPE_WITH_CODE (LunarxProviderModule, lunarx_provider_module, G_TYPE_TYPE_MODULE,
    G_IMPLEMENT_INTERFACE (LUNARX_TYPE_PROVIDER_PLUGIN, lunarx_provider_module_plugin_init))



static void
lunarx_provider_module_class_init (LunarxProviderModuleClass *klass)
{
  GTypeModuleClass *gtype_module_class;
  GObjectClass     *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = lunarx_provider_module_get_property;
  gobject_class->set_property = lunarx_provider_module_set_property;

  gtype_module_class = G_TYPE_MODULE_CLASS (klass);
  gtype_module_class->load = lunarx_provider_module_load;
  gtype_module_class->unload = lunarx_provider_module_unload;

  /* overload LunarxProviderPlugin's properties */
  g_object_class_override_property (gobject_class,
                                    PROP_RESIDENT,
                                    "resident");
}



static void
lunarx_provider_module_init (LunarxProviderModule *module)
{
}




static void
lunarx_provider_module_plugin_init (LunarxProviderPluginIface *iface)
{
  iface->get_resident = lunarx_provider_module_get_resident;
  iface->set_resident = lunarx_provider_module_set_resident;

  /* GTypeModule wrapper implementation */
  iface->register_type = (gpointer) g_type_module_register_type;
  iface->add_interface = (gpointer) g_type_module_add_interface;
  iface->register_enum = (gpointer) g_type_module_register_enum;
  iface->register_flags = (gpointer) g_type_module_register_flags;
}



static void
lunarx_provider_module_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  LunarxProviderPlugin *plugin = LUNARX_PROVIDER_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_RESIDENT:
      g_value_set_boolean (value, lunarx_provider_plugin_get_resident (plugin));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunarx_provider_module_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  LunarxProviderPlugin *plugin = LUNARX_PROVIDER_PLUGIN (object);

  switch (prop_id)
    {
    case PROP_RESIDENT:
      lunarx_provider_plugin_set_resident (plugin, g_value_get_boolean (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gboolean
lunarx_provider_module_load (GTypeModule *type_module)
{
  LunarxProviderModule *module = LUNARX_PROVIDER_MODULE (type_module);
  gchar                 *path;

  /* load the module using the runtime link editor */
  path = g_build_filename (LUNARX_DIRECTORY, type_module->name, NULL);
  module->library = g_module_open (path, G_MODULE_BIND_LAZY | G_MODULE_BIND_LOCAL);
  g_free (path);

  /* check if the load operation was successfull */
  if (G_UNLIKELY (module->library == NULL))
    {
      g_printerr ("Lunar :Failed to load plugin `%s': %s\n", type_module->name, g_module_error ());
      return FALSE;
    }

  /* verify that all required public symbols are present in the plugin's symbol table */
  if (!g_module_symbol (module->library, "lunar_extension_shutdown", (gpointer) &module->shutdown)
      || !g_module_symbol (module->library, "lunar_extension_initialize", (gpointer) &module->initialize)
      || !g_module_symbol (module->library, "lunar_extension_list_types", (gpointer) &module->list_types))
    {
      g_printerr ("Lunar :Plugin `%s' lacks required symbols.\n", type_module->name);
      g_module_close (module->library);
      return FALSE;
    }

  /* initialize the plugin */
  (*module->initialize) (module);

  /* ensure that the module will never be unloaded if it requests to be kept in memory */
  if (G_UNLIKELY (module->resident))
    g_module_make_resident (module->library);

  return TRUE;
}



static void
lunarx_provider_module_unload (GTypeModule *type_module)
{
  LunarxProviderModule *module = LUNARX_PROVIDER_MODULE (type_module);

  /* shutdown the plugin */
  (*module->shutdown) ();

  /* unload the plugin from memory */
  g_module_close (module->library);

  /* reset module state */
  module->library = NULL;
  module->shutdown = NULL;
  module->initialize = NULL;
  module->list_types = NULL;
}



static gboolean
lunarx_provider_module_get_resident (const LunarxProviderPlugin *plugin)
{
  return LUNARX_PROVIDER_MODULE (plugin)->resident;
}



static void
lunarx_provider_module_set_resident (LunarxProviderPlugin *plugin,
                                      gboolean               resident)
{
  LunarxProviderModule *module = LUNARX_PROVIDER_MODULE (plugin);

  if (G_LIKELY (module->resident != resident))
    {
      module->resident = resident;
      g_object_notify (G_OBJECT (module), "resident");
    }
}



/**
 * lunarx_provider_module_new:
 * @filename : the name of the library file.
 *
 * Allocates a new #LunarxProviderModule for @filename.
 *
 * Return value: the newly allocated #LunarxProviderModule.
 **/
LunarxProviderModule*
lunarx_provider_module_new (const gchar *filename)
{
  LunarxProviderModule *module;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (*filename != '\0', NULL);

  module = g_object_new (LUNARX_TYPE_PROVIDER_MODULE, NULL);
  g_type_module_set_name (G_TYPE_MODULE (module), filename);

  return module;
}



/**
 * lunarx_provider_module_list_types:
 * @module  : a #LunarxProviderModule.
 * @types   : return location for the #GType array pointer.
 * @n_types : return location for the number of types.
 *
 * Determines the #GType<!---->s provided by @module and returns
 * them in @types and @n_types.
 **/
void
lunarx_provider_module_list_types (const LunarxProviderModule *module,
                                    const GType                **types,
                                    gint                        *n_types)
{
  g_return_if_fail (LUNARX_IS_PROVIDER_MODULE (module));
  g_return_if_fail (module->list_types != NULL);
  g_return_if_fail (n_types != NULL);
  g_return_if_fail (types != NULL);

  (*module->list_types) (types, n_types);
}
