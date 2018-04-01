#include <stdint.h>
#include "i2c_reg.h"


typedef struct {
    uint8_t addr_write;
    uint8_t addr_read;

    uint8_t io_write_val;
    uint8_t display_control;
    uint8_t display_mode;
} _ISQUC;

int16_t main(void);
