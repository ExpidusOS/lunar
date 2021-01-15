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

#ifndef __LUNAR_COLUMN_MODEL_H__
#define __LUNAR_COLUMN_MODEL_H__

#include <lunar/lunar-enum-types.h>

G_BEGIN_DECLS;

typedef struct _LunarColumnModelClass LunarColumnModelClass;
typedef struct _LunarColumnModel      LunarColumnModel;

#define LUNAR_TYPE_COLUMN_MODEL            (lunar_column_model_get_type ())
#define LUNAR_COLUMN_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_COLUMN_MODEL, LunarColumnModel))
#define LUNAR_COLUMN_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_COLUMN_MODEL, LunarColumnModelClass))
#define LUNAR_IS_COLUMN_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_COLUMN_MODEL))
#define LUNAR_IS_COLUMN_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_COLUMN_MODEL))
#define LUNAR_COLUMN_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_COLUMN_MODEL, LunarColumnModelClass))

/**
 * LunarColumnModelColumn:
 * @LUNAR_COLUMN_MODEL_COLUMN_NAME    : the name of the column.
 * @LUNAR_COLUMN_MODEL_COLUMN_MUTABLE : TRUE if the visibility can be changed.
 * @LUNAR_COLUMN_MODEL_COLUMN_VISIBLE : whether the column is visible.
 *
 * The #LunarColumnModel columns used by the #LunarColumnEditor.
 **/
typedef enum
{
  LUNAR_COLUMN_MODEL_COLUMN_NAME,
  LUNAR_COLUMN_MODEL_COLUMN_MUTABLE,
  LUNAR_COLUMN_MODEL_COLUMN_VISIBLE,
  LUNAR_COLUMN_MODEL_N_COLUMNS,
} LunarColumnModelColumn;

GType               lunar_column_model_get_type            (void) G_GNUC_CONST;

LunarColumnModel  *lunar_column_model_get_default         (void);

void                lunar_column_model_exchange            (LunarColumnModel *column_model,
                                                             GtkTreeIter       *iter1,
                                                             GtkTreeIter       *iter2);

LunarColumn        lunar_column_model_get_column_for_iter (LunarColumnModel *column_model,
                                                             GtkTreeIter       *iter);

const LunarColumn *lunar_column_model_get_column_order    (LunarColumnModel *column_model);

const gchar        *lunar_column_model_get_column_name     (LunarColumnModel *column_model,
                                                             LunarColumn       column);

gboolean            lunar_column_model_get_column_visible  (LunarColumnModel *column_model,
                                                             LunarColumn       column);
void                lunar_column_model_set_column_visible  (LunarColumnModel *column_model,
                                                             LunarColumn       column,
                                                             gboolean           visible);

gint                lunar_column_model_get_column_width    (LunarColumnModel *column_model,
                                                             LunarColumn       column);
void                lunar_column_model_set_column_width    (LunarColumnModel *column_model,
                                                             LunarColumn       column,
                                                             gint               width);

G_END_DECLS;

#endif /* !__LUNAR_COLUMN_MODEL_H__ */
