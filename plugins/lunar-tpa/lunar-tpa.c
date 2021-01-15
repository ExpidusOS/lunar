/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2010 Nick Schermer <nick@expidus.org>
 * Copyright (c) 2010 Jannis Pohlmann <jannis@expidus.org>
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

#include <glib.h>

#include <gtk/gtk.h>

#include <libexpidus1util/libexpidus1util.h>

#include <libexpidus1ui/libexpidus1ui.h>

#include <libexpidus1panel/libexpidus1panel.h>
#include <libexpidus1panel/expidus-panel-macros.h>

#include <lunar-tpa/lunar-tpa-bindings.h>

typedef struct _LunarTpaClass LunarTpaClass;
typedef struct _LunarTpa      LunarTpa;



#define LUNAR_TYPE_TPA            (lunar_tpa_get_type ())
#define LUNAR_TPA(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_TPA, LunarTpa))
#define LUNAR_TPA_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_TPA, LunarTpaClass))
#define LUNAR_IS_TPA(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_TPA))
#define LUNAR_IS_TPA_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_TPA))
#define LUNAR_TPA_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_TPA, LunarTpaClass))



GType           lunar_tpa_get_type            (void);
void            lunar_tpa_register_type       (ExpidusPanelTypeModule *type_module);
static void     lunar_tpa_finalize            (GObject             *object);
static void     lunar_tpa_construct           (ExpidusPanelPlugin     *panel_plugin);
static gboolean lunar_tpa_size_changed        (ExpidusPanelPlugin     *panel_plugin,
                                                gint                 size);
static void     lunar_tpa_error               (LunarTpa           *plugin,
                                                GError              *error);
static void     lunar_tpa_state               (LunarTpa           *plugin,
                                                gboolean             full);
static void     lunar_tpa_display_trash_reply (GObject             *source_object,
                                                GAsyncResult        *result,
                                                gpointer             user_data);
static void     lunar_tpa_empty_trash_reply   (GObject             *source_object,
                                                GAsyncResult        *result,
                                                gpointer             user_data);
static void     lunar_tpa_move_to_trash_reply (GObject             *source_object,
                                                GAsyncResult        *result,
                                                gpointer             user_data);
static void     lunar_tpa_query_trash_reply   (GObject             *source_object,
                                                GAsyncResult        *result,
                                                gpointer             user_data);
static void     lunar_tpa_drag_data_received  (GtkWidget           *button,
                                                GdkDragContext      *context,
                                                gint                 x,
                                                gint                 y,
                                                GtkSelectionData    *selection_data,
                                                guint                info,
                                                guint                time,
                                                LunarTpa           *plugin);
static gboolean lunar_tpa_enter_notify_event  (GtkWidget           *button,
                                                GdkEventCrossing    *event,
                                                LunarTpa           *plugin);
static gboolean lunar_tpa_leave_notify_event  (GtkWidget           *button,
                                                GdkEventCrossing    *event,
                                                LunarTpa           *plugin);
static void     lunar_tpa_on_trash_changed    (lunarTPATrash      *proxy,
                                                gpointer             user_data);
static void     lunar_tpa_display_trash       (LunarTpa           *plugin);
static void     lunar_tpa_empty_trash         (LunarTpa           *plugin);
static gboolean lunar_tpa_move_to_trash       (LunarTpa           *plugin,
                                                const gchar        **uri_list);
static void     lunar_tpa_query_trash         (LunarTpa           *plugin);



struct _LunarTpaClass
{
  ExpidusPanelPluginClass __parent__;
};

struct _LunarTpa
{
  ExpidusPanelPlugin __parent__;

  /* widgets */
  GtkWidget      *button;
  GtkWidget      *image;
  GtkWidget      *mi;

  lunarTPATrash *proxy;
  GCancellable   *cancellable_display_trash;
  GCancellable   *cancellable_empty_trash;
  GCancellable   *cancellable_move_to_trash;
  GCancellable   *cancellable_query_trash;
};

/* Target types for dropping to the trash can */
enum
{
  TARGET_TEXT_URI_LIST,
};

