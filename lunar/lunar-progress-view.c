/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2007 Benedikt Meurer <benny@expidus.org>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <endo/endo.h>

#include <lunar/lunar-dialogs.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-job.h>
#include <lunar/lunar-pango-extensions.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-util.h>
#include <lunar/lunar-transfer-job.h>
#include <lunar/lunar-progress-view.h>



enum
{
  PROP_0,
  PROP_JOB,
  PROP_ICON_NAME,
  PROP_TITLE,
};



static void              lunar_progress_view_finalize     (GObject            *object);
static void              lunar_progress_view_dispose      (GObject            *object);
static void              lunar_progress_view_get_property (GObject            *object,
                                                            guint               prop_id,
                                                            GValue             *value,
                                                            GParamSpec         *pspec);
static void              lunar_progress_view_set_property (GObject            *object,
                                                            guint               prop_id,
                                                            const GValue       *value,
                                                            GParamSpec         *pspec);
static void              lunar_progress_view_pause_job    (LunarProgressView *view);
static void              lunar_progress_view_unpause_job  (LunarProgressView *view);
static void              lunar_progress_view_cancel_job   (LunarProgressView *view);
static LunarJobResponse lunar_progress_view_ask          (LunarProgressView *view,
                                                            const gchar        *message,
                                                            LunarJobResponse   choices,
                                                            LunarJob          *job);
static LunarJobResponse lunar_progress_view_ask_replace  (LunarProgressView *view,
                                                            LunarFile         *src_file,
                                                            LunarFile         *dst_file,
                                                            LunarJob          *job);
static void              lunar_progress_view_error        (LunarProgressView *view,
                                                            GError             *error,
                                                            EndoJob             *job);
static void              lunar_progress_view_finished     (LunarProgressView *view,
                                                            EndoJob             *job);
static void              lunar_progress_view_info_message (LunarProgressView *view,
                                                            const gchar        *message,
                                                            EndoJob             *job);
static void              lunar_progress_view_percent      (LunarProgressView *view,
                                                            gdouble             percent,
                                                            EndoJob             *job);
static void              lunar_progress_view_frozen       (LunarProgressView *view,
                                                            EndoJob             *job);
static void              lunar_progress_view_unfrozen     (LunarProgressView *view,
                                                            EndoJob             *job);
static void              lunar_progress_view_set_job      (LunarProgressView *view,
                                                            LunarJob          *job);



struct _LunarProgressViewClass
{
  GtkVBoxClass __parent__;
};

struct _LunarProgressView
{
  GtkVBox  __parent__;

  LunarJob *job;

  GtkWidget *progress_bar;
  GtkWidget *progress_label;
  GtkWidget *message_label;
  GtkWidget *pause_button;
  GtkWidget *unpause_button;

  gchar     *icon_name;
  gchar     *title;
};



G_DEFINE_TYPE (LunarProgressView, lunar_progress_view, GTK_TYPE_BOX)



