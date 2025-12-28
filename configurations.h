#ifndef _CONFIGURATION
#define _CONFIGURATION

// Project defenitions
#define INSTR_MEM_DEPTH 4096
#define DATA_MEM_DEPTH 4096

#define MAX_LINE_LENGTH 500
#define MAX_LABLE_LENGTH 50

#define INSTR_WIDTH_BITS 48
#define INSTR_WIDTH_CHARS 12

#define DATA_WIDTH_BITS 32
#define DATA_WIDTH_CHARS 8

#define NUM_OF_REGISTERS 16
#define NUM_OF_IO_REGISTERS 23

#define SCREEN_X_AXIS 256
#define SCREEN_Y_AXIS 256

#define DISK_TOTAL_SECTORS 128
#define WORDS_PER_SECTOR 128
#define BYTES_PER_SECTOR 512
#define DISK_OP_CYCLES 1024

// General defenitions
#define BOOL int
#define TRUE 1
#define FALSE 0
#define INSTR_TYPE unsigned long long int
#define DATA unsigned int
#define NO_OPERATION 0
#define OP_READ 1
#define OP_WRITE 2
#define FREE 0
#define BUSY 1
#define ENABLE_FLAG 1
#define DISABLE_FLAG 0

// Instructions feilds
#define OPCODE_SIZE_IN_BITS 8
#define	RD_SIZE_IN_BITS 4
#define RS_SIZE_IN_BITS 4
#define RT_SIZE_IN_BITS 4
#define RM_SIZE_IN_BITS 4
#define IMM1_SIZE_IN_BITS 12
#define IMM2_SIZE_IN_BITS 12

#define MASK_OPCODE 0xff0000000000
#define MASK_RD		0x00f000000000
#define MASK_RS		0x000f00000000
#define MASK_RT		0x0000f0000000
#define MASK_RM		0x00000f000000
#define MASK_IMM1	0x000000fff000
#define MASK_IMM2	0x000000000fff
#endif