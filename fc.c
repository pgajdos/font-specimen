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

#include "fc.h"
#include "error.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void fontconfig_version(char* version, int version_max)
{
  snprintf(version, version_max, "%d.%d.%d", FC_MAJOR, FC_MINOR, FC_REVISION);
  return;
}

FcPattern *fontconfig_get_pattern(const char *pattern)
{
  char sanitized_pattern[2*strlen(pattern)];
  int c1, c2;

  c1 = c2 = 0;
  while (pattern[c1])
  {
    if (pattern[c1] == '-') /* escape dash */
      sanitized_pattern[c2++] = '\\';
    sanitized_pattern[c2++] = pattern[c1++];
  }
  sanitized_pattern[c2] = '\0';

  return FcNameParse((FcChar8*)sanitized_pattern);
}

FcPattern *fontconfig_get_font(FcPattern *pattern)
{
  FcResult r;
  FcPattern *match, *p;

  if (! (p = FcPatternDuplicate(pattern)))
  {
    font_specimen_error("fontconfig: out of memory");
    return NULL;
  }

  if (!FcConfigSubstitute(NULL, p, FcMatchPattern))
  {
    font_specimen_error("fontconfig: out of memory");
    return NULL;
  }
  FcDefaultSubstitute(p);
  match = FcFontMatch(NULL, p, &r);
  FcPatternDestroy(p);

  if (r != FcResultMatch)
  {
    font_specimen_error("fontconfig: match failed");
    return NULL;
  }
  return match;
}

