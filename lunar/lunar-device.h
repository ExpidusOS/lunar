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

#ifndef __LUNAR_DEVICE_H__
#define __LUNAR_DEVICE_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS

typedef struct _LunarDeviceClass LunarDeviceClass;
typedef struct _LunarDevice      LunarDevice;
typedef enum   _LunarDeviceKind  LunarDeviceKind;

typedef void   (*LunarDeviceCallback) (LunarDevice *device,
                                        const GError *error,
                                        gpointer      user_data);

enum _LunarDeviceKind
{
  LUNAR_DEVICE_KIND_VOLUME,
  LUNAR_DEVICE_KIND_MOUNT_LOCAL,
  LUNAR_DEVICE_KIND_MOUNT_REMOTE
};

#define LUNAR_TYPE_DEVICE             (lunar_device_get_type ())
#define LUNAR_DEVICE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_DEVICE, LunarDevice))
#define LUNAR_DEVICE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_DEVICE, LunarDeviceClass))
#define LUNAR_IS_DEVICE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_DEVICE))
#define LUNAR_IS_DEVICE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((obj), LUNAR_TYPE_DEVICE))
#define LUNAR_DEVICE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_DEVICE, LunarDeviceClass))

GType                lunar_device_get_type         (void) G_GNUC_CONST;

gchar               *lunar_device_get_name         (const LunarDevice   *device) G_GNUC_MALLOC;

GIcon               *lunar_device_get_icon         (const LunarDevice   *device);

LunarDeviceKind     lunar_device_get_kind         (const LunarDevice   *device) G_GNUC_PURE;

gchar               *lunar_device_get_identifier   (const LunarDevice   *device) G_GNUC_MALLOC;

gchar               *lunar_device_get_identifier_unix   (const LunarDevice   *device) G_GNUC_MALLOC;

gboolean             lunar_device_get_hidden       (const LunarDevice   *device);

gboolean             lunar_device_can_eject        (const LunarDevice   *device);

gboolean             lunar_device_can_mount        (const LunarDevice   *device);

gboolean             lunar_device_can_unmount      (const LunarDevice   *device);

gboolean             lunar_device_is_mounted       (const LunarDevice   *device);

GFile               *lunar_device_get_root         (const LunarDevice   *device);

gint                 lunar_device_sort             (const LunarDevice   *device1,
                                                     const LunarDevice   *device2);

void                 lunar_device_mount            (LunarDevice         *device,
                                                     GMountOperation      *mount_operation,
                                                     GCancellable         *cancellable,
                                                     LunarDeviceCallback  callback,
                                                     gpointer              user_data);

void                 lunar_device_unmount          (LunarDevice         *device,
                                                     GMountOperation      *mount_operation,
                                                     GCancellable         *cancellable,
                                                     LunarDeviceCallback  callback,
                                                     gpointer              user_data);

void                 lunar_device_eject            (LunarDevice         *device,
                                                     GMountOperation      *mount_operation,
                                                     GCancellable         *cancellable,
                                                     LunarDeviceCallback  callback,
                                                     gpointer              user_data);

void                 lunar_device_reload_file      (LunarDevice         *device);

G_END_DECLS

#endif /* !__LUNAR_DEVICE_H__ */
