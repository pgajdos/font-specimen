/*
 * font-specimen
 *
 *  Display Font Specimen
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

#ifndef SPECIMEN_H
# define SPECIMEN_H

#include <stdio.h>

typedef enum
{
  SPECIMEN_WATERFALL,
  SPECIMEN_COMPACT
} specimen_type_t;

typedef enum
{
  SCRIPT_SORT_NONE,
  SCRIPT_SORT_ABSOLUTE,
  SCRIPT_SORT_PERCENT
} script_sort_t;

/* width = 0 => width automatic, height = 0 => height automatic */
extern int specimen_write(specimen_type_t type,
                          const char *font,
                          const char *script,
                          FILE *png,
                          int width,
                          int height);
extern int specimen_font_scripts(const char *font,
                                 script_sort_t sort,
                                 const char *scripts[],
                                 double coverages[],
                                 int maxscripts);
extern void specimen_set_debug(int on);

#endif