static const GtkTargetEntry drop_targets[] =
{
  { "text/uri-list", 0, TARGET_TEXT_URI_LIST, },
};



/* define the plugin */
EXPIDUS_PANEL_DEFINE_PLUGIN (LunarTpa, lunar_tpa)

/* Hint: For debugging the plugin run the panel with "PANEL_DEBUG=1 expidus1-panel" */

static void
lunar_tpa_class_init (LunarTpaClass *klass)
{
  ExpidusPanelPluginClass *plugin_class;
  GObjectClass         *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_tpa_finalize;

  plugin_class = EXPIDUS_PANEL_PLUGIN_CLASS (klass);
  plugin_class->construct = lunar_tpa_construct;
  plugin_class->size_changed = lunar_tpa_size_changed;
}



static void
lunar_tpa_init (LunarTpa *plugin)
{
  GError *error = NULL;

  /* setup the button for the trash plugin */
  plugin->button = expidus_panel_create_button ();
  expidus_panel_plugin_add_action_widget (EXPIDUS_PANEL_PLUGIN (plugin), plugin->button);
  gtk_drag_dest_set (plugin->button, GTK_DEST_DEFAULT_ALL, drop_targets, G_N_ELEMENTS (drop_targets), GDK_ACTION_MOVE);
  g_signal_connect_swapped (G_OBJECT (plugin->button), "clicked", G_CALLBACK (lunar_tpa_display_trash), plugin);
  g_signal_connect (G_OBJECT (plugin->button), "drag-data-received", G_CALLBACK (lunar_tpa_drag_data_received), plugin);
  g_signal_connect (G_OBJECT (plugin->button), "enter-notify-event", G_CALLBACK (lunar_tpa_enter_notify_event), plugin);
  g_signal_connect (G_OBJECT (plugin->button), "leave-notify-event", G_CALLBACK (lunar_tpa_leave_notify_event), plugin);
  gtk_container_add (GTK_CONTAINER (plugin), plugin->button);
  gtk_widget_show (plugin->button);

  /* setup the image for the trash plugin */
  plugin->image = gtk_image_new_from_icon_name ("user-trash", GTK_ICON_SIZE_BUTTON);
  gtk_container_add (GTK_CONTAINER (plugin->button), plugin->image);
  gtk_widget_show (plugin->image);

  /* prepare the menu item */
  plugin->mi = gtk_menu_item_new_with_mnemonic (_("_Empty Trash"));
  g_signal_connect_swapped (G_OBJECT (plugin->mi), "activate", G_CALLBACK (lunar_tpa_empty_trash), plugin);
  gtk_widget_show (plugin->mi);

  plugin->cancellable_display_trash = g_cancellable_new ();
  plugin->cancellable_empty_trash   = g_cancellable_new ();
  plugin->cancellable_move_to_trash = g_cancellable_new ();
  plugin->cancellable_query_trash   = g_cancellable_new ();

  plugin->proxy = lunar_tpa_trash_proxy_new_for_bus_sync (G_BUS_TYPE_SESSION, G_DBUS_PROXY_FLAGS_NONE, "com.expidus.FileManager", "/com/expidus/FileManager", NULL, &error);

  if(error != NULL)
    lunar_tpa_error (plugin, error);

  g_signal_connect (plugin->proxy, "trash_changed", G_CALLBACK (lunar_tpa_on_trash_changed), plugin);
}

static void
lunar_tpa_finalize (GObject *object)
{
  LunarTpa *plugin = LUNAR_TPA (object);

  /* cancel any pending calls */
  if (G_LIKELY (plugin->cancellable_display_trash != NULL))
    g_cancellable_cancel (plugin->cancellable_display_trash);
  if (G_LIKELY (plugin->cancellable_empty_trash != NULL))
    g_cancellable_cancel (plugin->cancellable_empty_trash);
  if (G_LIKELY (plugin->cancellable_move_to_trash != NULL))
    g_cancellable_cancel (plugin->cancellable_move_to_trash);
  if (G_LIKELY (plugin->cancellable_query_trash != NULL))
    g_cancellable_cancel (plugin->cancellable_query_trash);

  /* release the proxy object */
  if (G_LIKELY (plugin->proxy != NULL))
      g_object_unref (G_OBJECT (plugin->proxy));

  (*G_OBJECT_CLASS (lunar_tpa_parent_class)->finalize) (object);
}



