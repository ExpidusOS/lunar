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

#ifndef __LUNAR_FILE_MONITOR_H__
#define __LUNAR_FILE_MONITOR_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarFileMonitorClass LunarFileMonitorClass;
typedef struct _LunarFileMonitor      LunarFileMonitor;

#define LUNAR_TYPE_FILE_MONITOR            (lunar_file_monitor_get_type ())
#define LUNAR_FILE_MONITOR(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_FILE_MONITOR, LunarFileMonitor))
#define LUNAR_FILE_MONITOR_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_FILE_MONITOR, LunarFileMonitorClass))
#define LUNAR_IS_FILE_MONITOR(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_FILE_MONITOR))
#define LUNAR_IS_FILE_MONITOR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_FILE_MONITOR))
#define LUNAR_FILE_MONITOR_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_FILE_MONITOR, LunarFileMonitorClass))

GType              lunar_file_monitor_get_type       (void) G_GNUC_CONST;

LunarFileMonitor *lunar_file_monitor_get_default    (void);

void               lunar_file_monitor_file_changed   (LunarFile *file);
void               lunar_file_monitor_file_destroyed (LunarFile *file);

G_END_DECLS;

#endif /* !__LUNAR_FILE_MONITOR_H__ */
