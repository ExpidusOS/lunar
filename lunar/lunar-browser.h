/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __LUNAR_BROWSER_H__
#define __LUNAR_BROWSER_H__

#include <lunar/lunar-file.h>
#include <lunar/lunar-device.h>

G_BEGIN_DECLS

#define LUNAR_TYPE_BROWSER           (lunar_browser_get_type ())
#define LUNAR_BROWSER(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_BROWSER, LunarBrowser))
#define LUNAR_IS_BROWSER(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_BROWSER))
#define LUNAR_BROWSER_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNAR_TYPE_BROWSER, LunarBrowserIface))

typedef struct _LunarBrowser      LunarBrowser;
typedef struct _LunarBrowserIface LunarBrowserIface;

typedef void (*LunarBrowserPokeFileFunc)     (LunarBrowser *browser,
                                               LunarFile    *file,
                                               LunarFile    *target_file,
                                               GError        *error,
                                               gpointer       user_data);

typedef void (*LunarBrowserPokeDeviceFunc)   (LunarBrowser *browser,
                                               LunarDevice  *volume,
                                               LunarFile    *mount_point,
                                               GError        *error,
                                               gpointer       user_data,
                                               gboolean       cancelled);

typedef void (*LunarBrowserPokeLocationFunc) (LunarBrowser *browser,
                                               GFile         *location,
                                               LunarFile    *file,
                                               LunarFile    *target_file,
                                               GError        *error,
                                               gpointer       user_data);

struct _LunarBrowserIface
{
  GTypeInterface __parent__;

  /* signals */

  /* virtual methods */
};

GType lunar_browser_get_type      (void) G_GNUC_CONST;

void  lunar_browser_poke_file     (LunarBrowser                 *browser,
                                    LunarFile                    *file,
                                    gpointer                       widget,
                                    LunarBrowserPokeFileFunc      func,
                                    gpointer                       user_data);
void  lunar_browser_poke_device   (LunarBrowser                 *browser,
                                    LunarDevice                  *device,
                                    gpointer                       widget,
                                    LunarBrowserPokeDeviceFunc    func,
                                    gpointer                       user_data);
void  lunar_browser_poke_location (LunarBrowser                 *browser,
                                    GFile                         *location,
                                    gpointer                       widget,
                                    LunarBrowserPokeLocationFunc  func,
                                    gpointer                       user_data);

G_END_DECLS

#endif /* !__LUNAR_BROWSER_H__ */
