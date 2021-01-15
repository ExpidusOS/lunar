/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#include <lunar/lunar-private.h>
#include <lunar/lunar-renamer-pair.h>



static LunarRenamerPair *lunar_renamer_pair_copy (LunarRenamerPair *renamer_pair) G_GNUC_MALLOC;



GType
lunar_renamer_pair_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      type = g_boxed_type_register_static (I_("LunarRenamerPair"),
                                           (GBoxedCopyFunc) lunar_renamer_pair_copy,
                                           (GBoxedFreeFunc) lunar_renamer_pair_free);
    }

  return type;
}



/**
 * lunar_renamer_pair_new:
 * @file : a #LunarFile.
 * @name : the new name for @file.
 *
 * Allocates a new #LunarRenamerPair for the
 * given @file and @name.
 *
 * The caller is responsible to free the returned pair
 * using lunar_renamer_pair_free() when no longer
 * needed.
 *
 * Return value: the newly allocated #LunarRenamerPair.
 **/
LunarRenamerPair*
lunar_renamer_pair_new (LunarFile  *file,
                         const gchar *name)
{
  LunarRenamerPair *renamer_pair;

  _lunar_return_val_if_fail (LUNAR_IS_FILE (file), NULL);
  _lunar_return_val_if_fail (g_utf8_validate (name, -1, NULL), NULL);

  renamer_pair = g_slice_new (LunarRenamerPair);
  renamer_pair->file = LUNAR_FILE (g_object_ref (G_OBJECT (file)));
  renamer_pair->name = g_strdup (name);

  return renamer_pair;
}



/**
 * lunar_renamer_pair_copy:
 * @renamer_pair : a #LunarRenamerPair.
 *
 * Allocates a copy of the specified @renamer_pair.
 *
 * The caller is responsible to free the returned pair
 * using lunar_renamer_pair_free() when no longer
 * needed.
 *
 * Return value: the newly allocated copy of @renamer_pair.
 **/
static LunarRenamerPair*
lunar_renamer_pair_copy (LunarRenamerPair *renamer_pair)
{
  _lunar_return_val_if_fail (renamer_pair != NULL, NULL);
  return lunar_renamer_pair_new (renamer_pair->file, renamer_pair->name);
}



/**
 * lunar_renamer_pair_free:
 * @data : a #LunarRenamerPair.
 *
 * Frees the specified @renamer_pair.
 **/
void
lunar_renamer_pair_free (gpointer data)
{
  LunarRenamerPair *renamer_pair = data;
  if (G_LIKELY (renamer_pair != NULL))
    {
      g_object_unref (G_OBJECT (renamer_pair->file));
      g_free (renamer_pair->name);
      g_slice_free (LunarRenamerPair, renamer_pair);
    }
}



/**
 * lunar_renamer_pair_list_copy:
 * @renamer_pair_list : a #GList of #LunarRenamerPair<!---->s.
 *
 * Takes a deep copy of the @renamer_pair_list.
 *
 * The caller is responsible to free the returned list using
 * lunar_renamer_pair_list_free() when no longer needed.
 *
 * Return value: a deep copy of @renamer_pair_list.
 **/
GList*
lunar_renamer_pair_list_copy (GList *renamer_pair_list)
{
  GList *result = NULL;
  GList *lp;

  for (lp = g_list_last (renamer_pair_list); lp != NULL; lp = lp->prev)
    result = g_list_prepend (result, lunar_renamer_pair_copy (lp->data));

  return result;
}



/**
 * lunar_renamer_pair_list_free:
 * @renamer_pair_list : a #GList of #LunarRenamerPair<!---->s.
 *
 * Releases the @renamer_pair_list and all #LunarRenamerPair<!---->s
 * in the list.
 **/
void
lunar_renamer_pair_list_free (GList *renamer_pair_list)
{
  g_list_free_full (renamer_pair_list, lunar_renamer_pair_free);
}


