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

#include <lunar-sbr/lunar-sbr-case-renamer.h>
#include <lunar-sbr/lunar-sbr-insert-renamer.h>
#include <lunar-sbr/lunar-sbr-number-renamer.h>
#include <lunar-sbr/lunar-sbr-provider.h>
#include <lunar-sbr/lunar-sbr-remove-renamer.h>
#include <lunar-sbr/lunar-sbr-replace-renamer.h>
#include <lunar-sbr/lunar-sbr-date-renamer.h>



static void   lunar_sbr_provider_renamer_provider_init (LunarxRenamerProviderIface *iface);
static GList *lunar_sbr_provider_get_renamers          (LunarxRenamerProvider      *renamer_provider);



struct _LunarSbrProviderClass
{
  GObjectClass __parent__;
};

struct _LunarSbrProvider
{
  GObject __parent__;
};



LUNARX_DEFINE_TYPE_WITH_CODE (LunarSbrProvider,
                               lunar_sbr_provider,
                               G_TYPE_OBJECT,
                               LUNARX_IMPLEMENT_INTERFACE (LUNARX_TYPE_RENAMER_PROVIDER,
                                                            lunar_sbr_provider_renamer_provider_init));



static void
lunar_sbr_provider_class_init (LunarSbrProviderClass *klass)
{
}



static void
lunar_sbr_provider_renamer_provider_init (LunarxRenamerProviderIface *iface)
{
  iface->get_renamers = lunar_sbr_provider_get_renamers;
}



static void
lunar_sbr_provider_init (LunarSbrProvider *sbr_provider)
{
}



static GList*
lunar_sbr_provider_get_renamers (LunarxRenamerProvider *renamer_provider)
{
  GList *renamers = NULL;

  renamers = g_list_prepend (renamers, lunar_sbr_replace_renamer_new ());
  renamers = g_list_prepend (renamers, lunar_sbr_remove_renamer_new ());
  renamers = g_list_prepend (renamers, lunar_sbr_number_renamer_new ());
  renamers = g_list_prepend (renamers, lunar_sbr_insert_renamer_new ());
  renamers = g_list_prepend (renamers, lunar_sbr_case_renamer_new ());
  renamers = g_list_prepend (renamers, lunar_sbr_date_renamer_new ());

  return renamers;
}
