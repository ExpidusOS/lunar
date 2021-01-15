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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <glib-object.h>

#include <lunar/lunar-browser.h>
#include <lunar/lunar-file.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-util.h>
#include <lunar/lunar-gtk-extensions.h>



typedef struct _PokeFileData   PokeFileData;
typedef struct _PokeDeviceData PokeDeviceData;



static void lunar_browser_poke_file_internal (LunarBrowser                *browser,
                                               GFile                        *location,
                                               LunarFile                   *source,
                                               LunarFile                   *file,
                                               gpointer                      widget,
                                               LunarBrowserPokeFileFunc     func,
                                               LunarBrowserPokeLocationFunc location_func,
                                               gpointer                      user_data);



struct _PokeFileData
{
  GFile                        *location;
  LunarBrowser                *browser;
  LunarFile                   *source;
  LunarFile                   *file;
  LunarBrowserPokeFileFunc     func;
  LunarBrowserPokeLocationFunc location_func;
  gpointer                      user_data;
};

struct _PokeDeviceData
{
  LunarBrowser              *browser;
  LunarDevice               *device;
  LunarFile                 *mount_point;
  LunarBrowserPokeDeviceFunc func;
  gpointer                    user_data;
};



GType
lunar_browser_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("LunarBrowser"),
                                            sizeof (LunarBrowserIface),
                                            NULL,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, G_TYPE_OBJECT);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static PokeFileData *
lunar_browser_poke_file_data_new (LunarBrowser                *browser,
                                   GFile                        *location,
                                   LunarFile                   *source,
                                   LunarFile                   *file,
                                   LunarBrowserPokeFileFunc     func,
                                   LunarBrowserPokeLocationFunc location_func,
                                   gpointer                      user_data)
{
  PokeFileData *poke_data;

  _lunar_return_val_if_fail (LUNAR_IS_BROWSER (browser), NULL);

  poke_data = g_slice_new0 (PokeFileData);
  poke_data->browser = g_object_ref (browser);

  if (location != NULL)
    poke_data->location = g_object_ref (location);

  if (source != NULL)
    poke_data->source = g_object_ref (source);

  if (file != NULL)
    poke_data->file = g_object_ref (file);

  poke_data->func = func;
  poke_data->location_func = location_func;

  poke_data->user_data = user_data;

  return poke_data;
}



static void
lunar_browser_poke_file_data_free (PokeFileData *poke_data)
{
  _lunar_return_if_fail (poke_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));

  g_object_unref (poke_data->browser);

  if (poke_data->location != NULL)
    g_object_unref (poke_data->location);
  if (poke_data->source != NULL)
    g_object_unref (poke_data->source);
  if (poke_data->file != NULL)
    g_object_unref (poke_data->file);

  g_slice_free (PokeFileData, poke_data);
}



static PokeDeviceData *
lunar_browser_poke_device_data_new (LunarBrowser              *browser,
                                     LunarDevice               *device,
                                     LunarBrowserPokeDeviceFunc func,
                                     gpointer                    user_data)
{
  PokeDeviceData *poke_data;

  _lunar_return_val_if_fail (LUNAR_IS_BROWSER (browser), NULL);
  _lunar_return_val_if_fail (LUNAR_IS_DEVICE (device), NULL);

  poke_data = g_slice_new0 (PokeDeviceData);
  poke_data->browser = g_object_ref (browser);
  poke_data->device = g_object_ref (device);
  poke_data->func = func;
  poke_data->user_data = user_data;

  return poke_data;
}



static void
lunar_browser_poke_device_data_free (PokeDeviceData *poke_data)
{
  _lunar_return_if_fail (poke_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_DEVICE (poke_data->device));

  g_object_unref (poke_data->browser);
  g_object_unref (poke_data->device);

  g_slice_free (PokeDeviceData, poke_data);
}



static void
lunar_browser_poke_mountable_file_finish (GFile      *location,
                                           LunarFile *file,
                                           GError     *error,
                                           gpointer    user_data)
{
  PokeFileData *poke_data = user_data;

  _lunar_return_if_fail (G_IS_FILE (location));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (poke_data->file));
  _lunar_return_if_fail (LUNAR_IS_FILE (poke_data->source));

  if (poke_data->location_func != NULL)
    {
      (poke_data->location_func) (poke_data->browser,
                                  poke_data->location,
                                  poke_data->source,
                                  file,
                                  error,
                                  poke_data->user_data);
    }

  if (poke_data->func != NULL)
    {
      (poke_data->func) (poke_data->browser, poke_data->source, file, error,
                         poke_data->user_data);
    }

  lunar_browser_poke_file_data_free (poke_data);
}



