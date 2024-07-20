#pragma once

struct Shader {

	GLuint program;
	std::map<std::string, GLint> uniforms;

	Shader(const char* vertPath, const char* fragPath) {

		std::ifstream file;
		std::stringstream ss;
		std::string str;
		const char* c_str;

		file.open(vertPath);
		if (!file)throw;
		ss << file.rdbuf();
		str = ss.str();
		c_str = str.c_str();
		file.close();

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

		file.open(fragPath);
		if (!file)throw;
		ss.str("");
		ss << file.rdbuf();
		str = ss.str();
		c_str = str.c_str();
		file.close();

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

		int num;
		glGetProgramiv(program, GL_ACTIVE_UNIFORMS, &num);
		for (int i = 0; i < num; i++) {
			char name[256];
			GLsizei length;
			GLint size;
			GLenum type;
			glGetActiveUniform(program, i, sizeof name, &length, &size, &type, name);
			GLint location = glGetUniformLocation(program, name);
			uniforms[name] = location;
		}
	}

	void Use() {
		glUseProgram(program);
	}
};