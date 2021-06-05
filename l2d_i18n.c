
/*
 * This program converts numbers between 0 and one billion of billions, and correctly written in letters, to digits.
 * It works for french but can be translated for all languages for which numbers are as well expressed in (at most) terms of
 * billions, millions, thousands, hundreds, twenty and unity, in that order from beginning to end.
 * For example, in french, 'huit' yields 8, 'mille neuf cents quatre-vingts deux' yields 1982.
 * The program reads for command-line arguments, and writes the result to standard putput.
 * The converter does not control for correct syntax and grammar of numbers.
 * Conversion of uncorrect numbers is unspecified.
 * See https://www.languagesandnumbers.com/numbering-systems/en/ and http://villemin.gerard.free.fr/Wwwgvmm/Numerati/Langue.htm for a lot more.
 */
#include <stdio.h>
#include <string.h>
#include <locale.h>
#include <stdlib.h>
#include <assert.h>
#include <wchar.h>
#include <errno.h>

#define eol "\n"

#define __USE_GNU_GETTEXT
#ifdef  __USE_GNU_GETTEXT
// See https://www.labri.fr/perso/fleury/posts/programming/a-quick-gettext-tutorial.html for more.
#  include <libintl.h>
#  define PACKAGE "l2d_i18n"
#  define LOCALEDIR "./po"
#  define _(String) gettext (String)
#  define gettext_noop(String) String
#  define N_(String) gettext_noop (String)
#else
#  define _(String) (String)
#  define N_(String) (String)
#  define textdomain(Domain)
#  define bindtextdomain(Package, Directory)
#endif

#ifndef TRACE
// Evaluates arguments without any action:
#  define TRACE(...) snprintf (0, 0, __VA_ARGS__)
#else
#  undef TRACE
#  define TRACE(...) fprintf(stderr, __VA_ARGS__)
#endif

#define ADD_OPS \
OP(0) OP(1) OP(2) OP(3) OP(4) OP(5) OP(6) OP(7) OP(8) OP(9) \
OP(10) OP(11) OP(12) OP(13) OP(14) OP(15) OP(16) OP(17) OP(18) OP(19) \
OP(20) OP(30) OP(40) OP(50) OP(60) OP(70) OP(80) OP(90) \
OP(100) OP(500) OP(1000) OP(1000000) OP(1000000000) OP(1000000000000) OP(1000000000000000) OP(1000000000000000000)

enum eToken
{
  IGNORED = -3, AND = -2, UNDEF = -1,
};

static unsigned long long int
convert_to_number (unsigned long long int multiplier, signed long long int *tokens, size_t nb_tokens,
                   size_t first_included, size_t last_excluded, size_t depth)
{
  unsigned long long int ret = 0;

  // Read tokens from RIGHT to LEFT and search for multiplicative groups.
  // A multiplicative group start from a token 'm' (included) directly followed by a smaller value, to the following token (excluded) equal to or larger than 'm'.
  unsigned long long int current_multiplier = 1;        // indicates that we are not in a multiplicative group
  size_t first_index = first_included;  // index of the first token of a current multiplicative group
  for (size_t current_index = first_included; current_index < last_excluded; current_index++)
  {
    // Reads tokens in reverse order, from end to beginning (nb_tokens - current_index - 1).
    // 'read_token' is the value of the token at index 'current_index'
    signed long long int read_token = tokens[nb_tokens - current_index - 1];

    if (current_multiplier == 1 // we are not in a multiplicative group yet
        && nb_tokens > current_index + 1
        && tokens[nb_tokens - current_index - 2] >= 0 && tokens[nb_tokens - current_index - 2] < read_token)
    {                           // We enter a multiplicative group
      current_multiplier = read_token;
      first_index = current_index;
      TRACE ("%zi: + %'llu * %'llu * ..." eol, depth, multiplier, current_multiplier);
    }
    else if (current_multiplier > 1)
    {                           // We already are inside a multiplicatuve group
      if (read_token >= 0)
        TRACE ("%zi: ... %s[%zu] = %'lli" eol, depth + 1, _("token"), nb_tokens - current_index - 1, read_token);
      else
        TRACE ("%zi: ... %s[%zu]" eol, depth + 1, _("token"), nb_tokens - current_index - 1);
      if (nb_tokens == current_index + 1 || (tokens[nb_tokens - current_index - 2] >= 0 // Compare a signed positive value with an unsigned value
                                             && tokens[nb_tokens - current_index - 2] >= current_multiplier))
      {                         // We leave a multiplicative group
        unsigned long long int subgroup =
          convert_to_number (current_multiplier, tokens, nb_tokens, first_index + 1, current_index + 1, depth + 1);
        ret += multiplier * subgroup;
        TRACE ("%zi: + %'llu * (%'llu * %s[%zi,%zi] = %'llu)" eol, depth, multiplier, current_multiplier, _("token"),
               nb_tokens - current_index - 1, nb_tokens - 2 - first_index, subgroup);

        current_multiplier = 1;
        first_index = current_index + 1;
      }
    }
    else if (read_token >= 0)
    {                           // We are not in a multiplicative group
      assert (current_multiplier == 1);
      ret += multiplier * (unsigned long long int) read_token;
      TRACE ("%zi: + %'llu * 1 * %'lli" eol, depth, multiplier, read_token);
    }
  }                             // end for (size_t current_index = first_included; current_index < last_excluded; current_index++)

  TRACE ("%zu: = %'llu" eol, depth, ret);
  return ret;
}