static void
lunar_progress_view_class_init (LunarProgressViewClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_progress_view_finalize;
  gobject_class->dispose = lunar_progress_view_dispose;
  gobject_class->get_property = lunar_progress_view_get_property;
  gobject_class->set_property = lunar_progress_view_set_property;

  /**
   * LunarProgressView:job:
   *
   * The #LunarJob, whose progress is displayed by this view, or
   * %NULL if no job is set.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_JOB,
                                   g_param_spec_object ("job", "job", "job",
                                                        LUNAR_TYPE_JOB,
                                                        ENDO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_ICON_NAME,
                                   g_param_spec_string ("icon-name",
                                                        "icon-name",
                                                        "icon-name",
                                                        NULL,
                                                        ENDO_PARAM_READWRITE));

  g_object_class_install_property (gobject_class,
                                   PROP_TITLE,
                                   g_param_spec_string ("title",
                                                        "title",
                                                        "title",
                                                        NULL,
                                                        ENDO_PARAM_READWRITE));

  g_signal_new ("need-attention",
                LUNAR_TYPE_PROGRESS_VIEW,
                G_SIGNAL_RUN_LAST | G_SIGNAL_NO_HOOKS,
                0,
                NULL,
                NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE,
                0);

  g_signal_new ("finished",
                LUNAR_TYPE_PROGRESS_VIEW,
                G_SIGNAL_RUN_LAST | G_SIGNAL_NO_HOOKS,
                0,
                NULL,
                NULL,
                g_cclosure_marshal_VOID__VOID,
                G_TYPE_NONE,
                0);
}



static void
lunar_progress_view_init (LunarProgressView *view)
{
  GtkWidget *image;
  GtkWidget *label;
  GtkWidget *cancel_button;
  GtkWidget *vbox;
  GtkWidget *vbox2;
  GtkWidget *vbox3;
  GtkWidget *hbox;

  gtk_orientable_set_orientation (GTK_ORIENTABLE (view), GTK_ORIENTATION_VERTICAL);

  vbox = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_container_add (GTK_CONTAINER (view), vbox);
  gtk_widget_show (vbox);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  image = g_object_new (GTK_TYPE_IMAGE, "icon-size", GTK_ICON_SIZE_DND, NULL);
  gtk_image_set_pixel_size (GTK_IMAGE(image), 32);
  gtk_box_pack_start (GTK_BOX (hbox), image, FALSE, TRUE, 0);
  endo_binding_new (G_OBJECT (view), "icon-name", G_OBJECT (image), "icon-name");
  gtk_widget_show (image);

  vbox2 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 6);
  gtk_box_pack_start (GTK_BOX (hbox), vbox2, TRUE, TRUE, 0);
  gtk_widget_show (vbox2);

  label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_attributes (GTK_LABEL (label), lunar_pango_attr_list_big_bold ());
  gtk_label_set_ellipsize (GTK_LABEL (label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_box_pack_start (GTK_BOX (vbox2), label, TRUE, TRUE, 0);
  gtk_widget_show (label);

  view->message_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_ellipsize (GTK_LABEL (view->message_label), PANGO_ELLIPSIZE_MIDDLE);
  gtk_box_pack_start (GTK_BOX (vbox2), view->message_label, TRUE, TRUE, 0);
  gtk_widget_show (view->message_label);

  view->pause_button = gtk_button_new_from_icon_name ("media-playback-pause-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_relief (GTK_BUTTON (view->pause_button), GTK_RELIEF_NONE);
  g_signal_connect_swapped (view->pause_button, "clicked", G_CALLBACK (lunar_progress_view_pause_job), view);
  gtk_box_pack_start (GTK_BOX (hbox), view->pause_button, FALSE, FALSE, 0);
  gtk_widget_set_can_focus (view->pause_button, FALSE);
  gtk_widget_hide (view->pause_button);

  view->unpause_button = gtk_button_new_from_icon_name ("media-playback-start-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_relief (GTK_BUTTON (view->unpause_button), GTK_RELIEF_NONE);
  g_signal_connect_swapped (view->unpause_button, "clicked", G_CALLBACK (lunar_progress_view_unpause_job), view);
  gtk_box_pack_start (GTK_BOX (hbox), view->unpause_button, FALSE, FALSE, 0);
  gtk_widget_set_can_focus (view->unpause_button, FALSE);
  gtk_widget_hide (view->unpause_button);

  cancel_button = gtk_button_new_from_icon_name ("media-playback-stop-symbolic", GTK_ICON_SIZE_BUTTON);
  gtk_button_set_relief (GTK_BUTTON (cancel_button), GTK_RELIEF_NONE);
  g_signal_connect_swapped (cancel_button, "clicked", G_CALLBACK (lunar_progress_view_cancel_job), view);
  gtk_box_pack_start (GTK_BOX (hbox), cancel_button, FALSE, FALSE, 0);
  gtk_widget_set_can_focus (cancel_button, FALSE);
  gtk_widget_show (cancel_button);

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
  gtk_box_pack_start (GTK_BOX (vbox), hbox, TRUE, TRUE, 0);
  gtk_widget_show (hbox);

  vbox3 = gtk_box_new (GTK_ORIENTATION_VERTICAL, 3);
  gtk_box_pack_start (GTK_BOX (hbox), vbox3, TRUE, TRUE, 0);
  gtk_widget_show (vbox3);

  view->progress_bar = gtk_progress_bar_new ();
  gtk_box_pack_start (GTK_BOX (vbox3), view->progress_bar, TRUE, TRUE, 0);
  gtk_widget_show (view->progress_bar);

  view->progress_label = g_object_new (GTK_TYPE_LABEL, "xalign", 0.0f, NULL);
  gtk_label_set_ellipsize (GTK_LABEL (view->progress_label), PANGO_ELLIPSIZE_END);
  gtk_label_set_attributes (GTK_LABEL (view->progress_label), lunar_pango_attr_list_small ());
  gtk_box_pack_start (GTK_BOX (vbox3), view->progress_label, FALSE, TRUE, 0);
  gtk_widget_show (view->progress_label);

  /* connect the view title to the action label */
  endo_binding_new (G_OBJECT (view), "title", G_OBJECT (label), "label");
}



