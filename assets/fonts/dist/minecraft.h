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
#ifndef _MINECRAFT_H_
#define _MINECRAFT_H_

const unsigned char minecraft_index [] = {
	 0x00,0x06,  /* <sp> */
	 0x02,0x03,     /* ! */
	 0x03,0x04,     /* " */
	 0x05,0x06,     /* # */
	 0x07,0x06,     /* $ */
	 0x09,0x06,     /* % */
	 0x0B,0x06,     /* & */
	 0x0D,0x02,     /* ' */
	 0x0E,0x04,     /* ( */
	 0x10,0x04,     /* ) */
	 0x12,0x04,     /* * */
	 0x14,0x06,     /* + */
	 0x16,0x02,     /* , */
	 0x17,0x06,     /* - */
	 0x19,0x02,     /* . */
	 0x1A,0x06,     /* / */
	 0x1C,0x06,     /* 0 */
	 0x1E,0x06,     /* 1 */
	 0x20,0x06,     /* 2 */
	 0x22,0x06,     /* 3 */
	 0x24,0x06,     /* 4 */
	 0x26,0x06,     /* 5 */
	 0x28,0x06,     /* 6 */
	 0x2A,0x06,     /* 7 */
	 0x2C,0x06,     /* 8 */
	 0x2E,0x06,     /* 9 */
	 0x30,0x02,     /* : */
	 0x31,0x02,     /* ; */
	 0x32,0x05,     /* < */
	 0x34,0x06,     /* = */
	 0x36,0x05,     /* > */
	 0x38,0x06,     /* ? */
	 0x3A,0x07,     /* @ */
	 0x3D,0x06,     /* A */
	 0x3F,0x06,     /* B */
	 0x41,0x06,     /* C */
	 0x43,0x06,     /* D */
	 0x45,0x06,     /* E */
	 0x47,0x06,     /* F */
	 0x49,0x06,     /* G */
	 0x4B,0x06,     /* H */
	 0x4D,0x04,     /* I */
	 0x4F,0x06,     /* J */
	 0x51,0x06,     /* K */
	 0x53,0x06,     /* L */
	 0x55,0x06,     /* M */
	 0x57,0x06,     /* N */
	 0x59,0x06,     /* O */
	 0x5B,0x06,     /* P */
	 0x5D,0x06,     /* Q */
	 0x5F,0x06,     /* R */
	 0x61,0x06,     /* S */
	 0x63,0x06,     /* T */
	 0x65,0x06,     /* U */
	 0x67,0x06,     /* V */
	 0x69,0x06,     /* W */
	 0x6B,0x06,     /* X */
	 0x6D,0x06,     /* Y */
	 0x6F,0x06,     /* Z */
	 0x71,0x04,     /* [ */
	 0x73,0x06,     /* \ */
	 0x75,0x04,     /* ] */
	 0x77,0x06,     /* ^ */
	 0x79,0x06,     /* _ */
	 0x7B,0x03,     /* ` */
	 0x7C,0x06,     /* a */
	 0x7E,0x06,     /* b */
	 0x80,0x06,     /* c */
	 0x82,0x06,     /* d */
	 0x84,0x06,     /* e */
	 0x86,0x05,     /* f */
	 0x88,0x06,     /* g */
	 0x8A,0x06,     /* h */
	 0x8C,0x02,     /* i */
	 0x8D,0x06,     /* j */
	 0x8F,0x05,     /* k */
	 0x91,0x03,     /* l */
	 0x92,0x06,     /* m */
	 0x94,0x06,     /* n */
	 0x96,0x06,     /* o */
	 0x98,0x06,     /* p */
	 0x9A,0x06,     /* q */
	 0x9C,0x06,     /* r */
	 0x9E,0x06,     /* s */
	 0xA0,0x04,     /* t */
	 0xA2,0x06,     /* u */
	 0xA4,0x06,     /* v */
	 0xA6,0x06,     /* w */
	 0xA8,0x06,     /* x */
	 0xAA,0x06,     /* y */
	 0xAC,0x06,     /* z */
	 0xAE,0x04,     /* { */
	 0xB0,0x02,     /* | */
	 0xB1,0x04,     /* } */
	 0xB3,0x07,     /* ~ */
	 0xB6,0x06, /* <del> */
}

