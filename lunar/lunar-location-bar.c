/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2015 Jonas Kümmerlin <rgcjonas@gmail.com>
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

#include <lunar/lunar-location-bar.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-navigator.h>
#include <lunar/lunar-location-entry.h>
#include <lunar/lunar-location-buttons.h>
#include <lunar/lunar-preferences.h>


struct _LunarLocationBarClass
{
  GtkBinClass __parent__;

  /* signals */
  void (*reload_requested) (void);
  void (*entry_done) (void);
};

struct _LunarLocationBar
{
  GtkBin __parent__;

  LunarFile *current_directory;

  GtkWidget  *locationEntry;
  GtkWidget  *locationButtons;
};



enum
{
  PROP_0,
  PROP_CURRENT_DIRECTORY,
};


static void         lunar_location_bar_navigator_init             (LunarNavigatorIface *iface);
static void         lunar_location_bar_finalize                   (GObject              *object);
static void         lunar_location_bar_get_property               (GObject              *object,
                                                                    guint                 prop_id,
                                                                    GValue               *value,
                                                                    GParamSpec           *pspec);
static void         lunar_location_bar_set_property               (GObject              *object,
                                                                    guint                 prop_id,
                                                                    const GValue         *value,
                                                                    GParamSpec           *pspec);
static LunarFile  *lunar_location_bar_get_current_directory      (LunarNavigator      *navigator);
static void         lunar_location_bar_set_current_directory      (LunarNavigator      *navigator,
                                                                    LunarFile           *current_directory);
static GtkWidget   *lunar_location_bar_install_widget             (LunarLocationBar    *bar,
                                                                    GType                 type);
static void         lunar_location_bar_reload_requested           (LunarLocationBar    *bar);
static void         lunar_location_bar_settings_changed           (LunarLocationBar    *bar);
static void         lunar_location_bar_on_enry_edit_done          (LunarLocationEntry  *entry,
                                                                    LunarLocationBar    *bar);



G_DEFINE_TYPE_WITH_CODE (LunarLocationBar, lunar_location_bar, GTK_TYPE_BIN,
                         G_IMPLEMENT_INTERFACE (LUNAR_TYPE_NAVIGATOR, lunar_location_bar_navigator_init));



GtkWidget *
lunar_location_bar_new (void)
{
  return gtk_widget_new (LUNAR_TYPE_LOCATION_BAR, NULL);
}



static void
lunar_location_bar_class_init (LunarLocationBarClass *klass)
{
  GObjectClass *gobject_class = G_OBJECT_CLASS (klass);

  gobject_class->get_property = lunar_location_bar_get_property;
  gobject_class->set_property = lunar_location_bar_set_property;
  gobject_class->finalize = lunar_location_bar_finalize;

  klass->reload_requested = endo_noop;

  /* Override LunarNavigator's properties */
  g_object_class_override_property (gobject_class, PROP_CURRENT_DIRECTORY, "current-directory");

  /* install signals */

  /**
   * LunarLocationBar::reload-requested:
   * @location_bar : a #LunarLocationBar.
   *
   * Emitted by @location_bar whenever the user clicked a "reload" button
   **/
  g_signal_new ("reload-requested",
                G_TYPE_FROM_CLASS (klass),
                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                G_STRUCT_OFFSET (LunarLocationBarClass, reload_requested),
                NULL, NULL,
                NULL,
                G_TYPE_NONE, 0);

  /**
   * LunarLocationBar::entry-done:
   * @location_bar : a #LunarLocationBar.
   *
   * Emitted by @location_bar exactly once after an entry has been requested using
   * #lunar_location_bar_request_entry and the user has finished editing the entry.
   **/
  g_signal_new ("entry-done",
                G_TYPE_FROM_CLASS (klass),
                G_SIGNAL_RUN_LAST | G_SIGNAL_ACTION,
                G_STRUCT_OFFSET (LunarLocationBarClass, entry_done),
                NULL, NULL,
                NULL,
                G_TYPE_NONE, 0);
}



