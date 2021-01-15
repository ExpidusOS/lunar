/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_ICON_FACTORY_H__
#define __LUNAR_ICON_FACTORY_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarIconFactoryClass LunarIconFactoryClass;
typedef struct _LunarIconFactory      LunarIconFactory;

#define LUNAR_TYPE_ICON_FACTORY            (lunar_icon_factory_get_type ())
#define LUNAR_ICON_FACTORY(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_ICON_FACTORY, LunarIconFactory))
#define LUNAR_ICON_FACTORY_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_ICON_FACTORY, LunarIconFactoryClass))
#define LUNAR_IS_ICON_FACTORY(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_ICON_FACTORY))
#define LUNAR_IS_ICON_FACTORY_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_ICON_FACTORY))
#define LUNAR_ICON_FACTORY_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_ICON_FACTORY, LunarIconFactoryClass))



GType                  lunar_icon_factory_get_type           (void) G_GNUC_CONST;

LunarIconFactory     *lunar_icon_factory_get_default        (void);
LunarIconFactory     *lunar_icon_factory_get_for_icon_theme (GtkIconTheme             *icon_theme);

gboolean               lunar_icon_factory_get_show_thumbnail (const LunarIconFactory  *factory,
                                                               const LunarFile         *file);

GdkPixbuf             *lunar_icon_factory_load_icon          (LunarIconFactory        *factory,
                                                               const gchar              *name,
                                                               gint                      size,
                                                               gboolean                  wants_default);

GdkPixbuf             *lunar_icon_factory_load_file_icon     (LunarIconFactory        *factory,
                                                               LunarFile               *file,
                                                               LunarFileIconState       icon_state,
                                                               gint                      icon_size);

void                   lunar_icon_factory_clear_pixmap_cache (LunarFile               *file);

G_END_DECLS;

#endif /* !__LUNAR_ICON_FACTORY_H__ */
