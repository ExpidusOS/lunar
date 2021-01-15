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

#ifndef __LUNAR_PROGRESS_VIEW_H__
#define __LUNAR_PROGRESS_VIEW_H__

#include <gtk/gtk.h>

#include <lunar/lunar-job.h>

G_BEGIN_DECLS;

typedef struct _LunarProgressViewClass LunarProgressViewClass;
typedef struct _LunarProgressView      LunarProgressView;

#define LUNAR_TYPE_PROGRESS_VIEW            (lunar_progress_view_get_type ())
#define LUNAR_PROGRESS_VIEW(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_PROGRESS_VIEW, LunarProgressView))
#define LUNAR_PROGRESS_VIEW_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_PROGRESS_VIEW, LunarProgressViewClass))
#define LUNAR_IS_PROGRESS_VIEW(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_PROGRESS_VIEW))
#define LUNAR_IS_PROGRESS_VIEW_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_PROGRESS_VIEW))
#define LUNAR_PROGRESS_VIEW_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_PROGRESS_VIEW, LunarProgressViewClass))

GType      lunar_progress_view_get_type      (void) G_GNUC_CONST;

GtkWidget *lunar_progress_view_new_with_job  (LunarJob          *job) G_GNUC_MALLOC;

void       lunar_progress_view_set_icon_name (LunarProgressView *view,
                                               const gchar        *icon_name);
void       lunar_progress_view_set_title     (LunarProgressView *view,
                                               const gchar        *title);
LunarJob *lunar_progress_view_get_job       (LunarProgressView *view);

G_END_DECLS;

#endif /* !__LUNAR_PROGRESS_VIEW_H__ */
