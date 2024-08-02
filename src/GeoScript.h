#pragma once

struct Model3D {
	std::vector<DisplayList*> dls;

	void Draw(Shader* shader) {
		for (DisplayList* dl : dls) {
			dl->Draw(shader);
		}
	}
};

struct GeoScript : Script {

	static const bool debug = false;
	static void print(std::string str, byte* cmd, int len) {
		if (debug)Script::print(str, cmd, len);
	}

	static Model3D* parse(ROM* rom, byte seg, uint off) {
		Model3D* model = new Model3D();

		byte* cmd = rom->segments[seg].data() + off;

		bool end = false;
		std::stack<byte*> ret_addr;
		while (!end) {
			
			int len = get_cmd_len(cmd);

			switch (cmd[0]) {
			case 0x00: print("GEO_BRANCH_AND_LINK", cmd, len);
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				ret_addr.push(cmd + get_cmd_len(cmd));
				cmd = rom->segments[seg].data() + off;
				continue;
			}
			case 0x01: print("GEO_END", cmd, len);
				end = true;
				break;
			case 0x02: print("GEO_BRANCH", cmd, len);
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				if (cmd[1] == 0x01)
					ret_addr.push(cmd + get_cmd_len(cmd));
				cmd = rom->segments[seg].data() + off;
				continue;
			}
			case 0x03: print("GEO_RETURN", cmd, len);
				cmd = ret_addr.top();
				ret_addr.pop();
				continue;
			case 0x04: print("GEO_OPEN_NODE", cmd, len);
				break;
			case 0x05: print("GEO_CLOSE_NODE", cmd, len);
				break;
			case 0x08: print("GEO_NODE_SCREEN_AREA", cmd, len);
				break;
			case 0x09: print("GEO_NODE_ORTHO", cmd, len);
				break;
			case 0x0b: print("GEO_NODE_START", cmd, len);
				break;
			case 0x0a: print("GEO_CAMERA_FRUSTUM_WITH_FUNC", cmd, len);
				break;
			case 0x0c: print("GEO_ZBUFFER", cmd, len);
				break;
			case 0x0d: print("GEO_RENDER_RANGE", cmd, len);
				break;
			case 0x0e: print("GEO_SWITCH_CASE", cmd, len);
				break;
			case 0x0f: print("GEO_CAMERA", cmd, len);
				break;
			case 0x10: print("GEO_TRANSLATE_ROTATE", cmd, len);
				break;
			case 0x11: print("GEO_TRANSLATE_NODE", cmd, len);
				break;
			case 0x12: print("GEO_ROTATION_NODE", cmd, len);
				break;
			case 0x13: print("GEO_ANIMATED_PART", cmd, len);
			{
				byte layer = cmd[1];
				short x = Read(cmd, 2, 2);
				short y = Read(cmd, 4, 2);
				short z = Read(cmd, 6, 2);
				byte seg = cmd[8];
				uint off = Read(cmd, 9, 3);

				if (seg == 0x00 && off == 0x000000)break;

				DisplayList* dl = Fast3DScript::parse(rom, seg, off);
				dl->BuildBuffers();
				dl->layer = layer;
				model->dls.push_back(dl);
			}
			break;
			case 0x14: print("GEO_BILLBOARD", cmd, len);
				break;
			case 0x15: print("GEO_DISPLAY_LIST", cmd, len);
			{
				byte layer = cmd[1];
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				if (seg == 0x00 && off == 0x000000)break;

				DisplayList* dl = Fast3DScript::parse(rom, seg, off);
				dl->BuildBuffers();
				dl->layer = layer;
				model->dls.push_back(dl);
			}
			break;
			case 0x16: print("GEO_SHADOW", cmd, len);
				break;
			case 0x17: print("GEO_RENDER_OBJ", cmd, len);
				break;
			case 0x18: print("GEO_ASM", cmd, len);
				break;
			case 0x19: print("GEO_BACKGROUND", cmd, len);
				break;
			case 0x1c: print("GEO_HELD_OBJECT", cmd, len);
				break;
			case 0x1d: print("GEO_SCALE", cmd, len);
				break;
			case 0x20: print("GEO_CULLING_RADIUS", cmd, len);
				break;
			default: throw;
			}
			cmd += len;
		}

		return model;
	}

	static int get_cmd_len(byte* cmd) {
		switch (cmd[0]) {
		case 0x00:return 0x08;
		case 0x01:return 0x04;
		case 0x02:return 0x08;
		case 0x03:return 0x04;
		case 0x04:return 0x04;
		case 0x05:return 0x04;
		case 0x06:return 0x04;
		case 0x07:return 0x04;
		case 0x08:return 0x0C;
		case 0x09:return 0x04;
		case 0x0a:if (cmd[1] == 0x00)return 0x08; else return 0x0C;
		case 0x0b:return 0x04;
		case 0x0c:return 0x04;
		case 0x0d:return 0x08;
		case 0x0e:return 0x08;
		case 0x0f:return 0x14;
		case 0x10: {
			bool displayList = cmd[1] & 0x80;
			byte fieldLayout = (cmd[1] & 0b01110000) >> 4;
			switch (fieldLayout) {
			case 0:if (!displayList)return 0x10; else return 0x14; // GEO_TRANSLATE_ROTATE
			case 1:if (!displayList)return 0x8; else return 0x0C; // GEO_TRANSLATE
			case 2:if (!displayList)return 0x10; else return 0x14; // GEO_ROTATE
			case 3:if (!displayList)return 0x04; else return 0x08; // GEO_ROTATE_Y
			}
		}
		case 0x11:if (!(cmd[1] & 0x80))return 0x08; else return 0x0C;
		case 0x12:if (!(cmd[1] & 0x80))return 0x08; else return 0x0C;
		case 0x13:return 0x0C;
		case 0x14:if (!(cmd[1] & 0x80))return 0x08; else return 0x0C;
		case 0x15:return 0x08;
		case 0x16:return 0x08;
		case 0x17:return 0x04;
		case 0x18:return 0x08;
		case 0x19:return 0x08;
		case 0x1a:return 0x08;
		case 0x1b:return 0x04;
		case 0x1c:return 0x0C;
		case 0x1d:if (!(cmd[1] & 0x80))return 0x08; else return 0x0C;
		case 0x1e:return 0x08;
		case 0x1f:return 0x10;
		case 0x20:return 0x04;
		default:throw;
		}
	}
};