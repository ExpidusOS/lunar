/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
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

#include <endo/endo.h>

#include <lunar-sbr/lunar-sbr-case-renamer.h>



/* Property identifiers */
enum
{
  PROP_0,
  PROP_MODE,
};



static void   lunar_sbr_case_renamer_get_property  (GObject                    *object,
                                                     guint                       prop_id,
                                                     GValue                     *value,
                                                     GParamSpec                 *pspec);
static void   lunar_sbr_case_renamer_set_property  (GObject                    *object,
                                                     guint                       prop_id,
                                                     const GValue               *value,
                                                     GParamSpec                 *pspec);
static gchar *lunar_sbr_case_renamer_process       (LunarxRenamer             *renamer,
                                                     LunarxFileInfo            *file,
                                                     const gchar                *text,
                                                     guint                       idx);



struct _LunarSbrCaseRenamerClass
{
  LunarxRenamerClass __parent__;
};

struct _LunarSbrCaseRenamer
{
  LunarxRenamer           __parent__;
  LunarSbrCaseRenamerMode mode;
};



LUNARX_DEFINE_TYPE (LunarSbrCaseRenamer, lunar_sbr_case_renamer, LUNARX_TYPE_RENAMER);



static void
lunar_sbr_case_renamer_class_init (LunarSbrCaseRenamerClass *klass)
{
  LunarxRenamerClass *lunarxrenamer_class;
  GObjectClass        *gobject_class;

  gobject_class = G_OBJECT_CLASS (klass);
  gobject_class->get_property = lunar_sbr_case_renamer_get_property;
  gobject_class->set_property = lunar_sbr_case_renamer_set_property;

  lunarxrenamer_class = LUNARX_RENAMER_CLASS (klass);
  lunarxrenamer_class->process = lunar_sbr_case_renamer_process;

  /**
   * LunarSbrCaseRenamer:mode:
   *
   * The #LunarSbrCaseRenamerMode for this
   * #LunarSbrCaseRenamer instance.
   **/
  g_object_class_install_property (gobject_class,
                                   PROP_MODE,
                                   g_param_spec_enum ("mode", "mode", "mode",
                                                      LUNAR_SBR_TYPE_CASE_RENAMER_MODE,
                                                      LUNAR_SBR_CASE_RENAMER_MODE_LOWER,
                                                      G_PARAM_READWRITE));
}



static void
lunar_sbr_case_renamer_init (LunarSbrCaseRenamer *case_renamer)
{
  AtkRelationSet *relations;
  AtkRelation    *relation;
  GEnumClass     *klass;
  AtkObject      *object;
  GtkWidget      *combo;
  GtkWidget      *label;
  GtkWidget      *hbox;
  guint           n;

  hbox = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 12);
  gtk_box_pack_start (GTK_BOX (case_renamer), hbox, FALSE, FALSE, 0);
  gtk_widget_show (hbox);

  label = gtk_label_new_with_mnemonic (_("Con_vert to:"));
  gtk_box_pack_start (GTK_BOX (hbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  combo = gtk_combo_box_text_new ();
  klass = g_type_class_ref (LUNAR_SBR_TYPE_CASE_RENAMER_MODE);
  for (n = 0; n < klass->n_values; ++n)
    gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (combo), _(klass->values[n].value_nick));
  endo_mutual_binding_new (G_OBJECT (case_renamer), "mode", G_OBJECT (combo), "active");
  gtk_box_pack_start (GTK_BOX (hbox), combo, FALSE, FALSE, 0);
  gtk_label_set_mnemonic_widget (GTK_LABEL (label), combo);
  g_type_class_unref (klass);
  gtk_widget_show (combo);

  /* set Atk label relation for the combo */
  object = gtk_widget_get_accessible (combo);
  relations = atk_object_ref_relation_set (gtk_widget_get_accessible (label));
  relation = atk_relation_new (&object, 1, ATK_RELATION_LABEL_FOR);
  atk_relation_set_add (relations, relation);
  g_object_unref (G_OBJECT (relation));
}



