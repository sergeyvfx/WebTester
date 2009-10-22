/**
 * WebTester Server - server of on-line testing system
 *
 * This module contains utf-8 stuff and some other stuff to work with charsets.
 * (Based on the source of X MultiMedia Systems ans ENCA)
 *
 * Copyright 2008 Sergey I. Sharybin <g.ulairi@gmail.com>
 *
 * This program can be distributed under the terms of the GNU GPL.
 * See the file COPYING.
 */

#include "recode.h"
#include "fs.h"

#include <langinfo.h>
#include <stdlib.h>
#include <string.h>
#include <iconv.h>
#include <errno.h>
#include <malloc.h>

#ifdef USE_ENCA
#  include <enca.h>
#endif

#include <math.h>

/**
 * Convert string from one character set to another
 *
 * @param __string - string to convert
 * @param __from - string's character set
 * @param __to - convert string to character set
 * @return converted string
 * @sideeffect allocate memory for output value
 */
static char*
convert_string (const char *__string, const char *__from, const char *__to)
{
  size_t outleft, outsize, length;
  iconv_t cd;
  char *out, *outptr;
  const char *input = (const char *)__string;

  if (!__string)
    {
      return NULL;
    }

  length = strlen (__string);

  if ((cd = iconv_open (__to, __from)) == (iconv_t)-1)
    {
      return strdup (__string);
    }

  outsize = ((length + 3) & ~3) +  1;
  out = malloc (outsize);
  outleft = outsize - 1;
  outptr = out;

retry:
  if (iconv (cd, (char **__restrict)&input, &length,
             &outptr, &outleft) == (size_t)-1)
    {
      int used;
      switch (errno)
       {
         case E2BIG:
           used = outptr - out;
           outsize = (outsize - 1) * 2 + 1;
           out = realloc (out, outsize);
           outptr = out + used;
           outleft = outsize - 1 - used;
          goto retry;

        case EINVAL:
          break;

        case EILSEQ:
          /* Invalid sequence */
          input++;
          length = strlen (input);
          goto retry;

        default:
          /* General convertation error */
          break;
       }
    }

  *outptr = '\0';
  iconv_close (cd);

  return out;
}

/****
 * User's backend
 */

/**
 * Get current system character set
 *
 * @return current character set
 */
const char*
get_current_charset (void)
{
  char *charset = getenv ("CHARSET");

  if (!charset)
    {
      charset = nl_langinfo (CODESET);
    }

  if (!charset)
    {
      charset = "ISO-8859-1";
    }

  return charset;
}

/**
 * Recode string from one character set to another
 *
 * @param __string - string to convert
 * @param __from - string's character set
 * @param __to - recode string to character set
 * @return recoded string
 * @sideeffect allocate memory for output value
 */
char*
recode (const char *__string, const char *__from, const char *__to)
{
  if (!__string || !__from || !__to)
    {
      return NULL;
    }

  if (!strcmp (__from, __to))
    {
      return strdup (__string);
    }

  return convert_string (__string, __from, __to);
}

/**
 * Recode file content from one character set to another
 * NOTE: If file contents NULL-terminator all data after it will be truncated
 *
 * @param __filename - name of file to be recoded
 * @param __from - current file character set
 * @param __to - convert file content to character set
 */
void
recode_file (const char *__filename, const char *__from, const char *__to)
{
  char *buffer, *recoded;

  /* Some checking */
  if (!__filename || !__from || !__to)
    {
      return;
    }

  if (!strcmp (__from, __to))
    {
      return;
    }

  /* Load buffer from file */
  buffer = fload (__filename);

  if (!buffer)
    {
      return;
    }

  /* Recode buffer */
  recoded = recode (buffer, __from, __to);

  if (recoded)
    {
      /* Write new buffer to file */
      fwritebuf (__filename, recoded);
      free (recoded);
    }

  free (buffer);
}

#ifdef USE_ENCA
/**
 * Guess buffer's character set
 *
 * @param __buffer - buffer to guess charset of
 * @param __buflen - length of buffer
 * @param __preferred_language - preferred language
 * @param __fallback - character set to use in case guessing is failed
 * @return buffer's character set
 */
const char*
guess_buffer_cp (const char *__buffer, size_t __buflen,
                 const char *__preferred_language, const char *__fallback)
{
  const double mu = 0.005;  /* derivation in 0 */
  const double m = 15.0;    /* value in infinity */
  const char **languages;
  size_t langcnt, sgnf;
  EncaAnalyser analyser;
  EncaEncoding encoding;
  const char *detected_cp = NULL;
  int i, preferred;

  if (!__buffer)
    {
      return NULL;
    }

  /* The number of significant characters */
  if (!__buflen)
    {
      sgnf = 1;
    }
  else
    {
      sgnf = ceil ((double)__buflen / (__buflen / m + 1.0 / mu));
    }

  /* Get list of all supported languages */
  languages = enca_get_languages (&langcnt);

  /* Some debug logging */
#if 1
  {
    static int logged = 0;

    if (logged == 0)
      {
        char buffer[4096];
        static int len = BUF_SIZE (4096) - 1;

        strcpy (buffer, "ENCA supported languages: ");
        len -= strlen (buffer);
        for (i = 0; i < langcnt; i++)
          {
            strncat (buffer, languages[i], len);
            len -= strlen (languages[i]);

            if (i != langcnt - 1)
              {
                strncat (buffer, " ", len);
                --len;
              }
          }

        LOG ("recode", "%s\n", buffer);
      }

    logged = 1;
  }
#endif

  for (i = 0; i < langcnt; i++)
    {
      /* Is currently processing language preferred for guessing */
      preferred = __preferred_language &&
                    strcasecmp (languages[i], __preferred_language) == 0;

      /* Create new alalyser and set it's options */
      analyser = enca_analyser_alloc (languages[i]);
      enca_set_significant (analyser, sgnf);

      /* Make auto-guessing */
      encoding = enca_analyse_const (analyser, (const unsigned char *)__buffer,
                                     __buflen);

      enca_analyser_free  (analyser);

      if (encoding.charset != ENCA_CS_UNKNOWN)
        {
          /* If guessing succeed store result of guessing */
          /* If currently processing language is preferred stop guessing */

          if (preferred || detected_cp == NULL)
            {
              detected_cp = enca_charset_name (encoding.charset,
                                               ENCA_NAME_STYLE_ICONV);
            }

          if (preferred)
            {
              break;
            }
        }
    }

  free (languages);

  if (!detected_cp)
    {
      detected_cp = __fallback;
    }

  return detected_cp;
}

/**
 * Guess file's character set
 *
 * @param __filename - name of file to guess charset of
 * @param __preferred_language - preferred language
 * @param __fallback - character set to use in case guessing is failed
 * @return file's character set
 */
const char*
guess_file_cp (const char *__filename, const char *__preferred_language,
               const char *__fallback)
{
  char *buffer = fload (__filename);
  const char *detected_cp;

  if (!buffer)
    {
      return NULL;
    }

  detected_cp = guess_buffer_cp (buffer, strlen (buffer), __preferred_language,
                                 __fallback);

  free (buffer);

  return detected_cp;
}
#endif
