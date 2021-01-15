/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>.
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

#ifndef __LUNAR_CHOOSER_BUTTON_H__
#define __LUNAR_CHOOSER_BUTTON_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarChooserButtonClass LunarChooserButtonClass;
typedef struct _LunarChooserButton      LunarChooserButton;

#define LUNAR_TYPE_CHOOSER_BUTTON            (lunar_chooser_button_get_type ())
#define LUNAR_CHOOSER_BUTTON(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_CHOOSER_BUTTON, LunarChooserButton))
#define LUNAR_CHOOSER_BUTTON_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_CHOOSER_BUTTON, LunarChooserButtonClass))
#define LUNAR_IS_CHOOSER_BUTTON(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_CHOOSER_BUTTON))
#define LUNAR_IS_CHOOSER_BUTTON_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_CHOOSER_BUTTON))
#define LUNAR_CHOOSER_BUTTON_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_CHOOSER_BUTTON, LunarChooserButtonClass))

GType       lunar_chooser_button_get_type (void) G_GNUC_CONST;

GtkWidget  *lunar_chooser_button_new      (void) G_GNUC_MALLOC;

void        lunar_chooser_button_set_file (LunarChooserButton *chooser_button,
                                            LunarFile          *file);

G_END_DECLS;

#endif /* !__LUNAR_CHOOSER_BUTTON_H__ */
