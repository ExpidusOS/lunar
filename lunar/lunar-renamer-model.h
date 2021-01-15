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

#ifndef __LUNAR_RENAMER_MODEL_H__
#define __LUNAR_RENAMER_MODEL_H__

#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarRenamerModelClass LunarRenamerModelClass;
typedef struct _LunarRenamerModel      LunarRenamerModel;

#define LUNAR_TYPE_RENAMER_MODEL             (lunar_renamer_model_get_type ())
#define LUNAR_RENAMER_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_RENAMER_MODEL, LunarRenamerModel))
#define LUNAR_RENAMER_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_RENAMER_MODEL, LunarRenamerModelClass))
#define LUNAR_IS_RENAMER_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_RENAMER_MODEL))
#define LUNAR_IS_RENAMER_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_RENAMER_MODEL))
#define LUNAR_RENAMER_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_RENAMER_MODEL, LunarRenamerModelClass))

/**
 * LunarRenamerModelColumn:
 * @LUNAR_RENAMER_MODEL_COLUMN_CONFLICT        : the column which tells whether there's a name conflict.
 * @LUNAR_RENAMER_MODEL_COLUMN_CONFLICT_WEIGHT : Use to set the text to bold in case of a conflict
 * @LUNAR_RENAMER_MODEL_COLUMN_FILE            : the column with the #LunarFile.
 * @LUNAR_RENAMER_MODEL_COLUMN_NEWNAME         : the column with the new name.
 * @LUNAR_RENAMER_MODEL_COLUMN_OLDNAME         : the column with the old name.
 *
 * The column ids provided by #LunarRenamerModel instances.
 **/
typedef enum
{
  LUNAR_RENAMER_MODEL_COLUMN_CONFLICT,
  LUNAR_RENAMER_MODEL_COLUMN_CONFLICT_WEIGHT,
  LUNAR_RENAMER_MODEL_COLUMN_FILE,
  LUNAR_RENAMER_MODEL_COLUMN_NEWNAME,
  LUNAR_RENAMER_MODEL_COLUMN_OLDNAME,
  LUNAR_RENAMER_MODEL_N_COLUMNS,
} LunarRenamerModelColumn;

GType                lunar_renamer_model_get_type       (void) G_GNUC_CONST;

LunarRenamerModel  *lunar_renamer_model_new            (void) G_GNUC_MALLOC;

LunarRenamerMode    lunar_renamer_model_get_mode       (LunarRenamerModel *renamer_model);

LunarxRenamer      *lunar_renamer_model_get_renamer    (LunarRenamerModel *renamer_model);
void                 lunar_renamer_model_set_renamer    (LunarRenamerModel *renamer_model,
                                                          LunarxRenamer     *renamer);

void                 lunar_renamer_model_insert         (LunarRenamerModel *renamer_model,
                                                          LunarFile         *file,
                                                          gint                position);
void                 lunar_renamer_model_reorder        (LunarRenamerModel *renamer_model,
                                                          GList              *tree_paths,
                                                          gint                position);
void                 lunar_renamer_model_sort           (LunarRenamerModel *renamer_model,
                                                          GtkSortType         sort_order);
void                 lunar_renamer_model_clear          (LunarRenamerModel *renamer_model);
void                 lunar_renamer_model_remove         (LunarRenamerModel *renamer_model,
                                                          GtkTreePath        *path);


/**
 * lunar_renamer_model_append:
 * @model : a #LunarRenamerModel.
 * @file  : a #LunarFile instance.
 *
 * Appends the @file to the @renamer_model.
 **/
#define lunar_renamer_model_append(model,file) lunar_renamer_model_insert (model, file, -1)

G_END_DECLS;

#endif /* !__LUNAR_RENAMER_MODEL_H__ */