static void
lunar_tpa_construct (ExpidusPanelPlugin *panel_plugin)
{
  LunarTpa *plugin = LUNAR_TPA (panel_plugin);

  /* make the plugin fit a single row */
  expidus_panel_plugin_set_small (panel_plugin, TRUE);

  /* add the "Empty Trash" menu item */
  expidus_panel_plugin_menu_insert_item (panel_plugin, GTK_MENU_ITEM (plugin->mi));

  /* update the state of the trash plugin */
  lunar_tpa_query_trash (plugin);
}



static gboolean
lunar_tpa_size_changed (ExpidusPanelPlugin *panel_plugin,
                         gint             size)
{
  LunarTpa *plugin = LUNAR_TPA (panel_plugin);
  gint       image_size;

  /* make the plugin fit a single row */
  size /= expidus_panel_plugin_get_nrows (panel_plugin);
  gtk_widget_set_size_request (GTK_WIDGET (panel_plugin), size, size);

#if LIBEXPIDUS1PANEL_CHECK_VERSION (4,13,0)
  image_size = expidus_panel_plugin_get_icon_size (panel_plugin);
#else
  image_size = size - 2; // fall-back for older panel versions
#endif
  gtk_image_set_pixel_size (GTK_IMAGE (plugin->image), image_size);

  return TRUE;
}



static void
lunar_tpa_error (LunarTpa *plugin,
                  GError    *error)
{
  gchar *tooltip;

  /* reset to empty first */
  lunar_tpa_state (plugin, FALSE);

  /* strip off additional whitespace */
  g_strstrip (error->message);

  /* tell the user that we failed to connect to the trash */
  tooltip = g_strdup_printf ("%s: %s.", _("Failed to connect to the Trash"), error->message);
  gtk_widget_set_tooltip_text (plugin->button, tooltip);
  g_free (tooltip);

  /* setup an error plugin */
  gtk_image_set_from_icon_name (GTK_IMAGE (plugin->image), "stock_dialog-error", GTK_ICON_SIZE_BUTTON);
}



static void
lunar_tpa_state (LunarTpa *plugin,
                  gboolean   full)
{
  /* tell the user whether the trash is full or empty */
  gtk_widget_set_tooltip_text (plugin->button, full ? _("Trash contains files") : _("Trash is empty"));

  /* setup the appropriate plugin */
  gtk_image_set_from_icon_name (GTK_IMAGE (plugin->image), full ? "user-trash-full" : "user-trash", GTK_ICON_SIZE_BUTTON);

  /* sensitivity of the menu item */
  gtk_widget_set_sensitive (plugin->mi, full);
}



static void
lunar_tpa_display_trash_reply (GObject      *source_object,
                                GAsyncResult *result,
                                gpointer      user_data)
{
  lunarTPATrash *proxy   = LUNAR_TPA_TRASH (source_object);
  gboolean        success = FALSE;
  GError         *error   = NULL;

  success =  lunar_tpa_trash_call_display_trash_finish (proxy, result, &error);
  if (G_UNLIKELY (success != TRUE))
    {
      /* display an error message to the user */
      g_strstrip (error->message);
      expidus_dialog_show_error (NULL, error, "%s.", _("Failed to connect to the Trash"));
      g_error_free (error);
    }
}



static void
lunar_tpa_empty_trash_reply (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      user_data)
{
  lunarTPATrash *proxy   = LUNAR_TPA_TRASH (source_object);
  LunarTpa      *plugin  = LUNAR_TPA (user_data);
  gboolean        success = FALSE;
  GError         *error   = NULL;

  success = lunar_tpa_trash_call_empty_trash_finish (proxy, result, &error);
  if (G_LIKELY (success))
    {
      /* query the new state of the trash */
      lunar_tpa_query_trash (plugin);
    }
  else
    {
      /* display an error message to the user */
      g_strstrip (error->message);
      expidus_dialog_show_error (NULL, error, "%s.", _("Failed to connect to the Trash"));
      g_error_free (error);
    }
}



