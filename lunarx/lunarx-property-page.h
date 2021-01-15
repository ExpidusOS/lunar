/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNARX_PROPERTY_PAGE_H__
#define __LUNARX_PROPERTY_PAGE_H__

#include <gtk/gtk.h>

G_BEGIN_DECLS

typedef struct _LunarxPropertyPagePrivate LunarxPropertyPagePrivate;
typedef struct _LunarxPropertyPageClass   LunarxPropertyPageClass;
typedef struct _LunarxPropertyPage        LunarxPropertyPage;

#define LUNARX_TYPE_PROPERTY_PAGE            (lunarx_property_page_get_type ())
#define LUNARX_PROPERTY_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNARX_TYPE_PROPERTY_PAGE, LunarxPropertyPage))
#define LUNARX_PROPERTY_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNARX_TYPE_PROPERTY_PAGE, LunarxPropertyPageClass))
#define LUNARX_IS_PROPERTY_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNARX_TYPE_PROPERTY_PAGE))
#define LUNARX_IS_PROPERTY_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNARX_TYPE_PROPERTY_PAGE))
#define LUNARX_PROPERTY_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNARX_TYPE_PROPERTY_PAGE))

struct _LunarxPropertyPageClass
{
  GtkBinClass __parent__;

  /*< private >*/
  void (*reserved1) (void);
  void (*reserved2) (void);
  void (*reserved3) (void);
  void (*reserved4) (void);
};

struct _LunarxPropertyPage
{
  GtkBin __parent__;

  /*< private >*/
  LunarxPropertyPagePrivate *priv;
};

GType        lunarx_property_page_get_type              (void) G_GNUC_CONST;

GtkWidget   *lunarx_property_page_new                   (const gchar         *label) G_GNUC_MALLOC;
GtkWidget   *lunarx_property_page_new_with_label_widget (GtkWidget           *label_widget) G_GNUC_MALLOC;

const gchar *lunarx_property_page_get_label             (LunarxPropertyPage *property_page);
void         lunarx_property_page_set_label             (LunarxPropertyPage *property_page,
                                                          const gchar         *label);

GtkWidget   *lunarx_property_page_get_label_widget      (LunarxPropertyPage *property_page);
void         lunarx_property_page_set_label_widget      (LunarxPropertyPage *property_page,
                                                          GtkWidget           *label_widget);

G_END_DECLS

#endif /* !__LUNARX_PROPERTY_PAGE_H__ */
