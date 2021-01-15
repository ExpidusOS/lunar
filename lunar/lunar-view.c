/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#include <lunar/lunar-private.h>
#include <lunar/lunar-view.h>



static void lunar_view_class_init (gpointer klass);



GType
lunar_view_get_type (void)
{
  static volatile gsize type__volatile = 0;
  GType                 type;

  if (g_once_init_enter (&type__volatile))
    {
      type = g_type_register_static_simple (G_TYPE_INTERFACE,
                                            I_("LunarView"),
                                            sizeof (LunarViewIface),
                                            (GClassInitFunc) (void (*)(void)) lunar_view_class_init,
                                            0,
                                            NULL,
                                            0);

      g_type_interface_add_prerequisite (type, GTK_TYPE_WIDGET);
      g_type_interface_add_prerequisite (type, LUNAR_TYPE_COMPONENT);

      g_once_init_leave (&type__volatile, type);
    }

  return type__volatile;
}



static void
lunar_view_class_init (gpointer klass)
{
  /**
   * LunarView:loading:
   *
   * Indicates whether the given #LunarView is currently loading or
   * layouting its contents. Implementations should invoke
   * #g_object_notify() on this property whenever they start to load
   * the contents and then once they have finished loading.
   *
   * Other modules can use this property to display some kind of
   * user visible notification about the loading state, e.g. a
   * progress bar or an animated image.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boolean ("loading",
                                                             "loading",
                                                             "loading",
                                                             FALSE,
                                                             ENDO_PARAM_READABLE));

  /**
   * LunarView:statusbar-text:
   *
   * The text to be displayed in the status bar, which is associated
   * with this #LunarView instance. Implementations should invoke
   * #g_object_notify() on this property, whenever they have a new
   * text to be display in the status bar (e.g. the selection changed
   * or similar).
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_string ("statusbar-text",
                                                            "statusbar-text",
                                                            "statusbar-text",
                                                            NULL,
                                                            ENDO_PARAM_READABLE));

  /**
   * LunarView:show-hidden:
   *
   * Tells whether to display hidden and backup files in the
   * #LunarView or whether to hide them.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_boolean ("show-hidden",
                                                             "show-hidden",
                                                             "show-hidden",
                                                             FALSE,
                                                             ENDO_PARAM_READWRITE));

  /**
   * LunarView:zoom-level:
   *
   * The #LunarZoomLevel at which the items within this
   * #LunarView should be displayed.
   **/
  g_object_interface_install_property (klass,
                                       g_param_spec_enum ("zoom-level",
                                                          "zoom-level",
                                                          "zoom-level",
                                                          LUNAR_TYPE_ZOOM_LEVEL,
                                                          LUNAR_ZOOM_LEVEL_100_PERCENT,
                                                          ENDO_PARAM_READWRITE));
}



/**
 * lunar_view_get_loading:
 * @view : a #LunarView instance.
 *
 * Tells whether the given #LunarView is currently loading or
 * layouting its contents.
 *
 * Return value: %TRUE if @view is currently being loaded, else %FALSE.
 **/
gboolean
lunar_view_get_loading (LunarView *view)
{
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (view), FALSE);
  return (*LUNAR_VIEW_GET_IFACE (view)->get_loading) (view);
}



/**
 * lunar_view_get_statusbar_text:
 * @view : a #LunarView instance.
 *
 * Queries the text that should be displayed in the status bar
 * associated with @view.
 *
 * Return value: the text to be displayed in the status bar
 *               asssociated with @view.
 **/
const gchar*
lunar_view_get_statusbar_text (LunarView *view)
{
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (view), NULL);
  return (*LUNAR_VIEW_GET_IFACE (view)->get_statusbar_text) (view);
}



/**
 * lunar_view_get_show_hidden:
 * @view : a #LunarView instance.
 *
 * Returns %TRUE if hidden and backup files are shown
 * in @view. Else %FALSE is returned.
 *
 * Return value: whether @view displays hidden files.
 **/
gboolean
lunar_view_get_show_hidden (LunarView *view)
{
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (view), FALSE);
  return (*LUNAR_VIEW_GET_IFACE (view)->get_show_hidden) (view);
}



/**
 * lunar_view_set_show_hidden:
 * @view        : a #LunarView instance.
 * @show_hidden : &TRUE to display hidden files, else %FALSE.
 *
 * If @show_hidden is %TRUE then @view will display hidden and
 * backup files, else those files will be hidden from the user
 * interface.
 **/
