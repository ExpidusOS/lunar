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

#ifndef __LUNAR_SBR_ENUM_TYPES_H__
#define __LUNAR_SBR_ENUM_TYPES_H__

#include <lunarx/lunarx.h>

G_BEGIN_DECLS;

#define LUNAR_SBR_TYPE_CASE_RENAMER_MODE (lunar_sbr_case_renamer_mode_get_type ())

/**
 * LunarSbrCaseRenamerMode:
 * @LUNAR_SBR_CASE_RENAMER_MODE_LOWER       : convert to lower case.
 * @LUNAR_SBR_CASE_RENAMER_MODE_UPPER       : convert to upper case.
 * @LUNAR_SBR_CASE_RENAMER_MODE_TITLE       : convert to title case.
 * @LUNAR_SBR_CASE_RENAMER_MODE_FIRST_UPPER : convert to First character uppercase.
 *
 * The operation mode for the #LunarSbrCaseRenamer.
 **/
typedef enum
{
  LUNAR_SBR_CASE_RENAMER_MODE_LOWER,
  LUNAR_SBR_CASE_RENAMER_MODE_UPPER,
  LUNAR_SBR_CASE_RENAMER_MODE_TITLE,
  LUNAR_SBR_CASE_RENAMER_MODE_FIRST_UPPER,
} LunarSbrCaseRenamerMode;

GType lunar_sbr_case_renamer_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_SBR_TYPE_INSERT_MODE (lunar_sbr_insert_mode_get_type ())

/**
 * LunarSbrInsertMode:
 * @LUNAR_SBR_INSERT_MODE_INSERT    : insert characters.
 * @LUNAR_SBR_INSERT_MODE_OVERWRITE : overwrite existing characters.
 *
 * The operation mode for the #LunarSbrInsertRenamer.
 **/
typedef enum
{
  LUNAR_SBR_INSERT_MODE_INSERT,
  LUNAR_SBR_INSERT_MODE_OVERWRITE,
} LunarSbrInsertMode;

GType lunar_sbr_insert_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_SBR_TYPE_NUMBER_MODE (lunar_sbr_number_mode_get_type ())

/**
 * LunarSbrNumberMode:
 * @LUNAR_SBR_NUMBER_MODE_123          : 1, 2, 3, ...
 * @LUNAR_SBR_NUMBER_MODE_010203       : 01, 02, 03, ...
 * @LUNAR_SBR_NUMBER_MODE_001002003    : 001, 002, 003, ...
 * @LUNAR_SBR_NUMBER_MODE_000100020003 : 0001, 0002, 0003, ...
 * @LUNAR_SBR_NUMBER_MODE_000100020003 : 00001, 00002, 00003, ...
 * @LUNAR_SBR_NUMBER_MODE_ABC          : a, b, c, ...
 *
 * The numbering mode for the #LunarSbrNumberRenamer.
 **/
typedef enum
{
  LUNAR_SBR_NUMBER_MODE_123,
  LUNAR_SBR_NUMBER_MODE_010203,
  LUNAR_SBR_NUMBER_MODE_001002003,
  LUNAR_SBR_NUMBER_MODE_000100020003,
  LUNAR_SBR_NUMBER_MODE_000010000200003,
  LUNAR_SBR_NUMBER_MODE_ABC,
} LunarSbrNumberMode;

GType lunar_sbr_number_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_SBR_TYPE_OFFSET_MODE (lunar_sbr_offset_mode_get_type ())

/**
 * LunarSbrOffsetMode:
 * @LUNAR_SBR_OFFSET_MODE_LEFT  : offset starting from the left.
 * @LUNAR_SBR_OFFSET_MODE_RIGHT : offset starting from the right.
 *
 * The offset mode for the #LunarSbrInsertRenamer and the
 * #LunarSbrRemoveRenamer.
 **/
typedef enum
{
  LUNAR_SBR_OFFSET_MODE_LEFT,
  LUNAR_SBR_OFFSET_MODE_RIGHT,
} LunarSbrOffsetMode;

GType lunar_sbr_offset_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_SBR_TYPE_TEXT_MODE (lunar_sbr_text_mode_get_type ())

/**
 * LunarSbrTextMode:
 * @LUNAR_SBR_TEXT_MODE_OTN : Old Name - Text - Number
 * @LUNAR_SBR_TEXT_MODE_NTO : Number - Text - Old Name
 * @LUNAR_SBR_TEXT_MODE_TN  : Text - Number
 * @LUNAR_SBR_TEXT_MODE_NT  : Number - Text
 *
 * The text mode for the #LunarSbrNumberRenamer.
 **/
typedef enum
{
  LUNAR_SBR_TEXT_MODE_OTN,
  LUNAR_SBR_TEXT_MODE_NTO,
  LUNAR_SBR_TEXT_MODE_TN,
  LUNAR_SBR_TEXT_MODE_NT,
} LunarSbrTextMode;

GType lunar_sbr_text_mode_get_type (void) G_GNUC_CONST;


#define LUNAR_SBR_TYPE_DATE_MODE (lunar_sbr_date_mode_get_type ())

/**
 * LunarSbrDateMode:
 * @LUNAR_SBR_DATE_MODE_NOW   : Current Time
 * @LUNAR_SBR_DATE_MODE_ATIME : Access Time
 * @LUNAR_SBR_DATE_MODE_MTIME : Modification Time
 * @LUNAR_SBR_DATE_MODE_TAKEN : Picture Taken Time
 *
 * The date mode for the #LunarSbrDateRenamer.
 **/
typedef enum
{
  LUNAR_SBR_DATE_MODE_NOW,
  LUNAR_SBR_DATE_MODE_ATIME,
  LUNAR_SBR_DATE_MODE_MTIME,
#ifdef HAVE_EXIF
  LUNAR_SBR_DATE_MODE_TAKEN,
#endif
} LunarSbrDateMode;

GType lunar_sbr_date_mode_get_type (void) G_GNUC_CONST;


void lunar_sbr_register_enum_types (LunarxProviderPlugin *plugin);

G_END_DECLS;

#endif /* !__LUNAR_SBR_ENUM_TYPES_H__ */
