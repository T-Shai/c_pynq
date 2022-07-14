#pragma once

int load_bitstream(const char* bitstream_name);

#define DMA_BASE 0x41e00000

#define MM2S_CTRL 0x00 /4

#define S2MM_CTRL 0x30 /4
#define S2MM_STATUS 0x34 /4
#define S2MM_DST_ADDR_LOW 0x48 /4
#define S2MM_DST_ADDR_HIGH 0x4C /4
#define S2MM_LEN 0x58 /4

#define DST_ADDR UINT64_C(??)
#define DST_ADDR_LOW ??
#define DST_ADDR_HIGH ??