static void
lunar_browser_poke_mountable_finish (GObject      *object,
                                      GAsyncResult *result,
                                      gpointer      user_data)
{
  PokeFileData *poke_data = user_data;
  GError       *error = NULL;
  GFile        *location;

  _lunar_return_if_fail (G_IS_FILE (object));
  _lunar_return_if_fail (G_IS_ASYNC_RESULT (result));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (poke_data->file));

  if (!g_file_mount_mountable_finish (G_FILE (object), result, &error))
    {
      if (error->domain == G_IO_ERROR)
        {
          if (error->code == G_IO_ERROR_ALREADY_MOUNTED)
            g_clear_error (&error);
        }
    }

  if (error == NULL)
    {
      if (lunar_file_reload (poke_data->file))
        {
          location = lunar_file_get_target_location (poke_data->file);

          /* resolve the LunarFile for the target location asynchronously
           * and defer cleaning up the poke data until that has finished */
          lunar_file_get_async (location, NULL,
                                 lunar_browser_poke_mountable_file_finish,
                                 poke_data);

          g_object_unref (location);
        }
    }
  else
    {
      if (poke_data->location_func != NULL)
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      NULL,
                                      error,
                                      poke_data->user_data);
        }

      if (poke_data->func != NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->source, NULL, error,
                             poke_data->user_data);
        }

      lunar_browser_poke_file_data_free (poke_data);
    }

  g_clear_error (&error);
}



static void
lunar_browser_poke_file_finish (GObject      *object,
                                 GAsyncResult *result,
                                 gpointer      user_data)
{
  PokeFileData *poke_data = user_data;
  GError       *error = NULL;

  _lunar_return_if_fail (G_IS_FILE (object));
  _lunar_return_if_fail (G_IS_ASYNC_RESULT (result));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (poke_data->file));

  if (!g_file_mount_enclosing_volume_finish (G_FILE (object), result, &error))
    {
      if (error->domain == G_IO_ERROR)
        {
          if (error->code == G_IO_ERROR_ALREADY_MOUNTED)
            g_clear_error (&error);
        }
    }

  if (error == NULL)
    {
      if (lunar_file_reload (poke_data->file) == FALSE)
        {
          lunar_browser_poke_file_data_free (poke_data);
          return;
        }
    }

  if (poke_data->location_func != NULL)
    {
      if (error == NULL)
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      poke_data->file,
                                      NULL,
                                      poke_data->user_data);
        }
      else
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      NULL,
                                      error,
                                      poke_data->user_data);
        }
    }

  if (poke_data->func != NULL)
    {
      if (error == NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->source, poke_data->file,
                             NULL, poke_data->user_data);
        }
      else
        {
          (poke_data->func) (poke_data->browser, poke_data->source, NULL, error,
                             poke_data->user_data);
        }
    }

  g_clear_error (&error);

  lunar_browser_poke_file_data_free (poke_data);
}



static void
lunar_browser_poke_shortcut_file_finish (GFile *location,
                                          LunarFile *file,
                                          GError *error,
                                          gpointer user_data)
{
  PokeFileData *poke_data = user_data;

  _lunar_return_if_fail (G_IS_FILE (location));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (poke_data->file));

  if (error == NULL)
    {
      lunar_browser_poke_file_internal (poke_data->browser,
                                         poke_data->location,
                                         poke_data->source,
                                         file,
                                         NULL,
                                         poke_data->func,
                                         poke_data->location_func,
                                         poke_data->user_data);
    }
  else
    {
      if (poke_data->location_func != NULL)
        {
          (poke_data->location_func) (poke_data->browser,
                                      poke_data->location,
                                      poke_data->source,
                                      NULL,
                                      error,
                                      poke_data->user_data);
        }

      if (poke_data->func != NULL)
        {
          (poke_data->func) (poke_data->browser, poke_data->source, NULL, error,
                             poke_data->user_data);
        }
    }

  lunar_browser_poke_file_data_free (poke_data);
}



