#include <iostream>
#include <iomanip>
#include <map>
#include "program_data.h"

using namespace std;

int main()
{
	// Extract jump labels
	map<label_t, size_t> labels;
	size_t ofs = 0;
	for (const op_t op: program_code) {
		if (op.op == OpLabel) {
			label_t label = (label_t)op.data[0];
			if (labels.find(label) != labels.end()) {
				cerr << "Error: Label " << label << " redefined" << endl;
				return 1;
			}
			labels[label] = ofs;
			continue;
		}
		ofs += 1 + op.size;
	}
	size_t size = ofs;

	// Copy and correct jump labels
	uint8_t code[size];
	ofs = 0;
	for (const op_t op: program_code) {
		if (op.op == OpLabel)
			continue;
		code[ofs] = op.op;
		memcpy(&code[ofs + 1], op.data, op.size);
		switch (op.op) {
		case OpJump:
		case OpJumpZero:
		case OpJumpNotZero:
		case OpJumpNegative:
			label_t label = (label_t)op.data[0];
			if (labels.find(label) == labels.end()) {
				cerr << "Error: Label " << label << " not found" << endl;
				return 2;
			}
			off_t lofs = labels[label] - ofs;
			if (lofs > INT8_MAX || lofs < INT8_MIN) {
				cerr << "Error: Label " << label << " out of range" << endl;
				return 3;
			}
			code[ofs + 1] = lofs;
			break;
		}
		ofs += 1 + op.size;
	}

	// Print data
	cout << "Program size: " << size << " bytes" << endl;
	cout << "Data size: " << SizeData << " bytes";
	for (size_t i = 0; i < size; i++) {
		if (i % 16 == 0)
			cout << endl;
		cout << "0x" << hex << setw(2) << setfill('0') << (unsigned int)code[i] << ", ";
	}
	cout << endl;
	return 0;
}
