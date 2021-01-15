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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <lunar/lunar-file-monitor.h>
#include <lunar/lunar-private.h>



/* Signal identifiers */
enum
{
  FILE_CHANGED,
  FILE_DESTROYED,
  LAST_SIGNAL,
};



struct _LunarFileMonitorClass
{
  GObjectClass __parent__;
};

struct _LunarFileMonitor
{
  GObject __parent__;
};



static LunarFileMonitor *file_monitor_default;
static guint              file_monitor_signals[LAST_SIGNAL];



G_DEFINE_TYPE (LunarFileMonitor, lunar_file_monitor, G_TYPE_OBJECT)



static void
lunar_file_monitor_class_init (LunarFileMonitorClass *klass)
{
  /**
   * LunarFileMonitor::file-changed:
   * @file_monitor : the default #LunarFileMonitor.
   * @file         : the #LunarFile that changed.
   *
   * This signal is emitted on @file_monitor whenever any of the currently
   * existing #LunarFile instances changes. @file identifies the instance
   * that changed.
   **/
  file_monitor_signals[FILE_CHANGED] =
    g_signal_new (I_("file-changed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, LUNAR_TYPE_FILE);

  /**
   * LunarFileMonitor::file-destroyed:
   * @file_monitor : the default #LunarFileMonitor.
   * @file         : the #LunarFile that is about to be destroyed.
   *
   * This signal is emitted on @file_monitor whenever any of the currently
   * existing #LunarFile instances is about to be destroyed. @file identifies
   * the instance that is about to be destroyed.
   *
   * Note that this signal is only emitted if @file is explicitly destroyed,
   * i.e. because Lunar noticed that it was removed from disk, it is not
   * emitted when the last reference on @file is released.
   **/
  file_monitor_signals[FILE_DESTROYED] =
    g_signal_new (I_("file-destroyed"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1, LUNAR_TYPE_FILE);
}



static void
lunar_file_monitor_init (LunarFileMonitor *monitor)
{
}



/**
 * lunar_file_monitor_get_default:
 *
 * Returns a reference to the default #LunarFileMonitor
 * instance. The #LunarFileMonitor default instance can
 * be used to monitor the lifecycle of all currently existing
 * #LunarFile instances. The ::file-changed and ::file-destroyed
 * signals will be emitted whenever any of the currently
 * existing #LunarFile<!---->s is changed or destroyed.
 *
 * The caller is responsible to free the returned instance
 * using g_object_unref() when no longer needed.
 *
 * Return value: the default #LunarFileMonitor instance.
 **/
LunarFileMonitor*
lunar_file_monitor_get_default (void)
{
  if (G_UNLIKELY (file_monitor_default == NULL))
    {
      /* allocate the default monitor */
      file_monitor_default = g_object_new (LUNAR_TYPE_FILE_MONITOR, NULL);
      g_object_add_weak_pointer (G_OBJECT (file_monitor_default),
                                 (gpointer) &file_monitor_default);
    }
  else
    {
      /* take a reference for the caller */
      g_object_ref (G_OBJECT (file_monitor_default));
    }

  return file_monitor_default;
}



/**
 * lunar_file_monitor_file_changed:
 * @file : a #LunarFile.
 *
 * Emits the ::file-changed signal on the default
 * #LunarFileMonitor (if any). This method should
 * only be used by #LunarFile.
 **/
void
lunar_file_monitor_file_changed (LunarFile *file)
{
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  if (G_LIKELY (file_monitor_default != NULL))
    g_signal_emit (G_OBJECT (file_monitor_default), file_monitor_signals[FILE_CHANGED], 0, file);
}



/**
 * lunar_file_monitor_file_destroyed.
 * @file : a #LunarFile.
 *
 * Emits the ::file-destroyed signal on the default
 * #LunarFileMonitor (if any). This method should
 * only be used by #LunarFile.
 **/
void
lunar_file_monitor_file_destroyed (LunarFile *file)
{
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  if (G_LIKELY (file_monitor_default != NULL))
    g_signal_emit (G_OBJECT (file_monitor_default), file_monitor_signals[FILE_DESTROYED], 0, file);
}


