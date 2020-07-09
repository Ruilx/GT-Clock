// Layer: String
// Parameters:
//   [0]	Flags (unused)
//   [1]	X top left offset
//   [2]	Y top left offset
//   [3]	String length
//   [4]	Font ID

#include <stdlib.h>
#include <math.h>
#include <debug.h>
#include <peripheral/matrix.h>
#include <logic/logic_layers.h>
#include <fonts.h>
#include "logic_layer_string_fonts.h"

typedef struct PACKED {
	uint8_t flags;
	int8_t x;
	int8_t y;
	uint8_t len;
	uint8_t id;
	uint8_t _reserved[3];
	const FontInfo *info;
} param_t;

static void *config(void *param, unsigned int *ok)
{
	param_t *pp = param;
	pp->info = 0;
	// Find font info structure
	LIST_ITERATE(logic_fonts, const FontInfo, pfnt) {
		if (pfnt->fontIndex == pp->id) {
			pp->info = pfnt;
			break;
		}
	}
	if (!pp->info)
		return 0;
	// Allocate string buffer
	void *ptr = logic_layers_alloc(pp->len);
	if (ptr == 0) {
		*ok = 0;
		return 0;
	}
	char *pc = ptr;
	*pc = 0;
	// TODO TEST
	memcpy(pc, "ia", 3);
	return ptr;
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

static inline void draw_string(param_t *pp, char *ptr)
{
	unsigned int w, h;
	uint8_t *pfb = matrix_fb(0, &w, &h);
	unsigned int len = pp->len;
	int x = pp->x;
	int y = pp->y;
	const FontInfo *pinfo = pp->info;
	static const uint8_t bmh = 8;
	const uint8_t bmbs = pinfo->blockLength;
	if (y >= (int)h || y + bmh < 0)
		return;
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
}

static void proc(unsigned int tick, void *param, void *ptr)
{
	param_t *pp = param;
	if (!ptr || !pp->info)
		return;
	draw_string(pp, ptr);
}

LOGIC_LAYER_HANDLER() = {
	.id = LayerIdString,
	.config = &config,
	.proc = &proc,
};
