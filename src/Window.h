#pragma once

GLFWwindow* WindowInit() {

	const int width = 1920, height = 1080;
	glfwInit();
	glfwWindowHint(GLFW_VISIBLE, false);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(width, height, "", NULL, NULL);
	glfwMakeContextCurrent(window);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);
		});

	glfwSetFramebufferSizeCallback(window, [](GLFWwindow* window, int width, int height) {
		glViewport(0, 0, width, height);
		});

	glfwSetMouseButtonCallback(window, [](GLFWwindow* window, int button, int action, int mods) {
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		});

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int xpos = (mode->width - width) / 2;
	int ypos = (mode->height - height) / 2;
	glfwSetWindowPos(window, xpos, ypos);
	glfwShowWindow(window);

	gladLoadGL();
	stbi_set_flip_vertically_on_load(true);
	setlocale(LC_CTYPE, "");

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return window;
}