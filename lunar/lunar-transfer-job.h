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

#ifndef __LUNAR_TRANSFER_JOB_H__
#define __LUNAR_TRANSFER_JOB_H__

#include <glib-object.h>

G_BEGIN_DECLS

/**
 * LunarTransferJobFlags:
 *
 * Flags to control the behavior of the transfer job.
 **/
typedef enum /*< enum >*/
{
  LUNAR_TRANSFER_JOB_COPY,
  LUNAR_TRANSFER_JOB_LINK,
  LUNAR_TRANSFER_JOB_MOVE,
  LUNAR_TRANSFER_JOB_TRASH,
} LunarTransferJobType;

typedef struct _LunarTransferJobPrivate LunarTransferJobPrivate;
typedef struct _LunarTransferJobClass   LunarTransferJobClass;
typedef struct _LunarTransferJob        LunarTransferJob;

#define LUNAR_TYPE_TRANSFER_JOB            (lunar_transfer_job_get_type ())
#define LUNAR_TRANSFER_JOB(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_TRANSFER_JOB, LunarTransferJob))
#define LUNAR_TRANSFER_JOB_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_TRANSFER_JOB, LunarTransferJobClass))
#define LUNAR_IS_TRANSFER_JOB(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_TRANSFER_JOB))
#define LUNAR_IS_TRANSFER_JOB_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_TRANSFER_JOB)
#define LUNAR_TRANSFER_JOB_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_TRANSFER_JOB, LunarTransferJobClass))

GType      lunar_transfer_job_get_type (void) G_GNUC_CONST;

LunarJob *lunar_transfer_job_new        (GList                *source_file_list,
                                           GList                *target_file_list,
                                           LunarTransferJobType type) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

gchar     *lunar_transfer_job_get_status (LunarTransferJob    *job);

G_END_DECLS

#endif /* !__LUNAR_TRANSFER_JOB_H__ */
