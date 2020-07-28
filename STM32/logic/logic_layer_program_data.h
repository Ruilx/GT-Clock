#pragma once

typedef enum {PTypeNormal, PType02d} print_type_t;

typedef enum {
	// Exit
	OpExit = 0,
	// Unconditional jump
	//   Data: CodeAddr (1)
	OpJump,
	// Conditional jump if register is zero
	//   Data: CodeAddr (1)
	OpJumpZero,
	// Conditional jump if register is not zero
	//   Data: CodeAddr (1)
	OpJumpNotZero,
	// Conditional jump if register is less than zero
	//   Data: CodeAddr (1)
	OpJumpNegative,

	// Set pointer to layer mixer
	//   Data: Layer (1), Addr(1)
	OpPtrMixer,
	// Set pointer to layer parameter
	//   Data: Layer (1), Addr(1)
	OpPtrParam,
	// Set pointer to layer private data
	//   Data: Layer (1), Addr(1)
	OpPtrData,
	// Set pointer to RTC calendar data
	//   Data: Addr(1)
	OpPtrCalendar,

	// String print decimal 8-bit data
	//   Data: Type (1)
	//     Type 0: %d
	//     Type 1: %02d
	OpPrintI8,

	// Load register from constant
	//   Data: Constant (1)
	OpLoadConst,
	// Load register from data
	//   Data: DataAddr (1)
	OpLoadData,
	// Save register to data
	//   Data: DataAddr (1)
	OpSaveData,

	// Load register from pointer address
	OpLoadPtr,
	// Save register to pointer address
	OpSavePtr,

	// Register arithmetic operations
	// Bit-wise AND
	//   Data: Mask (1)
	OpAnd,
	// Bit-wise OR
	//   Data: Mask (1)
	OpOr,
	// Bit-wise XOR
	//   Data: Mask (1)
	OpXor,
	// Increment
	OpInc,
	// Decrement
	OpDec,

	NumOps} opcode_t;
