/* vi:set sw=2 sts=2 ts=2 et ai: */
/*-
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA  02111-1307  USA
 */

#ifndef __LUNAR_DEEP_COUNT_JOB_H__
#define __LUNAR_DEEP_COUNT_JOB_H__

#include <gio/gio.h>

#include <lunar/lunar-job.h>

G_BEGIN_DECLS;

typedef struct _LunarDeepCountJobPrivate LunarDeepCountJobPrivate;
typedef struct _LunarDeepCountJobClass   LunarDeepCountJobClass;
typedef struct _LunarDeepCountJob        LunarDeepCountJob;

#define LUNAR_TYPE_DEEP_COUNT_JOB            (lunar_deep_count_job_get_type ())
#define LUNAR_DEEP_COUNT_JOB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_DEEP_COUNT_JOB, LunarDeepCountJob))
#define LUNAR_DEEP_COUNT_JOB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_DEEP_COUNT_JOB, LunarDeepCountJobClass))
#define LUNAR_IS_DEEP_COUNT_JOB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_DEEP_COUNT_JOB))
#define LUNAR_IS_DEEP_COUNT_JOB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_DEEP_COUNT_JOB)
#define LUNAR_DEEP_COUNT_JOB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_DEEP_COUNT_JOB, LunarDeepCountJobClass))

GType               lunar_deep_count_job_get_type (void) G_GNUC_CONST;

LunarDeepCountJob *lunar_deep_count_job_new      (GList              *files,
                                                    GFileQueryInfoFlags flags) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS;

#endif /* !__LUNAR_DEEP_COUNT_JOB_H__ */
