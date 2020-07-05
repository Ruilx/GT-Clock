/*
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef _RKANJIPRO_H_
#define _RKANJIPRO_H_

const unsigned char rkanjipro_data [] = {
	 0xc7,0xbb,0xbb,0xbb,0xc7,0xff,     /* 0 */
	 0xef,0xef,0xef,0xef,0xef,0xff,     /* 1 */
	 0xfb,0xbb,0xbb,0xbb,0xfb,0xff,     /* 2 */
	 0x7d,0x6d,0x6d,0x6d,0x7d,0xff,     /* 3 */
	 0x01,0x0b,0x1b,0x6b,0x01,0xff,     /* 4 */
	 0x5d,0x01,0x5d,0x41,0x7d,0xff,     /* 5 */
	 0xed,0x6b,0xaf,0xeb,0xed,0xff,     /* 6 */
	 0xf7,0x01,0xed,0xdd,0xd9,0xff,     /* 7 */
	 0xf9,0x87,0xff,0x0f,0xf1,0xff,     /* 8 */
	 0xdd,0x03,0xdf,0xc1,0xfd,0xff,     /* 9 */
}

void setupFontrkanjipro(fontInfo *font){
	if(font != nullptr){
		font->fontIndex = 19;
		strncpy(font->fontName, "R Kanji Pro", 12);
		font->startUnicode = 48;
		font->endUnicode = 58;
		font->blockLength = 6;
		font->logicality = false;
		font->monospace = true;
		font->monoMask = 0xFF;
		font->index = nullptr;
		font->data = rkanjipro_data;
	}
}

/*
   Char Width Adjust Index    Total
    '0'   6     +0      0   6 bytes
    '1'   6     +0      1   6 bytes
    '2'   6     +0      2   6 bytes
    '3'   6     +0      3   6 bytes
    '4'   6     +0      4   6 bytes
    '5'   6     +0      5   6 bytes
    '6'   6     +0      6   6 bytes
    '7'   6     +0      7   6 bytes
    '8'   6     +0      8   6 bytes
    '9'   6     +0      9   6 bytes
Font Size Status: Index section 0 bytes, Pure Data 60 bytes, Data section 60 bytes, Total 60 bytes.
Using image: '19_RKanjiPro.png' size (60, 9), unicode from 48 to 57/57, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

