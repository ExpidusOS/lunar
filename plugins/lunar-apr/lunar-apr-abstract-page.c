/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <lunar-apr/lunar-apr-abstract-page.h>
#include <lunar-apr/lunar-apr-private.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_FILE,
};

/* Signal identifiers */
enum
{
  FILE_CHANGED,
  LAST_SIGNAL,
};



static void lunar_apr_abstract_page_dispose      (GObject                    *object);
static void lunar_apr_abstract_page_get_property (GObject                    *object,
                                                   guint                       prop_id,
                                                   GValue                     *value,
                                                   GParamSpec                 *pspec);
static void lunar_apr_abstract_page_set_property (GObject                    *object,
                                                   guint                       prop_id,
                                                   const GValue               *value,
                                                   GParamSpec                 *pspec);
static void lunar_apr_abstract_page_file_changed (LunarAprAbstractPage      *abstract_page,
                                                   LunarxFileInfo            *file);



static guint abstract_page_signals[LAST_SIGNAL];



LUNARX_DEFINE_ABSTRACT_TYPE (LunarAprAbstractPage,
                              lunar_apr_abstract_page,
                              LUNARX_TYPE_PROPERTY_PAGE);



static void
lunar_apr_abstract_page_class_init (LunarAprAbstractPageClass *klass)
{
  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->dispose = lunar_apr_abstract_page_dispose;
  gobject_class->get_property = lunar_apr_abstract_page_get_property;
  gobject_class->set_property = lunar_apr_abstract_page_set_property;

  /**
   * LunarAprAbstractPage:file:
   *
   * The #LunarxFileInfo for the file being displayed by
   * this property page.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_FILE,
                                   g_param_spec_object ("file", "file", "file",
                                                        LUNARX_TYPE_FILE_INFO,
                                                        G_PARAM_READWRITE));

  /**
   * LunarAprAbstractPage::file-changed:
   * @abstract_page : a #LunarAprAbstractPage.
   * @file          : a #LunarxFileInfo.
   *
   * Emitted by @abstract_page whenever the associated
   * @file changes.
   **/
  abstract_page_signals[FILE_CHANGED] =
    g_signal_new ("file-changed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_LAST,
                  G_STRUCT_OFFSET (LunarAprAbstractPageClass, file_changed),
                  NULL, NULL,
                  g_cclosure_marshal_VOID__OBJECT,
                  G_TYPE_NONE, 1,
                  LUNARX_TYPE_FILE_INFO);
}



static void
lunar_apr_abstract_page_init (LunarAprAbstractPage *abstract_page)
{
  /* be sure to setup the i18n support for the lunar-apr plugin */
  lunar_apr_i18n_init ();
}



static void
lunar_apr_abstract_page_dispose (GObject *object)
{
  LunarAprAbstractPage *abstract_page = LUNAR_APR_ABSTRACT_PAGE (object);

  /* disconnect from the file */
  lunar_apr_abstract_page_set_file (abstract_page, NULL);

  (*G_OBJECT_CLASS (lunar_apr_abstract_page_parent_class)->dispose) (object);
}



static void
lunar_apr_abstract_page_get_property (GObject    *object,
                                       guint       prop_id,
                                       GValue     *value,
                                       GParamSpec *pspec)
{
  LunarAprAbstractPage *abstract_page = LUNAR_APR_ABSTRACT_PAGE (object);

  switch (prop_id)
    {
    case PROP_FILE:
      g_value_set_object (value, lunar_apr_abstract_page_get_file (abstract_page));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_apr_abstract_page_set_property (GObject      *object,
                                       guint         prop_id,
                                       const GValue *value,
                                       GParamSpec   *pspec)
{
  LunarAprAbstractPage *abstract_page = LUNAR_APR_ABSTRACT_PAGE (object);

  switch (prop_id)
    {
    case PROP_FILE:
      lunar_apr_abstract_page_set_file (abstract_page, g_value_get_object (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_apr_abstract_page_file_changed (LunarAprAbstractPage *abstract_page,
                                       LunarxFileInfo       *file)
{
  /* emit the "file-changed" signal */
  g_signal_emit (G_OBJECT (abstract_page), abstract_page_signals[FILE_CHANGED], 0, file);
}



/**
 * lunar_apr_abstract_page_get_file:
 * @abstract_page : a #LunarAprAbstractPage.
 *
 * Returns the #LunarxFileInfo for the file being
 * displayed by the @abstract_page.
 *
 * Return value: the file for @abstract_page.
 **/
LunarxFileInfo*
lunar_apr_abstract_page_get_file (LunarAprAbstractPage *abstract_page)
{
  g_return_val_if_fail (LUNAR_APR_IS_ABSTRACT_PAGE (abstract_page), NULL);
  return abstract_page->file;
}



/**
 * lunar_apr_abstract_page_set_file:
 * @abstract_page : a #LunarAprAbstractPage.
 * @file          : a #LunarxFileInfo or %NULL.
 *
 * Sets the #LunarxFileInfo being displayed by the
 * @abstract_page to @file. If @file is %NULL, the
 * property will be reset.
 **/
void
lunar_apr_abstract_page_set_file (LunarAprAbstractPage *abstract_page,
                                   LunarxFileInfo       *file)
{
  g_return_if_fail (LUNAR_APR_IS_ABSTRACT_PAGE (abstract_page));
  g_return_if_fail (file == NULL || LUNARX_IS_FILE_INFO (file));

  /* verify that we don't already display that file */
  if (G_UNLIKELY (abstract_page->file == file))
    return;

  /* disconnect from the previous file */
  if (G_UNLIKELY (abstract_page->file != NULL))
    {
      g_signal_handlers_disconnect_by_func (G_OBJECT (abstract_page->file), lunar_apr_abstract_page_file_changed, abstract_page);
      g_object_unref (G_OBJECT (abstract_page->file));
    }

  /* activate the new file */
  abstract_page->file = file;

  /* connect to the new file */
  if (G_LIKELY (file != NULL))
    {
      /* connect "changed" signal and take a reference */
      g_signal_connect_swapped (G_OBJECT (file), "changed", G_CALLBACK (lunar_apr_abstract_page_file_changed), abstract_page);
      g_object_ref (G_OBJECT (file));

      /* update the initial state */
      lunar_apr_abstract_page_file_changed (abstract_page, file);
    }

  /* notify listeners */
  g_object_notify (G_OBJECT (abstract_page), "file");
}





