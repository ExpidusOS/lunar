/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#include <lunar/lunar-file-monitor.h>
#include <lunar/lunar-folder.h>
#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-io-jobs.h>
#include <lunar/lunar-job.h>
#include <lunar/lunar-private.h>

#define DEBUG_FILE_CHANGES FALSE



/* property identifiers */
enum
{
  PROP_0,
  PROP_CORRESPONDING_FILE,
  PROP_LOADING,
};

/* signal identifiers */
enum
{
  DESTROY,
  ERROR,
  FILES_ADDED,
  FILES_REMOVED,
  LAST_SIGNAL,
};



static void     lunar_folder_dispose                     (GObject                *object);
static void     lunar_folder_finalize                    (GObject                *object);
static void     lunar_folder_get_property                (GObject                *object,
                                                           guint                   prop_id,
                                                           GValue                 *value,
                                                           GParamSpec             *pspec);
static void     lunar_folder_set_property                (GObject                *object,
                                                           guint                   prop_uid,
                                                           const GValue           *value,
                                                           GParamSpec             *pspec);
static void     lunar_folder_real_destroy                (LunarFolder           *folder);
static void     lunar_folder_error                       (EndoJob                 *job,
                                                           GError                 *error,
                                                           LunarFolder           *folder);
static gboolean lunar_folder_files_ready                 (LunarJob              *job,
                                                           GList                  *files,
                                                           LunarFolder           *folder);
static void     lunar_folder_finished                    (EndoJob                 *job,
                                                           LunarFolder           *folder);
static void     lunar_folder_file_changed                (LunarFileMonitor      *file_monitor,
                                                           LunarFile             *file,
                                                           LunarFolder           *folder);
static void     lunar_folder_file_destroyed              (LunarFileMonitor      *file_monitor,
                                                           LunarFile             *file,
                                                           LunarFolder           *folder);
static void     lunar_folder_monitor                     (GFileMonitor           *monitor,
                                                           GFile                  *file,
                                                           GFile                  *other_file,
                                                           GFileMonitorEvent       event_type,
                                                           gpointer                user_data);



struct _LunarFolderClass
{
  GObjectClass __parent__;

  /* signals */
  void (*destroy)       (LunarFolder *folder);
  void (*error)         (LunarFolder *folder,
                         const GError *error);
  void (*files_added)   (LunarFolder *folder,
                         GList        *files);
  void (*files_removed) (LunarFolder *folder,
                         GList        *files);
};

struct _LunarFolder
{
  GObject __parent__;

  LunarJob         *job;

  LunarFile        *corresponding_file;
  GList             *new_files;
  GList             *files;
  gboolean           reload_info;

  GList             *content_type_ptr;
  guint              content_type_idle_id;

  guint              in_destruction : 1;

  LunarFileMonitor *file_monitor;

  GFileMonitor      *monitor;
};



static guint  folder_signals[LAST_SIGNAL];
static GQuark lunar_folder_quark;



G_DEFINE_TYPE (LunarFolder, lunar_folder, G_TYPE_OBJECT)



static void
lunar_folder_constructed (GObject *object)
{
  LunarFolder *folder = LUNAR_FOLDER (object);
  GError       *error  = NULL;

  folder->monitor = g_file_monitor_directory (lunar_file_get_file (folder->corresponding_file),
                                              G_FILE_MONITOR_WATCH_MOVES, NULL, &error);

  if (G_LIKELY (folder->monitor != NULL))
      g_signal_connect (folder->monitor, "changed", G_CALLBACK (lunar_folder_monitor), folder);
  else
    {
      g_debug ("Could not create folder monitor: %s", error->message);
      g_error_free (error);
    }

  G_OBJECT_CLASS (lunar_folder_parent_class)->constructed (object);
}



