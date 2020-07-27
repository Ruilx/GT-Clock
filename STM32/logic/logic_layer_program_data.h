#pragma once

typedef enum {
	// Exit
	//   Data: None
	OpExit = 0,
	// Unconditional jump
	//   Data: CodeAddr (1)
	OpJump,
	// Conditional jump if equal to zero
	//   Data: CodeAddr (1), SrcDataAddr (1)
	OpJumpNotZero,
	// Conditional jump if less than zero
	//   Data: CodeAddr (1), SrcDataAddr (1)
	OpJumpNegative,

	// Load layer mixer
	//   Data: DestDataAddr (1), Layer (1), Addr(1)
	OpLoadMixer,
	// Load layer parameter
	//   Data: DestDataAddr (1), Layer (1), Addr(1)
	OpLoadParam,
	// Load layer private data
	//   Data: DestDataAddr (1), Layer (1), Addr(1)
	OpLoadData,
	// Load const
	//   Data: DestDataAddr (1), Constant (1)
	OpLoadConst,

	// Save layer mixer
	//   Data: SrcDataAddr (1), Layer (1), Addr(1)
	OpSaveMixer,
	// Save layer parameter
	//   Data: SrcDataAddr (1), Layer (1), Addr(1)
	OpSaveParam,
	// Save layer private data
	//   Data: SrcDataAddr (1), Layer (1), Addr(1)
	OpSaveData,

	// Arithmetic operations
	// Bit-wise AND
	OpAnd,
	// Bit-wise OR
	OpOr,
	// Bit-wise XOR
	OpXor,

	NumOps} opcode_t;