static void
lunar_progress_view_finalize (GObject *object)
{
  LunarProgressView *view = LUNAR_PROGRESS_VIEW (object);

  g_free (view->icon_name);
  g_free (view->title);

  (*G_OBJECT_CLASS (lunar_progress_view_parent_class)->finalize) (object);
}



static void
lunar_progress_view_dispose (GObject *object)
{
  LunarProgressView *view = LUNAR_PROGRESS_VIEW (object);

  /* disconnect from the job (if any) */
  if (view->job != NULL)
    {
      endo_job_cancel (ENDO_JOB (view->job));
      lunar_progress_view_set_job (view, NULL);
    }

  (*G_OBJECT_CLASS (lunar_progress_view_parent_class)->dispose) (object);
}



static void
lunar_progress_view_get_property (GObject    *object,
                                   guint       prop_id,
                                   GValue     *value,
                                   GParamSpec *pspec)
{
  LunarProgressView *view = LUNAR_PROGRESS_VIEW (object);

  switch (prop_id)
    {
    case PROP_JOB:
      g_value_set_object (value, lunar_progress_view_get_job (view));
      break;

    case PROP_ICON_NAME:
      g_value_set_string (value, view->icon_name);
      break;

    case PROP_TITLE:
      g_value_set_string (value, view->title);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_progress_view_set_property (GObject      *object,
                                     guint         prop_id,
                                     const GValue *value,
                                     GParamSpec   *pspec)
{
  LunarProgressView *view = LUNAR_PROGRESS_VIEW (object);

  switch (prop_id)
    {
    case PROP_JOB:
      lunar_progress_view_set_job (view, g_value_get_object (value));
      break;

    case PROP_ICON_NAME:
      lunar_progress_view_set_icon_name (view, g_value_get_string (value));
      break;

    case PROP_TITLE:
      lunar_progress_view_set_title (view, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_progress_view_pause_job (LunarProgressView *view)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_JOB (view->job));

  if (view->job != NULL)
    {
      /* pause the job */
      lunar_job_pause (view->job);

      /* update the UI */
      gtk_widget_hide (view->pause_button);
      gtk_widget_show (view->unpause_button);
      gtk_label_set_text (GTK_LABEL (view->progress_label), _("Paused"));
    }
}



static void
lunar_progress_view_unpause_job (LunarProgressView *view)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_JOB (view->job));

  if (view->job != NULL)
    {
      if (lunar_job_is_paused (view->job))
        lunar_job_resume (view->job);
      if (lunar_job_is_frozen (view->job))
        lunar_job_unfreeze (view->job);
      /* update the UI */
      gtk_widget_hide (view->unpause_button);
      gtk_widget_show (view->pause_button);
      gtk_label_set_text (GTK_LABEL (view->progress_label), _("Resuming..."));
    }
}



static void
lunar_progress_view_cancel_job (LunarProgressView *view)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_JOB (view->job));

  if (view->job != NULL)
    {
      /* cancel the job */
      endo_job_cancel (ENDO_JOB (view->job));

      /* don't listen to frozen/unfrozen states updates any more */
      g_signal_handlers_disconnect_matched (view->job, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
                                            lunar_progress_view_frozen, NULL);
      g_signal_handlers_disconnect_matched (view->job, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
                                            lunar_progress_view_unfrozen, NULL);

      /* don't listen to percentage updates any more */
      g_signal_handlers_disconnect_matched (view->job, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
                                            lunar_progress_view_percent, NULL);

      /* don't listen to info messages any more */
      g_signal_handlers_disconnect_matched (view->job, G_SIGNAL_MATCH_FUNC, 0, 0, NULL,
                                            lunar_progress_view_info_message, NULL);

      /* update the status text */
      gtk_label_set_text (GTK_LABEL (view->progress_label), _("Cancelling..."));
    }
}



