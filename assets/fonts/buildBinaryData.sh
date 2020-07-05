#!/bin/bash

image_binarization="../../utilities/ImageBinarization/ImageBinarization.py"

mkdir -p dist

python $image_binarization -i 01_RProBold.png      -x  1 -b 3 -d "R Pro Bold"    -s 32 -e 127 -o dist/rprobold.h --special gpl3 --special preprocessflag
python $image_binarization -i 02_RBold.png         -x  2 -b 6 -d "R Bold"        -s 48 -e 57  -o dist/rbold.h --special gpl3 --special preprocessflag
python $image_binarization -i 03_RBoldSerif.png    -x  3 -b 6 -d "R Bold Serif"  -s 48 -e 57  -o dist/rboldserif.h --special gpl3 --special preprocessflag
python $image_binarization -i 04_RCBold.png        -x  4 -b 3 -d "RC Bold"       -s 32 -e 127 -o dist/rcbold.h --special gpl3 --special preprocessflag
python $image_binarization -i 05_RStd.png          -x  5 -b 6 -d "R Std"         -s 48 -e 57  -o dist/rstd.h --special gpl3 --special preprocessflag
python $image_binarization -i 06_RuilxFixedSys.png -x  6 -b 6 -d "RuilxFixedSys" -s 32 -e 127 -o dist/ruilxfixedsys.h --special gpl3 --special preprocessflag
python $image_binarization -i 07_RHSans.png        -x  7 -b 6 -d "RH Sans"       -s 48 -e 57  -o dist/rhsans.h --special gpl3 --special preprocessflag
python $image_binarization -i 08_CasioLCD.png      -x  8 -b 6 -d "Casio LCD"     -s 48 -e 57  -o dist/casiolcd.h --special gpl3 --special preprocessflag
python $image_binarization -i 09_Minecraftia.png   -x  9 -b 3 -d "Minecraftia"   -s 32 -e 127 -o dist/minecraftia.h --special gpl3 --special preprocessflag
python $image_binarization -i 10_RSans.png         -x 10 -b 6 -d "R Sans"        -s 48 -e 57  -o dist/rsans.h --special gpl3 --special preprocessflag
python $image_binarization -i 11_R4511.png         -x 11 -b 6 -d "R4511"         -s 48 -e 57  -o dist/r4511.h --special gpl3 --special preprocessflag
python $image_binarization -i 12_RSansPro.png      -x 12 -b 6 -d "R Sans Pro"    -s 48 -e 57  -o dist/rsanspro.h --special gpl3 --special preprocessflag
python $image_binarization -i 13_RPro.png          -x 13 -b 6 -d "R Pro"         -s 48 -e 57  -o dist/rpro.h --special gpl3 --special preprocessflag
python $image_binarization -i 14_OCRA.png          -x 14 -b 6 -d "OCR A"         -s 32 -e 127 -o dist/ocra.h --special gpl3 --special preprocessflag
python $image_binarization -i 15_RHand.png         -x 15 -b 6 -d "R Hand"        -s 48 -e 57  -o dist/rhand.h --special gpl3 --special preprocessflag
python $image_binarization -i 16_RSMG.png          -x 16 -b 6 -d "R SMG"         -s 32 -e 127 -o dist/rsmg.h --special gpl3 --special preprocessflag
python $image_binarization -i 17_RSMGBold.png      -x 17 -b 6 -d "R SMG bold"    -s 48 -e 57  -o dist/rsmgbold.h --special gpl3 --special preprocessflag
python $image_binarization -i 18_RKanji.png        -x 18 -b 6 -d "R Kanji"       -s 48 -e 57  -o dist/rkanji.h --special gpl3 --special preprocessflag
python $image_binarization -i 19_RKanjiPro.png     -x 19 -b 6 -d "R Kanji Pro"   -s 48 -e 57  -o dist/rkanjipro.h --special gpl3 --special preprocessflag
python $image_binarization -i 20_RHalfBold.png     -x 20 -b 6 -d "R Half Bold"   -s 48 -e 57  -o dist/rhalfbold.h --special gpl3 --special preprocessflag
python $image_binarization -i 21_RLSD.png          -x 21 -b 6 -d "R LSD"         -s 48 -e 57  -o dist/rlsd.h --special gpl3 --special preprocessflag
python $image_binarization -i 22_RBlock.png        -x 22 -b 6 -d "R Block"       -s 48 -e 57  -o dist/rblock.h --special gpl3 --special preprocessflag
python $image_binarization -i 23_RSmall.png        -x 23 -b 6 -d "R Small"       -s 48 -e 57  -o dist/rsmall.h --special gpl3 --special preprocessflag
python $image_binarization -i 24_RSidy.png         -x 24 -b 6 -d "R Sidy"        -s 48 -e 57  -o dist/rsidy.h --special gpl3 --special preprocessflag
python $image_binarization -i 25_RShiftSym.png     -x 25 -b 6 -d "R Shift Sym"   -s 48 -e 57  -o dist/rshiftsym.h --special gpl3 --special preprocessflag
python $image_binarization -i 26_RSq.png           -x 26 -b 6 -d "R Sq"          -s 48 -e 57  -o dist/rsq.h --special gpl3 --special preprocessflag
python $image_binarization -i 27_RSqPro.png        -x 27 -b 6 -d "R Sq Pro"      -s 48 -e 57  -o dist/rsqpro.h --special gpl3 --special preprocessflag
python $image_binarization -i 28_RSqStd.png        -x 28 -b 6 -d "R Sq Std"      -s 48 -e 57  -o dist/rsqstd.h --special gpl3 --special preprocessflag
python $image_binarization -i 29_RSqSerif.png      -x 29 -b 6 -d "R Sq Serif"    -s 48 -e 57  -o dist/rsqserif.h --special gpl3 --special preprocessflag
python $image_binarization -i 30_Georgia.png       -x 30 -b 6 -d "Georgia"       -s 48 -e 57  -o dist/georgia.h --special gpl3 --special preprocessflag
python $image_binarization -i 31_R6H.png           -x 31 -b 6 -d "R 6H"          -s 48 -e 57  -o dist/r6h.h --special gpl3 --special preprocessflag
python $image_binarization -i 32_Simsun.png        -x 32 -b 6 -d "Simsun"        -s 32 -e 127  -o dist/simsun.h --special gpl3 --special preprocessflag
python $image_binarization -i 33_Commodore64.png   -x 33 -b 8 -d "Commodore64"   -s 32 -e 127 -o dist/commodore64.h --special gpl3 --special preprocessflag
python $image_binarization -i 34_NesPixel.png      -x 34 -b 8 -d "NesPixel"      -s 32 -e 127 -o dist/nespixel.h --special gpl3 --special preprocessflag
python $image_binarization -i 35_Devanagari.png    -x 35 -b 6 -d "Devanagari"    -s 48 -e 57  -o dist/devanagari.h --special gpl3 --special preprocessflag
python $image_binarization -i 36_Arabic.png        -x 36 -b 6 -d "Arabic"        -s 48 -e 57  -o dist/arabic.h --special gpl3 --special preprocessflag
python $image_binarization -i 37_Minecraft.png     -x 37 -b 3 -d "Minecraft"     -s 32 -e 127 -o dist/minecraft.h --special gpl3 --special preprocessflag

python $image_binarization -i 64_kana_U+3041~U+30FF.png -x 64 -b 6 -d "Kana6" -s 12353 -e 12543 -o dist/kana6.h --special gpl3 --special preprocessflag
