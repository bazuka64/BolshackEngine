#pragma once

struct File {

	static std::vector<byte> ReadAllBytes(const std::string& path) {

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)throw;
		size_t size = file.tellg();
		file.seekg(0);
		std::vector<byte> data(size + 1);
		file.read((char*)data.data(), size);
		data[size] = 0;
		return data;
	}

	static std::vector<byte> ReadAllBytes(const std::wstring& path) {

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)throw;
		size_t size = file.tellg();
		file.seekg(0);
		std::vector<byte> data(size + 1);
		file.read((char*)data.data(), size);
		data[size] = 0;
		return data;
	}
};