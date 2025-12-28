#ifndef _STRUCTS
#define _STRUCTS
#include "configurations.h"
/* This file defines the primary structures utilized by both the assembler and simulator. */

// Declaration of registers present in the register file
typedef enum _Register {
	ZERO = 0,
	IMM1,
	IMM2,
	V0,
	A0,
	A1,
	A2,
	T0,
	T1,
	T2,
	S0,
	S1,
	S2,
	GP,
	SP,
	RA,
} Register;

// Defines the various opcodes used by SIMP
typedef enum _OpCode {
	ADD = 0,
	SUB,
	MAC,
	AND,
	OR,
	XOR,
	SLL,
	SRA,
	SRL,
	BEQ,
	BNE,
	BLT,
	BGT,
	BLE,
	BGE,
	JAL,
	LW,
	SW,
	RETI,
	IN,
	OUT,
	HALT,
} OpCode;

// Represents a single instruction
typedef struct _Instruction {
	OpCode opcode;
	int rd;
	int rs;
	int rt;
	int rm;
	int immediate1;
	int immediate2;
} Instruction;

// Input/Output hardware registers declared in the register file
typedef enum _IO_HW_Register {
	Irq0Enable = 0,
	Irq1Enable,
	Irq2Enable,
	Irq0Status,
	Irq1Status,
	Irq2Status,
	IrqHandler,
	IrqReturn,
	Clks,
	Leds,
	Display7Seg,
	TimerEnable,
	TimerCurrent,
	TimerMax,
	DiskCmd,
	DiskSector,
	DiskBuffer,
	DiskStatus,
	RESERVED1,
	RESERVED2,
	MonitorADDR,
	MonitorDATA,
	MonitorCMD,
} IO_HW_Register;

/* Each assembly file consists of four types of lines:
 * REGULAR_INST - A standard instruction
 * PSEUDO - Represents a .Word directive
 * LABEL - Defines a label
 * COMMENT - A line containing a comment written by the author
 */
typedef enum _Line_Type {
	REGULAR_INST,
	PSEUDO,
	LABEL,
	COMMENT,
} Line_Type;

// Represents the .Word directive
typedef struct _Word {
	int address;
	int value;
} Word;


#endif
