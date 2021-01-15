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

#if !defined(LUNARX_INSIDE_LUNARX_H) && !defined(LUNARX_COMPILATION)
#error "Only <lunarx/lunarx.h> can be included directly, this file may disappear or change contents"
#endif

#ifndef __LUNARX_RENAMER_H__
#define __LUNARX_RENAMER_H__

#include <gtk/gtk.h>

#include <lunarx/lunarx-file-info.h>

G_BEGIN_DECLS

typedef struct _LunarxRenamerPrivate LunarxRenamerPrivate;
typedef struct _LunarxRenamerClass   LunarxRenamerClass;
typedef struct _LunarxRenamer        LunarxRenamer;

#define LUNARX_TYPE_RENAMER            (lunarx_renamer_get_type ())
#define LUNARX_RENAMER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_RENAMER, LunarxRenamer))
#define LUNARX_RENAMER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNARX_TYPE_RENAMER, LunarxRenamerClass))
#define LUNARX_IS_RENAMER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_RENAMER))
#define LUNARX_IS_RENAMER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNARX_TYPE_RENAMER))
#define LUNARX_RENAMER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNARX_TYPE_RENAMER, LunarxRenamerClass))

/**
 * LunarxRenamerClass:
 * @process:        see lunarx_renamer_process().
 * @load:           see lunarx_renamer_load().
 * @save:           see lunarx_renamer_save().
 * @get_menu_items: see lunarx_renamer_get_menu_items().
 * @changed:        see lunarx_renamer_changed().
 *
 * Abstract base class with virtual methods implemented by extensions
 * that provide additional bulk renamers for the integrated bulk
 * rename module in Lunar.
 *
 */

struct _LunarxRenamerClass
{
  /*< private >*/
  GtkVBoxClass __parent__;

  /*< public >*/

  /* virtual methods */
  gchar *(*process)        (LunarxRenamer  *renamer,
                            LunarxFileInfo *file,
                            const gchar     *text,
                            guint            index);

  void   (*load)           (LunarxRenamer  *renamer,
                            GHashTable      *settings);
  void   (*save)           (LunarxRenamer  *renamer,
                            GHashTable      *settings);

  GList *(*get_menu_items) (LunarxRenamer  *renamer,
                            GtkWindow       *window,
                            GList           *files);

  /*< private >*/
  void (*reserved0) (void);
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);

  /*< public >*/

  /* signals */
  void (*changed) (LunarxRenamer *renamer);

  /*< private >*/
  void (*reserved6) (void);
  void (*reserved7) (void);
  void (*reserved8) (void);
  void (*reserved9) (void);
};

struct _LunarxRenamer
{
  /*< private >*/
  GtkVBox                __parent__;
  LunarxRenamerPrivate *priv;
};

GType        lunarx_renamer_get_type       (void) G_GNUC_CONST;

const gchar *lunarx_renamer_get_help_url   (LunarxRenamer   *renamer);
void         lunarx_renamer_set_help_url   (LunarxRenamer   *renamer,
                                             const gchar      *help_url);

const gchar *lunarx_renamer_get_name       (LunarxRenamer   *renamer);
void         lunarx_renamer_set_name       (LunarxRenamer   *renamer,
                                             const gchar      *name);

gchar       *lunarx_renamer_process        (LunarxRenamer   *renamer,
                                             LunarxFileInfo  *file,
                                             const gchar      *text,
                                             guint             index) G_GNUC_MALLOC;

void         lunarx_renamer_load           (LunarxRenamer   *renamer,
                                             GHashTable       *settings);
void         lunarx_renamer_save           (LunarxRenamer   *renamer,
                                             GHashTable       *settings);

GList       *lunarx_renamer_get_menu_items (LunarxRenamer   *renamer,
                                             GtkWindow        *window,
                                             GList            *files) G_GNUC_MALLOC;

void         lunarx_renamer_changed        (LunarxRenamer   *renamer);

G_END_DECLS

#endif /* !__LUNARX_RENAMER_H__ */
