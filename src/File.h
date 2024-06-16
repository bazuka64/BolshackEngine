#pragma once

struct File {

	static std::vector<char> ReadAllBytes(const std::wstring& path) {

		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file)throw;
		size_t size = file.tellg();
		file.seekg(0);
		std::vector<char> data(size + 1);
		file.read(data.data(), size);
		data[size] = 0;
		return data;
	}
};