int fontconfig_pattern_set_string(FcPattern *pattern, 
                                   const char *object, 
                                   const char *value)
{
  FcPatternDel(pattern, object);
  if (FcPatternAddString(pattern, object, (FcChar8 *)value) 
        != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot set string to pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_get_string(FcPattern *pattern, 
                                   const char *object,
                                   const char **value)
{
  if (FcPatternGetString(pattern, object, 0, (FcChar8**)value)
        != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot get string from pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_set_integer(FcPattern *pattern, 
                                   const char *object, 
                                   int value)
{
  FcPatternDel(pattern, object);
  if (FcPatternAddInteger(pattern, object, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot set integer to pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_get_integer(FcPattern *pattern, 
                                   const char *object,
                                   int *value)
{
  if (FcPatternGetInteger(pattern, object, 0, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot get integer from pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_set_double(FcPattern *pattern, 
                                   const char *object, 
                                   double value)
{
  FcPatternDel(pattern, object);
  if (FcPatternAddDouble(pattern, object, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot set double to pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_get_double(FcPattern *pattern, 
                                   const char *object,
                                   double *value)
{
  if (FcPatternGetDouble(pattern, object, 0, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot get double from pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_set_bool(FcPattern *pattern, 
                                const char *object,
                                int value)
{
  FcPatternDel(pattern, object);
  if (FcPatternAddBool(pattern, object, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot set boolean to pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_get_bool(FcPattern *pattern, 
                                const char *object,
                                int *value)
{
  if (FcPatternGetBool(pattern, object, 0, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot get boolean from pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_set_charset(FcPattern *pattern, 
                                   FcCharSet *value)
{
  FcPatternDel(pattern, FC_CHARSET);
  if (FcPatternAddCharSet(pattern, FC_CHARSET, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot set charset to pattern");
    return -1;
  }
  return 0;
}

int fontconfig_pattern_get_charset(FcPattern *pattern, 
                                   FcCharSet **value)
{
  if (FcPatternGetCharSet(pattern, FC_CHARSET, 0, value)
           != FcResultMatch)
  {
    font_specimen_error("fontconfig: cannot get charset from pattern");
    return -1;
  }
  return 0;
}

FcPattern *fontconfig_pattern_new(void)
{
  FcPattern *res = FcPatternCreate();
  if (! res)
    font_specimen_error("fontconfig: cannot create pattern");
  return res;
}

FcPattern *fontconfig_pattern_duplicate(FcPattern *orig)
{
  FcPattern *res = FcPatternDuplicate(orig);
  if (! res)
    font_specimen_error("fontconfig: cannot create duplicate pattern");
  return res;
}

void fontconfig_pattern_destroy(FcPattern *pat)
{
  FcPatternDestroy(pat);
  return;
}

/* character set: grid FcTrue, specimen: grid FcFalse */
/* grid FcTrue: keep blanks + add ' ' where character */
/* doesn't exist; of course leave out lines (maps) where */
/* no character exists */
uint32_t fontconfig_chars(FcPattern *pattern, 
                          uint32_t **chars,
                          const char *uinterval, 
                          uinterval_type_t uintype,
                          int grid, 
                          uint32_t maxchars)
{
  uint32_t available, nchars, n;
  FcBlanks *blanks = FcConfigGetBlanks(NULL);
  uint32_t ucs4;
  uint32_t map[FC_CHARSET_MAP_SIZE];
  uint32_t next;

  int i, j, left, right;
  int nlines;

  FcCharSet *charset;
  
  FcPatternGetCharSet(pattern, FC_CHARSET, 0, &charset);

  nlines = 0;
  for (ucs4 = FcCharSetFirstPage (charset, map, &next);
       ucs4 != FC_CHARSET_DONE;
       ucs4 = FcCharSetNextPage (charset, map, &next))
  {
    int i;
    /* for ucs4 == 0, skip i == 0 and i == 4, see below */
    for (i = (ucs4 == 0 ? 1 : 0);
         i < FC_CHARSET_MAP_SIZE;
         ucs4 == 0 && i == 3 ? i = 5 : i++)
      if (map[i] &&
          (!uinterval ||
           unicode_interval_contains(uinterval, uintype, ucs4 + 32*i) ||
           unicode_interval_contains(uinterval, uintype, ucs4 + 32*i + 0x10)))
        nlines++;
  }

  if (grid)
    available = 32*nlines;
  else
    available = FcCharSetCount(charset);

  nchars = available < maxchars ? available : maxchars;
  *chars = (uint32_t*)malloc(nchars*sizeof(FcChar32));

  if (! *chars)
    return 0;

  /* for() code below borrowed from fontconfig-2.10.2/src/fclang.c */
  n = 0;
  for (ucs4 = FcCharSetFirstPage (charset, map, &next);
       ucs4 != FC_CHARSET_DONE;
       ucs4 = FcCharSetNextPage (charset, map, &next))
  {
    /* for some fonts (e. g. Dina) fontconfig reports they contain 
       control characters;  skip them for sure (we get 'wrong unicode 
       character' while converting svg if not) */
    for (i = (ucs4 == 0 ? 1 : 0);
         i < FC_CHARSET_MAP_SIZE;
         ucs4 == 0 && i == 3 ? i = 5 : i++)
    {
      if (map[i])
      {
        left = 0;
        right = 32;
        if (uinterval && !unicode_interval_contains(uinterval, uintype, ucs4 + 32*i))
          left = 16;
        if (uinterval && !unicode_interval_contains(uinterval, uintype, ucs4 + 32*i + 16))
          right = 16;

        for (j = left; j < right; j++)
        {
          if (map[i] & (1 << j))
          {
            FcChar32 ch = ucs4 + 32*i + j;

            if (grid == FcTrue || !FcBlanksIsMember(blanks, ch))
              (*chars)[n++] = ch;

          }
          else if (grid == FcTrue)
            (*chars)[n++] = 0;

          if (n == nchars)
            return n;
        }
      }
    }
  }

  return n;
}

/* take first nchars characters from charset */
/* if script not NULL, then restrict to script interval only */
int fontconfig_generate_sentence(FcPattern *pattern,
                                 const char *script,
                                 uint32_t *sentence,
                                 int maxchars)
{
  uint32_t *chars;
  uint32_t i, available;

  available = fontconfig_chars(pattern, &chars, script, UI_SCRIPT, FcFalse, maxchars);

  if (available < 0)
    return -1;

  for (i = 0; i < available && i < maxchars; i++)
    sentence[i] = chars[i];

  free(chars);
  return i;
}


