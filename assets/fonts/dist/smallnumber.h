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
#ifndef _SMALLNUMBER_H_
#define _SMALLNUMBER_H_

static const unsigned char font_smallnumber_data [] = {
	 0xc1,0xdd,0xc1,0xff,     /* 0 */
	 0xed,0xc1,0xfd,0xff,     /* 1 */
	 0xd1,0xd5,0xc5,0xff,     /* 2 */
	 0xdd,0xd5,0xc1,0xff,     /* 3 */
	 0xe3,0xfb,0xc1,0xff,     /* 4 */
	 0xc5,0xd5,0xd1,0xff,     /* 5 */
	 0xc1,0xd5,0xd1,0xff,     /* 6 */
	 0xdf,0xdf,0xc1,0xff,     /* 7 */
	 0xc1,0xd5,0xc1,0xff,     /* 8 */
	 0xc5,0xd5,0xc1,0xff,     /* 9 */
};

static const FontInfo font_smallnumber = {
	.fontIndex = 70,
	//.fontName = "SmallNumber",
	.startUnicode = 48,
	.endUnicode = 58,
	.blockLength = 4,
	.logicality = false,
	.monospace = true,
	.numberHeight = 7,
	.index = nullptr,
	.data = font_smallnumber_data,
};

/*
   Char Width Adjust Index    Total
    '0'   4     +0      0   4 bytes
    '1'   4     +0      1   4 bytes
    '2'   4     +0      2   4 bytes
    '3'   4     +0      3   4 bytes
    '4'   4     +0      4   4 bytes
    '5'   4     +0      5   4 bytes
    '6'   4     +0      6   4 bytes
    '7'   4     +0      7   4 bytes
    '8'   4     +0      8   4 bytes
    '9'   4     +0      9   4 bytes
Font Size Status: Index section 0 bytes, Pure Data 40 bytes, Data section 40 bytes, Total 40 bytes.
Using image: '70_smallNumber.png' size (40, 9), unicode from 48 to 57/57, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