static void
lunar_folder_class_init (LunarFolderClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = lunar_folder_dispose;
  gobject_class->finalize = lunar_folder_finalize;
  gobject_class->get_property = lunar_folder_get_property;
  gobject_class->set_property = lunar_folder_set_property;
  gobject_class->constructed = lunar_folder_constructed;

  klass->destroy = lunar_folder_real_destroy;

  /**
   * LunarFolder::corresponding-file:
   *
   * The #LunarFile referring to the #LunarFolder.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_CORRESPONDING_FILE,
                                   g_param_spec_object ("corresponding-file",
                                                        "corresponding-file",
                                                        "corresponding-file",
                                                        LUNAR_TYPE_FILE,
                                                        G_PARAM_READABLE
                                                        | G_PARAM_WRITABLE
                                                        | G_PARAM_CONSTRUCT_ONLY));

  /**
   * LunarFolder::loading:
   *
   * Tells whether the contents of the #LunarFolder are
   * currently being loaded.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_LOADING,
                                   g_param_spec_boolean ("loading",
                                                         "loading",
                                                         "loading",
                                                         FALSE,
                                                         ENDO_PARAM_READABLE));
  /**
   * LunarFolder::destroy:
   * @folder : a #LunarFolder.
   *
   * Emitted when the #LunarFolder is destroyed.
   **/
  folder_signals[DESTROY] =
    g_signal_new (I_("destroy"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_CLEANUP | G_SIGNAL_NO_RECURSE | G_SIGNAL_NO_HOOKS,
                  G_STRUCT_OFFSET (LunarFolderClass, destroy),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__VOID,
                  G_TYPE_NONE, 0);

  /**
   * LunarFolder::error:
   * @folder : a #LunarFolder.
   * @error  : the #GError describing the problem.
   *
   * Emitted when the #LunarFolder fails to completly
   * load the directory content because of an error.
   **/
  folder_signals[ERROR] =
    g_signal_new (I_("error"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (LunarFolderClass, error),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * LunarFolder::files-added:
   *
   * Emitted by the #LunarFolder whenever new files have
   * been added to a particular folder.
   **/
  folder_signals[FILES_ADDED] =
    g_signal_new (I_("files-added"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (LunarFolderClass, files_added),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);

  /**
   * LunarFolder::files-removed:
   *
   * Emitted by the #LunarFolder whenever a bunch of files
   * is removed from the folder, which means they are not
   * necessarily deleted from disk. This can be used to implement
   * the reload of folders, which take longer to load.
   **/
  folder_signals[FILES_REMOVED] =
    g_signal_new (I_("files-removed"),
                  G_TYPE_FROM_CLASS (gobject_class),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (LunarFolderClass, files_removed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__POINTER,
                  G_TYPE_NONE, 1, G_TYPE_POINTER);
}



static void
lunar_folder_init (LunarFolder *folder)
{
  /* connect to the LunarFileMonitor instance */
  folder->file_monitor = lunar_file_monitor_get_default ();
  g_signal_connect (G_OBJECT (folder->file_monitor), "file-changed", G_CALLBACK (lunar_folder_file_changed), folder);
  g_signal_connect (G_OBJECT (folder->file_monitor), "file-destroyed", G_CALLBACK (lunar_folder_file_destroyed), folder);

  folder->monitor = NULL;
  folder->reload_info = FALSE;
}



static void
lunar_folder_dispose (GObject *object)
{
  LunarFolder *folder = LUNAR_FOLDER (object);

  if (!folder->in_destruction)
    {
      folder->in_destruction = TRUE;
      g_signal_emit (G_OBJECT (folder), folder_signals[DESTROY], 0);
      folder->in_destruction = FALSE;
    }

  (*G_OBJECT_CLASS (lunar_folder_parent_class)->dispose) (object);
}



static void
lunar_folder_finalize (GObject *object)
{
  LunarFolder *folder = LUNAR_FOLDER (object);

  if (folder->corresponding_file)
    lunar_file_unwatch (folder->corresponding_file);

  /* disconnect from the LunarFileMonitor instance */
  g_signal_handlers_disconnect_matched (folder->file_monitor, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, folder);
  g_object_unref (folder->file_monitor);

  /* disconnect from the file alteration monitor */
  if (G_LIKELY (folder->monitor != NULL))
    {
      g_signal_handlers_disconnect_matched (folder->monitor, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, folder);
      g_file_monitor_cancel (folder->monitor);
      g_object_unref (folder->monitor);
    }

  /* cancel the pending job (if any) */
  if (G_UNLIKELY (folder->job != NULL))
    {
      g_signal_handlers_disconnect_matched (folder->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, folder);
      g_object_unref (folder->job);
      folder->job = NULL;
    }

  /* disconnect from the corresponding file */
  if (G_LIKELY (folder->corresponding_file != NULL))
    {
      /* drop the reference */
      g_object_set_qdata (G_OBJECT (folder->corresponding_file), lunar_folder_quark, NULL);
      g_object_unref (G_OBJECT (folder->corresponding_file));
    }

  /* stop metadata collector */
  if (folder->content_type_idle_id != 0)
    g_source_remove (folder->content_type_idle_id);

  /* release references to the new files */
  lunar_g_file_list_free (folder->new_files);

  /* release references to the current files */
  lunar_g_file_list_free (folder->files);

  (*G_OBJECT_CLASS (lunar_folder_parent_class)->finalize) (object);
}



static void
lunar_folder_get_property (GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec)
{
  LunarFolder *folder = LUNAR_FOLDER (object);

  switch (prop_id)
    {
    case PROP_CORRESPONDING_FILE:
      g_value_set_object (value, folder->corresponding_file);
      break;

    case PROP_LOADING:
      g_value_set_boolean (value, lunar_folder_get_loading (folder));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_folder_set_property (GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec)
{
  LunarFolder *folder = LUNAR_FOLDER (object);

  switch (prop_id)
    {
    case PROP_CORRESPONDING_FILE:
      folder->corresponding_file = g_value_dup_object (value);
      if (folder->corresponding_file)
        lunar_file_watch (folder->corresponding_file);
      break;

    case PROP_LOADING:
      _lunar_assert_not_reached ();
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_folder_real_destroy (LunarFolder *folder)
{
  g_signal_handlers_destroy (G_OBJECT (folder));
}



static void
lunar_folder_error (EndoJob       *job,
                     GError       *error,
                     LunarFolder *folder)
{
  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));
  _lunar_return_if_fail (LUNAR_IS_JOB (job));

  /* tell the consumer about the problem */
  g_signal_emit (G_OBJECT (folder), folder_signals[ERROR], 0, error);
}



static gboolean
lunar_folder_files_ready (LunarJob    *job,
                           GList        *files,
                           LunarFolder *folder)
{
  _lunar_return_val_if_fail (LUNAR_IS_FOLDER (folder), FALSE);
  _lunar_return_val_if_fail (LUNAR_IS_JOB (job), FALSE);

  /* merge the list with the existing list of new files */
  folder->new_files = g_list_concat (folder->new_files, files);

  /* indicate that we took over ownership of the file list */
  return TRUE;
}



static gboolean
lunar_folder_content_type_loader_idle (gpointer data)
{
  LunarFolder *folder;
  GList        *lp;

  _lunar_return_val_if_fail (LUNAR_IS_FOLDER (data), FALSE);

  folder = LUNAR_FOLDER (data);

  /* load another files content type */
  for (lp = folder->content_type_ptr; lp != NULL; lp = lp->next)
    if (lunar_file_load_content_type (lp->data))
      {
        /* if this was the last file, abort */
        if (G_UNLIKELY (lp->next == NULL))
          break;

        /* set pointer to next file for the next iteration */
        folder->content_type_ptr = lp->next;

        return TRUE;
      }

  /* all content types loaded */
  return FALSE;
}



static void
lunar_folder_content_type_loader_idle_destroyed (gpointer data)
{
  _lunar_return_if_fail (LUNAR_IS_FOLDER (data));

  LUNAR_FOLDER (data)->content_type_idle_id = 0;
}



static void
lunar_folder_content_type_loader (LunarFolder *folder)
{
  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));
  _lunar_return_if_fail (folder->content_type_idle_id == 0);

  /* set the pointer to the start of the list */
  folder->content_type_ptr = folder->files;

  /* schedule idle */
  folder->content_type_idle_id = g_idle_add_full (G_PRIORITY_LOW, lunar_folder_content_type_loader_idle,
                                                  folder, lunar_folder_content_type_loader_idle_destroyed);
}



static void
lunar_folder_finished (EndoJob       *job,
                        LunarFolder *folder)
{
  LunarFile *file;
  GList      *files;
  GList      *lp;

  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));
  _lunar_return_if_fail (LUNAR_IS_JOB (job));
  _lunar_return_if_fail (LUNAR_IS_FILE (folder->corresponding_file));
  _lunar_return_if_fail (folder->content_type_idle_id == 0);

  /* check if we need to merge new files with existing files */
  if (G_UNLIKELY (folder->files != NULL))
    {
      /* determine all added files (files on new_files, but not on files) */
      for (files = NULL, lp = folder->new_files; lp != NULL; lp = lp->next)
        if (g_list_find (folder->files, lp->data) == NULL)
          {
            /* put the file on the added list */
            files = g_list_prepend (files, lp->data);

            /* add to the internal files list */
            folder->files = g_list_prepend (folder->files, lp->data);
            g_object_ref (G_OBJECT (lp->data));
          }

      /* check if any files were added */
      if (G_UNLIKELY (files != NULL))
        {
          /* emit a "files-added" signal for the added files */
          g_signal_emit (G_OBJECT (folder), folder_signals[FILES_ADDED], 0, files);

          /* release the added files list */
          g_list_free (files);
        }

      /* determine all removed files (files on files, but not on new_files) */
      for (files = NULL, lp = folder->files; lp != NULL; )
        {
          /* determine the file */
          file = LUNAR_FILE (lp->data);

          /* determine the next list item */
          lp = lp->next;

          /* check if the file is not on new_files */
          if (g_list_find (folder->new_files, file) == NULL)
            {
              /* put the file on the removed list (owns the reference now) */
              files = g_list_prepend (files, file);

              /* remove from the internal files list */
              folder->files = g_list_remove (folder->files, file);
            }
        }

      /* check if any files were removed */
      if (G_UNLIKELY (files != NULL))
        {
          /* emit a "files-removed" signal for the removed files */
          g_signal_emit (G_OBJECT (folder), folder_signals[FILES_REMOVED], 0, files);

          /* release the removed files list */
          lunar_g_file_list_free (files);
        }

      /* drop the temporary new_files list */
      lunar_g_file_list_free (folder->new_files);
      folder->new_files = NULL;
    }
  else
    {
      /* just use the new files for the files list */
      folder->files = folder->new_files;
      folder->new_files = NULL;

      if (folder->files != NULL)
        {
          /* emit a "files-added" signal for the new files */
          g_signal_emit (G_OBJECT (folder), folder_signals[FILES_ADDED], 0, folder->files);
        }
    }

  /* schedule a reload of the file information of all files if requested */
  if (folder->reload_info)
    {
      folder->reload_info = FALSE;
      for (lp = folder->files; lp != NULL; lp = lp->next)
        lunar_file_reload (lp->data);

      /* reload folder information too */
      if (lunar_file_reload (folder->corresponding_file))
        return;

    }

  /* we did it, the folder is loaded */
  if (G_LIKELY (folder->job != NULL))
    {
      g_signal_handlers_disconnect_matched (folder->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, folder);
      g_object_unref (folder->job);
      folder->job = NULL;
    }

  /* restart the content type idle loader */
  lunar_folder_content_type_loader (folder);

  /* tell the consumers that we have loaded the directory */
  g_object_notify (G_OBJECT (folder), "loading");
}



static void
lunar_folder_file_changed (LunarFileMonitor *file_monitor,
                            LunarFile        *file,
                            LunarFolder      *folder)
{
  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));
  _lunar_return_if_fail (LUNAR_IS_FILE_MONITOR (file_monitor));

  /* check if the corresponding file changed... */
  if (G_UNLIKELY (folder->corresponding_file == file))
    {
      /* ...and if so, reload the folder */
      lunar_folder_reload (folder, FALSE);
    }
}



static void
lunar_folder_file_destroyed (LunarFileMonitor *file_monitor,
                              LunarFile        *file,
                              LunarFolder      *folder)
{
  GList     files;
  GList    *lp;
  gboolean  restart = FALSE;

  _lunar_return_if_fail (LUNAR_IS_FILE (file));
  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));
  _lunar_return_if_fail (LUNAR_IS_FILE_MONITOR (file_monitor));

  /* check if the corresponding file was destroyed */
  if (G_UNLIKELY (folder->corresponding_file == file))
    {
      /* the folder is useless now */
      if (!folder->in_destruction)
        g_object_run_dispose (G_OBJECT (folder));
    }
  else
    {
      /* check if we have that file */
      lp = g_list_find (folder->files, file);
      if (G_LIKELY (lp != NULL))
        {
          if (folder->content_type_idle_id != 0)
            restart = g_source_remove (folder->content_type_idle_id);

          /* remove the file from our list */
          folder->files = g_list_delete_link (folder->files, lp);

          /* tell everybody that the file is gone */
          files.data = file; files.next = files.prev = NULL;
          g_signal_emit (G_OBJECT (folder), folder_signals[FILES_REMOVED], 0, &files);

          /* drop our reference to the file */
          g_object_unref (G_OBJECT (file));

          /* continue collecting the metadata */
          if (restart)
            lunar_folder_content_type_loader (folder);
        }
    }
}



