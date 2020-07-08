#ifndef LOGIC_ANIMATION_H
#define LOGIC_ANIMATION_H

typedef const struct {
	void (* const proc)(unsigned int tick);
} logic_ani_handler_t;

#define LOGIC_ANI_HANDLER()	LIST_ITEM(logic_ani, logic_ani_handler_t)

void logic_animation_enable(unsigned int e);
unsigned int logic_animation_refresh_cnt();

#endif // LOGIC_ANIMATION_H
