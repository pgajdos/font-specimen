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

#include "unicode.h"
#include "fc.h"
#include "error.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct
{ 
  const char *name;
  int is_collection;
} script_t;

typedef struct
{ 
  uint32_t l; /* lower bound */
  uint32_t u; /* upper bound */
  const char *interval_name;
} uinterval_map_t;

static const script_t script_consts[] =
{
  #include "unicode/scripts.txt"
};

static const uinterval_map_t script_map_consts[] =
{
  #include "unicode/scripts-map.txt"
};

static const uinterval_map_t block_map_consts[] =
{
  #include "unicode/blocks-map.txt"
};


#define SCRIPT_SENTENCE_LEN_MIN   4

#define NUM_CONSTS(constsArray)  \
        (int) (sizeof (constsArray) / sizeof (constsArray[0]))
#define NUMSCRIPTS    NUM_CONSTS(script_consts)
#define NUMBLOCKS     NUM_CONSTS(block_map_consts)
#define uinterval_maps(uintype) \
  (uintype == UI_SCRIPT ? script_map_consts : block_map_consts)
#define uinterval_num_maps(uintype) \
  (uintype == UI_SCRIPT ? NUM_CONSTS(script_map_consts) : NUMBLOCKS)
#define uinterval_num(uintype) \
  (uintype == UI_SCRIPT ? NUMSCRIPTS : NUMBLOCKS)
#define uinterval_name(index, uintype) \
  (uintype == UI_SCRIPT ? script_consts[index].name \
                     : block_map_consts[index].interval_name)

typedef struct
{
  const char *sent;
  const char *lang;
} sentence_t;

#define TAG(c1,c2,c3,c4) ((uint32_t)((((uint8_t)(c1))<<24)|(((uint8_t)(c2))<<16)|(((uint8_t)(c3))<<8)|((uint8_t)(c4))))

typedef struct
{
  const char *script;
  uint32_t tag;
  int nsentences;
  text_dir_t dir;
  img_transform_t transform;
  sentence_t *sentences;
} script_data_t;

static const script_data_t script_data_consts[] =
{
  #include "unicode/sentences.txt"
};

double unicode_interval_coverage(FcPattern *pattern,
                                 const char *uinname,
                                 uinterval_type_t uintype,
                                 uint32_t *ch_success,
                                 uint32_t *ui_size)
{
  int m;
  uint32_t c;
  uint32_t uinterval_size, charset_success;

  FcCharSet *charset;

  if (fontconfig_pattern_get_charset(pattern, &charset) < 0)
    return -1.0;

  uinterval_size = charset_success = 0;
  for (m = 0; m < uinterval_num_maps(uintype); m++)
  {
    if (! strcmp(uinname, uinterval_maps(uintype)[m].interval_name))
      for (c = uinterval_maps(uintype)[m].l;
           c < uinterval_maps(uintype)[m].u; c++)
      {
        if (FcCharSetHasChar(charset, c) == FcTrue)
          charset_success++;
        uinterval_size++;
      }
  }
  if (ch_success)
    *ch_success = charset_success;
  if (ui_size)
    *ui_size = uinterval_size;
  return (double)charset_success/(double)uinterval_size*100.0;
}

int unicode_interval_statistics(FcPattern *pattern,
                                uinterval_stat_t **stats,
                                uinterval_type_t uintype,
                                uinterval_sort_t sort_type)
{
  int nintervals = 0;
  int v, s, v2, loop_end;

  const int map_len = uinterval_num(uintype);

  uinterval_stat_t stat;

  FcCharSet *charset;

  *stats = (uinterval_stat_t*)malloc(sizeof(uinterval_stat_t)*map_len);
  if (!*stats)
  {
    font_specimen_error("unicode: out of memory");
    return -1;
  }
  bzero(*stats, sizeof(uinterval_stat_t)*map_len);

  if (fontconfig_pattern_get_charset(pattern, &charset) < 0)
    return -1;

  /* sort intervals according to coverage values in the font */
  for (s = 0; s < map_len; s++)
  {
    stat.ui_name = uinterval_name(s, uintype);
    stat.coverage
       = unicode_interval_coverage(pattern,
                                   stat.ui_name,
                                   uintype,
                                   &stat.success,
                                   &stat.uinterval_size);

    if (stat.coverage < 0)
      return -1;

    if (stat.success == 0)
      continue; /* trow stat away */

    v = 0;
    loop_end = 0;
    while (v < nintervals)
    {
      switch (sort_type)
      {
        case UI_SORT_NONE:
          break;
        case UI_SORT_ABSOLUTE:
          if (stat.success > (*stats)[v].success)
            loop_end = 1;
          break;
        case UI_SORT_PERCENT:
          if (stat.coverage > (*stats)[v].coverage)
            loop_end = 1;
          break;
      }

      if (loop_end)
        break;

      v++;
    }
    for (v2 = nintervals; v2 > v; v2--)
    {
      if (v2 == map_len)
        continue;

      (*stats)[v2] = (*stats)[v2 - 1];
    }

    (*stats)[v] = stat;
    nintervals++;
  }

  return nintervals;
}

int unicode_utf8toucs4(const char *utf8str, uint32_t *ucs4str, int max_len)
{
  const char *s = utf8str;
  int pos;
  int n;
  const int len = strlen(utf8str);

  pos = n = 0;
  while (pos < len && n < max_len)
  {
    pos += FcUtf8ToUcs4(&((FcChar8*)s)[pos], &((FcChar32*)ucs4str)[n],
                        len - pos);
    n++;
  }

  return n;
}

