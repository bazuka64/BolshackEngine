#pragma once

struct Globals {
	static bool WireFrame;
	static bool IKBone;
	static bool AABB;
	static bool Paused;
	static sf::Music* cur_music;
};

bool Globals::WireFrame = false;
bool Globals::IKBone = false;
bool Globals::AABB = false;
bool Globals::Paused = false;
sf::Music* Globals::cur_music = NULL;