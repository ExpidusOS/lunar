/* vi:set et ai sw=2 sts=2 ts=2: */
/*-
 * Copyright (c) 2006-2007 Benedikt Meurer <benny@expidus.org>
 * Copyright (c) 2009 Jannis Pohlmann <jannis@expidus.org>
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

#include <endo/endo.h>

#include <lunar/lunar-enum-types.h>



static void                lunar_icon_size_from_zoom_level     (const GValue   *src_value,
                                                                 GValue         *dst_value);
static void                lunar_thumbnail_size_from_icon_size (const GValue   *src_value,
                                                                 GValue         *dst_value);
static LunarIconSize      lunar_zoom_level_to_icon_size       (LunarZoomLevel zoom_level);
static LunarThumbnailSize lunar_icon_size_to_thumbnail_size   (LunarIconSize  icon_size);



GType
lunar_renamer_mode_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_RENAMER_MODE_NAME,   "LUNAR_RENAMER_MODE_NAME",   N_ ("Name only"),       },
        { LUNAR_RENAMER_MODE_SUFFIX, "LUNAR_RENAMER_MODE_SUFFIX", N_ ("Suffix only"),     },
        { LUNAR_RENAMER_MODE_BOTH,   "LUNAR_RENAMER_MODE_BOTH",   N_ ("Name and Suffix"), },
        { 0,                          NULL,                         NULL,                   },
      };

      type = g_enum_register_static (I_("LunarRenamerMode"), values);
    }

  return type;
}



GType
lunar_date_style_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_DATE_STYLE_SIMPLE,   "LUNAR_DATE_STYLE_SIMPLE",   "simple",   },
        { LUNAR_DATE_STYLE_SHORT,    "LUNAR_DATE_STYLE_SHORT",    "short",    },
        { LUNAR_DATE_STYLE_LONG,     "LUNAR_DATE_STYLE_LONG",     "long",     },
        { LUNAR_DATE_STYLE_YYYYMMDD, "LUNAR_DATE_STYLE_YYYYMMDD", "yyyymmdd", },
        { LUNAR_DATE_STYLE_MMDDYYYY, "LUNAR_DATE_STYLE_MMDDYYYY", "mmddyyyy", },
        { LUNAR_DATE_STYLE_DDMMYYYY, "LUNAR_DATE_STYLE_DDMMYYYY", "ddmmyyyy", },
        { LUNAR_DATE_STYLE_CUSTOM,   "LUNAR_DATE_STYLE_CUSTOM",   "custom",   },
        /* to stay backward compartible*/
        { LUNAR_DATE_STYLE_YYYYMMDD, "LUNAR_DATE_STYLE_ISO",      "iso",      },
        { 0,                          NULL,                         NULL,       },
      };

      type = g_enum_register_static (I_("LunarDateStyle"), values);
    }

  return type;
}



GType
lunar_column_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_COLUMN_DATE_ACCESSED, "LUNAR_COLUMN_DATE_ACCESSED", N_ ("Date Accessed"), },
        { LUNAR_COLUMN_DATE_MODIFIED, "LUNAR_COLUMN_DATE_MODIFIED", N_ ("Date Modified"), },
        { LUNAR_COLUMN_GROUP,         "LUNAR_COLUMN_GROUP",         N_ ("Group"),         },
        { LUNAR_COLUMN_MIME_TYPE,     "LUNAR_COLUMN_MIME_TYPE",     N_ ("MIME Type"),     },
        { LUNAR_COLUMN_NAME,          "LUNAR_COLUMN_NAME",          N_ ("Name"),          },
        { LUNAR_COLUMN_OWNER,         "LUNAR_COLUMN_OWNER",         N_ ("Owner"),         },
        { LUNAR_COLUMN_PERMISSIONS,   "LUNAR_COLUMN_PERMISSIONS",   N_ ("Permissions"),   },
        { LUNAR_COLUMN_SIZE,          "LUNAR_COLUMN_SIZE",          N_ ("Size"),          },
        { LUNAR_COLUMN_SIZE_IN_BYTES, "LUNAR_COLUMN_SIZE_IN_BYTES", N_ ("Size in Bytes"), },
        { LUNAR_COLUMN_TYPE,          "LUNAR_COLUMN_TYPE",          N_ ("Type"),          },
        { LUNAR_COLUMN_FILE,          "LUNAR_COLUMN_FILE",          N_ ("File"),          },
        { LUNAR_COLUMN_FILE_NAME,     "LUNAR_COLUMN_FILE_NAME",     N_ ("File Name"),     },
        { 0,                           NULL,                          NULL,                 },
      };

      type = g_enum_register_static (I_("LunarColumn"), values);
    }

  return type;
}