#if DEBUG_FILE_CHANGES
static void
lunar_file_infos_equal (LunarFile *file,
                         GFile      *event_file)
{
  gchar     **attrs;
  GFileInfo  *info1 = G_FILE_INFO (file->info);
  GFileInfo  *info2;
  guint       i;
  gchar      *attr1, *attr2;
  gboolean    printed = FALSE;
  gchar      *bname;

  attrs = g_file_info_list_attributes (info1, NULL);
  info2 = g_file_query_info (event_file, LUNARX_FILE_INFO_NAMESPACE,
                             G_FILE_QUERY_INFO_NONE, NULL, NULL);

  if (info1 != NULL && info2 != NULL)
    {
      for (i = 0; attrs[i] != NULL; i++)
        {
          if (g_file_info_has_attribute (info2, attrs[i]))
            {
              attr1 = g_file_info_get_attribute_as_string (info1, attrs[i]);
              attr2 = g_file_info_get_attribute_as_string (info2, attrs[i]);

              if (g_strcmp0 (attr1, attr2) != 0)
                {
                  if (!printed)
                    {
                      bname = g_file_get_basename (event_file);
                      g_print ("%s\n", bname);
                      g_free (bname);

                      printed = TRUE;
                    }

                  g_print ("  %s: %s -> %s\n", attrs[i], attr1, attr2);
                }

              g_free (attr1);
              g_free (attr2);
            }
        }

      g_object_unref (info2);
    }

  if (printed)
    g_print ("\n");

  g_free (attrs);
}
#endif



