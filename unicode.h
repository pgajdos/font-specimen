/*
 * font-specimen
 *
 * Display Font Specimen
 *
 * Copyright (C) 2014 Petr Gajdos (pgajdos at suse)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef UNICODE_H
# define UNICODE_H

#include <fontconfig/fontconfig.h>
#include <inttypes.h>

typedef struct
{
  const char *ui_name;
  double coverage;
  uint32_t success;
  uint32_t uinterval_size;
} uinterval_stat_t;

typedef enum
{
  UI_NONE,
  UI_SCRIPT,
  UI_BLOCK,
} uinterval_type_t;

typedef enum
{
  UI_SORT_NONE,
  UI_SORT_ABSOLUTE,
  UI_SORT_PERCENT,
} uinterval_sort_t;

typedef enum
{
  TRNS_NONE,
  TRNS_ROT270
} img_transform_t;

typedef enum
{
  TDIR_L2R = 0,
  TDIR_R2L = 1,
  TDIR_T2B = 2,
  TDIR_B2T = 3
} text_dir_t;


int unicode_interval_statistics(FcPattern *pattern,
                                uinterval_stat_t **stats,
                                uinterval_type_t uintype,
                                uinterval_sort_t sort_type);

int unicode_specimen_sentence(FcPattern *pattern, 
                              const char *wanted_sentence,
                              const char *wanted_script, 
                              int maxlen,
                              text_dir_t *dir, 
                              img_transform_t *transform, 
                              const char **lang,
                              int *random,  
                              uint32_t *ucs4str);

int unicode_interval_contains(const char *uinterval_name,
                              uinterval_type_t type,
                              uint32_t ch);

uint32_t unicode_script_tag(const char *script);
#endif
