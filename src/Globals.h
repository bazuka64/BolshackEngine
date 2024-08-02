#pragma once

struct Globals {
	static bool WireFrame;
	static bool IKBone;
	static bool AABB;
	static bool Paused;
	static bool Physics;
	static bool RigidBody;
	static sf::Music* cur_music;
	static const char* sm64_path;
};

bool Globals::WireFrame = false;
bool Globals::IKBone = false;
bool Globals::AABB = false;
bool Globals::Paused = false;
bool Globals::Physics = false;
bool Globals::RigidBody = false;
sf::Music* Globals::cur_music = NULL;
const char* Globals::sm64_path = "res/roms/sm64.z64";