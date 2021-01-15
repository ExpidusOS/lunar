/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@expidus.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <endo/endo.h>

#include <lunar/lunar-enum-types.h>
#include <lunar/lunar-job.h>
#include <lunar/lunar-marshal.h>
#include <lunar/lunar-private.h>



/* Signal identifiers */
enum
{
  ASK,
  ASK_REPLACE,
  ASK_JOBS,
  FILES_READY,
  NEW_FILES,
  FROZEN,
  UNFROZEN,
  LAST_SIGNAL,
};



static void              lunar_job_finalize            (GObject            *object);
static LunarJobResponse lunar_job_real_ask            (LunarJob          *job,
                                                         const gchar        *message,
                                                         LunarJobResponse   choices);
static LunarJobResponse lunar_job_real_ask_replace    (LunarJob          *job,
                                                         LunarFile         *source_file,
                                                         LunarFile         *target_file);



struct _LunarJobPrivate
{
  LunarJobResponse earlier_ask_create_response;
  LunarJobResponse earlier_ask_overwrite_response;
  LunarJobResponse earlier_ask_delete_response;
  LunarJobResponse earlier_ask_skip_response;
  GList            *total_files;
  guint             n_total_files;
  gboolean          pausable;
  gboolean          paused; /* the job has been manually paused using the UI */
  gboolean          frozen; /* the job has been automaticaly paused regarding some parallel copy behavior */
};



static guint job_signals[LAST_SIGNAL];



G_DEFINE_ABSTRACT_TYPE_WITH_PRIVATE (LunarJob, lunar_job, ENDO_TYPE_JOB)



static gboolean
_lunar_job_ask_accumulator (GSignalInvocationHint *ihint,
                             GValue                *return_accu,
                             const GValue          *handler_return,
                             gpointer               data)
{
  g_value_copy (handler_return, return_accu);
  return FALSE;
}



