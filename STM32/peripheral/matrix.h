#ifndef MATRIX_H
#define MATRIX_H

#include <stdint.h>

unsigned int matrix_refresh_cnt();
void matrix_set_pixel(unsigned int x, unsigned int y, uint8_t v);

#endif // MATRIX_H
