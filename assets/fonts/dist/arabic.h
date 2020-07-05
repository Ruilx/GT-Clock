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
#ifndef _ARABIC_H_
#define _ARABIC_H_

const unsigned char arabic_data [] = {
	 0xff,0xff,0xc7,0xff,0xff,0xff,     /* 0 */
	 0xff,0xff,0x01,0xff,0xff,0xff,     /* 1 */
	 0x01,0x7f,0x7f,0x7f,0x7f,0xff,     /* 2 */
	 0x01,0xef,0x0f,0xef,0x0f,0xff,     /* 3 */
	 0xbb,0x55,0x6d,0x6d,0x7d,0xff,     /* 4 */
	 0xc1,0xbd,0x7d,0xbd,0xc1,0xff,     /* 5 */
	 0x7f,0x7f,0x7f,0x7f,0x01,0xff,     /* 6 */
	 0x0f,0xf3,0xfd,0xf3,0x0f,0xff,     /* 7 */
	 0xe1,0x9f,0x7f,0x9f,0xe1,0xff,     /* 8 */
	 0x0f,0x6f,0x6f,0x6f,0x01,0xff,     /* 9 */
}

void setupFontarabic(fontInfo *font){
	if(font != nullptr){
		font->fontIndex = 36;
		strncpy(font->fontName, "Arabic", 7);
		font->startUnicode = 48;
		font->endUnicode = 58;
		font->blockLength = 6;
		font->logicality = false;
		font->monospace = true;
		font->monoMask = 0xFF;
		font->index = nullptr;
		font->data = arabic_data;
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
Using image: '36_Arabic.png' size (60, 9), unicode from 48 to 57/57, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

