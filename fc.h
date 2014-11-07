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

#ifndef FC_H
# define FC_H

#include "unicode.h"

#include <fontconfig/fontconfig.h>
#include <inttypes.h>

void fontconfig_version(char* version, int version_max);
FcPattern *fontconfig_get_pattern(const char *pattern);
FcPattern *fontconfig_get_font(FcPattern *pattern);

int fontconfig_pattern_set_string(FcPattern *pattern, 
                                   const char *object, 
                                   const char *value);
int fontconfig_pattern_get_string(FcPattern *pattern,         
                                   const char *object,
                                   const char **value);
int fontconfig_pattern_set_integer(FcPattern *pattern,
                                   const char *object, 
                                   int value);
int fontconfig_pattern_get_integer(FcPattern *pattern,
                                   const char *object,
                                   int *value);
int fontconfig_pattern_set_double(FcPattern *pattern, 
                                   const char *object,
                                   double value);
int fontconfig_pattern_get_double(FcPattern *pattern, 
                                   const char *object,
                                   double *value);
int fontconfig_pattern_set_bool(FcPattern *pattern,   
                                   const char *object,
                                   int value);
int fontconfig_pattern_get_bool(FcPattern *pattern,  
                                   const char *object,
                                   int *value);
int fontconfig_pattern_set_charset(FcPattern *pattern,   
                                   FcCharSet *value);
int fontconfig_pattern_get_charset(FcPattern *pattern,  
                                   FcCharSet **value);
FcPattern *fontconfig_pattern_new(void);
FcPattern *fontconfig_pattern_duplicate(FcPattern *orig);
void fontconfig_pattern_destroy(FcPattern *pat);
uint32_t fontconfig_chars(FcPattern *pattern,
                          uint32_t **chars,
                          const char *uinterval,
                          uinterval_type_t uintype,
                          int grid,
                          uint32_t maxchars);
int fontconfig_generate_sentence(FcPattern *pattern,
                                 const char *script,
                                 uint32_t *sentence,
                                 int maxchars);

#endif
