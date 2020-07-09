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
#ifndef _SIMSUN_H_
#define _SIMSUN_H_

static const unsigned char simsun_data [] = {
	 0xff,0xff,0xff,0xff,0xff,0xff,  /* <sp> */
	 0xff,0xff,0x82,0xff,0xff,0xff,     /* ! */
	 0xff,0x1f,0xff,0x1f,0xff,0xff,     /* " */
	 0xdb,0xd0,0x0b,0xd0,0x0b,0xdb,     /* # */
	 0xdb,0xad,0x00,0xb5,0xdb,0xff,     /* $ */
	 0x9f,0x6c,0x93,0xc9,0x36,0xf9,     /* % */
	 0xf1,0x8e,0x62,0x8d,0xfa,0xfe,     /* & */
	 0xff,0xff,0x1f,0xff,0xff,0xff,     /* ' */
	 0xff,0xff,0xc3,0xbd,0x7e,0xff,     /* ( */
	 0x7e,0xbd,0xc3,0xff,0xff,0xff,     /* ) */
	 0xdb,0xe7,0x81,0xe7,0xdb,0xff,     /* * */
	 0xef,0xef,0x01,0xef,0xef,0xff,     /* + */
	 0xff,0xfe,0xf9,0xff,0xff,0xff,     /* , */
	 0xef,0xef,0xef,0xef,0xef,0xff,     /* - */
	 0xff,0xfd,0xff,0xff,0xff,0xff,     /* . */
	 0xfe,0xf9,0xe7,0x9f,0x7f,0xff,     /* / */
	 0x81,0x7e,0x7e,0x7e,0x81,0xff,     /* 0 */
	 0xff,0xbe,0x00,0xfe,0xff,0xff,     /* 1 */
	 0x9c,0x7a,0x76,0x6e,0x9e,0xff,     /* 2 */
	 0xbd,0x7e,0x6e,0x6e,0x91,0xff,     /* 3 */
	 0xf7,0xcb,0xba,0x00,0xfa,0xff,     /* 4 */
	 0x0d,0x6e,0x6e,0x6e,0x71,0xff,     /* 5 */
	 0x81,0x6e,0x6e,0x6e,0xb1,0xff,     /* 6 */
	 0x3f,0x7f,0x60,0x5f,0x3f,0xff,     /* 7 */
	 0x91,0x6e,0x6e,0x6e,0x91,0xff,     /* 8 */
	 0x8d,0x76,0x76,0x76,0x81,0xff,     /* 9 */
	 0xff,0xff,0xc9,0xff,0xff,0xff,     /* : */
	 0xff,0xfe,0xc9,0xff,0xff,0xff,     /* ; */
	 0xff,0xef,0xd7,0xbb,0x7d,0xff,     /* < */
	 0xdb,0xdb,0xdb,0xdb,0xdb,0xff,     /* = */
	 0x7d,0xbb,0xd7,0xef,0xff,0xff,     /* > */
	 0x9f,0x7f,0x72,0x6f,0x9f,0xff,     /* ? */
	 0x81,0x7e,0x62,0x5a,0x86,0xff,     /* @ */
	 0xfe,0x80,0x1b,0x1b,0x80,0xfe,     /* A */
	 0x7e,0x00,0x6e,0x6e,0x91,0xff,     /* B */
	 0x81,0x7e,0x7e,0x7e,0xbd,0xff,     /* C */
	 0x7e,0x00,0x7e,0x7e,0x81,0xff,     /* D */
	 0x7e,0x00,0x6e,0x46,0x3c,0xff,     /* E */
	 0x7e,0x00,0x6e,0x47,0x3f,0xff,     /* F */
	 0x81,0x7e,0x7e,0x76,0x31,0xf7,     /* G */
	 0x7e,0x00,0xef,0xef,0x00,0x7e,     /* H */
	 0x7e,0x7e,0x00,0x7e,0x7e,0xff,     /* I */
	 0xfd,0xfe,0x7e,0x7e,0x01,0x7f,     /* J */
	 0x7e,0x00,0x6e,0xd3,0x3c,0x7e,     /* K */
	 0x7f,0x00,0x7e,0xfe,0xfe,0xfc,     /* L */
	 0x00,0x0f,0xf0,0x0f,0x00,0xff,     /* M */
	 0x7e,0x00,0xce,0x73,0x00,0x7e,     /* N */
	 0x81,0x7e,0x7e,0x7e,0x81,0xff,     /* O */
	 0x7e,0x00,0x6e,0x6f,0x9f,0xff,     /* P */
	 0x83,0x75,0x75,0x78,0x86,0xff,     /* Q */
	 0x7e,0x00,0x6e,0x67,0x98,0xfe,     /* R */
	 0x9d,0x6e,0x6e,0x76,0xb9,0xff,     /* S */
	 0x3f,0x7e,0x00,0x7e,0x3f,0xff,     /* T */
	 0x7f,0x01,0xfe,0xfe,0x01,0x7f,     /* U */
	 0x7f,0x07,0xf8,0xf8,0x07,0x7f,     /* V */
	 0x1f,0xe0,0x1f,0xe0,0x1f,0xff,     /* W */
	 0x7e,0x18,0xe7,0x18,0x7e,0xff,     /* X */
	 0x7f,0x1e,0xe0,0x1e,0x7f,0xff,     /* Y */
	 0x3e,0x78,0x66,0x1e,0x7c,0xff,     /* Z */
	 0xff,0xff,0x00,0x7e,0xff,0xff,     /* [ */
	 0x7f,0x9f,0xe7,0xf9,0xfe,0xff,     /* \ */
	 0xff,0x7e,0x00,0xff,0xff,0xff,     /* ] */
	 0xff,0xbf,0x7f,0xbf,0xff,0xff,     /* ^ */
	 0xfe,0xfe,0xfe,0xfe,0xfe,0xff,     /* _ */
	 0xff,0x7f,0xbf,0xff,0xff,0xff,     /* ` */
	 0xeb,0xd5,0xd5,0xe3,0xfd,0xff,     /* a */
	 0x7f,0x01,0xdd,0xdd,0xe3,0xff,     /* b */
	 0xff,0xe3,0xdd,0xdd,0xcd,0xff,     /* c */
	 0xe3,0xdd,0x5d,0x03,0xfd,0xff,     /* d */
	 0xe3,0xd5,0xd5,0xe5,0xff,0xff,     /* e */
	 0xed,0x81,0x6d,0x6d,0x7f,0xff,     /* f */
	 0xd1,0xaa,0xaa,0x9a,0xbd,0xff,     /* g */
	 0x7d,0x01,0xdf,0xdf,0xe1,0xfd,     /* h */
	 0xff,0xdd,0x41,0xfd,0xff,0xff,     /* i */
	 0xfe,0xfe,0xde,0x41,0xff,0xff,     /* j */
	 0x7d,0x01,0xf5,0xd7,0xc9,0xdd,     /* k */
	 0x7d,0x7d,0x01,0xfd,0xfd,0xff,     /* l */
	 0xc1,0xdf,0xc3,0xdf,0xe1,0xff,     /* m */
	 0xdd,0xc1,0xdd,0xdf,0xe1,0xfd,     /* n */
	 0xff,0xe3,0xdd,0xdd,0xe3,0xff,     /* o */
	 0xde,0xc0,0xda,0xdb,0xe7,0xff,     /* p */
	 0xe7,0xdb,0xda,0xc0,0xfe,0xff,     /* q */
	 0xdd,0xc1,0xed,0xdf,0xdf,0xff,     /* r */
	 0xff,0xed,0xd5,0xd5,0xdb,0xff,     /* s */
	 0xdf,0x03,0xdd,0xfd,0xff,0xff,     /* t */
	 0xdf,0xc3,0xfd,0xfd,0xc3,0xfd,     /* u */
	 0xdf,0xc7,0xf9,0xf9,0xc7,0xdf,     /* v */
	 0xcf,0xf1,0xcf,0xf1,0xcf,0xff,     /* w */
	 0xdd,0xc9,0xf7,0xc9,0xdd,0xff,     /* x */
	 0xde,0xc6,0xd9,0xf3,0xcf,0xdf,     /* y */
	 0xff,0xdd,0xd1,0xcd,0xdd,0xff,     /* z */
	 0xff,0xff,0xe7,0x18,0x7e,0xff,     /* { */
	 0xff,0xff,0x00,0xff,0xff,0xff,     /* | */
	 0xff,0x7e,0x18,0xe7,0xff,0xff,     /* } */
	 0xbf,0x7f,0x7f,0xbf,0xbf,0x7f,     /* ~ */
	 0xef,0xd7,0xbb,0xbb,0x83,0xff, /* <del> */
};

