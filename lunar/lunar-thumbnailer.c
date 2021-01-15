/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2009-2011 Jannis Pohlmann <jannis@expidus.org>
 * Copyright (c) 2012      Nick Schermer <nick@expidus.org>
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

#include <lunar/lunar-thumbnailer-proxy.h>
#include <lunar/lunar-marshal.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-thumbnailer.h>



/**
 * WARNING: The source code in this file may do harm to animals. Dead kittens
 * are to be expected.
 *
 *
 * LunarThumbnailer is a class for requesting thumbnails from com.expidus.tumbler.*
 * D-Bus services. This header contains an in-depth description of its design to make
 * it easier to understand why things are the way they are.
 *
 * Please note that all D-Bus calls are performed asynchronously.
 *
 * The proxy and the supported file types are intialized asynchronously, but
 * this is nearly completely transparent to the calling code because all requests
 * that arrive beforehand will be cached and sent out as soon as the initialization
 * is completed.
 *
 * When a request call is sent out, an internal request ID is created and tied
 * to the dbus call. While the call is running, an additional reference is held
 * to the thumbnailer so it cannot be finalized.
 *
 * The D-Bus reply handler then checks if there was an delivery error or
 * not. If the request method was sent successfully, the handle returned by the
 * D-Bus thumbnailer is associated bidirectionally with the internal request ID via
 * the request and handle values in the job structure.
 *
 *
 * Ready / Error
 * =============
 *
 * The Ready and Error signal handlers work exactly like Started except that
 * the Ready idle function sets the thumb state of the corresponding
 * LunarFile objects to _READY and the Error signal sets the state to _NONE.
 *
 *
 * Finished
 * ========
 *
 * The Finished signal handler looks up the internal request ID based on
 * the D-Bus thumbnailer handle. It then drops all corresponding information
 * from handle_request_mapping and request_handle_mapping.
 */



typedef enum
{
  LUNAR_THUMBNAILER_IDLE_ERROR,
  LUNAR_THUMBNAILER_IDLE_READY,
} LunarThumbnailerIdleType;



typedef enum
{
  LUNAR_THUMBNAILER_PROXY_WAITING = 0,
  LUNAR_THUMBNAILER_PROXY_AVAILABLE,
  LUNAR_THUMBNAILER_PROXY_FAILED
} LunarThumbnailerProxyState;



typedef struct _LunarThumbnailerJob  LunarThumbnailerJob;
typedef struct _LunarThumbnailerIdle LunarThumbnailerIdle;

/* Signal identifiers */
enum
{
  REQUEST_FINISHED,
  LAST_SIGNAL,
};

/* Property identifiers */
enum
{
  PROP_0,
  PROP_THUMBNAIL_SIZE,
};


static void                   lunar_thumbnailer_finalize               (GObject                    *object);
static void                   lunar_thumbnailer_init_thumbnailer_proxy (LunarThumbnailer          *thumbnailer);
static gboolean               lunar_thumbnailer_file_is_supported      (LunarThumbnailer          *thumbnailer,
                                                                         LunarFile                 *file);
static void                   lunar_thumbnailer_thumbnailer_finished   (GDBusProxy                 *proxy,
                                                                         guint                       handle,
                                                                         LunarThumbnailer          *thumbnailer);
static void                   lunar_thumbnailer_thumbnailer_error      (GDBusProxy                 *proxy,
                                                                         guint                       handle,
                                                                         const gchar               **uris,
                                                                         gint                        code,
                                                                         const gchar                *message,
                                                                         LunarThumbnailer          *thumbnailer);
static void                   lunar_thumbnailer_thumbnailer_ready      (GDBusProxy                 *proxy,
                                                                         guint32                     handle,
                                                                         const gchar               **uris,
                                                                         LunarThumbnailer          *thumbnailer);
static void                   lunar_thumbnailer_idle                   (LunarThumbnailer          *thumbnailer,
                                                                         guint                       handle,
                                                                         LunarThumbnailerIdleType   type,
                                                                         const gchar               **uris);
static gboolean               lunar_thumbnailer_idle_func              (gpointer                    user_data);
static void                   lunar_thumbnailer_idle_free              (gpointer                    data);
static void                   lunar_thumbnailer_get_property           (GObject                    *object,
                                                                         guint                       prop_id,
                                                                         GValue                     *value,
                                                                         GParamSpec                 *pspec);