static void
lunar_browser_poke_file_internal (LunarBrowser                *browser,
                                   GFile                        *location,
                                   LunarFile                   *source,
                                   LunarFile                   *file,
                                   gpointer                      widget,
                                   LunarBrowserPokeFileFunc     func,
                                   LunarBrowserPokeLocationFunc location_func,
                                   gpointer                      user_data)
{
  GMountOperation *mount_operation;
  PokeFileData    *poke_data;
  GFile           *target;

  _lunar_return_if_fail (LUNAR_IS_BROWSER (browser));
  _lunar_return_if_fail (G_IS_FILE (location));
  _lunar_return_if_fail (LUNAR_IS_FILE (source));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  if (lunar_file_get_kind (file) == G_FILE_TYPE_SHORTCUT)
    {
      target = lunar_file_get_target_location (file);

      poke_data = lunar_browser_poke_file_data_new (browser, location, source,
                                                     file, func, location_func, user_data);

      lunar_file_get_async (target, NULL,
                             lunar_browser_poke_shortcut_file_finish,
                             poke_data);

      g_object_unref (target);
    }
  else if (lunar_file_get_kind (file) == G_FILE_TYPE_MOUNTABLE)
    {
      if (lunar_file_is_mounted (file))
        {
          target = lunar_file_get_target_location (file);

          poke_data = lunar_browser_poke_file_data_new (browser, location, source,
                                                         file, func, location_func, user_data);

          lunar_file_get_async (target, NULL,
                                 lunar_browser_poke_mountable_file_finish,
                                 poke_data);

          g_object_unref (target);
        }
      else
        {
          poke_data = lunar_browser_poke_file_data_new (browser, location, source,
                                                         file, func, location_func, user_data);

          mount_operation = lunar_gtk_mount_operation_new (widget);

          g_file_mount_mountable (lunar_file_get_file (file),
                                  G_MOUNT_MOUNT_NONE, mount_operation, NULL,
                                  lunar_browser_poke_mountable_finish,
                                  poke_data);

          g_object_unref (mount_operation);
        }
    }
  else if (!lunar_file_is_mounted (file))
    {
      poke_data = lunar_browser_poke_file_data_new (browser, location, source,
                                                     file, func, location_func, user_data);

      mount_operation = lunar_gtk_mount_operation_new (widget);

      g_file_mount_enclosing_volume (lunar_file_get_file (file),
                                     G_MOUNT_MOUNT_NONE, mount_operation, NULL,
                                     lunar_browser_poke_file_finish,
                                     poke_data);

      g_object_unref (mount_operation);
    }
  else
    {
      if (location_func != NULL)
        location_func (browser, location, source, file, NULL, user_data);

      if (func != NULL)
        func (browser, source, file, NULL, user_data);
    }
}



/**
 * lunar_browser_poke_file:
 * @browser : a #LunarBrowser.
 * @file      : a #LunarFile.
 * @widget    : a #GtkWidget, a #GdkScreen or %NULL.
 * @func      : a #LunarBrowserPokeFileFunc callback or %NULL.
 * @user_data : pointer to arbitrary user data or %NULL.
 *
 * Pokes a #LunarFile to see what's behind it.
 *
 * If @file has the type %G_FILE_TYPE_SHORTCUT, it tries to load and mount
 * the file that is referred to by the %G_FILE_ATTRIBUTE_STANDARD_TARGET_URI
 * of the @file.
 *
 * If @file has the type %G_FILE_TYPE_MOUNTABLE, it tries to mount the @file.
 *
 * In the other cases, if the enclosing volume of the @file is not mounted
 * yet, it tries to mount it.
 *
 * When finished or if an error occured, it calls @func with the provided
 * @user data. The #GError parameter to @func is set if, and only if there
 * was an error.
 **/
void
lunar_browser_poke_file (LunarBrowser            *browser,
                          LunarFile               *file,
                          gpointer                  widget,
                          LunarBrowserPokeFileFunc func,
                          gpointer                  user_data)
{
  _lunar_return_if_fail (LUNAR_IS_BROWSER (browser));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));

  lunar_browser_poke_file_internal (browser, lunar_file_get_file (file), file, file, widget,
                                     func, NULL, user_data);
}



static void
lunar_browser_poke_device_file_finish (GFile      *location,
                                        LunarFile *file,
                                        GError     *error,
                                        gpointer    user_data)
{
  PokeDeviceData *poke_data = user_data;

  _lunar_return_if_fail (G_IS_FILE (location));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_DEVICE (poke_data->device));

  if (poke_data->func != NULL)
    {
      (poke_data->func) (poke_data->browser, poke_data->device, file, error,
                         poke_data->user_data, FALSE);
    }

  lunar_browser_poke_device_data_free (poke_data);
}



static void
lunar_browser_poke_device_finish (LunarDevice *device,
                                   const GError *error,
                                   gpointer      user_data)
{
  PokeDeviceData *poke_data = user_data;
  GFile          *mount_point = NULL;
  gboolean        cancelled = FALSE;

  _lunar_return_if_fail (LUNAR_IS_DEVICE (device));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (LUNAR_IS_DEVICE (poke_data->device));
  _lunar_return_if_fail (device == poke_data->device);

  if (error == NULL)
    mount_point = lunar_device_get_root (device);

  if (mount_point == NULL)
    {
      /* mount_point is NULL when user dismisses the authentication dialog */
      cancelled = TRUE;
    }
  else
    {
      /* resolve the LunarFile for the mount point asynchronously
       * and defer cleaning up the poke data until that has finished */
      lunar_file_get_async (mount_point, NULL,
                             lunar_browser_poke_device_file_finish,
                             poke_data);

      g_object_unref (mount_point);

      return;
    }

    if (poke_data->func != NULL)
      {
        (poke_data->func) (poke_data->browser, poke_data->device, NULL,
                           (GError *) error, poke_data->user_data, cancelled);
      }

    lunar_browser_poke_device_data_free (poke_data);
}