static const FontInfo fontsimsun = {
	32,
	"Simsun",
	32,
	128,
	6,
	false,
	true,
	0xFF,
	nullptr,
	simsun_data,
};

/*
   Char Width Adjust Index    Total
 '<sp>'   6     +0      0   6 bytes
    '!'   6     +0      1   6 bytes
    '"'   6     +0      2   6 bytes
    '#'   6     +0      3   6 bytes
    '$'   6     +0      4   6 bytes
    '%'   6     +0      5   6 bytes
    '&'   6     +0      6   6 bytes
    '''   6     +0      7   6 bytes
    '('   6     +0      8   6 bytes
    ')'   6     +0      9   6 bytes
    '*'   6     +0     10   6 bytes
    '+'   6     +0     11   6 bytes
    ','   6     +0     12   6 bytes
    '-'   6     +0     13   6 bytes
    '.'   6     +0     14   6 bytes
    '/'   6     +0     15   6 bytes
    '0'   6     +0     16   6 bytes
    '1'   6     +0     17   6 bytes
    '2'   6     +0     18   6 bytes
    '3'   6     +0     19   6 bytes
    '4'   6     +0     20   6 bytes
    '5'   6     +0     21   6 bytes
    '6'   6     +0     22   6 bytes
    '7'   6     +0     23   6 bytes
    '8'   6     +0     24   6 bytes
    '9'   6     +0     25   6 bytes
    ':'   6     +0     26   6 bytes
    ';'   6     +0     27   6 bytes
    '<'   6     +0     28   6 bytes
    '='   6     +0     29   6 bytes
    '>'   6     +0     30   6 bytes
    '?'   6     +0     31   6 bytes
    '@'   6     +0     32   6 bytes
    'A'   6     +0     33   6 bytes
    'B'   6     +0     34   6 bytes
    'C'   6     +0     35   6 bytes
    'D'   6     +0     36   6 bytes
    'E'   6     +0     37   6 bytes
    'F'   6     +0     38   6 bytes
    'G'   6     +0     39   6 bytes
    'H'   6     +0     40   6 bytes
    'I'   6     +0     41   6 bytes
    'J'   6     +0     42   6 bytes
    'K'   6     +0     43   6 bytes
    'L'   6     +0     44   6 bytes
    'M'   6     +0     45   6 bytes
    'N'   6     +0     46   6 bytes
    'O'   6     +0     47   6 bytes
    'P'   6     +0     48   6 bytes
    'Q'   6     +0     49   6 bytes
    'R'   6     +0     50   6 bytes
    'S'   6     +0     51   6 bytes
    'T'   6     +0     52   6 bytes
    'U'   6     +0     53   6 bytes
    'V'   6     +0     54   6 bytes
    'W'   6     +0     55   6 bytes
    'X'   6     +0     56   6 bytes
    'Y'   6     +0     57   6 bytes
    'Z'   6     +0     58   6 bytes
    '['   6     +0     59   6 bytes
    '\'   6     +0     60   6 bytes
    ']'   6     +0     61   6 bytes
    '^'   6     +0     62   6 bytes
    '_'   6     +0     63   6 bytes
    '`'   6     +0     64   6 bytes
    'a'   6     +0     65   6 bytes
    'b'   6     +0     66   6 bytes
    'c'   6     +0     67   6 bytes
    'd'   6     +0     68   6 bytes
    'e'   6     +0     69   6 bytes
    'f'   6     +0     70   6 bytes
    'g'   6     +0     71   6 bytes
    'h'   6     +0     72   6 bytes
    'i'   6     +0     73   6 bytes
    'j'   6     +0     74   6 bytes
    'k'   6     +0     75   6 bytes
    'l'   6     +0     76   6 bytes
    'm'   6     +0     77   6 bytes
    'n'   6     +0     78   6 bytes
    'o'   6     +0     79   6 bytes
    'p'   6     +0     80   6 bytes
    'q'   6     +0     81   6 bytes
    'r'   6     +0     82   6 bytes
    's'   6     +0     83   6 bytes
    't'   6     +0     84   6 bytes
    'u'   6     +0     85   6 bytes
    'v'   6     +0     86   6 bytes
    'w'   6     +0     87   6 bytes
    'x'   6     +0     88   6 bytes
    'y'   6     +0     89   6 bytes
    'z'   6     +0     90   6 bytes
    '{'   6     +0     91   6 bytes
    '|'   6     +0     92   6 bytes
    '}'   6     +0     93   6 bytes
    '~'   6     +0     94   6 bytes
'<del>'   6     +0     95   6 bytes
Font Size Status: Index section 0 bytes, Pure Data 576 bytes, Data section 576 bytes, Total 576 bytes.
Using image: '32_Simsun.png' size (576, 9), unicode from 32 to 127/127, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, MONOSPACE
*/
#endif

