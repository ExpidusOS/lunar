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

#ifndef __LUNARX_RENAMER_PROVIDER_H__
#define __LUNARX_RENAMER_PROVIDER_H__

#include <lunarx/lunarx-renamer.h>

G_BEGIN_DECLS

typedef struct _LunarxRenamerProviderIface LunarxRenamerProviderIface;
typedef struct _LunarxRenamerProvider      LunarxRenamerProvider;

#define LUNARX_TYPE_RENAMER_PROVIDER           (lunarx_renamer_provider_get_type ())
#define LUNARX_RENAMER_PROVIDER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_RENAMER_PROVIDER, LunarxRenamerProvider))
#define LUNARX_IS_RENAMER_PROVIDER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_RENAMER_PROVIDER))
#define LUNARX_RENAMER_PROVIDER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNARX_TYPE_RENAMER_PROVIDER, LunarxRenamerProviderIface))

/**
 * LunarxRenamerProviderIface:
 * @get_renamers: see lunarx_renamer_provider_get_renamers().
 *
 * Interface with virtual methods implemented by extensions that provide
 * additional bulk renamers for the integrated bulk renamer in Lunar.
 */

struct _LunarxRenamerProviderIface
{
  /*< private >*/
  GTypeInterface __parent__;

  /*< public >*/
  GList *(*get_renamers) (LunarxRenamerProvider *provider);

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
  void (*reserved5) (void);
};

GType  lunarx_renamer_provider_get_type     (void) G_GNUC_CONST;

GList *lunarx_renamer_provider_get_renamers (LunarxRenamerProvider *provider) G_GNUC_MALLOC;

G_END_DECLS

#endif /* !__LUNARX_RENAMER_PROVIDER_H__ */