static void
lunar_folder_monitor (GFileMonitor     *monitor,
                       GFile            *event_file,
                       GFile            *other_file,
                       GFileMonitorEvent event_type,
                       gpointer          user_data)
{
  LunarFolder *folder = LUNAR_FOLDER (user_data);
  LunarFile   *file;
  LunarFile   *other_parent;
  GList        *lp;
  GList         list;
  gboolean      restart = FALSE;

  _lunar_return_if_fail (G_IS_FILE_MONITOR (monitor));
  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));
  _lunar_return_if_fail (folder->monitor == monitor);
  _lunar_return_if_fail (LUNAR_IS_FILE (folder->corresponding_file));
  _lunar_return_if_fail (G_IS_FILE (event_file));

  /* check on which file the event occurred */
  if (!g_file_equal (event_file, lunar_file_get_file (folder->corresponding_file)))
    {
      /* check if we already ship the file */
      for (lp = folder->files; lp != NULL; lp = lp->next)
        if (g_file_equal (event_file, lunar_file_get_file (lp->data)))
          break;

      /* stop the content type collector */
      if (folder->content_type_idle_id != 0)
        restart = g_source_remove (folder->content_type_idle_id);

      /* if we don't have it, add it if the event is not an "deleted" event */
      if (G_UNLIKELY (lp == NULL && event_type != G_FILE_MONITOR_EVENT_DELETED))
        {
          /* allocate a file for the path */
          file = lunar_file_get (event_file, NULL);
          if (G_UNLIKELY (file != NULL))
            {
              /* prepend it to our internal list */
              folder->files = g_list_prepend (folder->files, file);

              /* tell others about the new file */
              list.data = file; list.next = list.prev = NULL;
              g_signal_emit (G_OBJECT (folder), folder_signals[FILES_ADDED], 0, &list);

              /* load the new file */
              lunar_file_reload (file);
            }
        }
      else if (lp != NULL)
        {
          if (event_type == G_FILE_MONITOR_EVENT_DELETED)
            {
              LunarFile *destroyed;

              /* destroy the file */
              lunar_file_destroy (lp->data);

              /* if the file has not been destroyed by now, reload it to invalidate it */
              destroyed = lunar_file_cache_lookup (event_file);
              if (destroyed != NULL)
                {
                  lunar_file_reload (destroyed);
                  g_object_unref (destroyed);
                }
            }
          else if (event_type == G_FILE_MONITOR_EVENT_RENAMED ||
                   event_type == G_FILE_MONITOR_EVENT_MOVED_IN ||
                   event_type == G_FILE_MONITOR_EVENT_MOVED_OUT)
            {
              /* destroy the old file and update the new one */
              lunar_file_destroy (lp->data);
              if (other_file != NULL)
                {
                  file = lunar_file_get(other_file, NULL);
                  if (file != NULL && LUNAR_IS_FILE (file))
                    {
                      if (lunar_file_reload (file))
                        {
                          /* if source and target folders are different, also tell
                             the target folder to reload for the changes */
                          if (lunar_file_has_parent (file))
                            {
                              other_parent = lunar_file_get_parent (file, NULL);
                              if (other_parent &&
                                  !g_file_equal (lunar_file_get_file(folder->corresponding_file),
                                                 lunar_file_get_file(other_parent)))
                                {
                                  lunar_file_reload (other_parent);
                                  g_object_unref (other_parent);
                                }
                            }
                        }

                      /* drop reference on the other file */
                      g_object_unref (file);
                    }
                }
            }
          else
            {
#if DEBUG_FILE_CHANGES
              lunar_file_infos_equal (lp->data, event_file);
#endif
              lunar_file_reload (lp->data);
            }
        }

      /* check if we need to restart the collector */
      if (restart)
        lunar_folder_content_type_loader (folder);
    }
  else
    {
      /* update/destroy the corresponding file */
      if (event_type == G_FILE_MONITOR_EVENT_DELETED)
        {
          if (!lunar_file_exists (folder->corresponding_file))
            lunar_file_destroy (folder->corresponding_file);
        }
      else
        {
          lunar_file_reload (folder->corresponding_file);
        }
    }
}