static void                   lunar_thumbnailer_set_property           (GObject                    *object,
                                                                         guint                       prop_id,
                                                                         const GValue               *value,
                                                                         GParamSpec                 *pspec);

#define _thumbnailer_lock(thumbnailer)    g_mutex_lock (&((thumbnailer)->lock))
#define _thumbnailer_unlock(thumbnailer)  g_mutex_unlock (&((thumbnailer)->lock))
#define _thumbnailer_trylock(thumbnailer) g_mutex_trylock (&((thumbnailer)->lock))



struct _LunarThumbnailerClass
{
  GObjectClass __parent__;
};

struct _LunarThumbnailer
{
  GObject __parent__;

  /* proxies to communicate with D-Bus services */
  LunarThumbnailerDBus      *thumbnailer_proxy;
  LunarThumbnailerProxyState proxy_state;

  /* running jobs */
  GSList     *jobs;

  GMutex      lock;

  /* cached MIME types -> URI schemes for which thumbs can be generated */
  GHashTable *supported;

  /* last LunarThumbnailer request ID */
  guint       last_request;

  /* size to use to store thumbnails */
  LunarThumbnailSize thumbnail_size;

  /* IDs of idle functions */
  GSList     *idles;
};

struct _LunarThumbnailerJob
{
  LunarThumbnailer *thumbnailer;

  /* if this job is cancelled */
  guint              cancelled : 1;

  guint              lazy_checks : 1;

  /* data is saved here in case the queueing is delayed */
  /* If this is NULL, the request has been sent off. */
  GList             *files; /* element type: LunarFile */

  /* request number returned by LunarThumbnailer */
  guint              request;

  /* handle returned by the tumbler dbus service */
  guint              handle;
};

struct _LunarThumbnailerIdle
{
  LunarThumbnailerIdleType  type;
  LunarThumbnailer          *thumbnailer;
  guint                       id;
  gchar                     **uris;
};


static guint thumbnailer_signals[LAST_SIGNAL];



G_DEFINE_TYPE (LunarThumbnailer, lunar_thumbnailer, G_TYPE_OBJECT);



static void
lunar_thumbnailer_class_init (LunarThumbnailerClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->finalize = lunar_thumbnailer_finalize;
  gobject_class->get_property = lunar_thumbnailer_get_property;
  gobject_class->set_property = lunar_thumbnailer_set_property;

  /**
   * LunarThumbnailer:request-finished:
   * @thumbnailer : a #LunarThumbnailer.
   * @request     : id of the request that is finished.
   *
   * Emitted by @thumbnailer, when a request is finished
   * by the thumbnail generator
   **/
  thumbnailer_signals[REQUEST_FINISHED] =
    g_signal_new (I_("request-finished"),
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  0, NULL, NULL,
                  g_cclosure_marshal_VOID__UINT,
                  G_TYPE_NONE, 1, G_TYPE_UINT);

  /**
   * LunarIconFactory:thumbnail-size:
   *
   * Size of the thumbnails to load
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_THUMBNAIL_SIZE,
                                   g_param_spec_enum ("thumbnail-size",
                                                      "thumbnail-size",
                                                      "thumbnail-size",
                                                      LUNAR_TYPE_THUMBNAIL_SIZE,
                                                      LUNAR_THUMBNAIL_SIZE_NORMAL,
                                                      ENDO_PARAM_READWRITE));
}



static void
lunar_thumbnailer_get_property (GObject    *object,
                                 guint       prop_id,
                                 GValue     *value,
                                 GParamSpec *pspec)
{
  LunarThumbnailer *thumbnailer = LUNAR_THUMBNAILER (object);

  switch (prop_id)
    {
    case PROP_THUMBNAIL_SIZE:
      g_value_set_enum (value, thumbnailer->thumbnail_size);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_thumbnailer_set_property (GObject      *object,
                                 guint         prop_id,
                                 const GValue *value,
                                 GParamSpec   *pspec)
{
  LunarThumbnailer *thumbnailer = LUNAR_THUMBNAILER (object);

  switch (prop_id)
    {
    case PROP_THUMBNAIL_SIZE:
      thumbnailer->thumbnail_size = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}

static void
lunar_thumbnailer_free_job (LunarThumbnailerJob *job)
{
  if (job->files)
    g_list_free_full (job->files, g_object_unref);

  if (job->thumbnailer && job->thumbnailer->thumbnailer_proxy && job->handle)
    lunar_thumbnailer_dbus_call_dequeue (job->thumbnailer->thumbnailer_proxy, job->handle, NULL, NULL, NULL);

  g_slice_free (LunarThumbnailerJob, job);
}



static void
lunar_thumbnailer_queue_async_reply (GObject      *proxy,
                                      GAsyncResult *res,
                                      gpointer      user_data)
{
  LunarThumbnailerJob *job = user_data;
  LunarThumbnailer    *thumbnailer;
  GError               *error = NULL;
  guint                 handle;

  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER_DBUS (proxy));
  _lunar_return_if_fail (job != NULL);

  thumbnailer = LUNAR_THUMBNAILER (job->thumbnailer);

  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer));

  _thumbnailer_lock (thumbnailer);

  lunar_thumbnailer_dbus_call_queue_finish (LUNAR_THUMBNAILER_DBUS (proxy), &handle, res, &error);

  if (job->cancelled)
    {
      /* job is cancelled while there was no handle jet, so dequeue it now */
      lunar_thumbnailer_dbus_call_dequeue (LUNAR_THUMBNAILER_DBUS (proxy), handle, NULL, NULL, NULL);

      /* cleanup */
      thumbnailer->jobs = g_slist_remove (thumbnailer->jobs, job);
      lunar_thumbnailer_free_job (job);
    }
  else if (error == NULL)
    {
      if (handle == 0)
        {
          g_printerr ("LunarThumbnailer: got 0 handle (Queue)\n");
        }
      else
        {
          /* store the handle returned by tumbler */
          job->handle = handle;
        }
    }
  else
    {
      g_printerr ("LunarThumbnailer: Queue failed: %s\n", error->message);
    }

  _thumbnailer_unlock (thumbnailer);
  g_clear_error (&error);

  /* remove the additional reference we held during the call */
  g_object_unref (thumbnailer);
}



