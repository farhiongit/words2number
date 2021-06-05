Words to number converter
=========================

> This program converts words into a number for any language.

Fo instance,

  - *thirteen million, seven hundred and eighty-four thousand, three hundred and sixty-three*,
  - *treize millions sept cent quatre-vingt-quatre mille trois cent soixante-trois* (in french),
  - *tredici milioni settecentottantaquattromila trecentosessantatré* (in italian),
  - *dreizehn Millionen siebenhundertvierundachtzigtausenddreihundertdreiundsechzig* (in german),
  - *trece millones setecientos ochenta y cuatro mil trescientos sesenta y tres* (in spanish),
  - *treze milhões setecentos e oitenta e quatro mil trezentos e sessenta e três* (in portuguese)

will all be converted to 13,784,363.

It works for any language that follows decimal rules and grouping by tens, hundreds, thousands, millions and so on.
See https://www.languagesandnumbers.com/numbering-systems/en/ and http://villemin.gerard.free.fr/Wwwgvmm/Numerati/Langue.htm for a lot more.

To support a specific language, just fill in a `.po` file containing the wording for this language (gettext is used behind the scene) following these steps:

  1. add a unit tests block `TU_ll_CC` in `Makefile` (where `ll` is a language code and `CC` is a country code, as listed by `locale -a`),
  2. run `make` (this will result in an error),
  3. fill in the newly created file `po/ll_CC.po` as needed with the specific words for numbers (in various forms) for the given language, and
  4. rerun `make` (this will be successful if step 3 has been completed correctly).

Look at examples in `Makefile` and `po` for steps 1 and 3.

The coding is simple and straight forward (the C language helps for that).

It converts correctly any syntactically correct number.

But it should be noticed that it would also convert uncorrect wording for numbers (following implementation defined specific rules).

Have fun and just let me know of any irrelevant results.
