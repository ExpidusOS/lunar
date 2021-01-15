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

#ifndef __LUNARX_H__
#define __LUNARX_H__

#define LUNARX_INSIDE_LUNARX_H

#include <lunarx/lunarx-config.h>
#include <lunarx/lunarx-file-info.h>
#include <lunarx/lunarx-menu.h>
#include <lunarx/lunarx-menu-provider.h>
#include <lunarx/lunarx-preferences-provider.h>
#include <lunarx/lunarx-property-page.h>
#include <lunarx/lunarx-property-page-provider.h>
#include <lunarx/lunarx-provider-factory.h>
#include <lunarx/lunarx-provider-plugin.h>
#include <lunarx/lunarx-renamer.h>
#include <lunarx/lunarx-renamer-provider.h>

#undef LUNARX_INSIDE_LUNARX_H

#define LUNARX_DEFINE_TYPE(TN, t_n, T_P)                         LUNARX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, 0, {})
#define LUNARX_DEFINE_TYPE_WITH_CODE(TN, t_n, T_P, _C_)          LUNARX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, 0, _C_)
#define LUNARX_DEFINE_ABSTRACT_TYPE(TN, t_n, T_P)                LUNARX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, G_TYPE_FLAG_ABSTRACT, {})
#define LUNARX_DEFINE_ABSTRACT_TYPE_WITH_CODE(TN, t_n, T_P, _C_) LUNARX_DEFINE_TYPE_EXTENDED (TN, t_n, T_P, G_TYPE_FLAG_ABSTRACT, _C_)

#define LUNARX_DEFINE_TYPE_EXTENDED(TypeName, type_name, TYPE_PARENT, flags, CODE) \
static gpointer type_name##_parent_class = NULL; \
static GType    type_name##_type = G_TYPE_INVALID; \
\
static void     type_name##_init              (TypeName        *self); \
static void     type_name##_class_init        (TypeName##Class *klass); \
static void     type_name##_class_intern_init (TypeName##Class *klass) \
{ \
  type_name##_parent_class = g_type_class_peek_parent (klass); \
  type_name##_class_init (klass); \
} \
\
GType \
type_name##_get_type (void) \
{ \
  return type_name##_type; \
} \
\
void \
type_name##_register_type (LunarxProviderPlugin *lunarx_define_type_plugin) \
{ \
  GType lunarx_define_type_id; \
  static const GTypeInfo lunarx_define_type_info = \
  { \
    sizeof (TypeName##Class), \
    NULL, \
    NULL, \
    (GClassInitFunc) (void (*)(void)) type_name##_class_intern_init, \
    NULL, \
    NULL, \
    sizeof (TypeName), \
    0, \
    (GInstanceInitFunc) (void (*)(void)) type_name##_init, \
    NULL, \
  }; \
  lunarx_define_type_id = lunarx_provider_plugin_register_type (lunarx_define_type_plugin, TYPE_PARENT, \
                                                                  #TypeName, &lunarx_define_type_info, flags); \
  { CODE ; } \
  type_name##_type = lunarx_define_type_id; \
}

#define LUNARX_IMPLEMENT_INTERFACE(TYPE_IFACE, iface_init) \
{ \
  static const GInterfaceInfo lunarx_implement_interface_info = \
  { \
    .interface_init = (GInterfaceInitFunc) (void (*)(void)) iface_init \
  }; \
  lunarx_provider_plugin_add_interface (lunarx_define_type_plugin, lunarx_define_type_id, TYPE_IFACE, &lunarx_implement_interface_info); \
}

#endif /* !__LUNARX_H__ */