static void
lunar_job_class_init (LunarJobClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_job_finalize;

  klass->ask = lunar_job_real_ask;
  klass->ask_replace = lunar_job_real_ask_replace;

  /**
   * LunarJob::ask:
   * @job     : a #LunarJob.
   * @message : question to display to the user.
   * @choices : a combination of #LunarJobResponse<!---->s.
   *
   * The @message is garantied to contain valid UTF-8.
   *
   * Return value: the selected choice.
   **/
  job_signals[ASK] =
    g_signal_new (I_("ask"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS | G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (LunarJobClass, ask),
                  _lunar_job_ask_accumulator, NULL,
                  _lunar_marshal_FLAGS__STRING_FLAGS,
                  LUNAR_TYPE_JOB_RESPONSE,
                  2, G_TYPE_STRING,
                  LUNAR_TYPE_JOB_RESPONSE);

  /**
   * LunarJob::ask-replace:
   * @job      : a #LunarJob.
   * @src_file : the #LunarFile of the source file.
   * @dst_file : the #LunarFile of the destination file, that
   *             may be replaced with the source file.
   *
   * Emitted to ask the user whether the destination file should
   * be replaced by the source file.
   *
   * Return value: the selected choice.
   **/
  job_signals[ASK_REPLACE] =
    g_signal_new (I_("ask-replace"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS | G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (LunarJobClass, ask_replace),
                  _lunar_job_ask_accumulator, NULL,
                  _lunar_marshal_FLAGS__OBJECT_OBJECT,
                  LUNAR_TYPE_JOB_RESPONSE,
                  2, LUNAR_TYPE_FILE, LUNAR_TYPE_FILE);

  /**
   * LunarJob::files-ready:
   * @job       : a #LunarJob.
   * @file_list : a list of #LunarFile<!---->s.
   *
   * This signal is used by #LunarJob<!---->s returned by
   * the lunar_io_jobs_list_directory() function whenever
   * there's a bunch of #LunarFile<!---->s ready. This signal
   * is garantied to be never emitted with an @file_list
   * parameter of %NULL.
   *
   * To allow some further optimizations on the handler-side,
   * the handler is allowed to take over ownership of the
   * @file_list, i.e. it can reuse the @infos list and just replace
   * the data elements with it's own objects based on the
   * #LunarFile<!---->s contained within the @file_list (and
   * of course properly unreffing the previously contained infos).
   * If a handler takes over ownership of @file_list it must return
   * %TRUE here, and no further handlers will be run. Else, if
   * the handler doesn't want to take over ownership of @infos,
   * it must return %FALSE, and other handlers will be run. Use
   * this feature with care, and only if you can be sure that
   * you are the only handler connected to this signal for a
   * given job!
   *
   * Return value: %TRUE if the handler took over ownership of
   *               @file_list, else %FALSE.
   **/
  job_signals[FILES_READY] =
    g_signal_new (I_("files-ready"),
                  G_TYPE_FROM_CLASS (klass), G_SIGNAL_NO_HOOKS,
                  0, g_signal_accumulator_true_handled, NULL,
                  _lunar_marshal_BOOLEAN__POINTER,
                  G_TYPE_BOOLEAN, 1, G_TYPE_POINTER);

  /**
   * LunarJob::new-files:
   * @job       : a #LunarJob.
   * @file_list : a list of #GFile<!---->s that were created by @job.
   *
   * This signal is emitted by the @job right before the @job is terminated
   * and informs the application about the list of created files in @file_list.
   * @file_list contains only the toplevel file items, that were specified by
   * the application on creation of the @job.
   **/
  job_signals[NEW_FILES] =
    g_signal_new (I_("new-files"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS, 0, NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * LunarJob::ask-jobs:
   * @job      : a #LunarJob.
   *
   * Emitted to ask the running job list.
   *
   * Return value: GList* of running jobs.
   **/
  job_signals[ASK_JOBS] =
    g_signal_new (I_("ask-jobs"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS, 0,
                  NULL, NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_POINTER, 0);

  /**
   * LunarJob::frozen:
   * @job       : a #LunarJob.
   *
   * This signal is emitted by the @job right after the @job is being frozen.
   **/
  job_signals[FROZEN] =
    g_signal_new (I_("frozen"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS, 0,
                  NULL, NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE, 0);

  /**
   * LunarJob::unfrozen:
   * @job       : a #LunarJob.
   *
   * This signal is emitted by the @job right after the @job is being unfrozen.
   **/
  job_signals[UNFROZEN] =
    g_signal_new (I_("unfrozen"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_NO_HOOKS, 0,
                  NULL, NULL,
                  g_cclosure_marshal_generic,
                  G_TYPE_NONE, 0);
}



static void
lunar_job_init (LunarJob *job)
{
  job->priv = lunar_job_get_instance_private (job);
  job->priv->earlier_ask_create_response = 0;
  job->priv->earlier_ask_overwrite_response = 0;
  job->priv->earlier_ask_delete_response = 0;
  job->priv->earlier_ask_skip_response = 0;
  job->priv->n_total_files = 0;
  job->priv->pausable = FALSE;
  job->priv->paused = FALSE;
  job->priv->frozen = FALSE;
}



static void
lunar_job_finalize (GObject *object)
{
  (*G_OBJECT_CLASS (lunar_job_parent_class)->finalize) (object);
}



static LunarJobResponse
lunar_job_real_ask (LunarJob        *job,
                     const gchar      *message,
                     LunarJobResponse choices)
{
  LunarJobResponse response;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  g_signal_emit (job, job_signals[ASK], 0, message, choices, &response);
  return response;
}



static LunarJobResponse
lunar_job_real_ask_replace (LunarJob  *job,
                             LunarFile *source_file,
                             LunarFile *target_file)
{
  LunarJobResponse response;
  gchar            *message;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (source_file), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (target_file), LUNAR_JOB_RESPONSE_CANCEL);

  message = g_strdup_printf (_("The file \"%s\" already exists. Would you like to replace it?\n\n"
                               "If you replace an existing file, its contents will be overwritten."),
                             lunar_file_get_display_name (source_file));

  g_signal_emit (job, job_signals[ASK], 0, message,
                 LUNAR_JOB_RESPONSE_REPLACE
                 | LUNAR_JOB_RESPONSE_REPLACE_ALL
                 | LUNAR_JOB_RESPONSE_RENAME
                 | LUNAR_JOB_RESPONSE_RENAME_ALL
                 | LUNAR_JOB_RESPONSE_SKIP
                 | LUNAR_JOB_RESPONSE_SKIP_ALL
                 | LUNAR_JOB_RESPONSE_CANCEL,
                 &response);

  /* clean up */
  g_free (message);

  return response;
}



static LunarJobResponse
_lunar_job_ask_valist (LunarJob        *job,
                        const gchar      *format,
                        va_list           var_args,
                        const gchar      *question,
                        LunarJobResponse choices)
{
  LunarJobResponse response;
  gchar            *text;
  gchar            *message;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (g_utf8_validate (format, -1, NULL), LUNAR_JOB_RESPONSE_CANCEL);

  /* generate the dialog message */
  text = g_strdup_vprintf (format, var_args);
  message = (question != NULL)
            ? g_strconcat (text, ".\n\n", question, NULL)
            : g_strconcat (text, ".", NULL);
  g_free (text);

  /* send the question and wait for the answer */
  endo_job_emit (ENDO_JOB (job), job_signals[ASK], 0, message, choices, &response);
  g_free (message);

  /* cancel the job as per users request */
  if (G_UNLIKELY (response == LUNAR_JOB_RESPONSE_CANCEL))
    endo_job_cancel (ENDO_JOB (job));

  return response;
}



LunarJobResponse
lunar_job_ask_overwrite (LunarJob   *job,
                          const gchar *format,
                          ...)
{
  LunarJobResponse response;
  va_list           var_args;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (format != NULL, LUNAR_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (endo_job_is_cancelled (ENDO_JOB (job))))
    return LUNAR_JOB_RESPONSE_CANCEL;

  /* check if the user said "Overwrite All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == LUNAR_JOB_RESPONSE_REPLACE_ALL))
    return LUNAR_JOB_RESPONSE_REPLACE;

  /* check if the user said "Overwrite None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == LUNAR_JOB_RESPONSE_SKIP_ALL))
    return LUNAR_JOB_RESPONSE_SKIP;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _lunar_job_ask_valist (job, format, var_args,
                                     _("Do you want to overwrite it?"),
                                     LUNAR_JOB_RESPONSE_REPLACE
                                     | LUNAR_JOB_RESPONSE_REPLACE_ALL
                                     | LUNAR_JOB_RESPONSE_SKIP
                                     | LUNAR_JOB_RESPONSE_SKIP_ALL
                                     | LUNAR_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  /* remember response for later */
  job->priv->earlier_ask_overwrite_response = response;

  /* translate response */
  switch (response)
    {
    case LUNAR_JOB_RESPONSE_REPLACE_ALL:
      response = LUNAR_JOB_RESPONSE_REPLACE;
      break;

    case LUNAR_JOB_RESPONSE_SKIP_ALL:
      response = LUNAR_JOB_RESPONSE_SKIP;
      break;

    default:
      break;
    }

  return response;
}



LunarJobResponse
lunar_job_ask_delete (LunarJob   *job,
                       const gchar *format,
                       ...)
{
  LunarJobResponse response;
  va_list           var_args;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (format != NULL, LUNAR_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (endo_job_is_cancelled (ENDO_JOB (job))))
    return LUNAR_JOB_RESPONSE_CANCEL;

  /* check if the user said "Delete All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_delete_response == LUNAR_JOB_RESPONSE_YES_ALL))
    return LUNAR_JOB_RESPONSE_YES;

  /* check if the user said "Delete None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_delete_response == LUNAR_JOB_RESPONSE_NO_ALL))
    return LUNAR_JOB_RESPONSE_NO;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _lunar_job_ask_valist (job, format, var_args,
                                     _("Do you want to permanently delete it?"),
                                     LUNAR_JOB_RESPONSE_YES
                                     | LUNAR_JOB_RESPONSE_YES_ALL
                                     | LUNAR_JOB_RESPONSE_NO
                                     | LUNAR_JOB_RESPONSE_NO_ALL
                                     | LUNAR_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  /* remember response for later */
  job->priv->earlier_ask_delete_response = response;

  /* translate response */
  switch (response)
    {
    case LUNAR_JOB_RESPONSE_YES_ALL:
      response = LUNAR_JOB_RESPONSE_YES;
      break;

    case LUNAR_JOB_RESPONSE_NO_ALL:
      response = LUNAR_JOB_RESPONSE_NO;
      break;

    default:
      break;
    }

  return response;
}



LunarJobResponse
lunar_job_ask_create (LunarJob   *job,
                       const gchar *format,
                       ...)
{
  LunarJobResponse response;
  va_list           var_args;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);

  if (G_UNLIKELY (endo_job_is_cancelled (ENDO_JOB (job))))
    return LUNAR_JOB_RESPONSE_CANCEL;

  /* check if the user said "Create All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_create_response == LUNAR_JOB_RESPONSE_YES_ALL))
    return LUNAR_JOB_RESPONSE_YES;

  /* check if the user said "Create None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_create_response == LUNAR_JOB_RESPONSE_NO_ALL))
    return LUNAR_JOB_RESPONSE_NO;

  va_start (var_args, format);
  response = _lunar_job_ask_valist (job, format, var_args,
                                     _("Do you want to create it?"),
                                     LUNAR_JOB_RESPONSE_YES
                                     | LUNAR_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  job->priv->earlier_ask_create_response = response;

  /* translate the response */
  if (response == LUNAR_JOB_RESPONSE_YES_ALL)
    response = LUNAR_JOB_RESPONSE_YES;
  else if (response == LUNAR_JOB_RESPONSE_NO_ALL)
    response = LUNAR_JOB_RESPONSE_NO;
  else if (response == LUNAR_JOB_RESPONSE_CANCEL)
    endo_job_cancel (ENDO_JOB (job));

  return response;
}



LunarJobResponse
lunar_job_ask_replace (LunarJob *job,
                        GFile     *source_path,
                        GFile     *target_path,
                        GError   **error)
{
  LunarJobResponse response;
  LunarFile       *source_file;
  LunarFile       *target_file;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (G_IS_FILE (source_path), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (G_IS_FILE (target_path), LUNAR_JOB_RESPONSE_CANCEL);

  if (G_UNLIKELY (endo_job_set_error_if_cancelled (ENDO_JOB (job), error)))
    return LUNAR_JOB_RESPONSE_CANCEL;

  /* check if the user said "Overwrite All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == LUNAR_JOB_RESPONSE_REPLACE_ALL))
    return LUNAR_JOB_RESPONSE_REPLACE;

  /* check if the user said "Rename All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == LUNAR_JOB_RESPONSE_RENAME_ALL))
    return LUNAR_JOB_RESPONSE_RENAME;

  /* check if the user said "Overwrite None" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_overwrite_response == LUNAR_JOB_RESPONSE_SKIP_ALL))
    return LUNAR_JOB_RESPONSE_SKIP;

  source_file = lunar_file_get (source_path, error);

  if (G_UNLIKELY (source_file == NULL))
    return LUNAR_JOB_RESPONSE_SKIP;

  target_file = lunar_file_get (target_path, error);

  if (G_UNLIKELY (target_file == NULL))
    {
      g_object_unref (source_file);
      return LUNAR_JOB_RESPONSE_SKIP;
    }

  endo_job_emit (ENDO_JOB (job), job_signals[ASK_REPLACE], 0,
                source_file, target_file, &response);

  g_object_unref (source_file);
  g_object_unref (target_file);

  /* remember the response for later */
  job->priv->earlier_ask_overwrite_response = response;

  /* translate the response */
  if (response == LUNAR_JOB_RESPONSE_REPLACE_ALL)
    response = LUNAR_JOB_RESPONSE_REPLACE;
  else if (response == LUNAR_JOB_RESPONSE_RENAME_ALL)
    response = LUNAR_JOB_RESPONSE_RENAME;
  else if (response == LUNAR_JOB_RESPONSE_SKIP_ALL)
    response = LUNAR_JOB_RESPONSE_SKIP;
  else if (response == LUNAR_JOB_RESPONSE_CANCEL)
    endo_job_cancel (ENDO_JOB (job));

  return response;
}



LunarJobResponse
lunar_job_ask_skip (LunarJob   *job,
                     const gchar *format,
                     ...)
{
  LunarJobResponse response;
  va_list           var_args;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (format != NULL, LUNAR_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (endo_job_is_cancelled (ENDO_JOB (job))))
    return LUNAR_JOB_RESPONSE_CANCEL;

  /* check if the user said "Skip All" earlier */
  if (G_UNLIKELY (job->priv->earlier_ask_skip_response == LUNAR_JOB_RESPONSE_SKIP_ALL))
    return LUNAR_JOB_RESPONSE_SKIP;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _lunar_job_ask_valist (job, format, var_args,
                                     _("Do you want to skip it?"),
                                     LUNAR_JOB_RESPONSE_SKIP
                                     | LUNAR_JOB_RESPONSE_SKIP_ALL
                                     | LUNAR_JOB_RESPONSE_CANCEL
                                     | LUNAR_JOB_RESPONSE_RETRY);
  va_end (var_args);

  /* remember the response */
  job->priv->earlier_ask_skip_response = response;

  /* translate the response */
  switch (response)
    {
    case LUNAR_JOB_RESPONSE_SKIP_ALL:
      response = LUNAR_JOB_RESPONSE_SKIP;
      break;

    case LUNAR_JOB_RESPONSE_SKIP:
    case LUNAR_JOB_RESPONSE_CANCEL:
    case LUNAR_JOB_RESPONSE_RETRY:
      break;

    default:
      _lunar_assert_not_reached ();
    }

  return response;
}



gboolean
lunar_job_ask_no_size (LunarJob   *job,
                        const gchar *format,
                        ...)
{
  LunarJobResponse response;
  va_list           var_args;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (format != NULL, LUNAR_JOB_RESPONSE_CANCEL);

  /* check if the user already cancelled the job */
  if (G_UNLIKELY (endo_job_is_cancelled (ENDO_JOB (job))))
    return LUNAR_JOB_RESPONSE_CANCEL;

  /* ask the user what he wants to do */
  va_start (var_args, format);
  response = _lunar_job_ask_valist (job, format, var_args,
                                     _("There is not enough space on the destination. Try to remove files to make space."),
                                     LUNAR_JOB_RESPONSE_FORCE
                                     | LUNAR_JOB_RESPONSE_CANCEL);
  va_end (var_args);

  return (response == LUNAR_JOB_RESPONSE_FORCE);
}



gboolean
lunar_job_files_ready (LunarJob *job,
                        GList     *file_list)
{
  gboolean handled = FALSE;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), FALSE);

  endo_job_emit (ENDO_JOB (job), job_signals[FILES_READY], 0, file_list, &handled);
  return handled;
}



void
lunar_job_new_files (LunarJob   *job,
                      const GList *file_list)
{
  LunarFile  *file;
  const GList *lp;

  _lunar_return_if_fail (LUNAR_IS_JOB (job));

  /* check if we have any files */
  if (G_LIKELY (file_list != NULL))
    {
      /* schedule a reload of cached files when idle */
      for (lp = file_list; lp != NULL; lp = lp->next)
        {
          file = lunar_file_cache_lookup (lp->data);
          if (file != NULL)
            {
              lunar_file_reload_idle_unref (file);
            }
        }

      /* emit the "new-files" signal */
      endo_job_emit (ENDO_JOB (job), job_signals[NEW_FILES], 0, file_list);
    }
}



GList *
lunar_job_ask_jobs (LunarJob *job)
{
  GList* jobs = NULL;

  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), NULL);

  endo_job_emit (ENDO_JOB (job), job_signals[ASK_JOBS], 0, &jobs);
  return jobs;
}



void
lunar_job_set_total_files (LunarJob *job,
                            GList     *total_files)
{
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (job->priv->total_files == NULL);
  _lunar_return_if_fail (total_files != NULL);

  job->priv->total_files = total_files;
  job->priv->n_total_files = g_list_length (total_files);
}



void
lunar_job_set_pausable (LunarJob *job,
                         gboolean   pausable)
{
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  job->priv->pausable = pausable;
}



gboolean
lunar_job_is_pausable (LunarJob *job)
{
  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), FALSE);
  return job->priv->pausable;
}



