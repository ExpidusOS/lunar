/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2011 Jannis Pohlmann <jannis@expidus.org>
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

#include <endo/endo.h>

#include <lunar/lunar-gobject-extensions.h>
#include <lunar/lunar-private.h>
#include <lunar/lunar-statusbar.h>



enum
{
  PROP_0,
  PROP_TEXT,
};



static void        lunar_statusbar_set_property          (GObject              *object,
                                                           guint                 prop_id,
                                                           const GValue         *value,
                                                           GParamSpec           *pspec);
static void        lunar_statusbar_set_text              (LunarStatusbar      *statusbar,
                                                           const gchar          *text);


struct _LunarStatusbarClass
{
  GtkStatusbarClass __parent__;
};

struct _LunarStatusbar
{
  GtkStatusbar __parent__;
  guint        context_id;
};



G_DEFINE_TYPE (LunarStatusbar, lunar_statusbar, GTK_TYPE_STATUSBAR)



static void
lunar_statusbar_class_init (LunarStatusbarClass *klass)
{
  static gboolean style_initialized = FALSE;

  GObjectClass *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->set_property = lunar_statusbar_set_property;

  /**
   * LunarStatusbar:text:
   *
   * The main text to be displayed in the statusbar. This property
   * can only be written.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_TEXT,
                                   g_param_spec_string ("text",
                                                        "text",
                                                        "text",
                                                        NULL,
                                                        ENDO_PARAM_WRITABLE));

  if (!style_initialized)
    {
    	  gtk_widget_class_install_style_property (GTK_WIDGET_CLASS (gobject_class),
          g_param_spec_enum (
	          "shadow-type",                //name
	          "shadow-type",                //nick
	          "type of shadow",             //blurb
	          gtk_shadow_type_get_type(),   //type
	          GTK_SHADOW_NONE,              //default
	          G_PARAM_READWRITE ));         //flags
    }
}



static void
lunar_statusbar_init (LunarStatusbar *statusbar)
{
  statusbar->context_id = gtk_statusbar_get_context_id (GTK_STATUSBAR (statusbar), "Main text");

  /* make the status thinner */
  gtk_widget_set_margin_top (GTK_WIDGET (statusbar), 0);
  gtk_widget_set_margin_bottom (GTK_WIDGET (statusbar), 0);
}



static void
lunar_statusbar_set_property (GObject      *object,
                               guint         prop_id,
                               const GValue *value,
                               GParamSpec   *pspec)
{
  LunarStatusbar *statusbar = LUNAR_STATUSBAR (object);

  switch (prop_id)
    {
    case PROP_TEXT:
      lunar_statusbar_set_text (statusbar, g_value_get_string (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



/**
 * lunar_statusbar_set_text:
 * @statusbar : a #LunarStatusbar instance.
 * @text      : the main text to be displayed in @statusbar.
 *
 * Sets up a new main text for @statusbar.
 **/
static void
lunar_statusbar_set_text (LunarStatusbar *statusbar,
                           const gchar     *text)
{
  _lunar_return_if_fail (LUNAR_IS_STATUSBAR (statusbar));
  _lunar_return_if_fail (text != NULL);

  gtk_statusbar_pop (GTK_STATUSBAR (statusbar), statusbar->context_id);
  gtk_statusbar_push (GTK_STATUSBAR (statusbar), statusbar->context_id, text);
}



/**
 * lunar_statusbar_new:
 *
 * Allocates a new #LunarStatusbar instance with no
 * text set.
 *
 * Return value: the newly allocated #LunarStatusbar instance.
 **/
GtkWidget*
lunar_statusbar_new (void)
{
  return g_object_new (LUNAR_TYPE_STATUSBAR, NULL);
}
