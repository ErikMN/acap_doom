#ifndef __I_MAIN__
#define __I_MAIN__

#include "../controls.h"

#include <stdint.h>

#ifdef DEBUG
#define DBUG(x) x
#else
#define DBUG(x)
#endif

#define print_debug(...) DBUG(printf(__VA_ARGS__))

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(x) (sizeof(x) / sizeof(0 [x]))
#endif

extern uint8_t *rgba_buffer;

int real_main(int argc, char **argv);

#endif
