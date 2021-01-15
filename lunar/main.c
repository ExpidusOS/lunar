/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2011 os-cillation e.K.
 *
 * Written by Benedikt Meurer <benny@expidus.org>.
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

#include <stdio.h>
#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib/gstdio.h>
#ifdef HAVE_GIO_UNIX
#include <gio/gdesktopappinfo.h>
#endif

#include <esconf/esconf.h>

#include <lunar/lunar-application.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-notify.h>
#include <lunar/lunar-session-client.h>
#include <lunar/lunar-preferences.h>



int
main (int argc, char **argv)
{
  LunarApplication   *application;
  GError              *error = NULL;

  /* setup translation domain */
  expidus_textdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR, "UTF-8");

  /* setup application name */
  g_set_application_name (_("Lunar"));

#ifdef G_ENABLE_DEBUG
  /* Do NOT remove this line for now, If something doesn't work,
   * fix your code instead!
   */
  g_log_set_always_fatal (G_LOG_LEVEL_CRITICAL | G_LOG_LEVEL_WARNING);
#endif

  /* initialize esconf */
  if (!esconf_init (&error))
    {
      g_printerr (PACKAGE_NAME ": Failed to initialize Esconf: %s\n\n", error->message);
      g_clear_error (&error);

      /* disable get/set properties */
      lunar_preferences_esconf_init_failed ();
    }

  /* register additional transformation functions */
  lunar_g_initialize_transformations ();

  /* acquire a reference on the global application */
  application = lunar_application_get ();

  /* use the Lunar icon as default for new windows */
  gtk_window_set_default_icon_name ("Lunar");

  /* do further processing inside gapplication */
  g_application_run (G_APPLICATION (application), argc, argv);

  /* release the application reference */
  g_object_unref (G_OBJECT (application));

#ifdef HAVE_LIBNOTIFY
  lunar_notify_uninit ();
#endif

  return EXIT_SUCCESS;
}