/**
 * lunar_folder_get_for_file:
 * @file : a #LunarFile.
 *
 * Opens the specified @file as #LunarFolder and
 * returns a reference to the folder.
 *
 * The caller is responsible to free the returned
 * object using g_object_unref() when no longer
 * needed.
 *
 * Return value: the #LunarFolder which corresponds
 *               to @file.
 **/
LunarFolder*
lunar_folder_get_for_file (LunarFile *file)
{
  LunarFolder *folder;

  _lunar_return_val_if_fail (LUNAR_IS_FILE (file), NULL);

  /* make sure the file is loaded */
  if (!lunar_file_check_loaded (file))
    return NULL;

  _lunar_return_val_if_fail (lunar_file_is_directory (file), NULL);

  /* load if the file is not a folder */
  if (!lunar_file_is_directory (file))
    return NULL;

  /* determine the "lunar-folder" quark on-demand */
  if (G_UNLIKELY (lunar_folder_quark == 0))
    lunar_folder_quark = g_quark_from_static_string ("lunar-folder");

  /* check if we already know that folder */
  folder = g_object_get_qdata (G_OBJECT (file), lunar_folder_quark);
  if (G_UNLIKELY (folder != NULL))
    {
      g_object_ref (G_OBJECT (folder));
    }
  else
    {
      /* allocate the new instance */
      folder = g_object_new (LUNAR_TYPE_FOLDER, "corresponding-file", file, NULL);

      /* connect the folder to the file */
      g_object_set_qdata (G_OBJECT (file), lunar_folder_quark, folder);

      /* schedule the loading of the folder */
      lunar_folder_reload (folder, FALSE);
    }

  return folder;
}



