#ifndef PYNQ_UTILS_H
#define PYNQ_UTILS_H

#define DMA_BASE (0x41E00000)

#define MM2S_CONTROL_REGISTER (0x00 >> 2)
#define MM2S_STATUS_REGISTER (0x04 >> 2)
#define MM2S_START_ADDRESS (0x18 >> 2)
#define MM2S_LENGTH (0x28 >> 2)

#define S2MM_CONTROL_REGISTER (0x30 >> 2)
#define S2MM_STATUS_REGISTER (0x34 >> 2)
#define S2MM_DESTINATION_ADDRESS (0x48 >> 2)
#define S2MM_LENGTH (0x58 >> 2)

#define REGISTER_SIZE (0x1000)

#define DMA_SEND_ADDR (0x0e000000)
#define DMA_RECV_ADDR (0x0f000000)

#define IP_AXI_LITE_BASE (0x40000000)

#endif // PYNQ_UTILS_H