static unsigned long int
get_number_from_tokens (signed long long int *tokens, size_t nb_tokens)
{
  return convert_to_number (1, tokens, nb_tokens, 0, nb_tokens, 1);
}

#define sAND "+"
#define OP(N) {N##LL , #N},
static struct sToken
{
  signed long long int token_id;
  const char *const text;
} Tokens[] = {
  {AND, sAND}, ADD_OPS
};

#undef OP

// Detect attached tokens: quarantotto, cinquantatré, settecentosessantacinquemila, siebenhundertvierundachtzigtausenddreihundertdreiundsechzig
static signed long long int
get_first_token_from_word (const wchar_t *wcword, size_t *token_length)
{
  signed long long int token = UNDEF;
  size_t longest_token_length = 0;
  if (wcslen (wcword))
    for (struct sToken * t = Tokens; t < Tokens + sizeof (Tokens) / sizeof (*Tokens); t++)
    {
      const char *const translated_token_texts = _(t->text);
#ifdef  __USE_GNU_GETTEXT
      // Numbers must have been translated into words.
      if (strcmp (translated_token_texts, t->text) == 0)
        continue;
#endif
      size_t length;
      wchar_t *const wcttts = malloc (((length = mbstowcs (0, translated_token_texts, 0)) + 1) * sizeof (*wcttts));
      assert (wcttts);
      mbstowcs (wcttts, translated_token_texts, length + 1);
      for (const wchar_t *wcttt = wcttts; wcttt && *wcttt;)
      {
        const wchar_t *wcttt_end;
        static wchar_t const separators[] = L";,:|";
        if (!(wcttt_end = wcspbrk (wcttt, separators)))
          wcttt_end = wcttt + wcslen (wcttt);
        if (wcttt_end > wcttt && !wcsncmp (wcword, wcttt, wcttt_end - wcttt)
            && wcttt_end > wcttt + longest_token_length
            && get_first_token_from_word (wcword + (wcttt_end - wcttt), 0) != UNDEF)
        {
          longest_token_length = wcttt_end - wcttt;
          token = t->token_id;
          if (token_length)
          {
            *token_length = wcttt_end - wcttt;
            TRACE ("(\"%ls\"[0;%zu[) ", wcttt, wcttt_end - wcttt);
          }
        }
        wcttt = wcttt_end;
        if (*wcttt)
          wcttt++;              // skip separator
      }                         // end for (const wchar_t *wcttt = wcntext; wcttt && *wcttt;)
      free (wcttts);
    }                           // end for (struct sToken * t = Tokens; t < Tokens + sizeof (Tokens) / sizeof (*Tokens); t++)
  else
  {
    token = IGNORED;
    if (token_length)
      *token_length = 0;
  }
  // end if (wcslen(wcword))

  return token;
}

