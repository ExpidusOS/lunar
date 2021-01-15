/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef __LUNAR_JOB_H__
#define __LUNAR_JOB_H__

#include <gio/gio.h>

#include <endo/endo.h>

#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-file.h>

G_BEGIN_DECLS

typedef struct _LunarJobPrivate LunarJobPrivate;
typedef struct _LunarJobClass   LunarJobClass;
typedef struct _LunarJob        LunarJob;

#define LUNAR_TYPE_JOB            (lunar_job_get_type ())
#define LUNAR_JOB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_JOB, LunarJob))
#define LUNAR_JOB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_JOB, LunarJobClass))
#define LUNAR_IS_JOB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_JOB))
#define LUNAR_IS_JOB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_JOB))
#define LUNAR_JOB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_JOB, LunarJobClass))

struct _LunarJobClass
{
  /*< private >*/
  EndoJobClass __parent__;

  /*< public >*/

  /* signals */
  LunarJobResponse (*ask)         (LunarJob        *job,
                                    const gchar      *message,
                                    LunarJobResponse choices);
  LunarJobResponse (*ask_replace) (LunarJob        *job,
                                    LunarFile       *source_file,
                                    LunarFile       *target_file);
};

struct _LunarJob
{
  /*< private >*/
  EndoJob            __parent__;
  LunarJobPrivate *priv;
};

GType             lunar_job_get_type               (void) G_GNUC_CONST;
void              lunar_job_set_total_files        (LunarJob       *job,
                                                     GList           *total_files);
void              lunar_job_set_pausable           (LunarJob       *job,
                                                     gboolean         pausable);
gboolean          lunar_job_is_pausable            (LunarJob       *job);
void              lunar_job_pause                  (LunarJob       *job);
void              lunar_job_resume                 (LunarJob       *job);
void              lunar_job_freeze                 (LunarJob       *job);
void              lunar_job_unfreeze               (LunarJob       *job);
gboolean          lunar_job_is_paused              (LunarJob       *job);
gboolean          lunar_job_is_frozen              (LunarJob       *job);
void              lunar_job_processing_file        (LunarJob       *job,
                                                     GList           *current_file,
                                                     guint            n_processed);

LunarJobResponse lunar_job_ask_create             (LunarJob       *job,
                                                     const gchar     *format,
                                                     ...);
LunarJobResponse lunar_job_ask_overwrite          (LunarJob       *job,
                                                     const gchar     *format,
                                                     ...);
LunarJobResponse lunar_job_ask_delete             (LunarJob       *job,
                                                     const gchar     *format,
                                                     ...);
LunarJobResponse lunar_job_ask_replace            (LunarJob       *job,
                                                     GFile           *source_path,
                                                     GFile           *target_path,
                                                     GError         **error);
LunarJobResponse lunar_job_ask_skip               (LunarJob       *job,
                                                     const gchar     *format,
                                                     ...);
gboolean          lunar_job_ask_no_size            (LunarJob       *job,
                                                     const gchar     *format,
                                                     ...);
gboolean          lunar_job_files_ready            (LunarJob       *job,
                                                     GList           *file_list);
void              lunar_job_new_files              (LunarJob       *job,
                                                     const GList     *file_list);
GList *           lunar_job_ask_jobs               (LunarJob       *job);

G_END_DECLS

#endif /* !__LUNAR_JOB_H__ */
