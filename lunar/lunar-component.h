/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_COMPONENT_H__
#define __LUNAR_COMPONENT_H__

#include <lunar/lunar-navigator.h>

G_BEGIN_DECLS;

typedef struct _LunarComponentIface LunarComponentIface;
typedef struct _LunarComponent      LunarComponent;

#define LUNAR_TYPE_COMPONENT           (lunar_component_get_type ())
#define LUNAR_COMPONENT(obj)           (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_COMPONENT, LunarComponent))
#define LUNAR_IS_COMPONENT(obj)        (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_COMPONENT))
#define LUNAR_COMPONENT_GET_IFACE(obj) (G_TYPE_INSTANCE_GET_INTERFACE ((obj), LUNAR_TYPE_COMPONENT, LunarComponentIface))

struct _LunarComponentIface
{
  GTypeInterface __parent__;

  /* methods */
  GList        *(*get_selected_files) (LunarComponent *component);
  void          (*set_selected_files) (LunarComponent *component,
                                       GList           *selected_files);
};

GType         lunar_component_get_type           (void) G_GNUC_CONST;

GList        *lunar_component_get_selected_files  (LunarComponent *component);
void          lunar_component_set_selected_files  (LunarComponent *component,
                                                    GList           *selected_files);

void          lunar_component_restore_selection   (LunarComponent *component);

G_END_DECLS;

#endif /* !__LUNAR_COMPONENT_H__ */