const gchar*
lunar_column_string_from_value (LunarColumn value)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;

  enum_class = g_type_class_ref (LUNAR_TYPE_COLUMN);
  enum_value = g_enum_get_value (enum_class, value);

  g_type_class_unref (enum_class);

  if (enum_value == NULL)
    return NULL;

  return enum_value->value_name;
}



gboolean
lunar_column_value_from_string (const gchar *value_string,
                                 gint        *value)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;

  enum_class = g_type_class_ref (LUNAR_TYPE_COLUMN);
  enum_value = g_enum_get_value_by_name (enum_class, value_string);

  g_type_class_unref (enum_class);

  if (enum_value == NULL)
    return FALSE;

  *value =  enum_value->value;
  return TRUE;
}



GType
lunar_icon_size_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_ICON_SIZE_16,   "LUNAR_ICON_SIZE_16",        "16px",   },
        { LUNAR_ICON_SIZE_24,   "LUNAR_ICON_SIZE_24",        "24px",   },
        { LUNAR_ICON_SIZE_32,   "LUNAR_ICON_SIZE_32",        "32px",   },
        { LUNAR_ICON_SIZE_48,   "LUNAR_ICON_SIZE_48",        "48px",   },
        { LUNAR_ICON_SIZE_64,   "LUNAR_ICON_SIZE_64",        "64px",   },
        { LUNAR_ICON_SIZE_96,   "LUNAR_ICON_SIZE_96",        "96px",   },
        { LUNAR_ICON_SIZE_128,  "LUNAR_ICON_SIZE_128",       "128px",  },
        { LUNAR_ICON_SIZE_160,  "LUNAR_ICON_SIZE_160",       "160px",  },
        { LUNAR_ICON_SIZE_192,  "LUNAR_ICON_SIZE_192",       "192px",  },
        { LUNAR_ICON_SIZE_256,  "LUNAR_ICON_SIZE_256",       "256px",  },
        /* Support of old type-strings for two lunar stable releases. Old strings will be transformed to new ones on write*/
        { LUNAR_ICON_SIZE_16,   "LUNAR_ICON_SIZE_SMALLEST",  "16px",   },
        { LUNAR_ICON_SIZE_24,   "LUNAR_ICON_SIZE_SMALLER",   "24px",   },
        { LUNAR_ICON_SIZE_32,   "LUNAR_ICON_SIZE_SMALL",     "32px",   },
        { LUNAR_ICON_SIZE_48,   "LUNAR_ICON_SIZE_NORMAL",    "48px",   },
        { LUNAR_ICON_SIZE_64,   "LUNAR_ICON_SIZE_LARGE",     "64px",   },
        { LUNAR_ICON_SIZE_96,   "LUNAR_ICON_SIZE_LARGER",    "96px",   },
        { LUNAR_ICON_SIZE_128,  "LUNAR_ICON_SIZE_LARGEST",   "128px",  },
        /* g_value_transform will pick the last value if nothing else matches. So we put the default there */
        /* this is required here, because the names of the enum values have changed since the previous lunar-version*/
        { LUNAR_ICON_SIZE_48,   "*",                          "*",      },
        { 0,                     NULL,                         NULL,     },
      };

      type = g_enum_register_static (I_("LunarIconSize"), values);

      /* register transformation function for LunarIconSize->LunarThumbnailSize */
      g_value_register_transform_func (type, LUNAR_TYPE_THUMBNAIL_SIZE, lunar_thumbnail_size_from_icon_size);
    }

  return type;
}



GType
lunar_recursive_permissions_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_RECURSIVE_PERMISSIONS_ASK,    "LUNAR_RECURSIVE_PERMISSIONS_ASK",    "ask",    },
        { LUNAR_RECURSIVE_PERMISSIONS_ALWAYS, "LUNAR_RECURSIVE_PERMISSIONS_ALWAYS", "always", },
        { LUNAR_RECURSIVE_PERMISSIONS_NEVER,  "LUNAR_RECURSIVE_PERMISSIONS_NEVER",  "never",  },
        { 0,                                   NULL,                                  NULL,     },
      };

      type = g_enum_register_static (I_("LunarRecursivePermissions"), values);
    }

  return type;
}



