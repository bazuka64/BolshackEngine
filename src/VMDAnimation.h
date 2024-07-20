#pragma once

struct VMDAnimation {

	struct BoneFrame {
		int frame;
		glm::vec3 translation;
		glm::quat rotation;
	};

	struct MorphFrame {
		int frame;
		float weight;
	};

	std::map<std::wstring, std::vector<BoneFrame>> bone_map;
	std::map<std::wstring, std::vector<MorphFrame>> morph_map;

	int max_frame;
	std::string filename;

	VMDAnimation(const fs::path& path) {

		filename = path.filename().string();

		std::vector<byte> data = File::ReadAllBytes(path);
		BinaryReader br(data.data());

		br.Seek(50);

		int bone_count = br.ReadInt();
		std::map<std::string, std::wstring> bone_name_map;
		for (int i = 0; i < bone_count; i++) {

			std::string name(15, 0);
			br.Read(name.data(), 15);
			if (bone_name_map.count(name) == 0) {
				std::wstring wstr(15, 0);
				int size = MultiByteToWideChar(CP_ACP, 0, name.c_str(), -1, wstr.data(), 15);
				wstr.resize(size - 1); // erase null terminator
				bone_name_map[name] = wstr;
			}

			BoneFrame bf;
			bf.frame = br.ReadInt();
			br.Read(&bf.translation, 12);
			bf.translation.z *= -1;
			br.Read(&bf.rotation, 16);
			bf.rotation.x *= -1;
			bf.rotation.y *= -1;
			br.Seek(64); // bezier interpolation

			bone_map[bone_name_map[name]].push_back(bf);

			max_frame = std::max(max_frame, bf.frame);
		}

		for (auto& [name, bone_frames] : bone_map) {
			std::sort(bone_frames.begin(), bone_frames.end(),
				[](BoneFrame& a, BoneFrame& b) {
					return a.frame < b.frame;
				});
		}

		int morph_count = br.ReadInt();
		std::map<std::string, std::wstring> morph_name_map;
		for (int i = 0; i < morph_count; i++) {

			std::string name(15, 0);
			br.Read(name.data(), 15);
			if (morph_name_map.count(name) == 0) {
				std::wstring wstr(15, 0);
				int size = MultiByteToWideChar(CP_ACP, 0, name.c_str(), -1, wstr.data(), 15);
				wstr.resize(size - 1); // erase null terminator
				morph_name_map[name] = wstr;
			}

			MorphFrame mf;
			mf.frame = br.ReadInt();
			mf.weight = br.ReadFloat();

			morph_map[morph_name_map[name]].push_back(mf);

			max_frame = std::max(max_frame, mf.frame);
		}

		for (auto& [name, morph_frames] : morph_map) {
			std::sort(morph_frames.begin(), morph_frames.end(),
				[](MorphFrame& a, MorphFrame& b) {
					return a.frame < b.frame;
				});
		}
	}
};