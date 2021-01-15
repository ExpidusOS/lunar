/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __LUNAR_UCA_MODEL_H__
#define __LUNAR_UCA_MODEL_H__

#include <lunarx/lunarx.h>

G_BEGIN_DECLS;

typedef struct _LunarUcaModelClass LunarUcaModelClass;
typedef struct _LunarUcaModel      LunarUcaModel;

#define LUNAR_UCA_TYPE_MODEL             (lunar_uca_model_get_type ())
#define LUNAR_UCA_MODEL(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_UCA_TYPE_MODEL, LunarUcaModel))
#define LUNAR_UCA_MODEL_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_UCA_TYPE_MODEL, LunarUcaModelClass))
#define LUNAR_UCA_IS_MODEL(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_UCA_TYPE_MODEL))
#define LUNAR_UCA_IS_MODEL_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_UCA_TYPE_MODEL))
#define LUNAR_UCA_MODEL_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_UCA_TYPE_MODEL, LunarUcaModelClass))

typedef enum
{
  LUNAR_UCA_MODEL_COLUMN_NAME,
  LUNAR_UCA_MODEL_COLUMN_SUB_MENU,
  LUNAR_UCA_MODEL_COLUMN_DESCRIPTION,
  LUNAR_UCA_MODEL_COLUMN_GICON,
  LUNAR_UCA_MODEL_COLUMN_ICON_NAME,
  LUNAR_UCA_MODEL_COLUMN_UNIQUE_ID,
  LUNAR_UCA_MODEL_COLUMN_COMMAND,
  LUNAR_UCA_MODEL_COLUMN_STARTUP_NOTIFY,
  LUNAR_UCA_MODEL_COLUMN_PATTERNS,
  LUNAR_UCA_MODEL_COLUMN_TYPES,
  LUNAR_UCA_MODEL_COLUMN_STOCK_LABEL,
  LUNAR_UCA_MODEL_N_COLUMNS,
} LunarUcaModelColumn;

/**
 * LunarUcaTypes:
 * @LUNAR_UCA_TYPE_DIRECTORIES : directories.
 * @LUNAR_UCA_TYPE_AUDIO_FILES : audio files.
 * @LUNAR_UCA_TYPE_IMAGE_FILES : image files.
 * @LUNAR_UCA_TYPE_OTHER_FILES : other files.
 * @LUNAR_UCA_TYPE_TEXT_FILES  : text files.
 * @LUNAR_UCA_TYPE_VIDEO_FILES : video files.
 **/
typedef enum /*< flags >*/
{
  LUNAR_UCA_TYPE_DIRECTORIES = 1 << 0,
  LUNAR_UCA_TYPE_AUDIO_FILES = 1 << 1,
  LUNAR_UCA_TYPE_IMAGE_FILES = 1 << 2,
  LUNAR_UCA_TYPE_OTHER_FILES = 1 << 3,
  LUNAR_UCA_TYPE_TEXT_FILES  = 1 << 4,
  LUNAR_UCA_TYPE_VIDEO_FILES = 1 << 5,
} LunarUcaTypes;

GType           lunar_uca_model_get_type       (void) G_GNUC_CONST;
void            lunar_uca_model_register_type  (LunarxProviderPlugin  *plugin);

LunarUcaModel *lunar_uca_model_get_default    (void);

GList          *lunar_uca_model_match          (LunarUcaModel         *uca_model,
                                                 GList                  *file_infos);

void            lunar_uca_model_append         (LunarUcaModel         *uca_model,
                                                 GtkTreeIter            *iter);

void            lunar_uca_model_exchange       (LunarUcaModel         *uca_model,
                                                 GtkTreeIter            *iter_a,
                                                 GtkTreeIter            *iter_b);

void            lunar_uca_model_remove         (LunarUcaModel         *uca_model,
                                                 GtkTreeIter            *iter);

void            lunar_uca_model_update         (LunarUcaModel         *uca_model,
                                                 GtkTreeIter            *iter,
                                                 const gchar            *name,
                                                 const gchar            *submenu,
                                                 const gchar            *unique_id,
                                                 const gchar            *description,
                                                 const gchar            *icon,
                                                 const gchar            *command,
                                                 gboolean                startup_notify,
                                                 const gchar            *patterns,
                                                 LunarUcaTypes          types,
                                                 guint                   accel_key,
                                                 GdkModifierType         accel_mods);

gboolean        lunar_uca_model_save           (LunarUcaModel         *uca_model,
                                                 GError                **error);

gboolean        lunar_uca_model_parse_argv     (LunarUcaModel         *uca_model,
                                                 GtkTreeIter            *iter,
                                                 GList                  *file_infos,
                                                 gint                   *argcp,
                                                 gchar                ***argvp,
                                                 GError                **error);

G_END_DECLS;

#endif /* !__LUNAR_UCA_MODEL_H__ */