GType
lunar_zoom_level_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_ZOOM_LEVEL_25_PERCENT,  "LUNAR_ZOOM_LEVEL_25_PERCENT",    "25%",  },
        { LUNAR_ZOOM_LEVEL_38_PERCENT,  "LUNAR_ZOOM_LEVEL_38_PERCENT",    "38%",  },
        { LUNAR_ZOOM_LEVEL_50_PERCENT,  "LUNAR_ZOOM_LEVEL_50_PERCENT",    "50%",  },
        { LUNAR_ZOOM_LEVEL_75_PERCENT,  "LUNAR_ZOOM_LEVEL_75_PERCENT",    "75%",  },
        { LUNAR_ZOOM_LEVEL_100_PERCENT, "LUNAR_ZOOM_LEVEL_100_PERCENT",   "100%", },
        { LUNAR_ZOOM_LEVEL_150_PERCENT, "LUNAR_ZOOM_LEVEL_150_PERCENT",   "150%", },
        { LUNAR_ZOOM_LEVEL_200_PERCENT, "LUNAR_ZOOM_LEVEL_200_PERCENT",   "200%", },
        { LUNAR_ZOOM_LEVEL_250_PERCENT, "LUNAR_ZOOM_LEVEL_250_PERCENT",   "250%", },
        { LUNAR_ZOOM_LEVEL_300_PERCENT, "LUNAR_ZOOM_LEVEL_300_PERCENT",   "300%", },
        { LUNAR_ZOOM_LEVEL_400_PERCENT, "LUNAR_ZOOM_LEVEL_400_PERCENT",   "400%", },
        /* Support of old type-strings for two lunar stable releases. Old strings will be transformed to new ones on write*/
        { LUNAR_ZOOM_LEVEL_25_PERCENT,  "LUNAR_ZOOM_LEVEL_SMALLEST",      "25%",  },
        { LUNAR_ZOOM_LEVEL_38_PERCENT,  "LUNAR_ZOOM_LEVEL_SMALLER",       "38%",  },
        { LUNAR_ZOOM_LEVEL_50_PERCENT,  "LUNAR_ZOOM_LEVEL_SMALL",         "50%",  },
        { LUNAR_ZOOM_LEVEL_75_PERCENT,  "LUNAR_ZOOM_LEVEL_NORMAL",        "75%",  },
        { LUNAR_ZOOM_LEVEL_100_PERCENT, "LUNAR_ZOOM_LEVEL_LARGE",         "100%", },
        { LUNAR_ZOOM_LEVEL_150_PERCENT, "LUNAR_ZOOM_LEVEL_LARGER",        "150%", },
        { LUNAR_ZOOM_LEVEL_200_PERCENT, "LUNAR_ZOOM_LEVEL_LARGEST",       "200%", },
        /* g_value_transform will pick the last value if nothing else matches. So we put the default there */
        /* this is required here, because the names of the enum values have changed since the previous lunar-version*/
        { LUNAR_ZOOM_LEVEL_100_PERCENT, "*",                               "*",    },
        { 0,                             NULL,                              NULL,   },
      };

      type = g_enum_register_static (I_("LunarZoomLevel"), values);

      /* register transformation function for LunarZoomLevel->LunarIconSize */
      g_value_register_transform_func (type, LUNAR_TYPE_ICON_SIZE, lunar_icon_size_from_zoom_level);
    }

  return type;
}



LunarThumbnailSize
lunar_zoom_level_to_thumbnail_size (LunarZoomLevel zoom_level)
{
  LunarIconSize icon_size = lunar_zoom_level_to_icon_size (zoom_level);
  return lunar_icon_size_to_thumbnail_size (icon_size);
}



GType
lunar_thumbnail_mode_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_THUMBNAIL_MODE_NEVER,      "LUNAR_THUMBNAIL_MODE_NEVER",      "never",      },
        { LUNAR_THUMBNAIL_MODE_ONLY_LOCAL, "LUNAR_THUMBNAIL_MODE_ONLY_LOCAL", "only-local", },
        { LUNAR_THUMBNAIL_MODE_ALWAYS,     "LUNAR_THUMBNAIL_MODE_ALWAYS",     "always",     },
        { 0,                                NULL,                               NULL,         },
      };

      type = g_enum_register_static (I_("LunarThumbnailMode"), values);
    }

  return type;
}