const unsigned char minecraft_data [] = {
	 0xff,0xff,0xff,0xff,0xff,0xff,  /* <sp> */
	 0xff,0x05,0xff,     /* ! */
	 0x3f,0xff,0x3f,0xff,0x00,0x00,     /* " */
	 0xd7,0x01,0xd7,0x01,0xd7,0xff,     /* # */
	 0xdb,0xab,0x29,0xab,0xb7,0xff,     /* $ */
	 0x3d,0xf3,0xef,0x9f,0x79,0xff,     /* % */
	 0xf3,0xad,0x45,0xb3,0xed,0xff,     /* & */
	 0x3f,0xff,0x00,     /* ' */
	 0xc7,0xbb,0x7d,0xff,0x00,0x00,     /* ( */
	 0x7d,0xbb,0xc7,0xff,0x00,0x00,     /* ) */
	 0x5f,0xbf,0x5f,0xff,0x00,0x00,     /* * */
	 0xef,0xef,0x83,0xef,0xef,0xff,     /* + */
	 0xfc,0xff,0x00,     /* , */
	 0xef,0xef,0xef,0xef,0xef,0xff,     /* - */
	 0xfd,0xff,0x00,     /* . */
	 0xfd,0xf3,0xef,0x9f,0x7f,0xff,     /* / */
	 0x83,0x75,0x6d,0x5d,0x83,0xff,     /* 0 */
	 0xfd,0xbd,0x01,0xfd,0xfd,0xff,     /* 1 */
	 0xb9,0x75,0x6d,0x6d,0x99,0xff,     /* 2 */
	 0xbb,0x7d,0x6d,0x6d,0x93,0xff,     /* 3 */
	 0xe7,0xd7,0xb7,0x77,0x01,0xff,     /* 4 */
	 0x1b,0x5d,0x5d,0x5d,0x63,0xff,     /* 5 */
	 0xc3,0xad,0x6d,0x6d,0xf3,0xff,     /* 6 */
	 0x3f,0x7f,0x71,0x6f,0x1f,0xff,     /* 7 */
	 0x93,0x6d,0x6d,0x6d,0x93,0xff,     /* 8 */
	 0x9f,0x6d,0x6d,0x6b,0x87,0xff,     /* 9 */
	 0xdd,0xff,0x00,     /* : */
	 0xdc,0xff,0x00,     /* ; */
	 0xef,0xd7,0xbb,0x7d,0xff,0x00,     /* < */
	 0xdb,0xdb,0xdb,0xdb,0xdb,0xff,     /* = */
	 0x7d,0xbb,0xd7,0xef,0xff,0x00,     /* > */
	 0xbf,0x7f,0x75,0x6f,0x9f,0xff,     /* ? */
	 0xc1,0xbe,0xa2,0xaa,0xba,0xc3,0xff,0x00,0x00,     /* @ */
	 0x81,0x5f,0x5f,0x5f,0x81,0xff,     /* A */
	 0x01,0x5d,0x5d,0x5d,0xa3,0xff,     /* B */
	 0x83,0x7d,0x7d,0x7d,0xbb,0xff,     /* C */
	 0x01,0x7d,0x7d,0x7d,0x83,0xff,     /* D */
	 0x01,0x5d,0x5d,0x7d,0x7d,0xff,     /* E */
	 0x01,0x5f,0x5f,0x7f,0x7f,0xff,     /* F */
	 0x83,0x7d,0x7d,0x5d,0x43,0xff,     /* G */
	 0x01,0xdf,0xdf,0xdf,0x01,0xff,     /* H */
	 0x7d,0x01,0x7d,0xff,0x00,0x00,     /* I */
	 0xfb,0xfd,0xfd,0xfd,0x03,0xff,     /* J */
	 0x01,0xdf,0xdf,0xaf,0x71,0xff,     /* K */
	 0x01,0xfd,0xfd,0xfd,0xfd,0xff,     /* L */
	 0x01,0xbf,0xdf,0xbf,0x01,0xff,     /* M */
	 0x01,0xbf,0xdf,0xef,0x01,0xff,     /* N */
	 0x83,0x7d,0x7d,0x7d,0x83,0xff,     /* O */
	 0x01,0x5f,0x5f,0x5f,0xbf,0xff,     /* P */
	 0x83,0x7d,0x7d,0x7b,0x85,0xff,     /* Q */
	 0x01,0x5f,0x5f,0x5f,0xa1,0xff,     /* R */
	 0xbb,0x5d,0x5d,0x5d,0xe3,0xff,     /* S */
	 0x7f,0x7f,0x01,0x7f,0x7f,0xff,     /* T */
	 0x03,0xfd,0xfd,0xfd,0x03,0xff,     /* U */
	 0x0f,0xf3,0xfd,0xf3,0x0f,0xff,     /* V */
	 0x01,0xfb,0xf7,0xfb,0x01,0xff,     /* W */
	 0x71,0xaf,0xdf,0xaf,0x71,0xff,     /* X */
	 0x7f,0xbf,0xc1,0xbf,0x7f,0xff,     /* Y */
	 0x79,0x75,0x6d,0x5d,0x3d,0xff,     /* Z */
	 0x01,0x7d,0x7d,0xff,0x00,0x00,     /* [ */
	 0x7f,0x9f,0xef,0xf3,0xfd,0xff,     /* \ */
	 0x7d,0x7d,0x01,0xff,0x00,0x00,     /* ] */
	 0xdf,0xbf,0x7f,0xbf,0xdf,0xff,     /* ^ */
	 0xfe,0xfe,0xfe,0xfe,0xfe,0xff,     /* _ */
	 0x7f,0xbf,0xff,     /* ` */
	 0xfb,0xd5,0xd5,0xd5,0xe1,0xff,     /* a */
	 0x01,0xed,0xdd,0xdd,0xe3,0xff,     /* b */
	 0xe3,0xdd,0xdd,0xdd,0xeb,0xff,     /* c */
	 0xe3,0xdd,0xdd,0xed,0x01,0xff,     /* d */
	 0xe3,0xd5,0xd5,0xd5,0xe5,0xff,     /* e */
	 0xdf,0x81,0x5f,0x5f,0xff,0x00,     /* f */
	 0xe6,0xda,0xda,0xda,0xc1,0xff,     /* g */
	 0x01,0xef,0xdf,0xdf,0xe1,0xff,     /* h */
	 0x41,0xff,0x00,     /* i */
	 0xf9,0xfe,0xfe,0xfe,0x41,0xff,     /* j */
	 0x01,0xf7,0xeb,0xdd,0xff,0x00,     /* k */
	 0x03,0xfd,0xff,     /* l */
	 0xc1,0xdf,0xe7,0xdf,0xe1,0xff,     /* m */
	 0xc1,0xdf,0xdf,0xdf,0xe1,0xff,     /* n */
	 0xe3,0xdd,0xdd,0xdd,0xe3,0xff,     /* o */
	 0xc0,0xeb,0xdb,0xdb,0xe7,0xff,     /* p */
	 0xe7,0xdb,0xdb,0xeb,0xc0,0xff,     /* q */
	 0xc1,0xef,0xdf,0xdf,0xef,0xff,     /* r */
	 0xed,0xd5,0xd5,0xd5,0xdb,0xff,     /* s */
	 0xdf,0x03,0xdd,0xff,0x00,0x00,     /* t */
	 0xc3,0xfd,0xfd,0xfd,0xc1,0xff,     /* u */
	 0xc7,0xfb,0xfd,0xfb,0xc7,0xff,     /* v */
	 0xc3,0xfd,0xe1,0xfd,0xc1,0xff,     /* w */
	 0xdd,0xeb,0xf7,0xeb,0xdd,0xff,     /* x */
	 0xc6,0xfa,0xfa,0xfa,0xc1,0xff,     /* y */
	 0xdd,0xd9,0xd5,0xcd,0xdd,0xff,     /* z */
	 0xef,0x93,0x7d,0xff,0x00,0x00,     /* { */
	 0x01,0xff,0x00,     /* | */
	 0x7d,0x93,0xef,0xff,0x00,0x00,     /* } */
	 0xbf,0x7f,0x7f,0xbf,0xbf,0x7f,0xff,0x00,0x00,     /* ~ */
	 0xef,0xd7,0xbb,0xbb,0x83,0xff, /* <del> */
}

