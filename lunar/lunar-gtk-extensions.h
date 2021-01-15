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

#ifndef __LUNAR_GTK_EXTENSIONS_H__
#define __LUNAR_GTK_EXTENSIONS_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;


void             lunar_gtk_label_set_a11y_relation           (GtkLabel           *label,
                                                               GtkWidget          *widget);
void             lunar_gtk_menu_clean                        (GtkMenu            *menu);
void             lunar_gtk_menu_run                          (GtkMenu            *menu);

void             lunar_gtk_menu_run_at_event                 (GtkMenu            *menu,
                                                               GdkEvent           *event);
void             lunar_gtk_widget_set_tooltip                (GtkWidget          *widget,
                                                               const gchar        *format,
                                                               ...) G_GNUC_PRINTF (2, 3);
GtkWidget       *lunar_gtk_menu_lunarx_menu_item_new        (GObject            *lunarx_menu_item,
                                                               GtkMenuShell       *menu_to_append_item);

GMountOperation *lunar_gtk_mount_operation_new               (gpointer            parent);

GtkWidget       *lunar_gtk_get_focused_widget                (void);

gboolean         lunar_gtk_editable_can_cut                  (GtkEditable        *editable);
gboolean         lunar_gtk_editable_can_copy                 (GtkEditable        *editable);
gboolean         lunar_gtk_editable_can_paste                (GtkEditable        *editable);

G_END_DECLS;

#endif /* !__LUNAR_GTK_EXTENSIONS_H__ */
