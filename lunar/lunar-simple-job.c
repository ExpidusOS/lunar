/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif
#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif
#ifdef HAVE_STRING_H
#include <string.h>
#endif

#include <gobject/gvaluecollector.h>

#include <lunar/lunar-job.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-simple-job.h>



static void     lunar_simple_job_finalize   (GObject              *object);
static gboolean lunar_simple_job_execute    (EndoJob               *job,
                                              GError              **error);



struct _LunarSimpleJobClass
{
  LunarJobClass __parent__;
};

struct _LunarSimpleJob
{
  LunarJob __parent__;

  LunarSimpleJobFunc  func;
  GArray              *param_values;
};



G_DEFINE_TYPE (LunarSimpleJob, lunar_simple_job, LUNAR_TYPE_JOB)



static void
lunar_simple_job_class_init (LunarSimpleJobClass *klass)
{
  GObjectClass *gobject_class;
  EndoJobClass  *endojob_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_simple_job_finalize;

  endojob_class = ENDO_JOB_CLASS (klass);
  endojob_class->execute = lunar_simple_job_execute;
}



static void
lunar_simple_job_init (LunarSimpleJob *simple_job)
{
}



static void
lunar_simple_job_finalize (GObject *object)
{
  LunarSimpleJob *simple_job = LUNAR_SIMPLE_JOB (object);
  guint            i;

  /* release the param values */
  for (i = 0; i < simple_job->param_values->len; i++)
    g_value_unset (&g_array_index (simple_job->param_values, GValue, i));
  g_array_free (simple_job->param_values, TRUE);

  (*G_OBJECT_CLASS (lunar_simple_job_parent_class)->finalize) (object);
}



static gboolean
lunar_simple_job_execute (EndoJob  *job,
                           GError **error)
{
  LunarSimpleJob *simple_job = LUNAR_SIMPLE_JOB (job);
  gboolean         success = TRUE;
  GError          *err = NULL;

  _lunar_return_val_if_fail (LUNAR_IS_SIMPLE_JOB (job), FALSE);
  _lunar_return_val_if_fail (simple_job->func != NULL, FALSE);

  /* try to execute the job using the supplied function */
  success = (*simple_job->func) (LUNAR_JOB (job), simple_job->param_values, &err);

  if (!success)
    {
      g_assert (err != NULL || endo_job_is_cancelled (job));

      /* set error if the job was cancelled. otherwise just propagate
       * the results of the processing function */
      if (endo_job_set_error_if_cancelled (job, error))
        {
          g_clear_error (&err);
        }
      else
        {
          if (err != NULL)
            g_propagate_error (error, err);
        }

      return FALSE;
    }
  else
    return TRUE;
}



/**
 * lunar_simple_job_launch:
 * @func           : the #LunarSimpleJobFunc to execute the job.
 * @n_param_values : the number of parameters to pass to the @func.
 * @...            : a list of #GType and parameter pairs (exactly
 *                   @n_param_values pairs) that are passed to @func.
 *
 * Allocates a new #LunarSimpleJob, which executes the specified
 * @func with the specified parameters.
 *
 * For example the listdir @func expects a #LunarPath for the
 * folder to list, so the call to lunar_simple_job_launch()
 * would look like this:
 *
 * <informalexample><programlisting>
 * lunar_simple_job_launch (_lunar_io_jobs_listdir, 1,
 *                               LUNAR_TYPE_PATH, path);
 * </programlisting></informalexample>
 *
 * The caller is responsible to release the returned object using
 * lunar_job_unref() when no longer needed.
 *
 * Return value: the launched #LunarJob.
 **/
LunarJob *
lunar_simple_job_launch (LunarSimpleJobFunc func,
                          guint               n_param_values,
                          ...)
{
  LunarSimpleJob *simple_job;
  va_list          var_args;
  GValue           value = { 0, };
  gchar           *error_message;
  guint            n;

  /* allocate and initialize the simple job */
  simple_job = g_object_new (LUNAR_TYPE_SIMPLE_JOB, NULL);
  simple_job->func = func;
  simple_job->param_values = g_array_sized_new (FALSE, TRUE, sizeof (GValue), n_param_values);

  /* collect the parameters */
  va_start (var_args, n_param_values);
  for (n = 0; n < n_param_values; ++n)
    {
      /* initialize the value to hold the next parameter */
      g_value_init (&value, va_arg (var_args, GType));

      /* collect the value from the stack */
      G_VALUE_COLLECT (&value, var_args, 0, &error_message);

      /* check if an error occurred */
      if (G_UNLIKELY (error_message != NULL))
        {
          g_error ("%s: %s", G_STRLOC, error_message);
          g_free (error_message);
        }

      g_array_insert_val (simple_job->param_values, n, value);

      /* manually unset the value, g_value_unset doesn't work
       * because we don't want to free the data */
      memset (&value, 0, sizeof (GValue));
    }
  va_end (var_args);

  /* launch the job */
  return LUNAR_JOB (endo_job_launch (ENDO_JOB (simple_job)));
}



GArray *
lunar_simple_job_get_param_values (LunarSimpleJob *job)
{
  _lunar_return_val_if_fail (LUNAR_IS_SIMPLE_JOB (job), NULL);
  return job->param_values;
}
