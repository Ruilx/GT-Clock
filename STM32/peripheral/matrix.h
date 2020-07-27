#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>
#include <list.h>

// Matrix frame buffer swap callback
typedef void (*const matrix_handler_t)();
#define MATRIX_SWAP_HANDLER()	LIST_ITEM(matrix_swap, matrix_handler_t)

unsigned int matrix_refresh_cnt();

void *matrix_fb(unsigned int active, unsigned int *w, unsigned int *h);
unsigned int matrix_fb_ready();
void matrix_fb_swap();
void matrix_fb_copy();
void matrix_orientation(unsigned int flipped);

#endif // MATRIX_H
