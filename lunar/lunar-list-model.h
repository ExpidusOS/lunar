/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2004-2007 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_LIST_MODEL_H__
#define __LUNAR_LIST_MODEL_H__

#include <lunar/lunar-folder.h>

G_BEGIN_DECLS;

typedef struct _LunarListModelClass LunarListModelClass;
typedef struct _LunarListModel      LunarListModel;

#define LUNAR_TYPE_LIST_MODEL            (lunar_list_model_get_type ())
#define LUNAR_LIST_MODEL(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_LIST_MODEL, LunarListModel))
#define LUNAR_LIST_MODEL_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_LIST_MODEL, LunarListModelClass))
#define LUNAR_IS_LIST_MODEL(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_LIST_MODEL))
#define LUNAR_IS_LIST_MODEL_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_LIST_MODEL))
#define LUNAR_LIST_MODEL_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_LIST_MODEL, LunarListModelClass))

GType            lunar_list_model_get_type               (void) G_GNUC_CONST;

LunarListModel *lunar_list_model_new                    (void);

LunarFolder    *lunar_list_model_get_folder             (LunarListModel  *store);
void             lunar_list_model_set_folder             (LunarListModel  *store,
                                                           LunarFolder     *folder);

void             lunar_list_model_set_folders_first      (LunarListModel  *store,
                                                           gboolean          folders_first);

gboolean         lunar_list_model_get_show_hidden        (LunarListModel  *store);
void             lunar_list_model_set_show_hidden        (LunarListModel  *store,
                                                           gboolean          show_hidden);

gboolean         lunar_list_model_get_file_size_binary   (LunarListModel  *store);
void             lunar_list_model_set_file_size_binary   (LunarListModel  *store,
                                                           gboolean          file_size_binary);

LunarFile      *lunar_list_model_get_file               (LunarListModel  *store,
                                                           GtkTreeIter      *iter);


GList           *lunar_list_model_get_paths_for_files    (LunarListModel  *store,
                                                           GList            *files);
GList           *lunar_list_model_get_paths_for_pattern  (LunarListModel  *store,
                                                           const gchar      *pattern);

gchar           *lunar_list_model_get_statusbar_text     (LunarListModel  *store,
                                                           GList            *selected_items);

G_END_DECLS;

#endif /* !__LUNAR_LIST_MODEL_H__ */
