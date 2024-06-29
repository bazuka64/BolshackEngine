#pragma once

struct Script {

	static uint Read(byte* cmd, int off, int size) {
		switch (size) {
		case 2:return cmd[off] << 8 | cmd[off + 1];
		case 3:return cmd[off] << 16 | cmd[off + 1] << 8 | cmd[off + 2];
		case 4:return cmd[off] << 24 | cmd[off + 1] << 16 | cmd[off + 2] << 8 | cmd[off + 3];
		default:throw;
		}
	}
};