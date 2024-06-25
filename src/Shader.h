#pragma once

struct Shader {

	GLuint program;
	std::map<std::string, GLint> uniforms;

	Shader(const std::wstring& vertPath, const std::wstring& fragPath) {

		std::vector<byte> data = File::ReadAllBytes(vertPath);
		const char* c_str = (char*)data.data();
		GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &c_str, 0);
		glCompileShader(vertex);

		int length;
		glGetShaderiv(vertex, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char* info = new char[length];
			glGetShaderInfoLog(vertex, length, 0, info);
			std::cout << info << std::endl;
			throw;
		}

		data = File::ReadAllBytes(fragPath);
		c_str = (char*)data.data();
		GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &c_str, 0);
		glCompileShader(fragment);

		glGetShaderiv(fragment, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char* info = new char[length];
			glGetShaderInfoLog(fragment, length, 0, info);
			std::cout << info << std::endl;
			throw;
		}

		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);

		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char* info = new char[length];
			glGetProgramInfoLog(program, length, 0, info);
			std::cout << info << std::endl;
			throw;
		}

		glDeleteShader(vertex);
		glDeleteShader(fragment);
		glDetachShader(program, vertex);
		glDetachShader(program, fragment);

		int count;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &count);
		for (int i = 0; i < count; i++) {
			char name[256];
			GLsizei length;
			GLint size;
			GLenum type;
			glGetActiveUniform(program, i, sizeof name, &length, &size, &type, name);
			uniforms[name] = glGetUniformLocation(program, name);
		}
	}

	void Use() {
		glUseProgram(program);
	}
};