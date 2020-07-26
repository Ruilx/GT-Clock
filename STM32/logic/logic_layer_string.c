// Layer: String
// Parameters:
//   [0]	Flags
//     0x02	Scroll to off-screen
//     0x01	Start scrolling
//   [1]	X top left offset
//   [2]	Y top left offset
//   [3]	String length
//   [4]	Font ID
//   [5]	Scrolling speed

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>
#include <fonts.h>
#include "logic_layer_string_fonts.h"

#define SCROLL_SCALE	(4 * 1000)

enum {FlagScrollStart = 0x01, FlagScrollOffScreen = 0x02};

#pragma pack(push,1)

typedef struct {
	union {
		uint32_t raw[4];	// Aligned
		struct {
			uint8_t flags;
			int8_t x;
			int8_t y;
			uint8_t len;
			uint8_t id;
			uint8_t scrollspd;
			int16_t scrollx;
			uint8_t scrollstart;
			unsigned int scrolltick;
		};
	};
	const FontInfo *info;
} param_t;

#pragma pack(pop)

static void init(layer_obj_t *pparam, layer_obj_t *pdata)
{
	// Allocate param buffer, data buffer need to wait until commit
	pparam->size = sizeof(param_t);
	logic_layers_alloc(pparam);
	pdata->size = 0;
}

static void config(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int *ok,
		   unsigned int w, unsigned int h)
{
	param_t *pp = pparam->p;
	if (pp->len == 0) {
		*ok = 0;
		return;
	}

	// Now, allocate data buffer
	if (pdata->size != pp->len) {
		pdata->size = pp->len;
		logic_layers_alloc(pdata);
		if (pdata->size == 0) {
			*ok = 0;
			return;
		}
		// Clear string buffer
		*(char *)pdata->p = 0;
	}

	// Find font info structure
	pp->info = 0;
	LIST_ITERATE(logic_fonts, const FontInfo, pfnt) {
		if (pfnt->fontIndex == pp->id) {
			pp->info = pfnt;
			break;
		}
	}
	if (!pp->info) {
		*ok = 0;
		pdata->size = 0;
		return;
	}

	// Reset scrolling
	pp->scrollstart = 0;
	pp->scrollx = pp->x;
}

static inline void draw_bitmap(int ox, int oy, unsigned int w, unsigned int h,
			       const uint8_t *pbm, uint8_t bmw, uint8_t bmh,
			       uint8_t *pfb)
{
	uint8_t sy = oy >= 0 ? 0 : -oy;
	uint8_t ey = oy + bmh <= (int)h ? bmh : h - oy;
	uint8_t sx = ox >= 0 ? 0 : -ox;
	uint8_t ex = ox + bmw <= (int)w ? bmw : w - ox;
	for (uint8_t bmy = sy; bmy != ey; bmy++) {
		for (uint8_t bmx = sx; bmx != ex; bmx++) {
			unsigned int x = ox + bmx;
			unsigned int y = oy + bmy;
			pfb[y * w + x] = (pbm[bmx] << bmy) & 0x80 ? 0 : 0xff;
		}
	}
}

static inline void draw_string(param_t *pp, char *ptr, unsigned int tick,
			       uint8_t *pfb, unsigned int w, unsigned int h)
{
	// Skip if scrolling from off-screen not started
	if ((pp->flags & FlagScrollStart) && !pp->scrollstart) {
		pp->scrollx = pp->x;
		pp->scrolltick = tick;
	}

	int x = pp->scrollspd ? pp->scrollx : pp->x;

	if (pp->flags & FlagScrollStart) {
		if (!pp->scrollstart) {
			pp->scrollstart = 1;
			//pp->flags &= ~FlagScrollStart;
		} else {
			unsigned int t = (tick - pp->scrolltick) * pp->scrollspd;
			x -= t / SCROLL_SCALE;
		}
	}

	int scrollx = x;
	int y = pp->y;
	const FontInfo *pinfo = pp->info;
	static const uint8_t bmh = 8;
	if (x >= (int)w || y >= (int)h || y + bmh < 0)
		return;

	const uint8_t bmbs = pinfo->blockLength;
	unsigned int len = pp->len;
	while (len-- && *ptr) {
		char c = *ptr++;
		if (c < pinfo->startUnicode || c >= pinfo->endUnicode)
			continue;
		const uint8_t o = c - pinfo->startUnicode;
		const uint8_t bmw = pinfo->monospace ? bmbs : pinfo->index[o * 2 + 1];
		if (x < (int)w && x + bmw >= 0) {
			const unsigned int bmo = bmbs * (pinfo->monospace ? o : pinfo->index[o * 2]);
			const uint8_t *pbm = &pinfo->data[bmo];
			draw_bitmap(x, y, w, h, pbm, bmw, bmh, pfb);
		}
		x += bmw;
	}

	// Check for end of scroll
	if (pp->flags & FlagScrollStart) {
		if (pp->flags & FlagScrollOffScreen) {
			// Hold at before start of display
			if (x <= 0) {
				pp->scrollx = w;
				pp->flags &= ~FlagScrollStart;
			}
		} else {
			// Hold at after end of screen
			if (x <= (int)w) {
				pp->scrollx = scrollx + ((int)w - x);
				pp->flags &= ~FlagScrollStart;
			}
		}
	}
}

static void proc(layer_obj_t *pparam, layer_obj_t *pdata, unsigned int tick,
		 uint8_t *pfb, unsigned int w, unsigned int h)
{
	memset(pfb, 0, w * h);
	if (pparam->size == 0 || pdata->size == 0)
		return;
	draw_string(pparam->p, pdata->p, tick, pfb, w, h);
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdString,
	.init = &init,
	.config = &config,
	.proc = &proc,
};