/* NOTE: assumes that the lock is held by the caller */
static gboolean
lunar_thumbnailer_begin_job (LunarThumbnailer *thumbnailer,
                              LunarThumbnailerJob *job)
{
  gboolean               success = FALSE;
  const gchar          **mime_hints;
  gchar                **uris;
  GList                 *lp;
  GList                 *supported_files = NULL;
  guint                  n;
  guint                  n_items = 0;
  LunarFileThumbState   thumb_state;
  const gchar           *thumbnail_path;
  gint                   request_no;

  if (thumbnailer->proxy_state == LUNAR_THUMBNAILER_PROXY_WAITING)
    {
      /* all pending jobs will be queued automatically once the proxy is available */
      return TRUE;
    }
  else if (thumbnailer->proxy_state == LUNAR_THUMBNAILER_PROXY_FAILED)
    {
      /* the job has no chance to be completed - ever */
      return FALSE;
    }

  /* collect all supported files from the list that are neither in the
   * about to be queued (wait queue), nor already queued, nor already
   * processed (and awaiting to be refreshed) */
  for (lp = job->files; lp != NULL; lp = lp->next)
    {
      /* the icon factory only loads icons for regular files and folders */
      if (!lunar_file_is_regular (lp->data) && !lunar_file_is_directory (lp->data))
        {
          lunar_file_set_thumb_state (lp->data, LUNAR_FILE_THUMB_STATE_NONE);
          continue;
        }

      /* get the current thumb state */
      thumb_state = lunar_file_get_thumb_state (lp->data);

      if (job->lazy_checks)
        {
          /* in lazy mode, don't both for files that have already
           * been loaded or are not supported */
          if (thumb_state == LUNAR_FILE_THUMB_STATE_NONE
              || thumb_state == LUNAR_FILE_THUMB_STATE_READY)
            continue;
        }

      /* check if the file is supported, assume it is when the state was ready previously */
      if (thumb_state == LUNAR_FILE_THUMB_STATE_READY
          || lunar_thumbnailer_file_is_supported (thumbnailer, lp->data))
        {
          supported_files = g_list_prepend (supported_files, lp->data);
          n_items++;
        }
      else
        {
          /* still a regular file, but the type is now known to tumbler but
           * maybe the application created a thumbnail */
          thumbnail_path = lunar_file_get_thumbnail_path (lp->data, thumbnailer->thumbnail_size);

          /* test if a thumbnail can be found */
          if (thumbnail_path != NULL && g_file_test (thumbnail_path, G_FILE_TEST_EXISTS))
            lunar_file_set_thumb_state (lp->data, LUNAR_FILE_THUMB_STATE_READY);
          else
            lunar_file_set_thumb_state (lp->data, LUNAR_FILE_THUMB_STATE_NONE);
        }
    }

  /* check if we have any supported files */
  if (n_items > 0)
    {
      /* allocate arrays for URIs and mime hints */
      uris = g_new0 (gchar *, n_items + 1);
      mime_hints = g_new0 (const gchar *, n_items + 1);

      /* fill URI and MIME hint arrays with items from the wait queue */
      for (lp = supported_files, n = 0; lp != NULL; lp = lp->next, ++n)
        {
          /* set the thumbnail state to loading */
          lunar_file_set_thumb_state (lp->data, LUNAR_FILE_THUMB_STATE_LOADING);

          /* save URI and MIME hint in the arrays */
          uris[n] = lunar_file_dup_uri (lp->data);
          mime_hints[n] = lunar_file_get_content_type (lp->data);
        }

      /* NULL-terminate both arrays */
      uris[n] = NULL;
      mime_hints[n] = NULL;

      /* queue a thumbnail request for the URIs from the wait queue */
      /* compute the next request ID, making sure it's never 0 */
      request_no = thumbnailer->last_request + 1;
      request_no = MAX (request_no, 1);

      /* remember the ID for the next request */
      thumbnailer->last_request = request_no;

      /* save the request number */
      job->request = request_no;

      /* increase the reference count while the dbus call is running */
      g_object_ref (thumbnailer);

      /* queue the request - asynchronously, of course */
      lunar_thumbnailer_dbus_call_queue (thumbnailer->thumbnailer_proxy,
                                          (const gchar *const *)uris,
                                          (const gchar *const *)mime_hints,
                                          lunar_thumbnail_size_get_nick (thumbnailer->thumbnail_size),
                                          "foreground", 0,
                                          NULL,
                                          lunar_thumbnailer_queue_async_reply,
                                          job);

      /* free mime hints array */
      g_free (mime_hints);
      g_strfreev (uris);

      /* free the list of supported files */
      g_list_free (supported_files);

      /* free the list of files passed in */
      g_list_free_full (job->files, g_object_unref);
      job->files = NULL;

      /* we assume success if we've come so far */
      success = TRUE;
    }

  return success;
}



