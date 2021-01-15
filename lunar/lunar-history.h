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

#ifndef __LUNAR_HISTORY_H__
#define __LUNAR_HISTORY_H__

#include <lunar/lunar-file.h>

G_BEGIN_DECLS;

typedef struct _LunarHistoryClass LunarHistoryClass;
typedef struct _LunarHistory      LunarHistory;

#define LUNAR_TYPE_HISTORY             (lunar_history_get_type ())
#define LUNAR_HISTORY(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_HISTORY, LunarHistory))
#define LUNAR_HISTORY_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_HISTORY, LunarHistoryClass))
#define LUNAR_IS_HISTORY(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_HISTORY))
#define LUNAR_IS_HISTORY_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_HISTORY))
#define LUNAR_HISTORY_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_HISTORY, LunarHistoryClass))

typedef enum
{
  LUNAR_HISTORY_MENU_BACK,
  LUNAR_HISTORY_MENU_FORWARD,


} LunarHistoryMenuType;

struct _LunarHistoryClass
{
  GObjectClass __parent__;

  /* external signals */
  void         (*history_changed)   (LunarHistory *history,
                                     const gchar   *initial_text);
};

GType           lunar_history_get_type         (void) G_GNUC_CONST;

LunarHistory  *lunar_history_copy             (LunarHistory         *history);
gboolean        lunar_history_has_back         (LunarHistory         *history);
gboolean        lunar_history_has_forward      (LunarHistory         *history);
LunarFile     *lunar_history_peek_back        (LunarHistory         *history);
LunarFile     *lunar_history_peek_forward     (LunarHistory         *history);
void            lunar_history_action_back      (LunarHistory         *history);
void            lunar_history_action_forward   (LunarHistory         *history);
void            lunar_history_show_menu        (LunarHistory         *history,
                                                 LunarHistoryMenuType  type,
                                                 GtkWidget             *parent);

G_END_DECLS;

#endif /* !__LUNAR_HISTORY_H__ */
