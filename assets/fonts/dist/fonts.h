#ifndef _FONT_H_
#define _FONT_H_

#include <stdint.h>
#include <string.h>

#ifndef __cplusplus
#define bool	char
#define false	0
#define true	1
#define nullptr	((void *)0)
#endif

#ifndef _FONT_INFO_STRUCT_
#define _FONT_INFO_STRUCT_
typedef struct FontInfo_t {
	uint8_t fontIndex;
	//char fontName[16];
	uint16_t startUnicode;
	uint16_t endUnicode;
	uint8_t blockLength;
	bool logicality;
	bool monospace;
	uint8_t numberHeight;
	const unsigned char *index;
	const unsigned char *data;
} FontInfo;
#endif

#include "arabic.h"
#include "casiolcd.h"
#include "commodore64.h"
#include "devanagari.h"
#include "georgia.h"
#include "kana6.h"
#include "minecraft.h"
#include "minecraftia.h"
#include "nespixel.h"
#include "ocra.h"
#include "r4511.h"
#include "r6h.h"
#include "rblock.h"
#include "rbold.h"
#include "rboldserif.h"
#include "rcbold.h"
#include "rhalfbold.h"
#include "rhand.h"
#include "rhsans.h"
#include "rkanji.h"
#include "rkanjipro.h"
#include "rlsd.h"
#include "rpro.h"
#include "rprobold.h"
#include "rsans.h"
#include "rsanspro.h"
#include "rshiftsym.h"
#include "rsidy.h"
#include "rsmall.h"
#include "rsmg.h"
#include "rsmgbold.h"
#include "rsq.h"
#include "rsqpro.h"
#include "rsqserif.h"
#include "rsqstd.h"
#include "rstd.h"
#include "ruilxfixedsys.h"
#include "simsun.h"

#endif