static void
lunar_thumbnailer_init (LunarThumbnailer *thumbnailer)
{
  g_mutex_init (&thumbnailer->lock);

  /* initialize the proxies */
  lunar_thumbnailer_init_thumbnailer_proxy (thumbnailer);
}



static void
lunar_thumbnailer_finalize (GObject *object)
{
  LunarThumbnailer     *thumbnailer = LUNAR_THUMBNAILER (object);
  LunarThumbnailerIdle *idle;
  GSList                *lp;

  /* acquire the thumbnailer lock */
  _thumbnailer_lock (thumbnailer);

  if (thumbnailer->thumbnailer_proxy != NULL)
    {
      /* disconnect from the thumbnailer proxy */
      g_signal_handlers_disconnect_matched (thumbnailer->thumbnailer_proxy,
                                            G_SIGNAL_MATCH_DATA, 0, 0,
                                            NULL, NULL, thumbnailer);
    }

  /* abort all pending idle functions */
  for (lp = thumbnailer->idles; lp != NULL; lp = lp->next)
    {
      idle = lp->data;
      g_source_remove (idle->id);
    }
  g_slist_free (thumbnailer->idles);

  /* remove all jobs */
  g_slist_free_full (thumbnailer->jobs, (GDestroyNotify)lunar_thumbnailer_free_job);

  /* release the thumbnailer proxy */
  if (thumbnailer->thumbnailer_proxy != NULL)
    g_object_unref (thumbnailer->thumbnailer_proxy);

  /* free the cached URI schemes and MIME types table */
  if (thumbnailer->supported != NULL)
    g_hash_table_unref (thumbnailer->supported);

  /* release the thumbnailer lock */
  _thumbnailer_unlock (thumbnailer);

/* release the mutex */
  g_mutex_clear (&thumbnailer->lock);

  (*G_OBJECT_CLASS (lunar_thumbnailer_parent_class)->finalize) (object);
}



