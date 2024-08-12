#pragma once

#include "MMDModel.h"

#include "libmio0.h"
#include "ROM.h"
#include "DisplayList.h"
#include "Script.h"
#include "Fast3DScript.h"
#include "GeoScript.h"
#include "LevelScript.h"

#include "Mario.h"

struct Application {

	GLFWwindow* window;
	Camera* camera;
	Shader* mmd_shader;
	Shader* sm64_shader;
	Shader* mario_shader;

	std::vector<MMDModel*> models;
	std::vector<VMDAnimation*> dances;
	std::vector<sf::Music*> musics;
	VMDAnimation* idle;
	VMDAnimation* run;

	ROM* rom = NULL;
	Level* level = NULL;
	Mario* mario = NULL;

	btDiscreteDynamicsWorld* dynamicsWorld;

	Application();
	void Run();
	~Application();

private:
	void WindowInit();
	static void FramebufferSizeCallback(GLFWwindow* window, int width, int height);
	static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
};