/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_SHORTCUTS_PANE_H__
#define __LUNAR_SHORTCUTS_PANE_H__

#include <lunar/lunar-side-pane.h>

G_BEGIN_DECLS;

typedef struct _LunarShortcutsPaneClass LunarShortcutsPaneClass;
typedef struct _LunarShortcutsPane      LunarShortcutsPane;

#define LUNAR_TYPE_SHORTCUTS_PANE             (lunar_shortcuts_pane_get_type ())
#define LUNAR_SHORTCUTS_PANE(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_SHORTCUTS_PANE, LunarShortcutsPane))
#define LUNAR_SHORTCUTS_PANE_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_SHORTCUTS_PANE, LunarShortcutsPaneClass))
#define LUNAR_IS_SHORTCUTS_PANE(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_SHORTCUTS_PANE))
#define LUNAR_IS_SHORTCUTS_PANE_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_SHORTCUTS_PANE))
#define LUNAR_SHORTCUTS_PANE_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_SHORTCUTS_PANE, LunarShortcutsPaneClass))

GType      lunar_shortcuts_pane_get_type     (void) G_GNUC_CONST;
void       lunar_shortcuts_pane_add_shortcut (LunarShortcutsPane *shortcuts_pane,
                                               LunarFile          *file);

G_END_DECLS;

#endif /* !__LUNAR_SHORTCUTS_PANE_H__ */