static gint
lunar_thumbnailer_file_schemes_compare (gconstpointer a,
                                         gconstpointer b)
{
  const gchar *scheme_a = *(gconstpointer *) a;
  const gchar *scheme_b = *(gconstpointer *) b;

  /* sort file before other schemes */
  if (strcmp (scheme_a, "file") == 0)
    return -1;
  if (strcmp (scheme_b, "file") == 0)
    return 1;

  /* sort trash before other schemes */
  if (strcmp (scheme_a, "trash") == 0)
    return -1;
  if (strcmp (scheme_b, "trash") == 0)
    return 1;

  /* other order is just fine */
  return 0;
}



static void
lunar_thumbnailer_file_sort_schemes (gpointer mime_type,
                                      gpointer schemes,
                                      gpointer user_data)
{
  g_ptr_array_sort (schemes, lunar_thumbnailer_file_schemes_compare);
}



static void
lunar_thumbnailer_received_supported_types (LunarThumbnailerDBus  *proxy,
                                             GAsyncResult           *result,
                                             LunarThumbnailer      *thumbnailer)
{
  guint       n;
  gchar     **schemes = NULL;
  gchar     **types = NULL;
  GPtrArray  *schemes_array;
  GSList     *lp = NULL;
  GError     *error = NULL;

  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer));
  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER_DBUS (proxy));

  _thumbnailer_lock (thumbnailer);


  if (!lunar_thumbnailer_dbus_call_get_supported_finish (proxy, &schemes, &types, result, &error))
    {
      g_slist_free_full (thumbnailer->jobs, (GDestroyNotify)lunar_thumbnailer_free_job);
      thumbnailer->jobs = NULL;

      g_printerr ("LunarThumbnailer: Failed to retrieve supported types: %s\n", error->message);
      g_clear_error (&error);

      thumbnailer->proxy_state = LUNAR_THUMBNAILER_PROXY_FAILED;
      g_object_unref (proxy);

      _thumbnailer_unlock (thumbnailer);

      g_object_unref (thumbnailer);
      return;
    }

  if (G_LIKELY (schemes != NULL && types != NULL))
    {
      /* combine content types and uri schemes */
      for (n = 0; types[n] != NULL; ++n)
        {
          schemes_array = g_hash_table_lookup (thumbnailer->supported, types[n]);
          if (G_UNLIKELY (schemes_array == NULL))
            {
              /* create an array for the uri schemes this content type supports */
              schemes_array = g_ptr_array_new_with_free_func (g_free);
              g_ptr_array_add (schemes_array, schemes[n]);
              g_hash_table_insert (thumbnailer->supported, types[n], schemes_array);
            }
          else
            {
              /* add the uri scheme to the array of the content type */
              g_ptr_array_add (schemes_array, schemes[n]);

              /* cleanup */
              g_free (types[n]);
            }
        }

      /* remove arrays, we stole the values */
      g_free (types);
      g_free (schemes);

      /* sort array to optimize for local files */
      g_hash_table_foreach (thumbnailer->supported, lunar_thumbnailer_file_sort_schemes, NULL);
    }

  thumbnailer->proxy_state = LUNAR_THUMBNAILER_PROXY_AVAILABLE;
  thumbnailer->thumbnailer_proxy = proxy;

  /* now start delayed jobs */
  for (lp = thumbnailer->jobs; lp; lp = lp->next)
    {
      if (!lunar_thumbnailer_begin_job (thumbnailer, lp->data))
        {
          lunar_thumbnailer_free_job (lp->data);
          lp->data = NULL;
        }
    }
  thumbnailer->jobs = g_slist_remove_all (thumbnailer->jobs, NULL);

  g_clear_error (&error);

  _thumbnailer_unlock (thumbnailer);

  g_object_unref (thumbnailer);
}



