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

#ifndef __LUNAR_DBUS_SERVICE_H__
#define __LUNAR_DBUS_SERVICE_H__

#include <endo/endo.h>

G_BEGIN_DECLS;

typedef struct _LunarDBusServiceClass LunarDBusServiceClass;
typedef struct _LunarDBusService      LunarDBusService;

#define LUNAR_TYPE_DBUS_SERVICE             (lunar_dbus_service_get_type ())
#define LUNAR_DBUS_SERVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_DBUS_SERVICE, LunarDBusService))
#define LUNAR_DBUS_SERVICE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_DBUS_SERVICE, LunarDBusServiceClass))
#define LUNAR_IS_DBUS_SERVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_DBUS_SERVICE))
#define LUNAR_IS_DBUS_SERVICE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_DBUS_BRIGDE))
#define LUNAR_DBUS_SERVICE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_DBUS_SERVICE, LunarDBusServicetClass))

GType    lunar_dbus_service_get_type       (void) G_GNUC_CONST;

gboolean lunar_dbus_service_export_on_connection (LunarDBusService *service,
                                                   GDBusConnection   *connection,
                                                   GError           **error);

G_END_DECLS;

#endif /* !__LUNAR_DBUS_SERVICE_H__ */
