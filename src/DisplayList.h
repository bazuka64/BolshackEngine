#pragma once

struct SM64Vertex {
	short x, y, z;
	float u, v;
};

struct DisplayList {

	byte layer;

	GLuint vao, vbo;
	std::vector<SM64Vertex> vertices;
	std::vector<Texture*> textures;

	struct Mesh {
		int vertex_offset;
		int vertex_count;
	};
	std::vector<Mesh> meshes;

	void BuildBuffers() {
		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);
		glGenBuffers(1, &vbo);
		glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(SM64Vertex), vertices.data(), GL_STATIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_SHORT, false, sizeof(SM64Vertex), (void*)offsetof(SM64Vertex, x));

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, false, sizeof(SM64Vertex), (void*)offsetof(SM64Vertex, u));

		vertices.clear();
	}

	void Draw(Shader* shader) {

		// LAYER_TRANSPARENT_DECAL 6 の場合、木の陰は非表示
		// LAYER_OPAQUE_DECAL      2 の場合、ドアの周りは非表示
		if (layer == 6 || layer == 2)return;

		shader->Use();
		glBindVertexArray(vao);

		for (int i = 0; i < meshes.size(); i++) {
			Mesh& mesh = meshes[i];

			if (textures[i])
				glBindTexture(GL_TEXTURE_2D, textures[i]->id);

			glDrawArrays(GL_TRIANGLES, mesh.vertex_offset, mesh.vertex_count);

			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}
};