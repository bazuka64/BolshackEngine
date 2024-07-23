#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <algorithm>
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
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

template <typename T>
void print(T x) { std::cout << x << std::endl; }
template <typename T>
void printw(T x) { std::wcout << x << std::endl; }

#include "Globals.h"
#include "Window.h"
#include "File.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "AABB.h"
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

int main() {

	GLFWwindow* window = WindowInit();

	Camera camera;

	Shader mmd_shader("shader/mmd.vert", "shader/mmd.frag");
	Shader sm64_shader("shader/sm64.vert", "shader/sm64.frag");
	Shader mario_shader("shader/mario.vert", "shader/mario.frag");

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

	for (auto& entry : fs::directory_iterator("res/motion/dance/")) {
		const fs::path& path = entry.path();
		fs::path dance_path = path / (path.stem().string() + ".vmd");
		animations.push_back(new VMDAnimation(dance_path));

		fs::path music_path = path / (path.stem().string() + ".wav");
		sf::Music* music = new sf::Music;
		if (!music->openFromFile(music_path.string()))throw;
		musics.push_back(music);
	}

	VMDAnimation* idle = new VMDAnimation("res/motion/1.‚Ú‚ñ‚â‚è‘Ò‚¿_(490f_ˆÚ“®‚È‚µ).vmd");
	for (auto model : models)
		model->SetAnimation(idle, NULL);

	ROM* rom = NULL;
	if (fs::exists("res/roms/sm64.z64"))
		rom = new ROM("res/roms/sm64.z64");

	Level* level = NULL;
	if (rom) {
		level = LevelScript::parse(*rom, 0x10); // castle grounds
		for (auto model : models)
			model->world[3] = glm::vec4(level->start_pos + model->local_pos, 1);
		camera.position = level->start_pos + camera.local_pos;
		camera.target = level->start_pos + camera.local_target;
	}

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

		for (MMDModel* model : models) {
			if (!Globals::Paused)
				model->Update(dt);
		}

		for (MMDModel* model : models)
			model->Draw(mmd_shader, camera);

		if (level) {
			Area* area = level->areas[level->start_area];
			if (area) {
				sm64_shader.Use();
				glm::mat4 vp = camera.proj * camera.view;
				glUniformMatrix4fv(sm64_shader.uniforms["wvp"], 1, false, (float*)&vp);
				area->model->Draw(sm64_shader);

				for (Object* obj : area->objects) {
					if (level->models.count(obj->modelID) == 0)continue;

					glm::mat4 world = glm::translate(glm::mat4(1), obj->position);
					glm::mat4 wvp = vp * world;
					glUniformMatrix4fv(sm64_shader.uniforms["wvp"], 1, false, (float*)&wvp);
					level->models[obj->modelID]->Draw(sm64_shader);
				}
			}
		}

		if (mario) mario->Draw(mario_shader, camera, dt);

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		if (ImGui::Checkbox("WireFrame", &Globals::WireFrame)) {
			if (Globals::WireFrame)glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		}
		ImGui::Checkbox("IKBone", &Globals::IKBone);
		ImGui::Checkbox("AABB", &Globals::AABB);
		if (ImGui::Checkbox("Paused", &Globals::Paused)) {
			if (Globals::cur_music) {
				if (Globals::Paused)Globals::cur_music->pause();
				else Globals::cur_music->play();
			}
		}

		if (rom) {
			ImGui::Separator();
			for (int i = 0; i < LevelNum.size(); i++) {
				if (i <= 3 || i == 32 || i == 35 || i >= 37)continue;
				static int selected = -1;
				if (ImGui::RadioButton(std::format("{:02X} {}", i, LevelNum[i]).c_str(), &selected, i)) {
					level = LevelScript::parse(*rom, i);
					for (auto model : models)
						model->world[3] = glm::vec4(level->start_pos + model->local_pos, 1);
					camera.position = level->start_pos + camera.local_pos;
					camera.target = level->start_pos + camera.local_target;
				}
			}
		}

		ImGui::Separator();

		if (ImGui::Button("idle")) {
			for (MMDModel* model : models)
				model->SetAnimation(idle, NULL);
			if (Globals::cur_music)Globals::cur_music->stop();
			Globals::cur_music = NULL;
		}
		for (int i = 0; i < animations.size(); i++) {
			VMDAnimation* anim = animations[i];
			sf::Music* music = musics[i];
			if (ImGui::Button(anim->filename.c_str())) {
				for (MMDModel* model : models)
					model->SetAnimation(anim, music);
				if (Globals::cur_music)Globals::cur_music->stop();
				Globals::cur_music = music;
				music->play();
			}
		}

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	for (sf::Music* music : musics)
		delete music;
}