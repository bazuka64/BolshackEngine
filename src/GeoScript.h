#pragma once

struct Model3D {
	std::vector<DisplayList*> dls;
};

struct GeoScript : Script {

	static const bool debug = false;
	static void print(const char* str) {
		if (debug)::print(str);
	}

	static Model3D* parse(ROM& rom, byte seg, uint off) {
		Model3D* model = new Model3D();

		byte* cmd = rom.segments[seg].data() + off;

		bool end = false;
		std::stack<byte*> ret_addr;
		while (!end) {
			switch (cmd[0]) {
			case 0x01: print("GEO_END");
				end = true;
				cmd += 4;
				break;
			case 0x02: print("GEO_BRANCH");
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				if (cmd[1] == 0x01)
					ret_addr.push(cmd + 8);
				cmd = rom.segments[seg].data() + off;
				continue;
			}
			case 0x03: print("GEO_RETURN");
				cmd = ret_addr.top();
				ret_addr.pop();
				continue;
			case 0x04: print("GEO_OPEN_NODE");
				cmd += 4;
				break;
			case 0x05: print("GEO_CLOSE_NODE");
				cmd += 4;
				break;
			case 0x08: print("GEO_NODE_SCREEN_AREA");
				cmd += 12;
				break;
			case 0x09: print("GEO_NODE_ORTHO");
				cmd += 4;
				break;
			case 0x0b: print("GEO_NODE_START");
				cmd += 4;
				break;
			case 0x0a: print("GEO_CAMERA_FRUSTUM_WITH_FUNC");
				cmd += 12;
				break;
			case 0x0c: print("GEO_ZBUFFER");
				cmd += 4;
				break;
			case 0x0e: print("GEO_SWITCH_CASE");
				cmd += 8;
				break;
			case 0x0f: print("GEO_CAMERA");
				cmd += 20;
				break;
			case 0x15: print("GEO_DISPLAY_LIST");
			{
				byte layer = cmd[1];
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				if (seg != 0x00 && off != 0x000000) {
					DisplayList* dl = Fast3DScript::parse(rom, seg, off);
					dl->BuildBuffers();
					dl->layer = layer;
					model->dls.push_back(dl);
				}
			}
			cmd += 8;
			break;
			case 0x17: print("GEO_RENDER_OBJ");
				cmd += 4;
				break;
			case 0x18: print("GEO_ASM");
				cmd += 8;
				break;
			case 0x19: print("GEO_BACKGROUND");
				cmd += 8;
				break;
			case 0x1d: print("GEO_SCALE");
				cmd += 8;
				break;
			default: throw;
			}
		}

		return model;
	}
};