static void
lunar_tpa_move_to_trash_reply (GObject      *source_object,
                                GAsyncResult *result,
                                gpointer      user_data)
{
  lunarTPATrash *proxy   = LUNAR_TPA_TRASH (source_object);
  LunarTpa      *plugin  = LUNAR_TPA (user_data);
  gboolean        success = FALSE;
  GError         *error   = NULL;

  success = lunar_tpa_trash_call_move_to_trash_finish (proxy, result, &error);
  if (G_LIKELY (success))
    {
      /* query the new state of the trash */
      lunar_tpa_query_trash (plugin);
    }
  else
    {
      /* display an error message to the user */
      g_strstrip (error->message);
      expidus_dialog_show_error (NULL, error, "%s.", _("Failed to connect to the Trash"));
      g_error_free (error);
    }
}



static void
lunar_tpa_query_trash_reply (GObject      *source_object,
                              GAsyncResult *result,
                              gpointer      user_data)
{
  lunarTPATrash *proxy   = LUNAR_TPA_TRASH (source_object);
  LunarTpa      *plugin  = LUNAR_TPA (user_data);
  gboolean        success = FALSE;
  GError         *error   = NULL;
  gboolean        full;

  success = lunar_tpa_trash_call_query_trash_finish (proxy, &full, result, &error);
  if(G_LIKELY (success))
    {
      /* update the tooltip/plugin accordingly */
      lunar_tpa_state (plugin, full);
    }
  else
    {
      /* setup an error tooltip/plugin */
      lunar_tpa_error (plugin, error);
      g_error_free (error);
    }
}



static void
lunar_tpa_drag_data_received (GtkWidget        *button,
                               GdkDragContext   *context,
                               gint              x,
                               gint              y,
                               GtkSelectionData *selection_data,
                               guint             info,
                               guint             timestamp,
                               LunarTpa        *plugin)
{
  gboolean succeed = FALSE;
  gchar  **uri_list;

  g_return_if_fail (LUNAR_IS_TPA (plugin));
  g_return_if_fail (plugin->button == button);

  /* determine the type of drop we received */
  if (G_LIKELY (info == TARGET_TEXT_URI_LIST))
    {
      /* check if the data is valid for text/uri-list */
      uri_list = gtk_selection_data_get_uris (selection_data);
      if (G_LIKELY (uri_list != NULL))
        {
          succeed = lunar_tpa_move_to_trash (plugin, (const gchar **) uri_list);
          g_strfreev (uri_list);
        }
    }

  /* finish the drag */
  gtk_drag_finish (context, succeed, TRUE, timestamp);
}



static gboolean
lunar_tpa_enter_notify_event (GtkWidget        *button,
                               GdkEventCrossing *event,
                               LunarTpa        *plugin)
{
  g_return_val_if_fail (LUNAR_IS_TPA (plugin), FALSE);
  g_return_val_if_fail (plugin->button == button, FALSE);

  /* query the new state of the trash */
  lunar_tpa_query_trash (plugin);

  return FALSE;
}



static gboolean
lunar_tpa_leave_notify_event (GtkWidget        *button,
                               GdkEventCrossing *event,
                               LunarTpa        *plugin)
{
  g_return_val_if_fail (LUNAR_IS_TPA (plugin), FALSE);
  g_return_val_if_fail (plugin->button == button, FALSE);

  /* query the new state of the trash */
  lunar_tpa_query_trash (plugin);

  return FALSE;
}



static void
lunar_tpa_on_trash_changed (lunarTPATrash *proxy,
                             gpointer        user_data)
{
  LunarTpa *plugin = LUNAR_TPA (user_data);

  g_return_if_fail (LUNAR_IS_TPA (plugin));
  g_return_if_fail (plugin->proxy == proxy);

  /* update the state of the trash plugin */
  lunar_tpa_query_trash (plugin);
}