static unsigned long long int
get_number_from_words (char *const *texts, size_t nb_texts)
{
// Check at compile-time that 'number' can fit into 'signed long long int':
#define OP(number)  \
  while (0) { \
    int __unused[0LL <= ((signed long long int)number##ULL) ? 1 : -1]; \
    (void) __unused; \
  }
  ADD_OPS
#undef OP
    // For xgettext purpose only:
#define OP(N) _(#N);
    while (0)
  {
    _(sAND);
  ADD_OPS}
#undef OP

  int saved_errno = errno;
  TRACE ("%s ", _("Known tokens are:"));
  for (struct sToken * t = Tokens; t < Tokens + sizeof (Tokens) / sizeof (*Tokens); t++)
#ifdef  __USE_GNU_GETTEXT
    // Numbers must have been translated into words.
    if (strcmp (t->text, _(t->text)) != 0)
#endif
    {
      if (t->token_id >= 0)
        TRACE ("[\"%s\" = %'lli]", _(t->text), t->token_id);
      else
        TRACE ("[\"%s\"]", _(t->text));
    }
  TRACE ("\n");

  signed long long int *tokens = 0;
  size_t nb_tokens = 0;
  for (size_t itext = 0; itext < nb_texts; itext++)
  {
    size_t length;
    wchar_t *const wctext = malloc (((length = mbstowcs (0, texts[itext], 0)) + 1) * sizeof (*wctext));
    assert (wctext);
    mbstowcs (wctext, texts[itext], length + 1);
    const wchar_t *wcword;
    wchar_t *next_word;
    for (wchar_t *first_word = wctext; (wcword = wcstok (first_word, L" -,", &next_word)); first_word = 0)
      while (*wcword)
      {
        TRACE ("\"%ls\" : ", wcword);
        size_t token_length = 0;
        signed long long int token = token = get_first_token_from_word (wcword, &token_length);
        wcword += token_length;

        if (token == UNDEF)
        {
          TRACE ("?" eol);
          fprintf (stderr, "%s %ls" eol, _("Unknown token:"), wcword);
          errno = EINVAL;
          return 0;
        }
        else if (token == IGNORED)
          TRACE ("%s" eol, _("Ignored"));
        else
        {
          if (token >= 0)
            TRACE ("= %s[%zu] = %'lli" eol, _("token"), nb_tokens, token);
          else
            TRACE ("= %s[%zu]" eol, _("token"), nb_tokens);
          nb_tokens++;
          tokens = realloc (tokens, nb_tokens * sizeof (*tokens));
          tokens[nb_tokens - 1] = token;
        }
      }                         // end while (*wcword)
    // end for (wchar_t *first_word = wctext; (wcword = wcstok (first_word, L" -,", &next_word)); first_word = 0)
    free (wctext);
  }                             // end for (size_t itext = 0; itext < nb_texts; itext++)

  unsigned long int number = get_number_from_tokens (tokens, nb_tokens);

  free (tokens);

  errno = saved_errno;          // in case errno was modified somewhere but not intercepted.
  return number;
}

int
main (int argc, char **argv)
{
  setlocale (LC_ALL, "");
#ifdef  __USE_GNU_GETTEXT
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

  // Maximum value is 18 446 744 073 709 551 615 for 64 bits system (test with:
  TRACE ("%s: [%'lu ; %'lu]" eol, _("Interval"), (unsigned long int) 0, (unsigned long int) (~0));

  if (argc <= 1)
    return EXIT_FAILURE;
  else
  {
    errno = 0;
    unsigned long long int number = get_number_from_words (argv + 1, argc - 1);
    if (errno)
      return EXIT_FAILURE;
    fprintf (stdout, "%'llu" eol, number);
    return EXIT_SUCCESS;
  }
}
