#pragma once

struct File {

	static std::vector<byte> ReadAllBytes(const fs::path& path) {
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)throw;
		size_t size = file.tellg();
		file.seekg(0);
		std::vector<byte> data(size);
		file.read((char*)data.data(), size);
		return data;
	}
};