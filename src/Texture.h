#pragma once

struct Texture {

	GLuint id;

	Texture(const std::wstring& path) {

		std::vector<byte> data = File::ReadAllBytes(path);
		int x, y, comp;
		byte* pixels = stbi_load_from_memory(data.data(), (int)data.size(), &x, &y, &comp, 4);

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(pixels);
	}

	~Texture() {
		glDeleteTextures(1, &id);
	}

	// 16-bit RGBA
	Texture(byte* addr, int width, int height) {

		byte* pixels = new byte[width * height * 4];

		for (int i = 0; i < height; i++) {
			for (int j = 0; j < width; j++) {
				// 5 5 5 1
				// rrrr rggg ggbb bbba
				byte* pixel = &addr[(i * width + j) * 2];
				pixels[(i * width + j) * 4 + 0] = (pixel[0] & 0b11111000);
				pixels[(i * width + j) * 4 + 1] = ((pixel[0] & 0b00000111) << 2 | pixel[1] >> 6) << 3;
				pixels[(i * width + j) * 4 + 2] = (pixel[1] & 0b00111110) << 2;
				pixels[(i * width + j) * 4 + 3] = (pixel[1] & 1) ? 0xFF : 0x00;
			}
		}

		glGenTextures(1, &id);
		glBindTexture(GL_TEXTURE_2D, id);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		delete[] pixels;
	}
};