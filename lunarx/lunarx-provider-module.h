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

#ifndef __LUNARX_PROVIDER_MODULE_H__
#define __LUNARX_PROVIDER_MODULE_H__

#include <glib-object.h>

typedef struct _LunarxProviderModuleClass LunarxProviderModuleClass;
typedef struct _LunarxProviderModule      LunarxProviderModule;

#define LUNARX_TYPE_PROVIDER_MODULE            (lunarx_provider_module_get_type ())
#define LUNARX_PROVIDER_MODULE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_PROVIDER_MODULE, LunarxProviderModule))
#define LUNARX_PROVIDER_MODULE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNARX_TYPE_PROVIDER_MODULE, LunarxProviderModuleClass))
#define LUNARX_IS_PROVIDER_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_PROVIDER_MODULE))
#define LUNARX_IS_PROVIDER_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNARX_TYPE_PROVIDER_MODULE))
#define LUNARX_PROVIDER_MODULE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNARX_TYPE_PROVIDER_MODULE, LunarxProviderModuleClass))

GType                  lunarx_provider_module_get_type   (void) G_GNUC_CONST;

LunarxProviderModule *lunarx_provider_module_new        (const gchar                 *filename) G_GNUC_MALLOC;

void                   lunarx_provider_module_list_types (const LunarxProviderModule *module,
                                                           const GType                **types,
                                                           gint                        *n_types);

#endif /* !__LUNARX_PROVIDER_MODULE_H__ */
