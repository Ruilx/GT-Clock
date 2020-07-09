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
#ifndef _GEORGIA_H_
#define _GEORGIA_H_

static const unsigned char georgia_data [] = {
	 0xc7,0xbb,0xbb,0xbb,0xc7,0xff,     /* 0 */
	 0xff,0xdb,0x83,0xfb,0xff,0xff,     /* 1 */
	 0xdb,0xb3,0xab,0xab,0xdb,0xff,     /* 2 */
	 0xdb,0xbd,0xad,0xad,0xd3,0xff,     /* 3 */
	 0xf3,0xeb,0xdb,0x81,0xfb,0xff,     /* 4 */
	 0x8b,0xad,0xad,0xad,0xb3,0xff,     /* 5 */
	 0x87,0x5b,0x5b,0x5b,0xe7,0xff,     /* 6 */
	 0x9f,0xb9,0xb7,0xaf,0x9f,0xff,     /* 7 */
	 0xa7,0x5b,0x5b,0x5b,0xa7,0xff,     /* 8 */
	 0xcf,0xb5,0xb5,0xb5,0xc3,0xff,     /* 9 */
};

static const FontInfo fontgeorgia = {
	30,
	"Georgia",
	48,
	58,
	6,
	false,
	true,
	0xFF,
	nullptr,
	georgia_data,
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
Using image: '30_Georgia.png' size (60, 9), unicode from 48 to 57/57, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

