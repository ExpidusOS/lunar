/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
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

#ifndef __LUNAR_USER_H__
#define __LUNAR_USER_H__

#include <glib-object.h>

G_BEGIN_DECLS;

typedef struct _LunarGroupClass LunarGroupClass;
typedef struct _LunarGroup      LunarGroup;

#define LUNAR_TYPE_GROUP            (lunar_group_get_type ())
#define LUNAR_GROUP(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_GROUP, LunarGroup))
#define LUNAR_GROUP_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_GROUP, LunarGroupClass))
#define LUNAR_IS_GROUP(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_GROUP))
#define LUNAR_IS_GROUP_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_GROUP))
#define LUNAR_GROUP_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_GROUP, LunarGroupClass))

GType         lunar_group_get_type  (void) G_GNUC_CONST;

guint32       lunar_group_get_id    (LunarGroup *group);
const gchar  *lunar_group_get_name  (LunarGroup *group);


typedef struct _LunarUserClass LunarUserClass;
typedef struct _LunarUser      LunarUser;

#define LUNAR_TYPE_USER            (lunar_user_get_type ())
#define LUNAR_USER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_USER, LunarUser))
#define LUNAR_USER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_USER, LunarUserClass))
#define LUNAR_IS_USER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_USER))
#define LUNAR_IS_USER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_USER))
#define LUNAR_USER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_USER, LunarUserClass))

GType         lunar_user_get_type          (void) G_GNUC_CONST;

GList        *lunar_user_get_groups        (LunarUser *user);
const gchar  *lunar_user_get_name          (LunarUser *user);
const gchar  *lunar_user_get_real_name     (LunarUser *user);
gboolean      lunar_user_is_me             (LunarUser *user);


typedef struct _LunarUserManagerClass LunarUserManagerClass;
typedef struct _LunarUserManager      LunarUserManager;

#define LUNAR_TYPE_USER_MANAGER            (lunar_user_manager_get_type ())
#define LUNAR_USER_MANAGER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_TYPE_USER_MANAGER, LunarUserManager))
#define LUNAR_USER_MANAGER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_TYPE_USER_MANAGER, LunarUserManagerClass))
#define LUNAR_IS_USER_MANAGER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_TYPE_USER_MANAGER))
#define LUNAR_IS_USER_MANAGER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_TYPE_USER_MANAGER))
#define LUNAR_USER_MANAGER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_TYPE_USER_MANAGER, LunarUserManagerClass))

GType              lunar_user_manager_get_type        (void) G_GNUC_CONST;

LunarUserManager *lunar_user_manager_get_default     (void) G_GNUC_WARN_UNUSED_RESULT;

LunarGroup       *lunar_user_manager_get_group_by_id (LunarUserManager *manager,
                                                        guint32            id) G_GNUC_WARN_UNUSED_RESULT;
LunarUser        *lunar_user_manager_get_user_by_id  (LunarUserManager *manager,
                                                        guint32            id) G_GNUC_WARN_UNUSED_RESULT;

GList             *lunar_user_manager_get_all_groups  (LunarUserManager *manager) G_GNUC_MALLOC G_GNUC_WARN_UNUSED_RESULT;

G_END_DECLS;

#endif /* !__LUNAR_USER_H__ */
