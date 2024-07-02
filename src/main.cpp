#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
#include <thread>
#include <format>
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
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

template <typename T>
void print(T x) { std::cout << x << "\n"; }
template <typename T>
void printw(T x) { std::wcout << x << L"\n"; }

#include "Globals.h"
#include "File.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "BinaryReader.h"
#include "VMDAnimation.h"
#include "MMDModel.h"

#include "libmio0.h"
#include "ROM.h"
#include "DisplayList.h"
#include "Script.h"
#include "Fast3DScript.h"
#include "GeoScript.h"
#include "LevelScript.h"

#include <libsm64.h>
#include "Mario.h"

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

	glfwSetKeyCallback(window, KeyCallback);
	glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);

	GLFWmonitor* monitor = glfwGetPrimaryMonitor();
	const GLFWvidmode* mode = glfwGetVideoMode(monitor);
	int xpos = (mode->width - width) / 2;
	int ypos = (mode->height - height) / 2;
	glfwSetWindowPos(window, xpos, ypos);
	glfwShowWindow(window);
	glfwMaximizeWindow(window);
	glfwSwapBuffers(window);

	ImGui::CreateContext();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Shader mmd_shader("shader/mmd.vert", "shader/mmd.frag");
	Shader sm64_shader("shader/sm64.vert", "shader/sm64.frag");
	Shader mario_shader("shader/mario.vert", "shader/mario.frag");

	Camera camera;

	float distance = 7.5f * 20;
	std::vector<MMDModel*> models;
	MMDModel* meirin = new MMDModel("res/model/meirin/meirin.pmx");
	meirin->local_pos = glm::vec3(distance, 0, 0);
	meirin->world[3] = glm::vec4(meirin->local_pos, 1);
	models.push_back(meirin);
	MMDModel* mima = new MMDModel("res/model/mima/mima.pmx");
	mima->local_pos = glm::vec3(-distance, 0, 0);
	mima->world[3] = glm::vec4(mima->local_pos, 1);
	models.push_back(mima);

	std::vector<VMDAnimation*> animations;
	std::vector<sf::Music*> musics;
	sf::Music* cur_music = NULL;
	for (auto& entry : fs::directory_iterator("res/motion/dance/")) {
		const fs::path& path = entry.path();
		fs::path dance_path = path / (path.stem().string() + ".vmd");
		animations.push_back(new VMDAnimation(dance_path));

		fs::path music_path = path / (path.stem().string() + ".wav");
		musics.push_back(new sf::Music);
		sf::Music* music = musics.back();
		if (!music->openFromFile(music_path.string()))throw;
	}

	VMDAnimation* idle = new VMDAnimation("res/motion/1.‚Ú‚ñ‚â‚è‘Ò‚¿_(490f_ˆÚ“®‚È‚µ).vmd");
	for (auto model : models)
		model->SetAnimation(idle, NULL);

	ROM* rom = NULL;
	if (fs::exists("res/roms/baserom.us.z64"))
		rom = new ROM("res/roms/baserom.us.z64");
	Level* level = NULL;

	Mario* mario = NULL;
	if (rom)mario = new Mario;

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

		for (MMDModel* model : models)
			model->Update(dt);

		for (MMDModel* model : models)
			model->Draw(mmd_shader, camera);

		sm64_shader.Use();
		glm::mat4 WVP = camera.proj * camera.view;
		glUniformMatrix4fv(sm64_shader.uniforms["WVP"], 1, false, (float*)&WVP);

		if (level) {
			Area* area = level->areas[level->start_area];
			if (area) {
				for (DisplayList* dl : area->model->dls) {
					dl->Draw(sm64_shader);
				}
			}
		}

		if(mario)
			mario->Draw(mario_shader, camera, dt);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Checkbox("WireFrame", &Globals::WireFrame)) {
			if (Globals::WireFrame)glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		ImGui::Checkbox("IKBone", &Globals::IKBone);
		ImGui::Checkbox("AABB", &Globals::AABB);

		if (rom) {
			ImGui::Separator();
			for (int i = 0; i < LevelNum.size(); i++) {
				if (i <= 3 || i == 32 || i == 35 || i >= 37)continue;
				static int selected = -1;
				if (ImGui::RadioButton(std::format("{:02X} {}", i, LevelNum[i]).c_str(), &selected, i)) {
					level = LevelScript::parse(*rom, i);
					for (auto model : models)
						model->world[3] = glm::vec4(level->start_pos + model->local_pos, 1);
					camera.position = camera.initial_pos + level->start_pos;
					camera.LookAt(camera.position + glm::vec3(0, 0, -1));
				}
			}
		}

		ImGui::Separator();

		if (ImGui::Button("idle")) {
			for (MMDModel* model : models)
				model->SetAnimation(idle, NULL);
			if (cur_music)cur_music->stop();
			cur_music = NULL;
		}
		for (int i = 0; i < animations.size(); i++) {
			VMDAnimation* anim = animations[i];
			sf::Music* music = musics[i];
			if (ImGui::Button(anim->filename.c_str())) {
				for (MMDModel* model : models)
					model->SetAnimation(anim, music);
				if (cur_music)cur_music->stop();
				cur_music = music;
				music->play();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}
}