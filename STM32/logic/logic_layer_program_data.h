#pragma once

typedef enum {
	// Exit
	OpExit = 0,
	// Unconditional jump
	//   Data: CodeAddr (1)
	OpJump,
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

	// Load register from constant
	//   Data: Constant (1)
	OpLoadConst,
	// Load register from data
	//   Data: DataAddr (1)
	OpLoadData,

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

	NumOps} opcode_t;
