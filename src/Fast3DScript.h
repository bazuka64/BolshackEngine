#pragma once

struct Fast3DScript : Script {

	static const bool debug = false;
	static void print(std::string str, byte* cmd) {
		if (debug)Script::print(str, cmd, 8);
	}

	static DisplayList* parse(ROM* rom, byte seg, uint off) {
		DisplayList* dl = new DisplayList();

		byte* cmd = rom->segments[seg].data() + off;

		struct F3DVertex {
			short x, y, z, f, u, v;
			byte r_nx, g_ny, b_nz, a;
		};
		F3DVertex buffer[16]{};

		int width = 0, height = 0;

		bool end = false;
		std::stack<byte*> ret_addr;
		while (!end) {
			switch (cmd[0]) {
			case 0x04: print("gsSPVertex", cmd);
			{
				byte num = (cmd[1] >> 4) + 1;
				byte dest_off = cmd[1] & 0x0F;
				ushort size = Read(cmd, 2, 2);
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				byte* ptr = rom->segments[seg].data() + off;

				for (int i = 0; i < num; i++) {
					buffer[dest_off + i].x = Read(ptr, 0, 2);
					buffer[dest_off + i].y = Read(ptr, 2, 2);
					buffer[dest_off + i].z = Read(ptr, 4, 2);
					buffer[dest_off + i].f = Read(ptr, 6, 2);
					buffer[dest_off + i].u = Read(ptr, 8, 2);
					buffer[dest_off + i].v = Read(ptr, 10, 2);
					buffer[dest_off + i].r_nx = ptr[12];
					buffer[dest_off + i].g_ny = ptr[13];
					buffer[dest_off + i].b_nz = ptr[14];
					buffer[dest_off + i].a = ptr[15];
					ptr += sizeof(F3DVertex);
				}
			}
			break;
			case 0x06: print("gsSPDisplayList", cmd);
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				if (rom->segments.count(seg) == 0)break;
				if (seg == 0x00 && off == 0x000000)break;

				if (cmd[1] == 0x00)
					ret_addr.push(cmd + 8);
				cmd = rom->segments[seg].data() + off;
				continue;
			}
			case 0xb8: print("gsSPEndDisplayList", cmd);
				if (!ret_addr.empty()) {
					cmd = ret_addr.top();
					ret_addr.pop();
					continue;
				}
				end = true;
				break;
			case 0xbf: print("gsSP1Triangle", cmd);
			{
				byte a = cmd[5] / 0x0A;
				byte b = cmd[6] / 0x0A;
				byte c = cmd[7] / 0x0A;

				byte array[3] = { a,b,c };

				for (int i = 0; i < 3; i++) {
					SM64Vertex vertex;
					vertex.x = buffer[array[i]].x;
					vertex.y = buffer[array[i]].y;
					vertex.z = buffer[array[i]].z;
					vertex.u = buffer[array[i]].u / (float)width / 32.f;
					vertex.v = buffer[array[i]].v / (float)height / 32.f;
					dl->vertices.push_back(vertex);
				}

				if (dl->textures.empty()) {
					dl->meshes.push_back({});
					dl->textures.push_back(NULL);
				}

				dl->meshes.back().vertex_count += 3;
			}
			break;
			case 0xf2: print("gsDPSetTileSize", cmd);
			{
				int w = cmd[5] << 4 | cmd[6] >> 4;
				int h = (cmd[6] & 0x0F) << 8 | cmd[7];

				width = (w >> 2) + 1;
				height = (h >> 2) + 1;
			}
			break;
			case 0xfd: print("gsDPSetTextureImage", cmd);
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				if (rom->segments.count(seg) == 0)break;

				byte* texture_address = rom->segments[seg].data() + off;
				dl->textures.push_back(new Texture(texture_address, width, height));

				dl->meshes.push_back({});
				dl->meshes.back().vertex_offset = (int)dl->vertices.size();
			}
			break;
			case 0x03: print("G_MOVEMEM", cmd); break;
			case 0xb6: print("gsSPClearGeometryMode", cmd); break;
			case 0xb7: print("gsSPSetGeometryMode", cmd); break;
			case 0xb9: print("G_SetOtherMode_L", cmd); break;
			case 0xba: print("G_SetOtherMode_H", cmd); break;
			case 0xbb: print("gsSPTexture", cmd); break;
			case 0xbc: print("G_MOVEWORD", cmd); break;
			case 0xe6: print("gsDPLoadSync", cmd); break;
			case 0xe7: print("gsDPPipeSync", cmd); break;
			case 0xe8: print("gsDPTileSync", cmd); break;
			case 0xf0: print("G_LOADTLUT", cmd); break;
			case 0xf3: print("gsDPLoadBlock", cmd); break;
			case 0xf5: print("gsDPSetTile", cmd); break;
			case 0xf8: print("G_SETFOGCOLOR", cmd); break;
			case 0xfb: print("G_SETENVCOLOR", cmd); break;
			case 0xfc: print("gsDPSetCombineMode", cmd); break;
			default:throw;
			}
			cmd += 8;
		}

		return dl;
	}
};