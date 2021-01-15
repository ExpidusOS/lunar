/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_CLIPBOARD_MANAGER_H__
#define __LUNAR_CLIPBOARD_MANAGER_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarClipboardManagerClass LunarClipboardManagerClass;
typedef struct _LunarClipboardManager      LunarClipboardManager;

#define LUNAR_TYPE_CLIPBOARD_MANAGER             (lunar_clipboard_manager_get_type ())
#define LUNAR_CLIPBOARD_MANAGER(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_CLIPBOARD_MANAGER, LunarClipboardManager))
#define LUNAR_CLIPBOARD_MANAGER_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((obj), LUNAR_TYPE_CLIPBOARD_MANAGER, LunarClipboardManagerClass))
#define LUNAR_IS_CLIPBOARD_MANAGER(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_CLIPBOARD_MANAGER))
#define LUNAR_IS_CLIPBOARD_MANAGER_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_CLIPBOARD_MANAGER))
#define LUNAR_CLIPBOARD_MANAGER_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_CLIPBOARD_MANAGER, LunarClipboardManagerClass))

GType                   lunar_clipboard_manager_get_type        (void) G_GNUC_CONST;

LunarClipboardManager *lunar_clipboard_manager_get_for_display (GdkDisplay             *display);

gboolean                lunar_clipboard_manager_get_can_paste   (LunarClipboardManager *manager);

gboolean                lunar_clipboard_manager_has_cutted_file (LunarClipboardManager *manager,
                                                                  const LunarFile       *file);

void                    lunar_clipboard_manager_copy_files      (LunarClipboardManager *manager,
                                                                  GList                  *files);
void                    lunar_clipboard_manager_cut_files       (LunarClipboardManager *manager,
                                                                  GList                  *files);
void                    lunar_clipboard_manager_paste_files     (LunarClipboardManager *manager,
                                                                  GFile                  *target_file,
                                                                  GtkWidget              *widget,
                                                                  GClosure               *new_files_closure);

G_END_DECLS;

#endif /* !__LUNAR_CLIPBOARD_MANAGER_H__ */