static void
lunar_thumbnailer_proxy_created (GObject       *object,
                                  GAsyncResult  *result,
                                  gpointer       userdata)
{
  LunarThumbnailer     *thumbnailer = LUNAR_THUMBNAILER (userdata);
  GError                *error = NULL;
  LunarThumbnailerDBus *proxy;

  proxy = lunar_thumbnailer_dbus_proxy_new_finish (result, &error);

  if (!proxy)
    {
      _thumbnailer_lock (thumbnailer);

      thumbnailer->proxy_state = LUNAR_THUMBNAILER_PROXY_FAILED;
      g_printerr ("LunarThumbnailer: failed to create proxy: %s", error->message);
      g_clear_error (&error);

      g_slist_free_full (thumbnailer->jobs, (GDestroyNotify)lunar_thumbnailer_free_job);
      thumbnailer->jobs = NULL;

      _thumbnailer_unlock (thumbnailer);

      g_object_unref (thumbnailer);

      return;
    }

  /* setup signals */
  g_signal_connect (proxy, "error",
                    G_CALLBACK (lunar_thumbnailer_thumbnailer_error), thumbnailer);
  g_signal_connect (proxy, "finished",
                    G_CALLBACK (lunar_thumbnailer_thumbnailer_finished), thumbnailer);
  g_signal_connect (proxy, "ready",
                    G_CALLBACK (lunar_thumbnailer_thumbnailer_ready), thumbnailer);

  /* begin retrieving supported file types */

  _thumbnailer_lock (thumbnailer);

  /* prepare table */
  g_clear_pointer (&thumbnailer->supported, g_hash_table_unref);
  thumbnailer->supported = g_hash_table_new_full (g_str_hash, g_str_equal,
                                                  g_free,
                                                  (GDestroyNotify) g_ptr_array_unref);

  /* request the supported types from the thumbnailer D-Bus service. */
  lunar_thumbnailer_dbus_call_get_supported (proxy, NULL,
                                              (GAsyncReadyCallback)lunar_thumbnailer_received_supported_types,
                                              thumbnailer);

  _thumbnailer_unlock (thumbnailer);
}



static void
lunar_thumbnailer_init_thumbnailer_proxy (LunarThumbnailer *thumbnailer)
{
  _thumbnailer_lock (thumbnailer);

  thumbnailer->thumbnailer_proxy = NULL;
  thumbnailer->proxy_state = LUNAR_THUMBNAILER_PROXY_WAITING;

  /* create the thumbnailer proxy */
  lunar_thumbnailer_dbus_proxy_new_for_bus (G_BUS_TYPE_SESSION,
                                             G_DBUS_PROXY_FLAGS_GET_INVALIDATED_PROPERTIES,
                                             "org.freedesktop.thumbnails.Thumbnailer1",
                                             "/org/freedesktop/thumbnails/Thumbnailer1",
                                             NULL,
                                             (GAsyncReadyCallback)lunar_thumbnailer_proxy_created,
                                             thumbnailer);

  _thumbnailer_unlock (thumbnailer);
}



/* NOTE: assumes the lock is being held by the caller*/
static gboolean
lunar_thumbnailer_file_is_supported (LunarThumbnailer *thumbnailer,
                                      LunarFile        *file)
{
  const gchar *content_type;
  gboolean     supported = FALSE;
  guint        n;
  GPtrArray   *schemes_array;
  const gchar *scheme;

  _lunar_return_val_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (file), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_THUMBNAILER_DBUS (thumbnailer->thumbnailer_proxy), FALSE);
  _lunar_return_val_if_fail (thumbnailer->supported != NULL, FALSE);

  /* determine the content type of the passed file */
  content_type = lunar_file_get_content_type (file);

  /* abort if the content type is unknown */
  if (content_type == NULL)
    return FALSE;

  /* lazy lookup the content type, no difficult parent type matching here */
  schemes_array = g_hash_table_lookup (thumbnailer->supported, content_type);
  if (schemes_array != NULL)
    {
      /* go through all the URI schemes this type supports */
      for (n = 0; !supported && n < schemes_array->len; ++n)
        {
          /* check if the file has the current URI scheme */
          scheme = g_ptr_array_index (schemes_array, n);
          if (lunar_file_has_uri_scheme (file, scheme))
            supported = TRUE;
        }
    }

  return supported;
}



static void
lunar_thumbnailer_thumbnailer_error (GDBusProxy        *proxy,
                                      guint              handle,
                                      const gchar      **uris,
                                      gint               code,
                                      const gchar       *message,
                                      LunarThumbnailer *thumbnailer)
{
  _lunar_return_if_fail (G_IS_DBUS_PROXY (proxy));
  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer));

  /* check if we have any ready URIs */
  lunar_thumbnailer_idle (thumbnailer,
                           handle,
                           LUNAR_THUMBNAILER_IDLE_ERROR,
                           uris);
}



static void
lunar_thumbnailer_thumbnailer_ready (GDBusProxy        *proxy,
                                      guint32            handle,
                                      const gchar      **uris,
                                      LunarThumbnailer *thumbnailer)
{
  _lunar_return_if_fail (G_IS_DBUS_PROXY (proxy));
  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer));

  lunar_thumbnailer_idle (thumbnailer,
                           handle,
                           LUNAR_THUMBNAILER_IDLE_READY,
                           uris);
}



