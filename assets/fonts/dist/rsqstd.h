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
#ifndef _RSQSTD_H_
#define _RSQSTD_H_

static const unsigned char font_rsqstd_data [] = {
	 0xc7,0xbb,0xbb,0xbb,0xc7,0xff,     /* 0 */
	 0xff,0xff,0x83,0xff,0xff,0xff,     /* 1 */
	 0xf3,0xab,0xab,0xab,0xdb,0xff,     /* 2 */
	 0xbb,0xab,0xab,0xab,0xd7,0xff,     /* 3 */
	 0xf7,0xe7,0xd7,0x83,0xf7,0xff,     /* 4 */
	 0x8b,0xab,0xab,0xab,0xf7,0xff,     /* 5 */
	 0xc7,0xab,0xab,0xab,0xf7,0xff,     /* 6 */
	 0xbf,0xbf,0xa3,0x9f,0xbf,0xff,     /* 7 */
	 0xd7,0xab,0xab,0xab,0xd7,0xff,     /* 8 */
	 0xdf,0xab,0xab,0xab,0xc7,0xff,     /* 9 */
};

static const FontInfo font_rsqstd = {
	.fontIndex = 28,
	//.fontName = "R Sq Std",
	.startUnicode = 48,
	.endUnicode = 58,
	.blockLength = 6,
	.logicality = false,
	.monospace = true,
	.numberHeight = 7,
	.index = nullptr,
	.data = rsqstd_data,
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
Using image: '28_RSqStd.png' size (60, 9), unicode from 48 to 57/57, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

