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
#ifndef _RLSD_H_
#define _RLSD_H_

static const unsigned char font_rlsd_data [] = {
	 0xff,0x93,0x7d,0x7d,0x93,0xff,     /* 0 */
	 0xff,0xff,0xff,0xff,0x93,0xff,     /* 1 */
	 0xff,0xf3,0x6d,0x6d,0x9f,0xff,     /* 2 */
	 0xff,0xff,0x6d,0x6d,0x93,0xff,     /* 3 */
	 0xff,0x9f,0xef,0xef,0x93,0xff,     /* 4 */
	 0xff,0x9f,0x6d,0x6d,0xf3,0xff,     /* 5 */
	 0xff,0x93,0x6d,0x6d,0xf3,0xff,     /* 6 */
	 0xff,0xff,0x7f,0x7f,0x93,0xff,     /* 7 */
	 0xff,0x93,0x6d,0x6d,0x93,0xff,     /* 8 */
	 0xff,0x9f,0x6d,0x6d,0x93,0xff,     /* 9 */
};

static const FontInfo font_rlsd = {
	.fontIndex = 21,
	//.fontName = "R LSD",
	.startUnicode = 48,
	.endUnicode = 58,
	.blockLength = 6,
	.logicality = false,
	.monospace = true,
	.numberHeight = 7,
	.index = nullptr,
	.data = rlsd_data,
};

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
Using image: '21_RLSD.png' size (60, 9), unicode from 48 to 57/57, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