static void
lunar_location_bar_init (LunarLocationBar *bar)
{
  LunarPreferences *preferences = lunar_preferences_get ();

  bar->current_directory = NULL;
  bar->locationEntry = NULL;
  bar->locationButtons = NULL;

  lunar_location_bar_settings_changed (bar);

  g_signal_connect_object (preferences, "notify::last-location-bar", G_CALLBACK (lunar_location_bar_settings_changed), bar, G_CONNECT_SWAPPED);
}



static void
lunar_location_bar_finalize (GObject *object)
{
  LunarLocationBar *bar = LUNAR_LOCATION_BAR (object);

  _lunar_return_if_fail (LUNAR_IS_LOCATION_BAR (bar));

  if (bar->locationEntry)
    g_object_unref (bar->locationEntry);
  if (bar->locationButtons)
    g_object_unref (bar->locationButtons);

  /* release from the current_directory */
  lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (bar), NULL);

  (*G_OBJECT_CLASS (lunar_location_bar_parent_class)->finalize) (object);
}



static void
lunar_location_bar_navigator_init (LunarNavigatorIface *iface)
{
  iface->set_current_directory = lunar_location_bar_set_current_directory;
  iface->get_current_directory = lunar_location_bar_get_current_directory;
}



static void
lunar_location_bar_get_property (GObject              *object,
                                  guint                 prop_id,
                                  GValue               *value,
                                  GParamSpec           *pspec)
{
  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      g_value_set_object (value, lunar_navigator_get_current_directory (LUNAR_NAVIGATOR (object)));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void lunar_location_bar_set_property   (GObject              *object,
                                                guint                 prop_id,
                                                const GValue         *value,
                                                GParamSpec           *pspec)
{
  switch (prop_id)
    {
    case PROP_CURRENT_DIRECTORY:
      lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (object), g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static LunarFile *
lunar_location_bar_get_current_directory (LunarNavigator *navigator)
{
  return LUNAR_LOCATION_BAR (navigator)->current_directory;
}



static void
lunar_location_bar_set_current_directory (LunarNavigator      *navigator,
                                           LunarFile           *current_directory)
{
  LunarLocationBar *bar = LUNAR_LOCATION_BAR (navigator);
  GtkWidget         *child;

  if (bar->current_directory) g_object_unref (bar->current_directory);
  bar->current_directory = current_directory;

  if (current_directory) g_object_ref (current_directory);

  if ((child = gtk_bin_get_child (GTK_BIN (bar))) && LUNAR_IS_NAVIGATOR (child))
    lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (child), current_directory);

  g_object_notify (G_OBJECT (bar), "current-directory");
}



static void
lunar_location_bar_reload_requested (LunarLocationBar *bar)
{
  g_signal_emit_by_name (bar, "reload-requested");
}



static GtkWidget *
lunar_location_bar_install_widget (LunarLocationBar    *bar,
                                    GType                 type)
{
  GtkWidget *installedWidget, *child;

  /* check if the the right type is already installed */
  if ((child = gtk_bin_get_child (GTK_BIN (bar))) && G_TYPE_CHECK_INSTANCE_TYPE (child, type))
    return child;

  if (type == LUNAR_TYPE_LOCATION_ENTRY)
    {
      if (bar->locationEntry == NULL)
        {
          bar->locationEntry = gtk_widget_new (LUNAR_TYPE_LOCATION_ENTRY, "current-directory", NULL, NULL);
          g_object_ref (bar->locationEntry);
          g_signal_connect_swapped (bar->locationEntry, "reload-requested", G_CALLBACK (lunar_location_bar_reload_requested), bar);
          g_signal_connect_swapped (bar->locationEntry, "change-directory", G_CALLBACK (lunar_navigator_change_directory), LUNAR_NAVIGATOR (bar));
          g_signal_connect_swapped (bar->locationEntry, "open-new-tab", G_CALLBACK (lunar_navigator_open_new_tab), LUNAR_NAVIGATOR (bar));
        }
      installedWidget = bar->locationEntry;
    }
  else
    {
      if (bar->locationButtons == NULL)
        {
          bar->locationButtons = gtk_widget_new (LUNAR_TYPE_LOCATION_BUTTONS, "current-directory", NULL, NULL);
          g_object_ref (bar->locationButtons);
          g_signal_connect_swapped (bar->locationButtons, "entry-requested", G_CALLBACK (lunar_location_bar_request_entry), bar);
          g_signal_connect_swapped (bar->locationButtons, "change-directory", G_CALLBACK (lunar_navigator_change_directory), LUNAR_NAVIGATOR (bar));
          g_signal_connect_swapped (bar->locationButtons, "open-new-tab", G_CALLBACK (lunar_navigator_open_new_tab), LUNAR_NAVIGATOR (bar));
        }
      installedWidget = bar->locationButtons;
    }

  lunar_navigator_set_current_directory (LUNAR_NAVIGATOR (installedWidget), bar->current_directory);

  if ((child = gtk_bin_get_child (GTK_BIN (bar))))
    gtk_container_remove (GTK_CONTAINER (bar), child);

  gtk_container_add (GTK_CONTAINER (bar), installedWidget);
  gtk_widget_show (installedWidget);

  return installedWidget;
}


static gboolean
lunar_location_bar_settings_changed_cb (gpointer user_data)
{
  LunarLocationBar *bar = user_data;

  lunar_location_bar_settings_changed (bar);
  return FALSE;
}


static void
lunar_location_bar_on_enry_edit_done (LunarLocationEntry *entry,
                                       LunarLocationBar   *bar)
{
  g_signal_handlers_disconnect_by_func (entry, lunar_location_bar_on_enry_edit_done, bar);

  g_object_ref (bar);
  g_idle_add_full (G_PRIORITY_HIGH_IDLE, lunar_location_bar_settings_changed_cb, bar, g_object_unref);

  g_signal_emit_by_name (bar, "entry-done");
}



/**
 * lunar_location_bar_request_entry
 * @bar          : The #LunarLocationBar
 * @initial_text : The initial text to be placed inside the entry, or NULL to
 *                 use the path of the current directory.
 *
 * Makes the location bar display an entry with the given text and places the cursor
 * accordingly. If the currently displayed location widget is a path bar, it will be
 * temporarily swapped for an entry widget and swapped back once the user completed
 * (or aborted) the input.
 */
void
lunar_location_bar_request_entry (LunarLocationBar *bar,
                                   const gchar       *initial_text)
{
  GtkWidget *child;

  child = gtk_bin_get_child (GTK_BIN (bar));

  _lunar_return_if_fail (child != NULL && GTK_IS_WIDGET (child));

  if (LUNAR_IS_LOCATION_ENTRY (child))
    {
      /* already have an entry */
      lunar_location_entry_accept_focus (LUNAR_LOCATION_ENTRY (child), initial_text);
    }
  else
    {
      /* not an entry => temporarily replace it */
      child = lunar_location_bar_install_widget (bar, LUNAR_TYPE_LOCATION_ENTRY);
      lunar_location_entry_accept_focus (LUNAR_LOCATION_ENTRY (child), initial_text);
    }

  g_signal_connect (child, "edit-done", G_CALLBACK (lunar_location_bar_on_enry_edit_done), bar);
}



static void
lunar_location_bar_settings_changed (LunarLocationBar *bar)
{
  gchar *last_location_bar;
  GType  type;

  g_object_get (lunar_preferences_get(), "last-location-bar", &last_location_bar, NULL);

  /* validate it */
  if (!strcmp (last_location_bar, g_type_name (LUNAR_TYPE_LOCATION_BUTTONS)))
    type = LUNAR_TYPE_LOCATION_BUTTONS;
  else if (!strcmp (last_location_bar, g_type_name (LUNAR_TYPE_LOCATION_ENTRY)))
    type = LUNAR_TYPE_LOCATION_ENTRY;
  else
    type = LUNAR_TYPE_LOCATION_ENTRY; /* fallback */

  g_free (last_location_bar);

  lunar_location_bar_install_widget (bar, type);
}


