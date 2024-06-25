#pragma once

struct BinaryReader {

	byte* data;
	int position = 0;

	BinaryReader(byte* data) :data(data) {}

	void Seek(int size) {
		position += size;
	}

	void Read(void* dest, int size) {
		memcpy(dest, &data[position], size);
		position += size;
	}

	char ReadChar() {
		char value = *(char*)&data[position];
		position += 1;
		return value;
	}

	byte ReadByte() {
		byte value = *(byte*)&data[position];
		position += 1;
		return value;
	}

	short ReadShort() {
		short value = *(short*)&data[position];
		position += 2;
		return value;
	}

	ushort ReadUShort() {
		ushort value = *(ushort*)&data[position];
		position += 2;
		return value;
	}

	int ReadInt() {
		int value = *(int*)&data[position];
		position += 4;
		return value;
	}

	uint ReadUInt() {
		uint value = *(uint*)&data[position];
		position += 4;
		return value;
	}

	float ReadFloat() {
		float value = *(float*)&data[position];
		position += 4;
		return value;
	}

	int Read124(int size) {
		switch (size) {
		case 1: return ReadChar();
		case 2: return ReadShort();
		case 4: return ReadInt();
		default:throw;
		}
	}

	int ReadU124(int size) {
		switch (size) {
		case 1: return ReadByte();
		case 2: return ReadUShort();
		case 4: return ReadUInt();
		default:throw;
		}
	}
};