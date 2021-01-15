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

#ifndef __LUNAR_RENAMER_DIALOG_H__
#define __LUNAR_RENAMER_DIALOG_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarRenamerDialogClass LunarRenamerDialogClass;
typedef struct _LunarRenamerDialog      LunarRenamerDialog;

#define LUNAR_TYPE_RENAMER_DIALOG            (lunar_renamer_dialog_get_type ())
#define LUNAR_RENAMER_DIALOG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_RENAMER_DIALOG, LunarRenamerDialog))
#define LUNAR_RENAMER_DIALOG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_RENAMER_DIALOG, LunarRenamerDialogClass))
#define LUNAR_IS_RENAMER_DIALOG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_RENAMER_DIALOG))
#define LUNAR_IS_RENAMER_DIALOG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_RENAMER_DIALOG))
#define LUNAR_RENAMER_DIALOG_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_RENAMER_DIALOG, LunarRenamerDialogClass))

GType       lunar_renamer_dialog_get_type              (void) G_GNUC_CONST;

void        lunar_show_renamer_dialog                  (gpointer             parent,
                                                         LunarFile          *current_directory,
                                                         GList               *files,
                                                         gboolean             standalone,
                                                         const gchar         *startup_id);

G_END_DECLS;

#endif /* !__LUNAR_RENAMER_DIALOG_H__ */
