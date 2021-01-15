/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>.
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

#ifndef __LUNAR_TREE_MODEL_H__
#define __LUNAR_TREE_MODEL_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarTreeModelClass LunarTreeModelClass;
typedef struct _LunarTreeModel      LunarTreeModel;

typedef gboolean (* LunarTreeModelVisibleFunc) (LunarTreeModel *model,
                                                 LunarFile      *file,
                                                 gpointer         data);

#define LUNAR_TYPE_TREE_MODEL            (lunar_tree_model_get_type ())
#define LUNAR_TREE_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_TREE_MODEL, LunarTreeModel))
#define LUNAR_TREE_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_TREE_MODEL, LunarTreeModelClass))
#define LUNAR_IS_TREE_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_TREE_MODEL))
#define LUNAR_IS_TREE_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_TREE_MODEL))
#define LUNAR_TREE_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_TREE_MODEL, LunarTreeModelClass))

/**
 * LunarTreeModelColumn:
 * @LUNAR_TREE_MODEL_COLUMN_FILE   : the index of the file column.
 * @LUNAR_TREE_MODEL_COLUMN_NAME   : the index of the name column.
 * @LUNAR_TREE_MODEL_COLUMN_ATTR   : the index of the #PangoAttrList column.
 * @LUNAR_TREE_MODEL_COLUMN_DEVICE : the index of the #LunarDevice column.
 *
 * Columns exported by the #LunarTreeModel using the
 * #GtkTreeModel interface.
 **/
typedef enum
{
  LUNAR_TREE_MODEL_COLUMN_FILE,
  LUNAR_TREE_MODEL_COLUMN_NAME,
  LUNAR_TREE_MODEL_COLUMN_ATTR,
  LUNAR_TREE_MODEL_COLUMN_DEVICE,
  LUNAR_TREE_MODEL_N_COLUMNS,
} LunarTreeModelColumn;

GType            lunar_tree_model_get_type           (void) G_GNUC_CONST;

void             lunar_tree_model_set_visible_func   (LunarTreeModel            *model,
                                                       LunarTreeModelVisibleFunc  func,
                                                       gpointer                    data);
void             lunar_tree_model_refilter           (LunarTreeModel            *model);

void             lunar_tree_model_cleanup            (LunarTreeModel            *model);
gboolean         lunar_tree_model_node_has_dummy     (LunarTreeModel            *model,
                                                       GNode                      *node);
void             lunar_tree_model_add_child          (LunarTreeModel            *model,
                                                       GNode                      *node,
                                                       LunarFile                 *file);

G_END_DECLS;

#endif /* !__LUNAR_TREE_MODEL_H__ */
