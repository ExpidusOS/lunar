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

#ifndef __LUNAR_PREFERENCES_H__
#define __LUNAR_PREFERENCES_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _LunarPreferencesClass LunarPreferencesClass;
typedef struct _LunarPreferences      LunarPreferences;

#define LUNAR_TYPE_PREFERENCES             (lunar_preferences_get_type ())
#define LUNAR_PREFERENCES(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_PREFERENCES, LunarPreferences))
#define LUNAR_PREFERENCES_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_PREFERENCES, LunarPreferencesClass))
#define LUNAR_IS_PREFERENCES(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_PREFERENCES))
#define LUNAR_IS_PREFERENCES_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_PREFERENCES))
#define LUNAR_PREFERENCES_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_PREFERENCES, LunarPreferencesClass))

GType              lunar_preferences_get_type           (void) G_GNUC_CONST;

LunarPreferences *lunar_preferences_get                (void);

void               lunar_preferences_esconf_init_failed (void);

G_END_DECLS;

#endif /* !__LUNAR_PREFERENCES_H__ */