void setupFontminecraft(fontInfo *font){
	if(font != nullptr){
		font->fontIndex = 37;
		strncpy(font->fontName, "Minecraft", 10);
		font->startUnicode = 32;
		font->endUnicode = 128;
		font->blockLength = 3;
		font->logicality = false;
		font->monospace = false;
		font->monoMask = 0xFF;
		font->index = minecraft_index;
		font->data = minecraft_data;
	}
}

/*
   Char Width Adjust Index    Total
 '<sp>'   6     +0      0   6 bytes
    '!'   3     +0      2   3 bytes
    '"'   4     +2      3   6 bytes
    '#'   6     +0      5   6 bytes
    '$'   6     +0      7   6 bytes
    '%'   6     +0      9   6 bytes
    '&'   6     +0     11   6 bytes
    '''   2     +1     13   3 bytes
    '('   4     +2     14   6 bytes
    ')'   4     +2     16   6 bytes
    '*'   4     +2     18   6 bytes
    '+'   6     +0     20   6 bytes
    ','   2     +1     22   3 bytes
    '-'   6     +0     23   6 bytes
    '.'   2     +1     25   3 bytes
    '/'   6     +0     26   6 bytes
    '0'   6     +0     28   6 bytes
    '1'   6     +0     30   6 bytes
    '2'   6     +0     32   6 bytes
    '3'   6     +0     34   6 bytes
    '4'   6     +0     36   6 bytes
    '5'   6     +0     38   6 bytes
    '6'   6     +0     40   6 bytes
    '7'   6     +0     42   6 bytes
    '8'   6     +0     44   6 bytes
    '9'   6     +0     46   6 bytes
    ':'   2     +1     48   3 bytes
    ';'   2     +1     49   3 bytes
    '<'   5     +1     50   6 bytes
    '='   6     +0     52   6 bytes
    '>'   5     +1     54   6 bytes
    '?'   6     +0     56   6 bytes
    '@'   7     +2     58   9 bytes
    'A'   6     +0     61   6 bytes
    'B'   6     +0     63   6 bytes
    'C'   6     +0     65   6 bytes
    'D'   6     +0     67   6 bytes
    'E'   6     +0     69   6 bytes
    'F'   6     +0     71   6 bytes
    'G'   6     +0     73   6 bytes
    'H'   6     +0     75   6 bytes
    'I'   4     +2     77   6 bytes
    'J'   6     +0     79   6 bytes
    'K'   6     +0     81   6 bytes
    'L'   6     +0     83   6 bytes
    'M'   6     +0     85   6 bytes
    'N'   6     +0     87   6 bytes
    'O'   6     +0     89   6 bytes
    'P'   6     +0     91   6 bytes
    'Q'   6     +0     93   6 bytes
    'R'   6     +0     95   6 bytes
    'S'   6     +0     97   6 bytes
    'T'   6     +0     99   6 bytes
    'U'   6     +0    101   6 bytes
    'V'   6     +0    103   6 bytes
    'W'   6     +0    105   6 bytes
    'X'   6     +0    107   6 bytes
    'Y'   6     +0    109   6 bytes
    'Z'   6     +0    111   6 bytes
    '['   4     +2    113   6 bytes
    '\'   6     +0    115   6 bytes
    ']'   4     +2    117   6 bytes
    '^'   6     +0    119   6 bytes
    '_'   6     +0    121   6 bytes
    '`'   3     +0    123   3 bytes
    'a'   6     +0    124   6 bytes
    'b'   6     +0    126   6 bytes
    'c'   6     +0    128   6 bytes
    'd'   6     +0    130   6 bytes
    'e'   6     +0    132   6 bytes
    'f'   5     +1    134   6 bytes
    'g'   6     +0    136   6 bytes
    'h'   6     +0    138   6 bytes
    'i'   2     +1    140   3 bytes
    'j'   6     +0    141   6 bytes
    'k'   5     +1    143   6 bytes
    'l'   3     +0    145   3 bytes
    'm'   6     +0    146   6 bytes
    'n'   6     +0    148   6 bytes
    'o'   6     +0    150   6 bytes
    'p'   6     +0    152   6 bytes
    'q'   6     +0    154   6 bytes
    'r'   6     +0    156   6 bytes
    's'   6     +0    158   6 bytes
    't'   4     +2    160   6 bytes
    'u'   6     +0    162   6 bytes
    'v'   6     +0    164   6 bytes
    'w'   6     +0    166   6 bytes
    'x'   6     +0    168   6 bytes
    'y'   6     +0    170   6 bytes
    'z'   6     +0    172   6 bytes
    '{'   4     +2    174   6 bytes
    '|'   2     +1    176   3 bytes
    '}'   4     +2    177   6 bytes
    '~'   7     +2    179   9 bytes
'<del>'   6     +0    182   6 bytes
Font Size Status: Index section 192 bytes, Pure Data 517 bytes, Data section 552 bytes, Total 744 bytes.
Using image: '37_Minecraft.png' size (517, 9), unicode from 32 to 127/127, grayscale threshold: 127, converted by ImageBinarization.
With options: MSB_AT_TOP, NEGATIVE_LOGIC, WIDTH_DETECT_AT_TOP, WIDTH_DETECT_MODE_SAME_COLOR, 
*/
#endif