GType
lunar_thumbnail_size_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_THUMBNAIL_SIZE_NORMAL,  "LUNAR_THUMBNAIL_SIZE_NORMAL", "normal", },
        { LUNAR_THUMBNAIL_SIZE_LARGE,   "LUNAR_THUMBNAIL_SIZE_LARGE",  "large",  },
        { 0,                             NULL,                           NULL,     },
      };

      type = g_enum_register_static (I_("LunarThumbnailSize"), values);
    }

  return type;
}

const char*
lunar_thumbnail_size_get_nick (LunarThumbnailSize thumbnail_size)
{
  GEnumValue *thumbnail_size_enum_value;

  thumbnail_size_enum_value = g_enum_get_value (g_type_class_ref (LUNAR_TYPE_THUMBNAIL_SIZE), thumbnail_size);
  return thumbnail_size_enum_value->value_nick;
}



GType
lunar_parallel_copy_mode_get_type (void)
{
  static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
      static const GEnumValue values[] =
      {
        { LUNAR_PARALLEL_COPY_MODE_ALWAYS,                  "LUNAR_PARALLEL_COPY_MODE_ALWAYS",                  "always",                  },
        { LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL,              "LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL",              "only-local",              },
        { LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL_SAME_DEVICES, "LUNAR_PARALLEL_COPY_MODE_ONLY_LOCAL_SAME_DEVICES", "only-local-same-devices", },
        { LUNAR_PARALLEL_COPY_MODE_NEVER,                   "LUNAR_PARALLEL_COPY_MODE_NEVER",                   "never",                   },
        { 0,                                                 NULL,                                                NULL,                      },
      };

      type = g_enum_register_static (I_("LunarParallelCopyMode"), values);
    }

  return type;
}


/**
 * lunar_zoom_level_to_icon_size:
 * @zoom_level : a #LunarZoomLevel.
 *
 * Returns the #LunarIconSize corresponding to the @zoom_level.
 *
 * Return value: the #LunarIconSize for @zoom_level.
 **/
static LunarIconSize
lunar_zoom_level_to_icon_size (LunarZoomLevel zoom_level)
{
  switch (zoom_level)
    {
    case LUNAR_ZOOM_LEVEL_25_PERCENT:  return LUNAR_ICON_SIZE_16;
    case LUNAR_ZOOM_LEVEL_38_PERCENT:  return LUNAR_ICON_SIZE_24;
    case LUNAR_ZOOM_LEVEL_50_PERCENT:  return LUNAR_ICON_SIZE_32;
    case LUNAR_ZOOM_LEVEL_75_PERCENT:  return LUNAR_ICON_SIZE_48;
    case LUNAR_ZOOM_LEVEL_100_PERCENT: return LUNAR_ICON_SIZE_64;
    case LUNAR_ZOOM_LEVEL_150_PERCENT: return LUNAR_ICON_SIZE_96;
    case LUNAR_ZOOM_LEVEL_200_PERCENT: return LUNAR_ICON_SIZE_128;
    case LUNAR_ZOOM_LEVEL_250_PERCENT: return LUNAR_ICON_SIZE_160;
    case LUNAR_ZOOM_LEVEL_300_PERCENT: return LUNAR_ICON_SIZE_192;
    case LUNAR_ZOOM_LEVEL_400_PERCENT: return LUNAR_ICON_SIZE_256;
    default:                            return LUNAR_ICON_SIZE_64; // default = 100 %zoom
    }
}



static LunarThumbnailSize
lunar_icon_size_to_thumbnail_size (LunarIconSize icon_size)
{
  if (icon_size > LUNAR_ICON_SIZE_128)
    return LUNAR_THUMBNAIL_SIZE_LARGE;

  return LUNAR_THUMBNAIL_SIZE_NORMAL;
}



static void
lunar_icon_size_from_zoom_level (const GValue *src_value,
                                  GValue       *dst_value)
{
  g_value_set_enum (dst_value, lunar_zoom_level_to_icon_size (g_value_get_enum (src_value)));
}



static void
lunar_thumbnail_size_from_icon_size (const GValue *src_value,
                                      GValue       *dst_value)
{
  g_value_set_enum (dst_value, lunar_icon_size_to_thumbnail_size (g_value_get_enum (src_value)));
}



