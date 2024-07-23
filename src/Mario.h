#pragma once

struct Mario {

	int marioId;
	SM64MarioInputs marioInputs{};
	SM64MarioState marioState;
	SM64MarioGeometryBuffers marioGeometry;

	GLuint texture;
	GLuint vao;
	GLuint position_buffer;
	GLuint normal_buffer;
	GLuint color_buffer;
	GLuint uv_buffer;

	int gamepad = -1;
	float tick = 0;

	Mario() {

		std::vector<byte> bytes = File::ReadAllBytes("res/roms/sm64.z64");

		byte* outTexture = new byte[4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT];
		sm64_global_init(bytes.data(), outTexture);

		SM64Surface surfaces[] = {
			{0,0,0,{{1000,0,1000},{1000,0,-1000},{-1000,0,1000}}},
			{0,0,0,{{-1000,0,-1000},{-1000,0,1000},{1000,0,-1000}}},
		};
		sm64_static_surfaces_load(surfaces, 2);

		marioId = sm64_mario_create(0, 0, 0);

		marioGeometry.position = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
		marioGeometry.color = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
		marioGeometry.normal = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
		marioGeometry.uv = (float*)malloc(sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES);
		marioGeometry.numTrianglesUsed = 0;

		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, SM64_TEXTURE_WIDTH, SM64_TEXTURE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, outTexture);

		delete[] outTexture;

		glGenVertexArrays(1, &vao);
		glBindVertexArray(vao);

		glGenBuffers(1, &position_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

		glGenBuffers(1, &normal_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);

		glGenBuffers(1, &color_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 3, GL_FLOAT, false, 0, 0);

		glGenBuffers(1, &uv_buffer);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 2, GL_FLOAT, false, 0, 0);

		for (int i = GLFW_JOYSTICK_1; i <= GLFW_JOYSTICK_16; i++)
			if (glfwJoystickIsGamepad(i))gamepad = i;
	}

	void Draw(Shader& shader, Camera& camera, float dt) {

		if (gamepad != -1) {
			GLFWgamepadstate state;
			glfwGetGamepadState(gamepad, &state);
			marioInputs.buttonA = state.buttons[GLFW_GAMEPAD_BUTTON_CROSS];
			marioInputs.buttonB = state.buttons[GLFW_GAMEPAD_BUTTON_SQUARE];
			marioInputs.buttonZ = state.buttons[GLFW_GAMEPAD_BUTTON_LEFT_BUMPER];
			marioInputs.stickX = read_axis(state.axes[GLFW_GAMEPAD_AXIS_LEFT_X]);
			marioInputs.stickY = read_axis(state.axes[GLFW_GAMEPAD_AXIS_LEFT_Y]);
			marioInputs.camLookX = marioState.position[0] - camera.position.x;
			marioInputs.camLookZ = marioState.position[2] - camera.position.z;
		}

		tick += dt;
		if (tick > 1 / 30.f) {
			tick -= 1 / 30.f;
			sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
		}

		glBindBuffer(GL_ARRAY_BUFFER, position_buffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * 3 * marioGeometry.numTrianglesUsed, marioGeometry.position, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, normal_buffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * 3 * marioGeometry.numTrianglesUsed, marioGeometry.normal, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, color_buffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * 3 * marioGeometry.numTrianglesUsed, marioGeometry.color, GL_DYNAMIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, uv_buffer);
		glBufferData(GL_ARRAY_BUFFER, 3 * sizeof(float) * 3 * marioGeometry.numTrianglesUsed, marioGeometry.uv, GL_DYNAMIC_DRAW);

		shader.Use();
		glm::mat4 WVP = camera.proj * camera.view;
		glUniformMatrix4fv(shader.uniforms["WVP"], 1, false, (float*)&WVP);
		glBindVertexArray(vao);
		glBindTexture(GL_TEXTURE_2D, texture);
		glDrawArrays(GL_TRIANGLES, 0, marioGeometry.numTrianglesUsed * 3);
	}

private:
	float read_axis(float val) {

		if (val < 0.2f && val > -0.2f)
			return 0.0f;

		return val > 0.0f ? (val - 0.2f) / 0.8f : (val + 0.2f) / 0.8f;
	}
};