static LunarJobResponse
lunar_progress_view_ask (LunarProgressView *view,
                          const gchar        *message,
                          LunarJobResponse   choices,
                          LunarJob          *job)
{
  GtkWidget *window;

  _lunar_return_val_if_fail (LUNAR_IS_PROGRESS_VIEW (view), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (g_utf8_validate (message, -1, NULL), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (view->job == job, LUNAR_JOB_RESPONSE_CANCEL);

  /* be sure to display the corresponding dialog prior to opening the question view */
  g_signal_emit_by_name (view, "need-attention");

  /* determine the toplevel window of the view */
  window = gtk_widget_get_toplevel (GTK_WIDGET (view));

  /* display the question view */
  return lunar_dialogs_show_job_ask (window != NULL ? GTK_WINDOW (window) : NULL,
                                      message, choices);
}



static LunarJobResponse
lunar_progress_view_ask_replace (LunarProgressView *view,
                                  LunarFile         *src_file,
                                  LunarFile         *dst_file,
                                  LunarJob          *job)
{
  GtkWidget *window;

  _lunar_return_val_if_fail (LUNAR_IS_PROGRESS_VIEW (view), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (view->job == job, LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (src_file), LUNAR_JOB_RESPONSE_CANCEL);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (dst_file), LUNAR_JOB_RESPONSE_CANCEL);

  /* be sure to display the corresponding dialog prior to opening the question view */
  g_signal_emit_by_name (view, "need-attention");

  /* determine the toplevel window of the view */
  window = gtk_widget_get_toplevel (GTK_WIDGET (view));

  /* display the question view */
  return lunar_dialogs_show_job_ask_replace (window != NULL ? GTK_WINDOW (window) : NULL,
                                              src_file, dst_file);
}



static void
lunar_progress_view_error (LunarProgressView *view,
                            GError             *error,
                            EndoJob             *job)
{
  GtkWidget *window;

  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (error != NULL && error->message != NULL);
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (view->job == LUNAR_JOB (job));

  /* be sure to display the corresponding dialog prior to opening the question view */
  g_signal_emit_by_name (view, "need-attention");

  /* determine the toplevel window of the view */
  window = gtk_widget_get_toplevel (GTK_WIDGET (view));

  /* display the error message */
  lunar_dialogs_show_job_error (window != NULL ? GTK_WINDOW (window) : NULL, error);
}



static void
lunar_progress_view_finished (LunarProgressView *view,
                               EndoJob             *job)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (view->job == LUNAR_JOB (job));

  /* emit finished signal to notify others that the job is finished */
  g_signal_emit_by_name (view, "finished");
}



static void
lunar_progress_view_info_message (LunarProgressView *view,
                                   const gchar        *message,
                                   EndoJob             *job)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (g_utf8_validate (message, -1, NULL));
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (view->job == LUNAR_JOB (job));

  gtk_label_set_text (GTK_LABEL (view->message_label), message);
}



static void
lunar_progress_view_percent (LunarProgressView *view,
                              gdouble             percent,
                              EndoJob             *job)
{
  gchar *text;

  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (percent >= 0.0 && percent <= 100.0);
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (view->job == LUNAR_JOB (job));

  /* update progressbar */
  gtk_progress_bar_set_fraction (GTK_PROGRESS_BAR (view->progress_bar), percent / 100.0);

  /* set progress text */
  if (LUNAR_IS_TRANSFER_JOB (job))
    text = lunar_transfer_job_get_status (LUNAR_TRANSFER_JOB (job));
  else
    text = g_strdup_printf ("%.2f%%", percent);

  gtk_label_set_text (GTK_LABEL (view->progress_label), text);
  g_free (text);
}



static void
lunar_progress_view_frozen (LunarProgressView *view,
                             EndoJob             *job)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (view->job == LUNAR_JOB (job));

  if (LUNAR_IS_TRANSFER_JOB (job))
    {
      /* update the UI */
      gtk_widget_hide (view->pause_button);
      gtk_widget_show (view->unpause_button);
      gtk_label_set_text (GTK_LABEL (view->progress_label), _("Frozen by another job on same device"));
    }
}



