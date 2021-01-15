/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2012 Nick Schermer <nick@expidus.org>
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

#ifndef __LUNAR_DEVICE_MONITOR_H__
#define __LUNAR_DEVICE_MONITOR_H__

#include <lunar/lunar-device.h>

G_BEGIN_DECLS

typedef struct _LunarDeviceMonitorClass LunarDeviceMonitorClass;
typedef struct _LunarDeviceMonitor      LunarDeviceMonitor;

#define LUNAR_TYPE_DEVICE_MONITOR             (lunar_device_monitor_get_type ())
#define LUNAR_DEVICE_MONITOR(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_DEVICE_MONITOR, LunarDeviceMonitor))
#define LUNAR_DEVICE_MONITOR_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_DEVICE_MONITOR, LunarDeviceMonitorClass))
#define LUNAR_IS_DEVICE_MONITOR(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_DEVICE_MONITOR))
#define LUNAR_IS_DEVICE_MONITOR_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), LUNAR_TYPE_DEVICE_MONITOR))
#define LUNAR_DEVICE_MONITOR_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_DEVICE_MONITOR, LunarDeviceMonitorClass))

GType                lunar_device_monitor_get_type    (void) G_GNUC_CONST;

LunarDeviceMonitor *lunar_device_monitor_get         (void);

GList               *lunar_device_monitor_get_devices (LunarDeviceMonitor *monitor);

void                 lunar_device_monitor_set_hidden  (LunarDeviceMonitor *monitor,
                                                        LunarDevice        *device,
                                                        gboolean             hidden);

G_END_DECLS

#endif /* !__LUNAR_DEVICE_MONITOR_H__ */