int unicode_coveres_sentence(FcPattern *pattern,
                             uint32_t *sentence,
                             int len,
                             const char *script)
{
  int c;
  FcCharSet *charset;

  if (fontconfig_pattern_get_charset(pattern, &charset) < 0)
    return -1;

  for (c = 0; c < len; c++)
    if (! FcCharSetHasChar(charset, sentence[c]))
      return c;

  return c;
}

int unicode_interval_contains(const char *uinterval_name,
                              uinterval_type_t type,
                              uint32_t ch)
{
  int in;
  const uinterval_map_t *map = uinterval_maps(type);
  for (in = 0; in < uinterval_num_maps(type); in++)
  {
    if (strcmp(map[in].interval_name, uinterval_name) == 0)
    {
      if (map[in].l <= ch && ch <= map[in].u)
        return 1;
    }
    else
    {
      if (map[in].l <= ch && ch <= map[in].u)
        return 0;
    }

  }
  return 0;
}

int unicode_script_exists(const char *script)
{
  int s;
  for (s = 0; s < NUMSCRIPTS; s++)
    if (strcmp(script_consts[s].name, script) == 0)
      return 1;

  return 0;
}

void unicode_script_sentences(const char *script,
                              int *nsentences,
                              sentence_t **sentences,
                              text_dir_t *dir,
                              img_transform_t *transform)
{
  int sd;
  *sentences = NULL;
  *nsentences = 0;
  for (sd = 0; sd < NUM_CONSTS(script_data_consts); sd++)
    if (strcmp(script_data_consts[sd].script, script) == 0)
    {
      *nsentences = script_data_consts[sd].nsentences;
      *sentences = script_data_consts[sd].sentences;
      *dir = script_data_consts[sd].dir;
      if (transform)
        *transform = script_data_consts[sd].transform;
      break;
    }

  return;
}

uint32_t unicode_script_tag(const char *script)
{
  int s;
  for (s = 0; s < NUM_CONSTS(script_data_consts); s++)
    if (strcmp(script_data_consts[s].script, script) == 0)
      return script_data_consts[s].tag;

  return 0;
}

int unicode_specimen_sentence(FcPattern *pattern, 
                              const char *wanted_sentence,
                              const char *wanted_script,
                              int maxlen,
                              text_dir_t *dir, 
                              img_transform_t *transform,
                              const char **lang,
                              int *random,  
                              uint32_t *ucs4str)
{
  int n;

  int s, nsentences;
  sentence_t *sentences;
  int missing; /* first missing character index */
  FcCharSet *charset;

  if (fontconfig_pattern_get_charset(pattern, &charset) < 0)
    return -1;

  if (wanted_sentence)
  {
    n = unicode_utf8toucs4(wanted_sentence,
                          ucs4str, maxlen - 1);

    if ((missing
          = unicode_coveres_sentence(pattern, ucs4str, n, NULL))
         < n)
    {
      if (missing < 0)
        return -1;

      n = fontconfig_generate_sentence(pattern, wanted_script,
                                           ucs4str, maxlen - 1);
      
      if (n < 0)
        return -1;

      *random = 1;
      if (n < SCRIPT_SENTENCE_LEN_MIN)
      { /* there's nearly nothing in charset defined by script */
        n = fontconfig_generate_sentence(pattern, NULL,
                                         ucs4str, maxlen - 1);

        if (n < 0)
          return -1;
 
        /* look for some chars in font's universe */
        *random = 2;
      }

      ucs4str[n] = 0;
      return n;
    }

    ucs4str[n] = 0;
    *random = 0;
    return n;
  }

  if (wanted_script && unicode_script_exists(wanted_script))
  {
    unicode_script_sentences(wanted_script, &nsentences,
                             &sentences, dir, transform);
  }
  else /* no or unknown script */
  {
    n = fontconfig_generate_sentence(pattern, NULL, ucs4str, maxlen - 1);

    if (n < 0)
      return -1; 

    ucs4str[n] = 0;

    *dir = 0;
    *lang = "";
    if (transform)
      *transform = TRNS_NONE;
    *random = 2;
    return n;
  }

  for (s = 0; s < nsentences; s++)
  {
    n = unicode_utf8toucs4(sentences[s].sent, ucs4str, maxlen - 1);
    if ((missing =
          unicode_coveres_sentence(pattern, ucs4str, n,
                                      wanted_script)) == n)
    {
      if (missing < 0)
        return -1;

      /* sentence must have at least one character from requested script */
      /* otherwise it is sentence from other script */
      ucs4str[n] = 0;
      *lang = sentences[s].lang;
      /* dir and transform set in unicode_script_sentences() */
      *random = 0;
      return n;
    }
  }

  n = fontconfig_generate_sentence(pattern, wanted_script,
                                   ucs4str, maxlen - 1);

  if (n < 0)
    return -1;

  *random = 1;
  if (n < SCRIPT_SENTENCE_LEN_MIN)
  { /* there's nearly nothing in charset defined by script */
    n = fontconfig_generate_sentence(pattern, NULL, ucs4str, maxlen - 1);
    /* look for some chars in font's universe */
    *random = 2;
  }

  ucs4str[n] = 0;

  *dir = 0;
  *lang = "";
  if (transform)
    *transform = TRNS_NONE;
  return n;
}