static void
lunar_thumbnailer_thumbnailer_finished (GDBusProxy        *proxy,
                                         guint              handle,
                                         LunarThumbnailer *thumbnailer)
{
  LunarThumbnailerJob *job;
  GSList               *lp;

  _lunar_return_if_fail (G_IS_DBUS_PROXY (proxy));
  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer));

  if (handle == 0)
    {
      g_printerr ("LunarThumbnailer: got 0 handle (Finished)\n");
      return;
    }

  _thumbnailer_lock (thumbnailer);

  for (lp = thumbnailer->jobs; lp != NULL; lp = lp->next)
    {
      job = lp->data;

      if (job->handle == handle)
        {
          /* this job is finished, forget about the handle */
          job->handle = 0;

          /* tell everybody we're done here */
          g_signal_emit (G_OBJECT (thumbnailer), thumbnailer_signals[REQUEST_FINISHED], 0, job->request);

          /* remove job from the list */
          thumbnailer->jobs = g_slist_delete_link (thumbnailer->jobs, lp);

          lunar_thumbnailer_free_job (job);
          break;
        }
    }

  _thumbnailer_unlock (thumbnailer);
}



static void
lunar_thumbnailer_idle (LunarThumbnailer          *thumbnailer,
                         guint                       handle,
                         LunarThumbnailerIdleType   type,
                         const gchar               **uris)
{
  GSList                *lp;
  LunarThumbnailerIdle *idle;
  LunarThumbnailerJob  *job;

  /* leave if there are no uris */
  if (G_UNLIKELY (uris == NULL))
    return;

  if (handle == 0)
    {
      g_printerr ("LunarThumbnailer: got 0 handle (Error or Ready)\n");
      return;
    }

  _thumbnailer_lock (thumbnailer);

  /* look for the job so we don't emit unknown handles, the reason
   * we do this is when you have multiple windows opened, you don't
   * want each window (because they all have a connection to the
   * same proxy) emit the file change, only the window that requested
   * the data */
  for (lp = thumbnailer->jobs; lp != NULL; lp = lp->next)
    {
      job = lp->data;

      if (job->handle == handle)
        {
          /* allocate a new idle struct */
          idle = g_slice_new0 (LunarThumbnailerIdle);
          idle->type = type;
          idle->thumbnailer = thumbnailer;

          /* copy the URI array because we need it in the idle function */
          idle->uris = g_strdupv ((gchar **)uris);

          /* remember the idle struct because we might have to remove it in finalize() */
          thumbnailer->idles = g_slist_prepend (thumbnailer->idles, idle);

          /* call the idle function when we have the time */
          idle->id = g_idle_add_full (G_PRIORITY_LOW,
                                      lunar_thumbnailer_idle_func, idle,
                                      lunar_thumbnailer_idle_free);

          break;
        }
    }

  _thumbnailer_unlock (thumbnailer);
}



static gboolean
lunar_thumbnailer_idle_func (gpointer user_data)
{
  LunarThumbnailerIdle *idle = user_data;
  LunarFile            *file;
  GFile                 *gfile;
  guint                  n;

  _lunar_return_val_if_fail (idle != NULL, FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_THUMBNAILER (idle->thumbnailer), FALSE);

  /* iterate over all failed URIs */
  for (n = 0; idle->uris != NULL && idle->uris[n] != NULL; ++n)
    {
      /* look up the corresponding LunarFile from the cache */
      gfile = g_file_new_for_uri (idle->uris[n]);
      file = lunar_file_cache_lookup (gfile);
      g_object_unref (gfile);

      /* check if we have a file for this URI in the cache */
      if (file != NULL)
        {
          if (idle->type == LUNAR_THUMBNAILER_IDLE_ERROR)
            {
              /* set thumbnail state to none unless the thumbnail has already been created.
               * This is to prevent race conditions with the other idle functions */
              if (lunar_file_get_thumb_state (file) != LUNAR_FILE_THUMB_STATE_READY)
                lunar_file_set_thumb_state (file, LUNAR_FILE_THUMB_STATE_NONE);
            }
          else if (idle->type == LUNAR_THUMBNAILER_IDLE_READY)
            {
              /* set thumbnail state to ready - we now have a thumbnail */
              lunar_file_set_thumb_state (file, LUNAR_FILE_THUMB_STATE_READY);
            }
          else
            {
              _lunar_assert_not_reached ();
            }
          g_object_unref (file);
        }
    }

  /* remove the idle struct */
  _thumbnailer_lock (idle->thumbnailer);
  idle->thumbnailer->idles = g_slist_remove (idle->thumbnailer->idles, idle);
  _thumbnailer_unlock (idle->thumbnailer);

  /* remove the idle source, which also destroys the idle struct */
  return FALSE;
}