GType
lunar_job_response_get_type (void)
{
	static GType type = G_TYPE_INVALID;

  if (G_UNLIKELY (type == G_TYPE_INVALID))
    {
	    static const GFlagsValue values[] =
      {
	      { LUNAR_JOB_RESPONSE_YES,         "LUNAR_JOB_RESPONSE_YES",         "yes"         },
	      { LUNAR_JOB_RESPONSE_YES_ALL,     "LUNAR_JOB_RESPONSE_YES_ALL",     "yes-all"     },
	      { LUNAR_JOB_RESPONSE_NO,          "LUNAR_JOB_RESPONSE_NO",          "no"          },
	      { LUNAR_JOB_RESPONSE_CANCEL,      "LUNAR_JOB_RESPONSE_CANCEL",      "cancel"      },
	      { LUNAR_JOB_RESPONSE_NO_ALL,      "LUNAR_JOB_RESPONSE_NO_ALL",      "no-all"      },
	      { LUNAR_JOB_RESPONSE_RETRY,       "LUNAR_JOB_RESPONSE_RETRY",       "retry"       },
	      { LUNAR_JOB_RESPONSE_FORCE,       "LUNAR_JOB_RESPONSE_FORCE",       "force"       },
	      { LUNAR_JOB_RESPONSE_REPLACE,     "LUNAR_JOB_RESPONSE_REPLACE",     "replace"     },
	      { LUNAR_JOB_RESPONSE_REPLACE_ALL, "LUNAR_JOB_RESPONSE_REPLACE_ALL", "replace-all" },
	      { LUNAR_JOB_RESPONSE_SKIP,        "LUNAR_JOB_RESPONSE_SKIP",        "skip"        },
	      { LUNAR_JOB_RESPONSE_SKIP_ALL,    "LUNAR_JOB_RESPONSE_SKIP_ALL",    "skip-all"    },
	      { LUNAR_JOB_RESPONSE_RENAME,      "LUNAR_JOB_RESPONSE_RENAME",      "rename"      },
	      { LUNAR_JOB_RESPONSE_RENAME_ALL,  "LUNAR_JOB_RESPONSE_RENAME_ALL",  "rename-all " },
	      { 0,                               NULL,                              NULL          }
	    };

	    type = g_flags_register_static (I_("LunarJobResponse"), values);
    }

	return type;
}



GType
lunar_file_mode_get_type (void)
{
	static GType type = G_TYPE_INVALID;

	if (type == G_TYPE_INVALID)
    {
	    static const GFlagsValue values[] =
      {
	      { LUNAR_FILE_MODE_SUID,      "LUNAR_FILE_MODE_SUID",      "suid"      },
	      { LUNAR_FILE_MODE_SGID,      "LUNAR_FILE_MODE_SGID",      "sgid"      },
	      { LUNAR_FILE_MODE_STICKY,    "LUNAR_FILE_MODE_STICKY",    "sticky"    },
	      { LUNAR_FILE_MODE_USR_ALL,   "LUNAR_FILE_MODE_USR_ALL",   "usr-all"   },
	      { LUNAR_FILE_MODE_USR_READ,  "LUNAR_FILE_MODE_USR_READ",  "usr-read"  },
	      { LUNAR_FILE_MODE_USR_WRITE, "LUNAR_FILE_MODE_USR_WRITE", "usr-write" },
	      { LUNAR_FILE_MODE_USR_EXEC,  "LUNAR_FILE_MODE_USR_EXEC",  "usr-exec"  },
	      { LUNAR_FILE_MODE_GRP_ALL,   "LUNAR_FILE_MODE_GRP_ALL",   "grp-all"   },
	      { LUNAR_FILE_MODE_GRP_READ,  "LUNAR_FILE_MODE_GRP_READ",  "grp-read"  },
	      { LUNAR_FILE_MODE_GRP_WRITE, "LUNAR_FILE_MODE_GRP_WRITE", "grp-write" },
	      { LUNAR_FILE_MODE_GRP_EXEC,  "LUNAR_FILE_MODE_GRP_EXEC",  "grp-exec"  },
	      { LUNAR_FILE_MODE_OTH_ALL,   "LUNAR_FILE_MODE_OTH_ALL",   "oth-all"   },
	      { LUNAR_FILE_MODE_OTH_READ,  "LUNAR_FILE_MODE_OTH_READ",  "oth-read"  },
	      { LUNAR_FILE_MODE_OTH_WRITE, "LUNAR_FILE_MODE_OTH_WRITE", "oth-write" },
	      { LUNAR_FILE_MODE_OTH_EXEC,  "LUNAR_FILE_MODE_OTH_EXEC",  "oth-exec"  },
	      { 0,                          NULL,                         NULL        }
	    };

      type = g_flags_register_static ("LunarFileMode", values);
    }
	return type;
}
