#pragma once

struct Object {
	byte modelID;
	glm::vec3 position;
};

struct Area {
	Model3D* model;
	std::vector<Object*> objects;
};

struct Level {
	std::map<byte, Area*> areas;
	std::map<int, Model3D*> models;

	byte start_area;
	glm::vec3 start_pos;
};

struct LevelScript : Script {

	static const bool debug = false;
	static void print(std::string str, byte* cmd) {
		if (debug)Script::print(str, cmd, cmd[1]);
	}

	static Level* parse(ROM& rom, int levelID) {
		Level* level = new Level();

		rom.SetSegment(0x15, 0x2ABCA0, 0x2AC6B0);
		rom.SetSegmentMIO0(0x02, 0x108A40, 0x114750);
		byte* cmd = rom.segments[0x15].data();

		Area* cur_area = NULL;

		bool end = false;
		std::stack<byte*> ret_addr;
		while (!end) {
			int len = cmd[1];

			switch (cmd[0]) {
			case 0x00: print("EXECUTE", cmd);
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
			case 0x02: print("END", cmd);
				end = true;
				break;
			case 0x03: print("SLEEP", cmd);
				end = true;
				break;
			case 0x05: print("JUMP", cmd);
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				cmd = rom.segments[seg].data() + off;
				continue;
			}
			case 0x06: print("JUMP_LINK", cmd);
			{
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				ret_addr.push(cmd + len);
				cmd = rom.segments[seg].data() + off;
				continue;
			}
			case 0x07: print("RETURN", cmd);
				cmd = ret_addr.top();
				ret_addr.pop();
				continue;
			case 0x0c: print("JUMP_IF", cmd);
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
			case 0x17: print("LOAD_RAW", cmd); goto label;
			case 0x18: print("LOAD_MIO0", cmd); goto label;
			case 0x1a: print("LOAD_MIO0_TEXTURE", cmd);
			label:
			{
				byte seg = cmd[3];
				uint start = Read(cmd, 4, 4);
				uint end = Read(cmd, 8, 4);
				if (cmd[0] == 0x17)rom.SetSegment(seg, start, end);
				else rom.SetSegmentMIO0(seg, start, end);
			}
			break;
			case 0x1f: print("AREA", cmd);
			{
				byte areaID = cmd[2];
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);

				Model3D* model = GeoScript::parse(rom, seg, off);
				Area* area = new Area();
				area->model = model;
				level->areas[areaID] = area;

				cur_area = area;
			}
			break;
			case 0x20: print("END_AREA", cmd);
				cur_area = NULL;
				break;
			case 0x21: print("LOAD_MODEL_FROM_DL", cmd); break;
			case 0x22: print("LOAD_MODEL_FROM_GEO", cmd);
			{
				byte modelID = cmd[3];
				byte seg = cmd[4];
				uint off = Read(cmd, 5, 3);
				Model3D* model = GeoScript::parse(rom, seg, off);

				level->models[modelID] = model;
			}
			break;
			case 0x24: print("OBJECT", cmd);
			{
				byte modelID = cmd[3];
				short x = Read(cmd, 4, 2);
				short y = Read(cmd, 6, 2);
				short z = Read(cmd, 8, 2);

				// degrees
				short rx = Read(cmd, 10, 2);
				short ry = Read(cmd, 12, 2);
				short rz = Read(cmd, 14, 2);

				// behavior
				uint param = Read(cmd, 16, 4);
				byte seg = cmd[21];
				uint off = Read(cmd, 22, 3);

				Object* obj = new Object;
				obj->modelID = modelID;
				obj->position = glm::vec3(x, y, z);

				cur_area->objects.push_back(obj);
			}
			break;
			case 0x2b: print("MARIO_POS", cmd);
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

			case 0x0a: print("LOOP_BEGIN", cmd); break;
			case 0x10: print("No Operation", cmd); break;
			case 0x11: print("CALL", cmd); break;
			case 0x1b: print("INIT_LEVEL", cmd); break;
			case 0x1d: print("ALLOC_LEVEL_POOL", cmd); break;
			case 0x1e: print("FREE_LEVEL_POOL", cmd); break;
			case 0x25: print("MARIO", cmd); break;
			case 0x26: print("WARP_NODE", cmd); break;
			case 0x27: print("INSTANT_WARP", cmd); break;
			case 0x28: print("WARP_NODE", cmd); break;
			case 0x2e: print("TERRAIN", cmd); break; // special object
			case 0x2f: print("ROOMS", cmd); break;
			case 0x30: print("SHOW_DIALOG", cmd); break;
			case 0x31: print("TERRAIN_TYPE", cmd); break;
			case 0x36: print("SET_BACKGROUND_MUSIC", cmd); break;
			case 0x39: print("MACRO_OBJECTS", cmd); break;
			case 0x3b: print("WHIRLPOOL", cmd); break;
			case 0x3c: print("GET_OR_SET", cmd); break;
			default:throw;
			}

			cmd += len;
		}

		return level;
	}
};