static void
lunar_thumbnailer_idle_free (gpointer data)
{
  LunarThumbnailerIdle *idle = data;

  _lunar_return_if_fail (idle != NULL);

  /* free the URI array if necessary */
  if (idle->type == LUNAR_THUMBNAILER_IDLE_READY
      || idle->type == LUNAR_THUMBNAILER_IDLE_ERROR)
    {
      g_strfreev (idle->uris);
    }

  /* free the struct */
  g_slice_free (LunarThumbnailerIdle, idle);
}



/**
 * lunar_thumbnailer_get:
 *
 * Return a shared new #LunarThumbnailer object, which can be used to
 * generate and store thumbnails for files.
 *
 * The caller is responsible to free the returned
 * object using g_object_unref() when no longer needed.
 *
 * Return value: a #LunarThumbnailer.
 **/
LunarThumbnailer*
lunar_thumbnailer_get (void)
{
  static LunarThumbnailer *thumbnailer = NULL;

  if (G_UNLIKELY (thumbnailer == NULL))
    {
      thumbnailer = g_object_new (LUNAR_TYPE_THUMBNAILER, NULL);
      g_object_add_weak_pointer (G_OBJECT (thumbnailer), (gpointer) &thumbnailer);
    }

  return g_object_ref_sink (thumbnailer);
}



gboolean
lunar_thumbnailer_queue_file (LunarThumbnailer *thumbnailer,
                               LunarFile        *file,
                               guint             *request)
{
  GList files;

  _lunar_return_val_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_FILE (file), FALSE);

  /* fake a file list */
  files.data = file;
  files.next = NULL;
  files.prev = NULL;

  /* queue a thumbnail request for the file */
  return lunar_thumbnailer_queue_files (thumbnailer, FALSE, &files, request);
}



gboolean
lunar_thumbnailer_queue_files (LunarThumbnailer *thumbnailer,
                                gboolean           lazy_checks,
                                GList             *files,
                                guint             *request)
{
  gboolean               success = FALSE;
  LunarThumbnailerJob  *job = NULL;

  _lunar_return_val_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer), FALSE);
  _lunar_return_val_if_fail (files != NULL, FALSE);

  /* acquire the thumbnailer lock */
  _thumbnailer_lock (thumbnailer);

  /* allocate a job */
  job = g_slice_new0 (LunarThumbnailerJob);
  job->thumbnailer = thumbnailer;
  job->files = g_list_copy_deep (files, (GCopyFunc) (void (*)(void)) g_object_ref, NULL);
  job->lazy_checks = lazy_checks ? 1 : 0;

  success = lunar_thumbnailer_begin_job (thumbnailer, job);
  if (success)
    {
      thumbnailer->jobs = g_slist_prepend (thumbnailer->jobs, job);
      if (*request)
        *request = job->request;
    }
  else
    {
      lunar_thumbnailer_free_job (job);
    }

  /* release the lock */
  _thumbnailer_unlock (thumbnailer);

  return success;
}



void
lunar_thumbnailer_dequeue (LunarThumbnailer *thumbnailer,
                            guint              request)
{
  LunarThumbnailerJob *job;
  GSList               *lp;

  _lunar_return_if_fail (LUNAR_IS_THUMBNAILER (thumbnailer));

  /* acquire the thumbnailer lock */
  _thumbnailer_lock (thumbnailer);

  for (lp = thumbnailer->jobs; lp != NULL; lp = lp->next)
    {
      job = lp->data;

      /* find the request in the list */
      if (job->request == request)
        {
          /* this job is cancelled */
          job->cancelled = TRUE;

          if (job->handle != 0)
            {
              /* remove job */
              thumbnailer->jobs = g_slist_delete_link (thumbnailer->jobs, lp);

              lunar_thumbnailer_free_job (job);
            }

          break;
        }
    }

  /* release the thumbnailer lock */
  _thumbnailer_unlock (thumbnailer);
}