/**
 * lunar_folder_get_corresponding_file:
 * @folder : a #LunarFolder instance.
 *
 * Returns the #LunarFile corresponding to this @folder.
 *
 * No reference is taken on the returned #LunarFile for
 * the caller, so if you need a persistent reference to
 * the file, you'll have to call g_object_ref() yourself.
 *
 * Return value: the #LunarFile corresponding to @folder.
 **/
LunarFile*
lunar_folder_get_corresponding_file (const LunarFolder *folder)
{
  _lunar_return_val_if_fail (LUNAR_IS_FOLDER (folder), NULL);
  return folder->corresponding_file;
}



/**
 * lunar_folder_get_files:
 * @folder : a #LunarFolder instance.
 *
 * Returns the list of files currently known for @folder.
 * The returned list is owned by @folder and may not be freed!
 *
 * Return value: the list of #LunarFiles for @folder.
 **/
GList*
lunar_folder_get_files (const LunarFolder *folder)
{
  _lunar_return_val_if_fail (LUNAR_IS_FOLDER (folder), NULL);
  return folder->files;
}



/**
 * lunar_folder_get_loading:
 * @folder : a #LunarFolder instance.
 *
 * Tells whether the contents of the @folder are currently
 * being loaded.
 *
 * Return value: %TRUE if @folder is loading, else %FALSE.
 **/