static void
lunar_sbr_case_renamer_get_property (GObject    *object,
                                      guint       prop_id,
                                      GValue     *value,
                                      GParamSpec *pspec)
{
  LunarSbrCaseRenamer *case_renamer = LUNAR_SBR_CASE_RENAMER (object);

  switch (prop_id)
    {
    case PROP_MODE:
      g_value_set_enum (value, lunar_sbr_case_renamer_get_mode (case_renamer));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static void
lunar_sbr_case_renamer_set_property (GObject      *object,
                                      guint         prop_id,
                                      const GValue *value,
                                      GParamSpec   *pspec)
{
  LunarSbrCaseRenamer *case_renamer = LUNAR_SBR_CASE_RENAMER (object);

  switch (prop_id)
    {
    case PROP_MODE:
      lunar_sbr_case_renamer_set_mode (case_renamer, g_value_get_enum (value));
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
    }
}



static gchar*
tscr_utf8_strcase (const gchar *text,
                   gboolean     title_case)
{
  const gchar *t;
  gboolean     upper = TRUE;
  gunichar     c;
  GString     *result;

  /* allocate a new string for the result */
  result = g_string_sized_new (32);

  /* convert the input text */
  for (t = text; *t != '\0'; t = g_utf8_next_char (t))
    {
      /* check the next char */
      c = g_utf8_get_char (t);
      if (title_case && g_unichar_isspace (c))
        {
          upper = TRUE;
        }
      else if (upper && g_unichar_isalpha (c))
        {
          c = g_unichar_toupper (c);
          upper = FALSE;
        }
      else
        {
          c = g_unichar_tolower (c);
        }

      /* append the char to the result */
      g_string_append_unichar (result, c);
    }

  return g_string_free (result, FALSE);
}



static gchar*
lunar_sbr_case_renamer_process (LunarxRenamer  *renamer,
                                 LunarxFileInfo *file,
                                 const gchar     *text,
                                 guint            idx)
{
  LunarSbrCaseRenamer *case_renamer = LUNAR_SBR_CASE_RENAMER (renamer);

  switch (case_renamer->mode)
    {
    case LUNAR_SBR_CASE_RENAMER_MODE_LOWER:
      return g_utf8_strdown (text, -1);

    case LUNAR_SBR_CASE_RENAMER_MODE_UPPER:
      return g_utf8_strup (text, -1);

    case LUNAR_SBR_CASE_RENAMER_MODE_TITLE:
      return tscr_utf8_strcase (text, TRUE);

   case LUNAR_SBR_CASE_RENAMER_MODE_FIRST_UPPER:
      return tscr_utf8_strcase (text, FALSE);

    default:
      g_assert_not_reached ();
      return NULL;
    }
}



/**
 * lunar_sbr_case_renamer_new:
 *
 * Allocates a new #LunarSbrCaseRenamer instance.
 *
 * Return value: the newly allocated #LunarSbrCaseRenamer.
 **/
LunarSbrCaseRenamer*
lunar_sbr_case_renamer_new (void)
{
  return g_object_new (LUNAR_SBR_TYPE_CASE_RENAMER,
                       "name", _("Uppercase / Lowercase"),
                       NULL);
}



/**
 * lunar_sbr_case_renamer_get_mode:
 * @case_renamer : a #LunarSbrCaseRenamer.
 *
 * Returns the current #LunarSbrCaseRenamerMode
 * for the @case_renamer.
 *
 * Return value: the current mode of @case_renamer.
 **/
LunarSbrCaseRenamerMode
lunar_sbr_case_renamer_get_mode (LunarSbrCaseRenamer *case_renamer)
{
  g_return_val_if_fail (LUNAR_SBR_IS_CASE_RENAMER (case_renamer), LUNAR_SBR_CASE_RENAMER_MODE_LOWER);
  return case_renamer->mode;
}



/**
 * lunar_sbr_case_renamer_set_mode:
 * @case_renamer : a #LunarSbrCaseRenamer.
 * @mode         : the new #LunarSbrCaseRenamerMode.
 *
 * Sets the #LunarSbrCaseRenamerMode of the @case_renamer
 * to the specified @mode.
 **/
void
lunar_sbr_case_renamer_set_mode (LunarSbrCaseRenamer    *case_renamer,
                                  LunarSbrCaseRenamerMode mode)
{
  g_return_if_fail (LUNAR_SBR_IS_CASE_RENAMER (case_renamer));

  /* check if we already use that mode */
  if (G_UNLIKELY (case_renamer->mode == mode))
    return;

  /* apply the new mode */
  case_renamer->mode = mode;

  /* notify listeners */
  g_object_notify (G_OBJECT (case_renamer), "mode");

  /* emit the "changed" signal */
  lunarx_renamer_changed (LUNARX_RENAMER (case_renamer));
}


