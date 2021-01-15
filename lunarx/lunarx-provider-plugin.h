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

#if !defined(LUNARX_INSIDE_LUNARX_H) && !defined(LUNARX_COMPILATION)
#error "Only <lunarx/lunarx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __LUNARX_PROVIDER_PLUGIN_H__
#define __LUNARX_PROVIDER_PLUGIN_H__

#include <glib-object.h>

typedef struct _LunarxProviderPluginIface LunarxProviderPluginIface;
typedef struct _LunarxProviderPlugin      LunarxProviderPlugin;

#define LUNARX_TYPE_PROVIDER_PLUGIN           (lunarx_provider_plugin_get_type ())
#define LUNARX_PROVIDER_PLUGIN(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_PROVIDER_PLUGIN, LunarxProviderPlugin))
#define LUNARX_IS_PROVIDER_PLUGIN(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_PROVIDER_PLUGIN))
#define LUNARX_PROVIDER_PLUGIN_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNARX_TYPE_PROVIDER_PLUGIN, LunarxProviderPluginIface))

/**
 * LunarxProviderPluginIface:
 * @get_resident: see lunarx_provider_plugin_get_resident().
 * @set_resident: see lunarx_provider_plugin_set_resident().
 * @register_type: see lunarx_provider_plugin_register_type().
 * @add_interface: see lunarx_provider_plugin_add_interface().
 * @register_enum: see lunarx_provider_plugin_register_enum().
 * @register_flags: see lunarx_provider_plugin_register_flags().
 *
 * Interface with virtual methods implemented by the file manager and
 * accessible from the extensions.
 */

struct _LunarxProviderPluginIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/
  gboolean (*get_resident)    (const LunarxProviderPlugin *plugin);
  void     (*set_resident)    (LunarxProviderPlugin       *plugin,
                               gboolean                     resident);

  GType    (*register_type)   (LunarxProviderPlugin       *plugin,
                               GType                        type_parent,
                               const gchar                 *type_name,
                               const GTypeInfo             *type_info,
                               GTypeFlags                   type_flags);
  void     (*add_interface)   (LunarxProviderPlugin       *plugin,
                               GType                        instance_type,
                               GType                        interface_type,
                               const GInterfaceInfo        *interface_info);
  GType    (*register_enum)   (LunarxProviderPlugin       *plugin,
                               const gchar                 *name,
                               const GEnumValue            *const_static_values);
  GType    (*register_flags)  (LunarxProviderPlugin       *plugin,
                               const gchar                 *name,
                               const GFlagsValue           *const_static_values);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
};

GType     lunarx_provider_plugin_get_type       (void) G_GNUC_CONST;

gboolean  lunarx_provider_plugin_get_resident   (const LunarxProviderPlugin *plugin);
void      lunarx_provider_plugin_set_resident   (LunarxProviderPlugin       *plugin,
                                                  gboolean                     resident);

GType     lunarx_provider_plugin_register_type  (LunarxProviderPlugin *plugin,
                                                  GType                  type_parent,
                                                  const gchar           *type_name,
                                                  const GTypeInfo       *type_info,
                                                  GTypeFlags             type_flags);
void      lunarx_provider_plugin_add_interface  (LunarxProviderPlugin *plugin,
                                                  GType                  instance_type,
                                                  GType                  interface_type,
                                                  const GInterfaceInfo  *interface_info);
GType     lunarx_provider_plugin_register_enum  (LunarxProviderPlugin *plugin,
                                                  const gchar           *name,
                                                  const GEnumValue      *const_static_values);
GType     lunarx_provider_plugin_register_flags (LunarxProviderPlugin *plugin,
                                                  const gchar           *name,
                                                  const GFlagsValue     *const_static_values);

#endif /* !__LUNARX_PROVIDER_PLUGIN_H__ */