static void
lunar_tpa_display_trash (LunarTpa *plugin)
{
  gchar     *display_name;
  gchar     *startup_id;
  GdkScreen *screen;

  g_return_if_fail (LUNAR_IS_TPA (plugin));

  /* check if we are connected to the bus */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending call and reset the cancellable */
      g_cancellable_cancel (plugin->cancellable_display_trash);
      g_cancellable_reset (plugin->cancellable_display_trash);

      /* schedule a new call */
      screen = gtk_widget_get_screen (GTK_WIDGET (plugin));
      display_name = g_strdup (gdk_display_get_name (gdk_screen_get_display (screen)));
      startup_id = g_strdup_printf ("_TIME%d", gtk_get_current_event_time ());
      lunar_tpa_trash_call_display_trash (plugin->proxy, display_name, startup_id, plugin->cancellable_display_trash, lunar_tpa_display_trash_reply, plugin);
      g_free (startup_id);
      g_free (display_name);
    }
}



static void
lunar_tpa_empty_trash (LunarTpa *plugin)
{
  gchar     *display_name;
  gchar     *startup_id;
  GdkScreen *screen;

  g_return_if_fail (LUNAR_IS_TPA (plugin));

  /* check if we are connected to the bus */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending call and reset the cancellable */
      g_cancellable_cancel (plugin->cancellable_empty_trash);
      g_cancellable_reset (plugin->cancellable_empty_trash);

      /* schedule a new call */
      screen = gtk_widget_get_screen (GTK_WIDGET (plugin));
      display_name = g_strdup (gdk_display_get_name (gdk_screen_get_display (screen)));
      startup_id = g_strdup_printf ("_TIME%d", gtk_get_current_event_time ());
      lunar_tpa_trash_call_empty_trash (plugin->proxy, display_name, startup_id, plugin->cancellable_empty_trash, lunar_tpa_empty_trash_reply, plugin);
      g_free (startup_id);
      g_free (display_name);
    }
}



static gboolean
lunar_tpa_move_to_trash (LunarTpa    *plugin,
                          const gchar **uri_list)
{
  gchar     *display_name;
  gchar     *startup_id;
  GdkScreen *screen;

  g_return_val_if_fail (LUNAR_IS_TPA (plugin), FALSE);
  g_return_val_if_fail (uri_list != NULL, FALSE);

  /* check if we are connected to the bus */
  if (G_UNLIKELY (plugin->proxy == NULL))
    return FALSE;

  /* cancel any pending call and reset the cancellable */
  g_cancellable_cancel (plugin->cancellable_move_to_trash);
  g_cancellable_reset (plugin->cancellable_move_to_trash);

  /* schedule a new call */
  screen = gtk_widget_get_screen (GTK_WIDGET (plugin));
  display_name = g_strdup (gdk_display_get_name (gdk_screen_get_display (screen)));
  startup_id = g_strdup_printf ("_TIME%d", gtk_get_current_event_time ());
  lunar_tpa_trash_call_move_to_trash (plugin->proxy, uri_list, display_name, startup_id, plugin->cancellable_move_to_trash, lunar_tpa_move_to_trash_reply, plugin);
  g_free (startup_id);
  g_free (display_name);

  return TRUE;
}



static void
lunar_tpa_query_trash (LunarTpa *plugin)
{
  g_return_if_fail (LUNAR_IS_TPA (plugin));

  /* check if we are connected to the bus */
  if (G_LIKELY (plugin->proxy != NULL))
    {
      /* cancel any pending call and reset the cancellable */
      g_cancellable_cancel (plugin->cancellable_query_trash);
      g_cancellable_reset (plugin->cancellable_query_trash);

      /* schedule a new call */
      lunar_tpa_trash_call_query_trash (plugin->proxy, plugin->cancellable_query_trash, lunar_tpa_query_trash_reply,plugin);
    }
}
