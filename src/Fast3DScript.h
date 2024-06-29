#pragma once

struct Vertex {
	short x, y, z;
	float u, v;
};

struct DisplayList {

	byte layer;

	GLuint vao, vbo;
	std::vector<Vertex> vertices;
	std::vector<Texture*> textures;

	struct Mesh {
		int vertex_offset;
		int vertex_count;
	};
	std::vector<Mesh> meshes;

	void BuildBuffers() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_SHORT, false, sizeof(Vertex), (void*)offsetof(Vertex, x));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(Vertex), (void*)offsetof(Vertex, u));
	}

	void Draw(Shader& shader) {

		// LAYER_TRANSPARENT_DECAL 6 の場合、木の陰は非表示
		// LAYER_OPAQUE_DECAL      2 の場合、ドアの周りは非表示
		if (layer == 6 || layer == 2)return;

		shader.Use();
		glBindVertexArray(vao);

		for (int i = 0; i < meshes.size(); i++) {
			Mesh& mesh = meshes[i];

			if (textures[i])
				glBindTexture(GL_TEXTURE_2D, textures[i]->id);

			glDrawArrays(GL_TRIANGLES, mesh.vertex_offset, mesh.vertex_count);

			glBindTexture(GL_TEXTURE_2D, 0);
		}

	}
};

struct Fast3DScript : Script {

	static const bool debug = false;
	static void print(const char* str) {
		if (debug)::print(str);
	}

	static DisplayList* parse(ROM& rom, byte seg, uint off) {
		DisplayList* dl = new DisplayList();

		byte* cmd = rom.segments[seg].data() + off;

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
			case 0x04: print("gsSPVertex");
			{
				byte num = (cmd[1] >> 4) + 1;
				byte dest_off = cmd[1] & 0x0F;
				ushort size = Read(cmd, 2, 2);
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				byte* ptr = rom.segments[seg].data() + off;

				for (int i = dest_off; i < dest_off + num; i++) {
					buffer[i].x = Read(ptr, i * sizeof(F3DVertex) + 0, 2);
					buffer[i].y = Read(ptr, i * sizeof(F3DVertex) + 2, 2);
					buffer[i].z = Read(ptr, i * sizeof(F3DVertex) + 4, 2);
					buffer[i].f = Read(ptr, i * sizeof(F3DVertex) + 6, 2);
					buffer[i].u = Read(ptr, i * sizeof(F3DVertex) + 8, 2);
					buffer[i].v = Read(ptr, i * sizeof(F3DVertex) + 10, 2);
					buffer[i].r_nx = ptr[i * sizeof(F3DVertex) + 12];
					buffer[i].g_ny = ptr[i * sizeof(F3DVertex) + 13];
					buffer[i].b_nz = ptr[i * sizeof(F3DVertex) + 14];
					buffer[i].a = ptr[i * sizeof(F3DVertex) + 15];
				}
			}
			break;
			case 0x06: print("gsSPDisplayList");
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				if (rom.segments.count(seg) > 0 &&
					seg != 0x00 &&
					off != 0x000000) {
					if (cmd[1] == 0x00)
						ret_addr.push(cmd + 8);
					cmd = rom.segments[seg].data() + off;
					continue;
				}
			}
			break;
			case 0xb8: print("gsSPEndDisplayList");
				if (!ret_addr.empty()) {
					cmd = ret_addr.top();
					ret_addr.pop();
					continue;
				}
				end = true;
				break;
			case 0xbf: print("gsSP1Triangle");
			{
				byte a = cmd[5] / 0x0A;
				byte b = cmd[6] / 0x0A;
				byte c = cmd[7] / 0x0A;

				byte array[3] = { a,b,c };

				for (int i = 0; i < 3; i++) {
					Vertex vertex;
					vertex.x = buffer[array[i]].x;
					vertex.y = buffer[array[i]].y;
					vertex.z = buffer[array[i]].z;
					vertex.u = buffer[array[i]].u / (float)width / 32.f;
					vertex.v = buffer[array[i]].v / (float)height / 32.f;
					dl->vertices.push_back(vertex);
				}

				if (dl->textures.empty()) {
					dl->meshes.push_back({});
					dl->meshes.back().vertex_offset = 0;
					dl->textures.push_back(NULL);
				}

				dl->meshes.back().vertex_count += 3;
			}
			break;
			case 0xf2: print("gsDPSetTileSize");
			{
				int w = cmd[5] << 4 | cmd[6] >> 4;
				int h = (cmd[6] & 0x0F) << 8 | cmd[7];

				width = (w >> 2) + 1;
				height = (h >> 2) + 1;
			}
			break;
			case 0xfd: print("gsDPSetTextureImage");
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				if (rom.segments.count(seg) == 0)break;

				byte* texture_address = rom.segments[seg].data() + off;
				dl->textures.push_back(new Texture(texture_address, width, height));

				dl->meshes.push_back({});
				dl->meshes.back().vertex_offset = (int)dl->vertices.size();
			}
			break;
			case 0x03: print("G_MOVEMEM"); break;
			case 0xb6: print("gsSPClearGeometryMode"); break;
			case 0xb7: print("gsSPSetGeometryMode"); break;
			case 0xb9: print("G_SetOtherMode_L"); break;
			case 0xba: print("G_SetOtherMode_H"); break;
			case 0xbb: print("gsSPTexture"); break;
			case 0xbc: print("G_MOVEWORD"); break;
			case 0xe6: print("gsDPLoadSync"); break;
			case 0xe7: print("gsDPPipeSync"); break;
			case 0xe8: print("gsDPTileSync"); break;
			case 0xf0: print("G_LOADTLUT"); break;
			case 0xf3: print("gsDPLoadBlock"); break;
			case 0xf5: print("gsDPSetTile"); break;
			case 0xf8: print("G_SETFOGCOLOR"); break;
			case 0xfb: print("G_SETENVCOLOR"); break;
			case 0xfc: print("gsDPSetCombineMode"); break;
			default:throw;
			}
			cmd += 8;
		}

		return dl;
	}
};