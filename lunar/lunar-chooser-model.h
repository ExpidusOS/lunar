/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
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

#ifndef __LUNAR_CHOOSER_MODEL_H__
#define __LUNAR_CHOOSER_MODEL_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS;

typedef struct _LunarChooserModelClass LunarChooserModelClass;
typedef struct _LunarChooserModel      LunarChooserModel;

#define LUNAR_TYPE_CHOOSER_MODEL            (lunar_chooser_model_get_type ())
#define LUNAR_CHOOSER_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_CHOOSER_MODEL, LunarChooserModel))
#define LUNAR_CHOOSER_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_CHOOSER_MODEL, LunarChooserModelClass))
#define LUNAR_IS_CHOOSER_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_CHOOSER_MODEL))
#define LUNAR_IS_CHOOSER_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_CHOOSER_MODEL))
#define LUNAR_CHOOSER_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_CHOOSER_MODEL, LunarChooserModelClass))

/**
 * LunarChooserModelColumn:
 * @LUNAR_CHOOSER_MODEL_COLUMN_NAME        : the name of the application.
 * @LUNAR_CHOOSER_MODEL_COLUMN_ICON        : the name or absolute path of the application's icon.
 * @LUNAR_CHOOSER_MODEL_COLUMN_APPLICATION : the #GAppInfo object.
 * @LUNAR_CHOOSER_MODEL_COLUMN_STYLE       : custom font style.
 * @LUNAR_CHOOSER_MODEL_N_COLUMNS          : the number of columns in #LunarChooserModel.
 *
 * The identifiers for the columns provided by the #LunarChooserModel.
 **/
typedef enum
{
  LUNAR_CHOOSER_MODEL_COLUMN_NAME,
  LUNAR_CHOOSER_MODEL_COLUMN_ICON,
  LUNAR_CHOOSER_MODEL_COLUMN_APPLICATION,
  LUNAR_CHOOSER_MODEL_COLUMN_STYLE,
  LUNAR_CHOOSER_MODEL_COLUMN_WEIGHT,
  LUNAR_CHOOSER_MODEL_N_COLUMNS,
} LunarChooserModelColumn;

GType               lunar_chooser_model_get_type         (void) G_GNUC_CONST;

LunarChooserModel *lunar_chooser_model_new              (const gchar        *content_type) G_GNUC_MALLOC;
const gchar        *lunar_chooser_model_get_content_type (LunarChooserModel *model);
gboolean            lunar_chooser_model_remove           (LunarChooserModel *model,
                                                           GtkTreeIter        *iter,
                                                           gboolean            delete,
                                                           GError            **error);

G_END_DECLS;

#endif /* !__LUNAR_CHOOSER_MODEL_H__ */