void
lunar_job_pause (LunarJob *job)
{
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  job->priv->paused = TRUE;
}



void
lunar_job_resume (LunarJob *job)
{
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  job->priv->paused = FALSE;
}



void
lunar_job_freeze (LunarJob *job)
{
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  job->priv->frozen = TRUE;
  endo_job_emit (ENDO_JOB (job), job_signals[FROZEN], 0);
}



void
lunar_job_unfreeze (LunarJob *job)
{
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  job->priv->frozen = FALSE;
  endo_job_emit (ENDO_JOB (job), job_signals[UNFROZEN], 0);
}



gboolean
lunar_job_is_paused (LunarJob *job)
{
  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), FALSE);
  return job->priv->paused;
}



gboolean
lunar_job_is_frozen (LunarJob *job)
{
  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), FALSE);
  return job->priv->frozen;
}



void
lunar_job_processing_file (LunarJob *job,
                            GList     *current_file,
                            guint      n_processed)
{
  gchar *base_name;
  gchar *display_name;

  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (current_file != NULL);

  /* emit only if n_processed is a multiple of 8 */
  if ((n_processed % 8) != 0)
    return;

  base_name = g_file_get_basename (current_file->data);
  display_name = g_filename_display_name (base_name);
  g_free (base_name);

  endo_job_info_message (ENDO_JOB (job), "%s", display_name);
  g_free (display_name);

  /* verify that we have total files set */
  if (G_LIKELY (job->priv->n_total_files > 0))
    endo_job_percent (ENDO_JOB (job), (n_processed * 100.0) / job->priv->n_total_files);
}