void
lunar_view_set_show_hidden (LunarView *view,
                             gboolean    show_hidden)
{
  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  (*LUNAR_VIEW_GET_IFACE (view)->set_show_hidden) (view, show_hidden);
}



/**
 * lunar_view_get_zoom_level:
 * @view : a #LunarView instance.
 *
 * Returns the #LunarZoomLevel currently used for the @view.
 *
 * Return value: the #LunarZoomLevel currently used for the @view.
 **/
LunarZoomLevel
lunar_view_get_zoom_level (LunarView *view)
{
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (view), LUNAR_ZOOM_LEVEL_100_PERCENT);
  return (*LUNAR_VIEW_GET_IFACE (view)->get_zoom_level) (view);
}



/**
 * lunar_view_set_zoom_level:
 * @view       : a #LunarView instance.
 * @zoom_level : the new #LunarZoomLevel for @view.
 *
 * Sets the zoom level used for @view to @zoom_level.
 **/
void
lunar_view_set_zoom_level (LunarView     *view,
                            LunarZoomLevel zoom_level)
{
  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  _lunar_return_if_fail (zoom_level < LUNAR_ZOOM_N_LEVELS);
  (*LUNAR_VIEW_GET_IFACE (view)->set_zoom_level) (view, zoom_level);
}



/**
 * lunar_view_reset_zoom_level:
 * @view : a #LunarView instance.
 *
 * Resets the zoom level of @view to the default
 * #LunarZoomLevel for @view.
 **/
void
lunar_view_reset_zoom_level (LunarView *view)
{
  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  (*LUNAR_VIEW_GET_IFACE (view)->reset_zoom_level) (view);
}



/**
 * lunar_view_reload:
 * @view : a #LunarView instance.
 * @reload_info : whether to reload file info for all files too
 *
 * Tells @view to reread the currently displayed folder
 * contents from the underlying media. If reload_info is
 * TRUE, it will reload information for all files too.
 **/
void
lunar_view_reload (LunarView *view,
                    gboolean    reload_info)
{
  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  (*LUNAR_VIEW_GET_IFACE (view)->reload) (view, reload_info);
}



/**
 * lunar_view_get_visible_range:
 * @view       : a #LunarView instance.
 * @start_file : return location for start of region, or %NULL.
 * @end_file   : return location for end of region, or %NULL.
 *
 * Sets @start_file and @end_file to be the first and last visible
 * #LunarFile.
 *
 * The files should be freed with g_object_unref() when no
 * longer needed.
 *
 * Return value: %TRUE if valid files were placed in @start_file
 *               and @end_file.
 **/
gboolean
lunar_view_get_visible_range (LunarView  *view,
                               LunarFile **start_file,
                               LunarFile **end_file)
{
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (view), FALSE);
  return (*LUNAR_VIEW_GET_IFACE (view)->get_visible_range) (view, start_file, end_file);
}



/**
 * lunar_view_scroll_to_file:
 * @view        : a #LunarView instance.
 * @file        : the #LunarFile to scroll to.
 * @select_file : %TRUE to also select the @file in the @view.
 * @use_align   : whether to use alignment arguments.
 * @row_align   : the vertical alignment.
 * @col_align   : the horizontal alignment.
 *
 * Tells @view to scroll to the @file. If @view is currently
 * loading, it'll remember to scroll to @file later when
 * the contents are loaded.
 **/
void
lunar_view_scroll_to_file (LunarView *view,
                            LunarFile *file,
                            gboolean    select_file,
                            gboolean    use_align,
                            gfloat      row_align,
                            gfloat      col_align)
{
  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (row_align >= 0.0f && row_align <= 1.0f);
  _lunar_return_if_fail (col_align >= 0.0f && col_align <= 1.0f);
  (*LUNAR_VIEW_GET_IFACE (view)->scroll_to_file) (view, file, select_file, use_align, row_align, col_align);
}



GList*
lunar_view_get_selected_files (LunarView *view)
{
  _lunar_return_val_if_fail (LUNAR_IS_VIEW (view), NULL);
  return (*LUNAR_VIEW_GET_IFACE (view)->get_selected_files) (view);
}



void
lunar_view_set_selected_files (LunarView *view,
                                GList      *path_list)
{
  _lunar_return_if_fail (LUNAR_IS_VIEW (view));
  (*LUNAR_VIEW_GET_IFACE (view)->set_selected_files) (view, path_list);
}
