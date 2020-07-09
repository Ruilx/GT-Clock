#ifndef LOGIC_LAYER_STRING_FONTS_H
#define LOGIC_LAYER_STRING_FONTS_H

#include <fonts.h>
#include <list.h>

#define FONTS_ITEM()	LIST_ITEM(logic_fonts, const FontInfo)

FONTS_ITEM() = font_arabic;
FONTS_ITEM() = font_casiolcd;
FONTS_ITEM() = font_commodore64;
FONTS_ITEM() = font_devanagari;
FONTS_ITEM() = font_georgia;
FONTS_ITEM() = font_kana6;
FONTS_ITEM() = font_minecraft;
FONTS_ITEM() = font_minecraftia;
FONTS_ITEM() = font_nespixel;
FONTS_ITEM() = font_ocra;
FONTS_ITEM() = font_r4511;
FONTS_ITEM() = font_r6h;
FONTS_ITEM() = font_rblock;
FONTS_ITEM() = font_rbold;
FONTS_ITEM() = font_rboldserif;
FONTS_ITEM() = font_rcbold;
FONTS_ITEM() = font_rhalfbold;
FONTS_ITEM() = font_rhand;
FONTS_ITEM() = font_rhsans;
FONTS_ITEM() = font_rkanji;
FONTS_ITEM() = font_rkanjipro;
FONTS_ITEM() = font_rlsd;
FONTS_ITEM() = font_rpro;
FONTS_ITEM() = font_rprobold;
FONTS_ITEM() = font_rsans;
FONTS_ITEM() = font_rsanspro;
FONTS_ITEM() = font_rshiftsym;
FONTS_ITEM() = font_rsidy;
FONTS_ITEM() = font_rsmall;
FONTS_ITEM() = font_rsmg;
FONTS_ITEM() = font_rsmgbold;
FONTS_ITEM() = font_rsq;
FONTS_ITEM() = font_rsqpro;
FONTS_ITEM() = font_rsqserif;
FONTS_ITEM() = font_rsqstd;
FONTS_ITEM() = font_rstd;
FONTS_ITEM() = font_ruilxfixedsys;
FONTS_ITEM() = font_simsun;

LIST(logic_fonts, const FontInfo);

#endif // LOGIC_LAYER_STRING_FONTS_H