/**
 * lunar_browser_poke_device:
 * @browser   : a #LunarBrowser.
 * @device    : a #LunarDevice.
 * @widget    : a #GtkWidget, a #GdkScreen or %NULL.
 * @func      : a #LunarBrowserPokeDeviceFunc callback or %NULL.
 * @user_data : pointer to arbitrary user data or %NULL.
 *
 * This function checks if @device is mounted or not. If it is, it loads
 * a #LunarFile for the mount root and calls @func. If it is not mounted,
 * it first mounts the device asynchronously and calls @func with the
 * #LunarFile corresponding to the mount root when the mounting is finished.
 *
 * The #LunarFile passed to @func will be %NULL if, and only if mounting
 * the @device failed. The #GError passed to @func will be set if, and only if
 * mounting failed.
 **/
void
lunar_browser_poke_device (LunarBrowser              *browser,
                            LunarDevice               *device,
                            gpointer                    widget,
                            LunarBrowserPokeDeviceFunc func,
                            gpointer                    user_data)
{
  GMountOperation *mount_operation;
  PokeDeviceData  *poke_data;
  GFile           *mount_point;

  _lunar_return_if_fail (LUNAR_IS_BROWSER (browser));
  _lunar_return_if_fail (LUNAR_DEVICE (device));

  if (lunar_device_is_mounted (device))
    {
      mount_point = lunar_device_get_root (device);

      poke_data = lunar_browser_poke_device_data_new (browser, device, func, user_data);

      lunar_file_get_async (mount_point, NULL,
                             lunar_browser_poke_device_file_finish,
                             poke_data);

      g_object_unref (mount_point);
    }
  else
    {
      poke_data = lunar_browser_poke_device_data_new (browser, device, func, user_data);

      mount_operation = lunar_gtk_mount_operation_new (widget);

      lunar_device_mount (device,
                           mount_operation,
                           NULL,
                           lunar_browser_poke_device_finish,
                           poke_data);

      g_object_unref (mount_operation);
    }
}



static void
lunar_browser_poke_location_file_finish (GFile      *location,
                                          LunarFile *file,
                                          GError     *error,
                                          gpointer    user_data)
{
  PokeFileData *poke_data = user_data;

  _lunar_return_if_fail (G_IS_FILE (location));
  _lunar_return_if_fail (user_data != NULL);
  _lunar_return_if_fail (LUNAR_IS_BROWSER (poke_data->browser));
  _lunar_return_if_fail (G_IS_FILE (poke_data->location));

  if (error == NULL)
    {
      lunar_browser_poke_file_internal (poke_data->browser,
                                         location,
                                         file,
                                         file,
                                         NULL,
                                         poke_data->func,
                                         poke_data->location_func,
                                         poke_data->user_data);
    }
  else
    {
      if (poke_data->location_func != NULL)
        {
          (poke_data->location_func) (poke_data->browser, location, NULL, NULL, error,
                                      poke_data->user_data);
        }
    }

  lunar_browser_poke_file_data_free (poke_data);
}



/**
 * lunar_browser_poke_location:
 * @browser   : a #LunarBrowser.
 * @location  : a #GFile.
 * @widget    : a #GtkWidget, a #GdkScreen or %NULL.
 * @func      : a #LunarBrowserPokeDeviceFunc callback or %NULL.
 * @user_data : pointer to arbitrary user data or %NULL.
 *
 * Pokes a #GFile to see what's behind it.
 *
 * It first resolves the #GFile into a #LunarFile asynchronously using
 * lunar_file_get_async(). It then performs the same steps as
 * lunar_browser_poke_file().
 **/
void
lunar_browser_poke_location (LunarBrowser                *browser,
                              GFile                        *location,
                              gpointer                      widget,
                              LunarBrowserPokeLocationFunc func,
                              gpointer                      user_data)
{
  PokeFileData *poke_data;

  _lunar_return_if_fail (LUNAR_IS_BROWSER (browser));
  _lunar_return_if_fail (G_IS_FILE (location));

  poke_data = lunar_browser_poke_file_data_new (browser, location, NULL, NULL,
                                                 NULL, func, user_data);

  lunar_file_get_async (location, NULL,
                         lunar_browser_poke_location_file_finish,
                         poke_data);
}
