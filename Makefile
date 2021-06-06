all : TU

l2d_i18n: CFLAGS=-Wall -Werror
l2d_i18n: CFLAGS+=-DLOCALEDIR="\"${PWD}/po\""

po/%.pot: %.c
	@mkdir -p "${@D}"
	$(CPP) -P "$<" | xgettext --force-po --no-location -LC -o - --from-code=UTF-8 - | sed 's/Content-Type: text\/plain; charset=CHARSET/Content-Type: text\/plain; charset=UTF-8/' > "$@"

.PRECIOUS: po/%.po # Don't delete intermediate files .po
po/%.po: po/l2d_i18n.pot
	[ ! -f "$@" ] || msgmerge -N -U "$@" "$<"
	[ -f "$@" ] || msginit -i "$<" -l"$(LOCALE)" -o "$@"
	@touch "$@"

po/%/LC_MESSAGES/l2d_i18n.mo: po/%.po
	@mkdir -p "${@D}"
	msgfmt -o "$@" "$<"

.PHONY: TU
.PHONY: TU_%

## ----- Modify below
l2d_i18n: CFLAGS+=-DTRACE    # comment or uncomment and run 'make -B l2d_i18n' to activate or desactivate traces during execution.

# To support a new language: 1. add a unit tests block TU_ll_CC below (where ll is a language code and CC is a country code),
# 2. run make, 3. fill in the newly created '.po' file as needed, and 4. rerun make.

TU_fr_FR: LOCALE=fr_FR
TU_fr_FR: l2d_i18n po/fr_FR/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="${LOCALE}" "./$<" sept mille deux cent quatre
	LANGUAGE="${LOCALE}" "./$<" treize millions sept cent quatre-vingt-quatre mille trois cent soixante-trois  # 13784363

TU_en_GB: LOCALE=en_GB
TU_en_GB: l2d_i18n po/en_GB/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" seven thousand two hundred and four
	LANGUAGE="$(LOCALE)" "./$<" thirteen million, seven hundred and eighty-four thousand, three hundred and sixty-three  # 13784363

TU_de_DE: LOCALE=de_DE
TU_de_DE: l2d_i18n po/de_DE/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" neunzehnhundertfünfundachtzig
	LANGUAGE="$(LOCALE)" "./$<" tausendzweihundertneunzehn
	LANGUAGE="$(LOCALE)" "./$<" dreizehn Millionen siebenhundertvierundachtzigtausenddreihundertdreiundsechzig  # 13784363

TU_it_IT: LOCALE=it_IT
TU_it_IT: l2d_i18n po/it_IT/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" tredici milioni settecentottantaquattromila trecentosessantatré  # 13784363

TU_es_ES: LOCALE=es_ES
TU_es_ES: l2d_i18n po/es_ES/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" trece millones setecientos ochenta y cuatro mil trescientos sesenta y tres  # 13784363

TU_pt_PT: LOCALE=pt_PT
TU_pt_PT: l2d_i18n po/pt_PT/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" treze milhões setecentos e oitenta e quatro mil trezentos e sessenta e três  # 13784363

TU_dk_DK: LOCALE=dk_DK
TU_dk_DK: l2d_i18n po/dk_DK/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" tretten millioner syv hundrede og fireogfirs tusinde tre hundrede og treogtres  # 13784363

TU_ru_RU: LOCALE=ru_RU
TU_ru_RU: l2d_i18n po/ru_RU/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" три тысячи, четыреста пятьдесят шесть
	LANGUAGE="$(LOCALE)" "./$<" тринадцать миллион, семьсот восемьдесят четыре тысячи, триста шестьдесят три  # 13784363

TU_pl_PL: LOCALE=pl_PL
TU_pl_PL: l2d_i18n po/pl_PL/LC_MESSAGES/l2d_i18n.mo
	LANGUAGE="$(LOCALE)" "./$<" trzynaście miliony siedemset osiemdziesiąt cztery tysiące trzysta sześćdziesiąt trzy


TU: TU_fr_FR TU_it_IT TU_de_DE TU_en_GB TU_es_ES TU_pt_PT TU_dk_DK TU_ru_RU TU_pl_PL