static void
lunar_progress_view_unfrozen (LunarProgressView *view,
                               EndoJob             *job)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (view->job == LUNAR_JOB (job));

  if (LUNAR_IS_TRANSFER_JOB (job))
    {
      /* update the UI */
      gtk_widget_hide (view->unpause_button);
      gtk_widget_show (view->pause_button);
      gtk_label_set_text (GTK_LABEL (view->progress_label), _("Unfreezing..."));
    }
}



/**
 * lunar_progress_view_new_with_job:
 * @job : a #LunarJob or %NULL.
 *
 * Allocates a new #LunarProgressView and associates it with the @job.
 *
 * Return value: the newly allocated #LunarProgressView.
 **/
GtkWidget*
lunar_progress_view_new_with_job (LunarJob *job)
{
  _lunar_return_val_if_fail (job == NULL || LUNAR_IS_JOB (job), NULL);
  return g_object_new (LUNAR_TYPE_PROGRESS_VIEW, "job", job, NULL);
}



/**
 * lunar_progress_view_get_job:
 * @view : a #LunarProgressView.
 *
 * Returns the #LunarJob associated with @view
 * or %NULL if no job is currently associated with @view.
 *
 * The #LunarJob is owned by the @view and should
 * not be freed by the caller.
 *
 * Return value: the job associated with @view or %NULL.
 **/
LunarJob *
lunar_progress_view_get_job (LunarProgressView *view)
{
  _lunar_return_val_if_fail (LUNAR_IS_PROGRESS_VIEW (view), NULL);
  return view->job;
}



/**
 * lunar_progress_view_set_job:
 * @view : a #LunarProgressView.
 * @job    : a #LunarJob or %NULL.
 *
 * Associates @job with @view.
 **/
static void
lunar_progress_view_set_job (LunarProgressView *view,
                              LunarJob          *job)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));
  _lunar_return_if_fail (job == NULL || LUNAR_IS_JOB (job));

  /* check if we're already on that job */
  if (G_UNLIKELY (view->job == job))
    return;

  /* disconnect from the previous job */
  if (G_LIKELY (view->job != NULL))
    {
      g_signal_handlers_disconnect_matched (view->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, view);
      g_object_unref (G_OBJECT (view->job));
    }

  /* activate the new job */
  view->job = job;

  /* connect to the new job */
  if (G_LIKELY (job != NULL))
    {
      g_object_ref (job);

      g_signal_connect_swapped (job, "ask", G_CALLBACK (lunar_progress_view_ask), view);
      g_signal_connect_swapped (job, "ask-replace", G_CALLBACK (lunar_progress_view_ask_replace), view);
      g_signal_connect_swapped (job, "error", G_CALLBACK (lunar_progress_view_error), view);
      g_signal_connect_swapped (job, "finished", G_CALLBACK (lunar_progress_view_finished), view);
      g_signal_connect_swapped (job, "info-message", G_CALLBACK (lunar_progress_view_info_message), view);
      g_signal_connect_swapped (job, "percent", G_CALLBACK (lunar_progress_view_percent), view);
      g_signal_connect_swapped (job, "frozen", G_CALLBACK (lunar_progress_view_frozen), view);
      g_signal_connect_swapped (job, "unfrozen", G_CALLBACK (lunar_progress_view_unfrozen), view);
      if (lunar_job_is_pausable (job))
        {
          gtk_widget_show (view->pause_button);
        }
    }

  g_object_notify (G_OBJECT (view), "job");
}



void
lunar_progress_view_set_icon_name (LunarProgressView *view,
                                    const gchar        *icon_name)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));

  if (endo_str_is_equal (view->icon_name, icon_name))
    return;

  g_free (view->icon_name);
  view->icon_name = g_strdup (icon_name);

  g_object_notify (G_OBJECT (view), "icon-name");
}



void
lunar_progress_view_set_title (LunarProgressView *view,
                                const gchar        *title)
{
  _lunar_return_if_fail (LUNAR_IS_PROGRESS_VIEW (view));

  if (endo_str_is_equal (view->title, title))
    return;

  g_free (view->title);
  view->title = g_strdup (title);

  g_object_notify (G_OBJECT (view), "title");
}
