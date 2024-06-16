#pragma once

struct Texture {

	GLuint id;

	void Load(const std::wstring& path) {

		std::vector<char> data = File::ReadAllBytes(path);
		int x, y, comp;
		byte* pixels = stbi_load_from_memory((byte*)data.data(), (int)data.size(), &x, &y, &comp, 4);

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(pixels);
	}
};