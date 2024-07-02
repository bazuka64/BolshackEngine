#pragma once

struct Area {
	Model3D* model;
};

struct Level {
	std::map<byte, Area*> areas;
	byte start_area;
	glm::vec3 start_pos;
};

struct LevelScript : Script{

	static const bool debug = false;
	static void print(const char* str) {
		if (debug)::print(str);
	}

	static Level* parse(ROM& rom, int levelID) {
		Level* level = new Level();

		rom.SetSegment(0x15, 0x2ABCA0, 0x2AC6B0);
		byte* cmd = rom.segments[0x15].data();

		bool end = false;
		std::stack<byte*> ret_addr;
		while (!end) {
			int len = cmd[1];

			switch (cmd[0]) {
			case 0x00: print("EXECUTE");
			{
				byte seg = cmd[3];
				if (seg == 0x14)break; // star select screen

				uint start = Read(cmd, 4, 4);
				uint end = Read(cmd, 8, 4);
				rom.SetSegment(seg, start, end);

				byte seg_jump = cmd[12];
				uint off = Read(cmd, 13, 3);
				cmd = rom.segments[seg_jump].data() + off;
				continue;
			}
			case 0x02: print("END");
				end = true;
				break;
			case 0x03: print("SLEEP");
				end = true;
				break;
			case 0x05: print("JUMP");
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				cmd = rom.segments[seg].data() + off;
				continue;
			}
			case 0x06: print("JUMP_LINK");
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				ret_addr.push(cmd + len);
				cmd = rom.segments[seg].data() + off;
				continue;
			}
			case 0x07: print("RETURN");
				cmd = ret_addr.top();
				ret_addr.pop();
				continue;
			case 0x0c: print("JUMP_IF");
			{
				uint arg = Read(cmd, 4, 4);
				if (arg == levelID) {
					byte seg = cmd[8];
					uint off = Read(cmd, 9, 3);
					cmd = rom.segments[seg].data() + off;
					continue;
				}
			}
			break;
			case 0x17: print("LOAD_RAW"); goto label;
			case 0x18: print("LOAD_MIO0"); goto label;
			case 0x1a: print("LOAD_MIO0_TEXTURE");
			label:
			{
				byte seg = cmd[3];
				uint start = Read(cmd, 4, 4);
				uint end = Read(cmd, 8, 4);
				if (cmd[0] == 0x17)rom.SetSegment(seg, start, end);
				else rom.SetSegmentMIO0(seg, start, end);
			}
			break;
			case 0x1f: print("AREA");
			{
				byte areaID = cmd[2];
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				Model3D* model = GeoScript::parse(rom, seg, off);
				Area* area = new Area();
				area->model = model;
				level->areas[areaID] = area;
			}
			break;
			case 0x20: print("END_AREA"); break;
			case 0x21: print("LOAD_MODEL_FROM_DL"); break;
			case 0x22: print("LOAD_MODEL_FROM_GEO"); break;
			case 0x2b: print("MARIO_POS");
			{
				byte start_area = cmd[2];
				short posX = Read(cmd, 6, 2);
				short posY = Read(cmd, 8, 2);
				short posZ = Read(cmd, 10, 2);

				level->start_area = start_area;
				level->start_pos = glm::vec3(posX, posY, posZ);
			}
			end = true;
			break;

			case 0x0a: print("LOOP_BEGIN"); break;
			case 0x10: print("No Operation"); break;
			case 0x11: print("CALL"); break;
			case 0x1b: print("INIT_LEVEL"); break;
			case 0x1d: print("ALLOC_LEVEL_POOL"); break;
			case 0x1e: print("FREE_LEVEL_POOL"); break;
			case 0x24: print("OBJECT"); break;
			case 0x25: print("MARIO"); break;
			case 0x26: print("WARP_NODE"); break;
			case 0x27: print("INSTANT_WARP"); break;
			case 0x28: print("WARP_NODE"); break;
			case 0x2e: print("TERRAIN"); break; // special object
			case 0x2f: print("ROOMS"); break;
			case 0x30: print("SHOW_DIALOG"); break;
			case 0x31: print("TERRAIN_TYPE"); break;
			case 0x36: print("SET_BACKGROUND_MUSIC"); break;
			case 0x39: print("MACRO_OBJECTS"); break;
			case 0x3b: print("WHIRLPOOL"); break;
			case 0x3c: print("GET_OR_SET"); break;
			default:throw;
			}

			cmd += len;
		}

		return level;
	}
};