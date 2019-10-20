#ifndef __BITS_OP_H__
#define __BITS_OP_H__

#include <stdint.h>
#include <stdbool.h>

#define bit_clear(var, bit)     ((var) &= ~(bit))
#define bit_set(var, bit)       ((var) |= (bit))
#define bit_test_any(var, bit)  ((var) & (bit))
#define bit_test_all(var, bit)  (((var) & (bit)) == (bit))
#define bit_test_eq(var, bit)   ((var) == (bit))

#define TEST_MODE_ALL   0x1
#define TEST_MODE_ANY   0x2
#define TEST_MODE_EQ    0x3

static inline bool bit_test(uint32_t flg, uint32_t ptn, int mod)
{
	if (mod == TEST_MODE_ALL) {
		return bit_test_all(flg, ptn);
	} else if (mod == TEST_MODE_ANY) {
		return bit_test_any(flg, ptn);
	} else if (mod == TEST_MODE_EQ) {
		return bit_test_eq(flg, ptn);
	}

	return false;
}

#endif
