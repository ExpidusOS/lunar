/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2005-2006 Benedikt Meurer <benny@expidus.org>
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

#ifndef __LUNAR_APR_IMAGE_PAGE_H__
#define __LUNAR_APR_IMAGE_PAGE_H__

#include <lunar-apr/lunar-apr-abstract-page.h>

G_BEGIN_DECLS;

typedef struct _LunarAprImagePageClass LunarAprImagePageClass;
typedef struct _LunarAprImagePage      LunarAprImagePage;

#define LUNAR_APR_TYPE_IMAGE_PAGE            (lunar_apr_image_page_get_type ())
#define LUNAR_APR_IMAGE_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_APR_TYPE_IMAGE_PAGE, LunarAprImagePage))
#define LUNAR_APR_IMAGE_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_APR_TYPE_IMAGE_PAGE, LunarAprImagePageClass))
#define LUNAR_APR_IS_IMAGE_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_APR_TYPE_IMAGE_PAGE))
#define LUNAR_APR_IS_IMAGE_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_APR_TYPE_IMAGE_PAGE))
#define LUNAR_APR_IMAGE_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_APR_TYPE_IMAGE_PAGE, LunarAprImagePageClass))

GType lunar_apr_image_page_get_type      (void) G_GNUC_CONST;
void  lunar_apr_image_page_register_type (LunarxProviderPlugin *plugin);

G_END_DECLS;

#endif /* !__LUNAR_APR_IMAGE_PAGE_H__ */
