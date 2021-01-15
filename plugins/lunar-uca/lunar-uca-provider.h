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

#ifndef __LUNAR_UCA_PROVIDER_H__
#define __LUNAR_UCA_PROVIDER_H__

#include <lunarx/lunarx.h>

G_BEGIN_DECLS;

typedef struct _LunarUcaProviderClass LunarUcaProviderClass;
typedef struct _LunarUcaProvider      LunarUcaProvider;

#define LUNAR_UCA_TYPE_PROVIDER            (lunar_uca_provider_get_type ())
#define LUNAR_UCA_PROVIDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_UCA_TYPE_PROVIDER, LunarUcaProvider))
#define LUNAR_UCA_PROVIDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_UCA_TYPE_PROVIDER, LunarUcaProviderClass))
#define LUNAR_UCA_IS_PROVIDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_UCA_TYPE_PROVIDER))
#define LUNAR_UCA_IS_PROVIDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_UCA_TYPE_PROVIDER))
#define LUNAR_UCA_PROVIDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_UCA_TYPE_PROVIDER, LunarUcaProviderClass))

GType lunar_uca_provider_get_type      (void) G_GNUC_CONST;
void  lunar_uca_provider_register_type (LunarxProviderPlugin *plugin);

G_END_DECLS;

#endif /* !__LUNAR_UCA_PROVIDER_H__ */
