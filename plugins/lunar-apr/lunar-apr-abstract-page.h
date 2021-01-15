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

#ifndef __LUNAR_APR_ABSTRACT_PAGE_H__
#define __LUNAR_APR_ABSTRACT_PAGE_H__

#include <lunarx/lunarx.h>

G_BEGIN_DECLS;

typedef struct _LunarAprAbstractPageClass LunarAprAbstractPageClass;
typedef struct _LunarAprAbstractPage      LunarAprAbstractPage;

#define LUNAR_APR_TYPE_ABSTRACT_PAGE            (lunar_apr_abstract_page_get_type ())
#define LUNAR_APR_ABSTRACT_PAGE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), LUNAR_APR_TYPE_ABSTRACT_PAGE, LunarAprAbstractPage))
#define LUNAR_APR_ABSTRACT_PAGE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), LUNAR_APR_TYPE_ABSTRACT_PAGE, LunarAprAbstractPageClass))
#define LUNAR_APR_IS_ABSTRACT_PAGE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), LUNAR_APR_TYPE_ABSTRACT_PAGE))
#define LUNAR_APR_IS_ABSTRACT_PAGE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), LUNAR_APR_TYPE_ABSTRACT_PAGE))
#define LUNAR_APR_ABSTRACT_PAGE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), LUNAR_APR_TYPE_ABSTRACT_PAGE, LunarAprAbstractPageClass))

struct _LunarAprAbstractPageClass
{
  LunarxPropertyPageClass __parent__;

  /* signals */
  void (*file_changed) (LunarAprAbstractPage *abstract_page,
                        LunarxFileInfo       *file);
};

struct _LunarAprAbstractPage
{
  LunarxPropertyPage __parent__;
  LunarxFileInfo    *file;
};

GType            lunar_apr_abstract_page_get_type      (void) G_GNUC_CONST;
void             lunar_apr_abstract_page_register_type (LunarxProviderPlugin *plugin);

LunarxFileInfo *lunar_apr_abstract_page_get_file      (LunarAprAbstractPage *abstract_page);
void             lunar_apr_abstract_page_set_file      (LunarAprAbstractPage *abstract_page,
                                                         LunarxFileInfo       *file);

G_END_DECLS;

#endif /* !__LUNAR_APR_ABSTRACT_PAGE_H__ */