gboolean
lunar_folder_get_loading (const LunarFolder *folder)
{
  _lunar_return_val_if_fail (LUNAR_IS_FOLDER (folder), FALSE);
  return (folder->job != NULL);
}



/**
 * lunar_folder_has_folder_monitor:
 * @folder : a #LunarFolder instance.
 *
 * Tells whether the @folder has a folder monitor running
 *
 * Return value: %TRUE if @folder has a folder monitor, else %FALSE.
 **/
gboolean
lunar_folder_has_folder_monitor (const LunarFolder *folder)
{
  _lunar_return_val_if_fail (LUNAR_IS_FOLDER (folder), FALSE);
  return (folder->monitor != NULL);
}



/**
 * lunar_folder_reload:
 * @folder : a #LunarFolder instance.
 * @reload_info : reload all information for the files too
 *
 * Tells the @folder object to reread the directory
 * contents from the underlying media.
 **/
void
lunar_folder_reload (LunarFolder *folder,
                      gboolean      reload_info)
{
  _lunar_return_if_fail (LUNAR_IS_FOLDER (folder));

  /* reload file info too? */
  folder->reload_info = reload_info;

  /* stop metadata collector */
  if (folder->content_type_idle_id != 0)
    g_source_remove (folder->content_type_idle_id);

  /* check if we are currently connect to a job */
  if (G_UNLIKELY (folder->job != NULL))
    {
      /* disconnect from the job */
      g_signal_handlers_disconnect_matched (folder->job, G_SIGNAL_MATCH_DATA, 0, 0, NULL, NULL, folder);
      g_object_unref (folder->job);
      folder->job = NULL;
    }

  /* reset the new_files list */
  lunar_g_file_list_free (folder->new_files);
  folder->new_files = NULL;

  /* start a new job */
  folder->job = lunar_io_jobs_list_directory (lunar_file_get_file (folder->corresponding_file));
  g_signal_connect (folder->job, "error", G_CALLBACK (lunar_folder_error), folder);
  g_signal_connect (folder->job, "finished", G_CALLBACK (lunar_folder_finished), folder);
  g_signal_connect (folder->job, "files-ready", G_CALLBACK (lunar_folder_files_ready), folder);

  /* tell all consumers that we're loading */
  g_object_notify (G_OBJECT (folder), "loading");
}
