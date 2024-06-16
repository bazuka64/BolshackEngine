#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <stack>
#include <algorithm>
#include <thread>
#include <filesystem>
namespace fs = std::filesystem;

#include <Windows.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <SFML/Audio.hpp>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

template <typename T>
void print(T x) {
	std::cout << x << std::endl;
}

template <typename T>
void printw(T x) {
	std::wcout << x << std::endl;
}

#include "File.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "BinaryReader.h"
#include "VMDAnimation.h"
#include "MMDModel.h"

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

void FramebufferSizeCallback(GLFWwindow* window, int width, int height) {
	glViewport(0, 0, width, height);
}

void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
							
int main() {

	const int width = 1920, height = 1080;
	glfwInit();
	glfwWindowHint(GLFW_VISIBLE, false);
	glfwWindowHint(GLFW_SAMPLES, 4);
	GLFWwindow* window = glfwCreateWindow(width, height, "", NULL, NULL);
	glfwMakeContextCurrent(window);
	gladLoadGL();
	stbi_set_flip_vertically_on_load(true);
	setlocale(LC_CTYPE, "");

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int xpos = (mode->width - width) / 2;
	int ypos = (mode->height - height) / 2;
	glfwSetWindowPos(window, xpos, ypos);
	glfwShowWindow(window);

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader shader(L"shader/shader.vert", L"shader/shader.frag");
	Camera camera;
	MMDModel model(L"res/model/meirin/meirin.pmx");
	VMDAnimation anim(L"res/motion/zettai/zettai.vmd");
	//VMDAnimation anim(L"res/motion/2.走り86L_ダッシュ加速_(14f_前移動60)修正.vmd");
	model.SetAnimation(&anim);

	/*sf::Music music;
	if (!music.openFromFile("res/motion/zettai/zettai.wav"))throw;
	music.play();*/

	float prev_time = (float)glfwGetTime();
	glm::ivec2 prev_pos(0);
	while (!glfwWindowShouldClose(window)) {

		float time = (float)glfwGetTime();
		float dt = time - prev_time;
		prev_time = time;

		int fps = (int)(1 / dt);
		glfwSetWindowTitle(window, std::to_string(fps).c_str());

		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		glm::ivec2 pos(xpos, ypos);
		glm::ivec2 delta_pos = pos - prev_pos;
		prev_pos = pos;

		glClearColor(.1f, .1f, .5f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		camera.Update(window, dt, delta_pos);
		model.Update(dt);
		model.Draw(shader, camera);

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}