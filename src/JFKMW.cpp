/*
	JFKMW cpp file. This is the main file of JFKMW and the start of the game.
	Some features are enabled/disabled in different platforms, such as Linux distributions.
	This is a very old codebase, and has been maintained for over 3 years. Some of the coding practices on here are not recommended to use normally (and I apologize for any shitcode), and are to be improved soon.
*/

#ifdef _WIN32

//Windows headers
#include <Windows.h>
#include "psapi.h"

#elif defined(__linux__)

//Linux headers
#define OTHER_INPUT_METHOD
#define VK_OEM_PERIOD 0xBE
#define VK_OEM_COMMA 0xBC
#define VK_OEM_MINUS 0xBD
#define VK_OEM_PLUS 0xBB

#include <unistd.h>
#include <list>
void Sleep(int time) {
	usleep(time * 1000);
}
#endif

//C++ Headers
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <ratio>
#include <chrono>
#include <ctype.h>
#include <cstring>
#include <cassert>
#include <filesystem>

//SNES SPC700 Emulator
#include "snes_spc/spc.h"

//Namespaces
using namespace std;
namespace fs = std::filesystem;

//Lua
#include "lua.hpp"

//SDL
#include <SDL.h>
#undef main
#ifdef _WIN32
#include <SDL_syswm.h>
#endif
#include <SDL_image.h>
#include <SDL_mixer.h>

//OPENGL
#include <GL/glew.h>

//LZ4
#include "lz4.h"

//SFML
#ifndef DISABLE_NETWORK
#include <SFML/Network.hpp>
#endif

#ifdef __linux__
#ifndef DISABLE_NETWORK
#define uint_fast8_t sf::Uint8
#define uint_fast16_t sf::Uint32
#define uint_fast32_t sf::Uint32
#define uint_fast64_t sf::Uint64
#define int_fast8_t sf::Int8
#define int_fast16_t sf::Int32
#define int_fast32_t sf::Int32
#define int_fast64_t sf::Int64
#endif

#undef max
#undef min
#define max(a, b) ((a<b)?b:a)
#define min(a, b) ((a>b)?b:a)

istream& getline(istream& stream, string& str) {
	char ch;
	str.clear();
	while (stream.get(ch)) {
		if (ch == '\r') {
			stream.get(ch);
			if (ch == '\n') {
				break;
			}
		}
		str.push_back(ch);
	}
	return stream;
}
#endif

//Game
#include "cout.h"
#include "videoglobals.h"
#include "globalgamevariables.h"
#include "OAM.h"
#include "DMA.h"
#include "HDMA.h"
#include "ASM.h"
#include "config.h"
#include "Sprite_shared.h"
#include "Particle_System.h"
#include "LuaImplementation.h"
#include "Sound.h"
#include "SDL_General.h"
#include "sprite.h"
#include "map16block.h"
#include "exanimation.h"
#include "level_and_load.h"
#include "input.h"
#include "Sprite_system.h"
#include "player.h"
#include "messageboxes.h"
#include "chat.h"
#include "debugging.h"
#include "overworld.h"
#include "transitions.h"
#include "titlescreen.h"
#include "game.h"
#ifndef DISABLE_NETWORK
#include "Netplay.h"
#endif
#include "renderer.h"
#ifndef DISABLE_NETWORK
#include "server.h"
#endif
#include "main.h"

int main(int argc, char* argv[]) {
#ifdef _WIN32
	std::set_terminate([]() { cout << red << "[JFKMW] Crashed. Unhandled exception." << endl; system("pause"); std::abort(); });
#endif
	load_configuration();

#ifndef DISABLE_NETWORK
	bool hosting = true;
	if (argc > 1) {
		if (strcmp(argv[1], "-h") == 0) { hosting = true; }
		if (strcmp(argv[1], "-c") == 0) { hosting = false; }
		if (strcmp(argv[1], "-l") == 0) {
			vector<string> args = split(argv[2], ',');
			testing_level = args[0];
			LoadPack(args[1]);
		}
	}

#ifdef _WIN32
	string t = "RaspcallionJFKMW Console - " + string(hosting ? "Server" : "Client") + " " + GAME_VERSION + " (" + __DATE__ + " " + __TIME__ + ")";
	SetConsoleTitle(wstring(t.begin(), t.end()).c_str());
#endif

	if (hosting) {
		server_code();
	}
	else {
		PORT = 25500;
		networking = true;
		gamemode = GAMEMODE_ATTEMPTCONNECTION;
		player_code();
	}
#else
	player_code();
#endif

	return 1;
}
//35353735204E69782052442C204661796574746576696C6C65204E43 一見Watonあなたがpichulazoをつかんcagao Iキャッチと太極拳くちばし渡しIは、Web-IOバッグにvengai私にはここにいないんだリータ再culiao mantecolもWEAを残して、古き良き仲間Cornetinでocupao私を確認するあなたを残しconchetumadre'll あなたのお母さんのパーティー、太極拳avisao conchetumare