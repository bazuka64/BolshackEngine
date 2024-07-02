#pragma once

struct File {

	static std::vector<byte> ReadAllBytes(const std::string& path) {
		std::ifstream file(path, std::ios::binary);
		return ReadAllBytes(file);
	}

	static std::vector<byte> ReadAllBytes(const std::wstring& path) {
		std::ifstream file(path, std::ios::binary);
		return ReadAllBytes(file);
	}

private:
	static std::vector<byte> ReadAllBytes(std::ifstream& file) {
		if (!file)throw;
		file.seekg(0, std::ios::end);
		size_t size = file.tellg();
		file.seekg(0, std::ios::beg);
		std::vector<byte> data(size);
		file.read((char*)data.data(), size);
		return data;
	}
};