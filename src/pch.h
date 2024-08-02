#pragma once

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

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/euler_angles.hpp>
#include <SFML/Audio.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <bullet/btBulletDynamicsCommon.h>

#include <Windows.h>

#include <libsm64.h>

typedef unsigned char byte;
typedef unsigned short ushort;
typedef unsigned int uint;

template <typename T>
void print(T x) { std::cout << x << std::endl; }
template <typename T>
void printw(T x) { std::wcout << x << std::endl; }