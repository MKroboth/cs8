//
// Created by mkr on 7/24/21.
//

#ifndef CS8_BADASM_HXX
#define CS8_BADASM_HXX

#define CS8_RDST 0
#define CS8_RSC0 1
#define CS8_RSC1 2
#define CS8_RIDX 3
#define CS8_RTMP 4
#define CS8_RSP0 5
#define CS8_RSP1 6

// Instructions
#define CS8_LIMM(ADDR) 0x00, (uint8_t)((((uint16_t)ADDR) & 0xFF00) >> 8), (uint8_t)((((uint16_t)ADDR) & 0x00FF)),
#define CS8_LDIR(ADDR) 0x01, (uint8_t)((((uint16_t)ADDR) & 0xFF00) >> 8), (uint8_t)((((uint16_t)ADDR) & 0x00FF)),
#define CS8_SDIR(ADDR) 0x02, (uint8_t)((((uint16_t)ADDR) & 0xFF00) >> 8), (uint8_t)((((uint16_t)ADDR) & 0x00FF)),
#define CS8_LIDX() 0x03,
#define CS8_SIDX() 0x04,
#define CS8_TREG(TARGET, SOURCE) (0x05 | (TARGET << 4)), SOURCE,
#define CS8_PSH0(TARGET) (0x06 | (TARGET << 4)),
#define CS8_PSH1(TARGET) (0x07 | (TARGET << 4)),
#define CS8_POP0(TARGET) (0x08 | (TARGET << 4)),
#define CS8_POP1(TARGET) (0x09 | (TARGET << 4)),
#define CS8_ADD() 0x0A
#define CS8_SUB() 0x0B
#define CS8_MUL() 0x0C
#define CS8_DIVMOD() 0x0D
#define CS8_NAND() 0x0E
#define CS8_JLEQ() 0x0F,

// Macro Instructions
#define CS8_MACRO_LOAD(VALUE, REG) CS8_LIMM(VALUE) CS8_TREG(CS8_RTMP, REG)
#define CS8_MACRO_JUMP(ADDR) CS8_MACRO_LOAD(ADDR, CS8_RDST) CS8_MACRO_LOAD(0, CS8_RIDX) CS8_JLEQ()

#endif //CS8_BADASM_HXX
