#pragma once

string GAME_VERSION = "4.1.0b";

//General
#define Calculate_Speed(x) double(x) / 256.0
#define TotalBlocksCollisionCheck 2
#define bounds_x 8.0 //for collision crap
#define bounds_y 8.0
#define bounds_y_sprite 8.0
#define button_b 0
#define button_y 1
#define button_a 2
#define button_left 3
#define button_right 4
#define button_up 5
#define button_down 6
#define button_start 7
#define camBoundX 32.0
#define camBoundY 32.0
#define total_inputs 8
#define player_expected_packet_size 69 //Strings apparently add 4 so we have to make sure of this so it wont crash.
#define player_name_size 11
#define ram_level_low 0x10000
#define ram_level_high 0x18000
#define MAX_L3_TILES 0x80
#define level_ram_decay_time 40 //Server ticks before level data RAM becomes invalid to send

//Misc
uint_fast16_t mapWidth = 256;
uint_fast16_t mapHeight = 32;

//Audio
uint_fast8_t audio_sync[5];
uint_fast8_t audio_id[5];
int ogg_sample_rate = 44100;
int spc_buffer_size = 128;
int sfx_volume = 128;
int music_volume = 128;
bool multichannel_sounds = false;
string midi_patchset = "NONE";

//RAM sizes.
#define RAM_Size 0x34000
#define VRAM_Size 0x14000
#define VRAM_Location 0x20000
#define VRAM_Convert(x) VRAM_Location+x
#define LEVEL_DECAY_SIZE 0x8000
#define RAM_OLD_SIZE 0x7000
#define MAP16_LOCATION 0xC000
#define LEVEL_SIZE 0x8000

//useful macros for when doing_write or doing_read are modified, or just for data safety when transferring things between net thread and main game thread.
#ifdef DISABLE_NETWORK
#define DATA_SAFETY_WAIT Sleep(1);
#else
#define DATA_SAFETY_WAIT sf::sleep(sf::milliseconds(1));
#endif
#define WAIT_READ_COMPLETE chrono::high_resolution_clock::time_point START_CHECK = chrono::high_resolution_clock::now(); chrono::high_resolution_clock::time_point CURRENT_CHECK; bool hung_check = false; \
while (doing_write || doing_read) { \
	DATA_SAFETY_WAIT \
	CURRENT_CHECK = chrono::high_resolution_clock::now(); \
	if(chrono::duration_cast<chrono::duration<double>>(CURRENT_CHECK - START_CHECK).count() > 1.0 && !hung_check) { \
		hung_check = true; \
		cout << red << "[Timer] Read/write frozen. Game network might have crashed." << endl; \
	} \
}

//Debug
bool special_input_disabled = false;
bool debugging_enabled = false;

//RAM
uint_fast8_t RAM[RAM_Size];
uint_fast8_t RAM_decay_time[RAM_OLD_SIZE];
uint_fast8_t RAM_decay_time_level[LEVEL_DECAY_SIZE]; //for multiplayer
uint_fast8_t* RAM_compressed = new uint_fast8_t[RAM_Size];
uint_fast8_t* OAM_data_comp = new uint_fast8_t[0x10000];
uint_fast16_t hdma_size[8];
uint_fast32_t palette_array[256]; //These are cached lol
uint_fast8_t VRAM[VRAM_Size];
uint_fast8_t pipe_colors[4] = { 3, 5, 6, 7 };

//Some bitflags.
#define top 8
#define bottom 4
#define left 2
#define right 1
#define inside 0

//DMA/DHMA
bool hdmaModeEnabled[4];
int_fast16_t hdmaLineData[512][4];

#define HDMA_L1_MODEX 0
#define HDMA_L1_MODEY 1
#define HDMA_L2_MODEX 2
#define HDMA_L2_MODEY 3

//Threads
#ifndef DISABLE_NETWORK
sf::Thread* thread = 0;
sf::Thread* thread_alt = 0;
#endif

//Player Frames
#define POSE_STAND 0
#define POSE_WALK 1
#define POSE_RUN 4
#define POSE_JUMP 7
#define POSE_JUMPB 8
#define POSE_FALL 9
#define POSE_SLIDE 10
#define POSE_CROUCH 11
#define POSE_CLIMB 12
#define POSE_LOOKUP 13
#define POSE_BACK 14
#define POSE_FRONT 15
#define POSE_SKID 16
#define POSE_DEAD 17
#define POSE_KICK 18
#define POSE_GRAB 19 //and grab walk
#define POSE_GRABLOOKUP 22
#define POSE_GRABCROUCH 23
#define POSE_SWIM 24
#define POSE_POWER_TRANSITION 25
#define POSE_POWER_FIRE 26
uint_fast8_t PlayerSpinFrames[4] = { POSE_FRONT, POSE_STAND, POSE_BACK, POSE_STAND };
int_fast8_t PlayerSpinScale[4] = { 1, 1, -1, -1 };
uint_fast8_t PlayerCapeSmokeFrames[5] = { 0x60,0x62,0x60,0x62,0x66 };

//ASM
#define GAMEMODE_TITLE 0
#define GAMEMODE_OVERWORLD 1
#define GAMEMODE_MAIN 2
#define GAMEMODE_ATTEMPTCONNECTION 3

uint_fast8_t gamemode = GAMEMODE_TITLE;
uint_fast8_t my_skin = 0;
uint_fast32_t global_frame_counter = 0; //Like 0x13
uint_fast32_t ingame_frame_counter = 0; //Like 0x14

chrono::duration<double> total_time_ticks;
int latest_server_response;

unsigned int network_update_rate = 16;
unsigned int packet_wait_time = 16;
unsigned int network_update_rate_c = 16;
unsigned int packet_wait_time_c = 16;
int mouse_x, mouse_y;
bool mouse_down;
bool mouse_down_r;
bool mouse_w_up;
bool mouse_w_down;
bool use_mouse = true;

bool automatic_fps_cap = false;
Uint32 automatic_fps_cap_start = 0;

bool local_multiplayer = false;
bool players_synced = true;

int data_size_current = 0;
int data_size_now = 0;

int controller = 0;
int haptic = 0;

string testing_level = "";

//Sprite spawn bounds
#define spawn_bound_x 256
#define spawn_bound_y 140

//Stuff hud toggles here and hud stuff
uint_fast8_t hudMode = 0;
bool drawDiag = false;
bool drawBg = true;
bool drawSprites = true;
bool drawL1 = true;

int blocks_on_screen = 0;

//Statuses/Username
string username = "No username";
string last_status = "";
string latest_error = "";

///TO-DO: Make chat more secure and unspammable (For now you can't send similar messages, which is good.)
#define chat_onscreen_timer 420
string Curr_ChatString[6] = { "","","","","","" };
char Number_Caps[10] = { '=','!',':', '#', '$', '%', '_', '/', '(', ')' };
string Curr_PChatString = "";
string Old_ChatString = "";
string Raw_PChatString = "";
int Time_ChatString[6] = { 0,0,0,0,0,0 }; //How long it will take for this string to disappear, set to 300 every time a message is sent. In frames

//Input related stuff
SDL_Scancode input_settings[18] = {
	SDL_SCANCODE_S,
	SDL_SCANCODE_Z,
	SDL_SCANCODE_X,
	SDL_SCANCODE_A,
	SDL_SCANCODE_LEFT,
	SDL_SCANCODE_RIGHT,
	SDL_SCANCODE_DOWN,
	SDL_SCANCODE_UP,
	SDL_SCANCODE_LSHIFT,
	SDL_SCANCODE_RSHIFT,
	SDL_SCANCODE_T,
	SDL_SCANCODE_1,
	SDL_SCANCODE_2,
	SDL_SCANCODE_3,
	SDL_SCANCODE_4,
	SDL_SCANCODE_5,
	SDL_SCANCODE_6,
	SDL_SCANCODE_7
};
string button_configurations[18] = {
	"button_y", "button_b", "button_a", "button_x",
	"button_left", "button_right", "button_down", "button_up",
	"button_select","button_start","button_chat",
	"button_togglehud","button_togglediag","button_togglelayer1",
	"button_togglebg","button_togglesprites","button_dumpram","button_dumplevel" };
bool s_pad[total_inputs];
bool pad_p[total_inputs];
bool pad_s[total_inputs];

bool BUTTONS_GAMEPAD[10];

const Uint8* state = SDL_GetKeyboardState(NULL);
SDL_GameController* gGameController[4];
SDL_Haptic* haptic_device;

double controller_mouse_x = 0.0;
double controller_mouse_y = 0.0;

bool left_st_pr = false;
bool right_st_pr = false;

bool esc_pr = false;

//Some shared functions for files & strings.
vector<string> split(const string &s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}

void replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}

bool is_file_exist(const char* fileName) {
	ifstream infile(fileName);
	return infile.good();
}

string int2hexcache[16] = { "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F" };
string int_to_hex(int T, bool add_0 = false) {
	string STR = "";
	if (T < 16 && add_0) {
		STR = "0" + int2hexcache[T & 0xF];
	}
	else {
		while (T > 0) {
			STR = int2hexcache[T & 0xF] + STR; T >>= 4;
		}
	}
	return STR;
}

uint_fast8_t char_to_smw(char l) {
	if (l >= 0x30 && l < 0x3A) { return l - 0x30; }
	if (l >= 0x41 && l <= 0x5A) { return l - 0x41 + 0xA; }
	if (l >= 0x61 && l <= 0x7A) { return l - 0x61 + 0x50; }
	if (l == '<') { return 0x2C; }
	if (l == '>') { return 0x2D; }
	if (l == '!') { return 0x28; }
	if (l == '.') { return 0x24; }
	if (l == ',') { return 0x25; }
	if (l == '-') { return 0x27; }
	if (l == '+') { return 0x29; }
	if (l == '_') { return 0x3D; }
	if (l == '?') { return 0x38; }
	if (l == '(') { return 0x3E; }
	if (l == ')') { return 0x3C; }
	if (l == ':') { return 0x2B; }
	if (l == '\'') { return 0x39; }
	if (l == '/') { return 0x3A; }
	return 0xFF;
}

int safe_stoi(std::string str, int base) {
	try {
		return stoi(str, nullptr, base);
	}
	catch (...) {
		return 0;
	}
}

bool quit = false;
bool actuallyquitgame = false;
//NET
bool doing_write = false;
bool doing_read = false;
bool recent_big_change = false;
bool disconnected = false;
bool isClient = false;

//Game
bool pvp = true;
bool Chatting = false;
bool use_vertical_spawning = false;
double WaterLevel;

//WP Retry
bool useRetry = true;
bool retryPromptOpen = false;
bool doRetry = true;

//Stuff
string Typing_In_Chat = "";

bool showing_player_list;
bool pressed_select;
bool pressed_start;

bool need_preload_sprites = false;
bool ow_has_been_loaded = false;

bool smooth_camera = false;
bool midway_activated = false;
double smooth_camera_speed = 0;
uint_fast8_t SelfPlayerNumber = 0;
uint_fast8_t PlayerAmount = 0;
uint_fast8_t player_netcommand[256];

//Game Data Pack
string Modpack;
void LoadPack(string NewPack) {
	if (fs::is_directory("Packs/" + NewPack)) { Modpack = "Packs/" + NewPack;  }
	else { Modpack = "Packs/default"; }
	cout << yellow << "[JFKMW] Modpack switched to " << Modpack << endl;
	ow_has_been_loaded = false;
}

//Messages
string Messages[16];
int MessageBoxTimer = 0;

//Network
string ip = "127.0.0.1"; int PORT = 0;
vector<string> username_storage;

#ifndef DISABLE_NETWORK
class GNetSocket : public sf::TcpSocket
{
public:
	string username = "";

	uint_fast8_t latest_sync_p = 0;
	int sync_tries = 0;
	int last_send_timer = 0;

	uint_fast8_t music_latest_sync_p = 0;
	int music_sync_tries = 0;
	int last_music_send_timer = 0;
};

GNetSocket socketG; sf::SocketSelector selector; //no idea how this works
sf::TcpListener listener; vector<GNetSocket*> clients;
vector<sf::IpAddress> bans;
uint_fast8_t latest_sync;
uint_fast8_t music_latest_sync;

uint_fast8_t CurrentPacket_header;
//TO-DO : add packet compression. (Is this even possible?)
sf::Packet CurrentPacket;

//Trigger major change
void TriggerRAMSync() {
	need_preload_sprites = true; recent_big_change = true; latest_sync++;
}
#else
void TriggerRAMSync() {
	need_preload_sprites = true;
}
#endif

//Discord webhook logging for servers, only works in WIN32
#if !defined(DISABLE_NETWORK) && defined(_WIN32)
	string discord_webhook;
	void do_d_msg(string msg) {
		replaceAll(msg, "@", ""); replaceAll(msg, "<", "**["); replaceAll(msg, ">", "]**");

		time_t currentTime; struct tm localTime;
		time(&currentTime);                   // Get the current time
		localtime_s(&localTime, &currentTime);  // Convert the current time to the local time
		int Hour = localTime.tm_hour; int Min = localTime.tm_min; int Sec = localTime.tm_sec;
		string H, M, S;
		H = Hour < 10 ? ("0" + to_string(Hour)) : to_string(Hour);
		M = Min < 10 ? ("0" + to_string(Min)) : to_string(Min);
		S = Sec < 10 ? ("0" + to_string(Sec)) : to_string(Sec);

		msg = "[" + H + ":" + M + ":" + S + "] " + msg;
		string cmd = "start /b cmd /c curl --silent -o nul -i -H \"Accept: application/json\" -H \"Content-Type:application/json\" -X POST --data \"{\\\"content\\\": \\\"" + msg + "\\\"}\" " + discord_webhook;
		system(cmd.c_str());
		return;
	}
	void discord_message(string msg) {
		if (discord_webhook != "") {
			sf::Thread t1(do_d_msg, msg);
			t1.launch();
		}
	}
#else
	string discord_webhook;
	void discord_message(string msg) {
	}
#endif

//Palette array
void ConvertPalette() {
	if (gamemode == GAMEMODE_OVERWORLD) {
		uint_fast8_t b = ingame_frame_counter;
		if ((ingame_frame_counter & 0x1F) > 0x0F) {
			b = 0x10 - (ingame_frame_counter - 0x10);
		}
		b = b << 4;
		uint_fast16_t col = 0x3FF + ((b >> 3) << 10);
		RAM[0x3D6D] = col; RAM[0x3E6D] = col >> 8;

		col = 0x1F + ((b >> 3) << 5);
		RAM[0x3D7D] = col; RAM[0x3E7D] = col >> 8;

	}
	else {
		uint_fast8_t b = global_frame_counter;
		if ((global_frame_counter & 0x1F) > 0x0F) {
			b = 0x10 - (global_frame_counter - 0x10);
		}
		b = b << 4;
		uint_fast16_t col = 0x3FF + ((b >> 3) << 10);
		RAM[0x3D64] = col; RAM[0x3E64] = col >> 8;
	}
	//Plr Name Color
	switch (my_skin % 3)
	{
	case 0:
		palette_array[0x0E] = 0xFF1838D8; break;
	case 1:
		palette_array[0x0E] = 0xFF58F858; break;
	case 2:
		palette_array[0x0E] = 0xFF505050; break;
	}

	//Hud border colors
	palette_array[0x09] = 0xFF000000;
	palette_array[0x19] = 0xFF000000;
	palette_array[0x0D] = 0xFF000000;
	palette_array[0x1D] = 0xFF000000;
	palette_array[0x0A] = 0xFF000000;
	palette_array[0x0B] = 0xFF000000;

	//Itembox
	palette_array[0x0F] = 0xFFF0A858;

	//Text 1 (White)
	palette_array[0x1A] = 0xFFFFFFFF;
	palette_array[0x1B] = 0xFFFFFFFF;

	//Text 2 (Yellow)
	if (gamemode == GAMEMODE_OVERWORLD) {
		palette_array[0x1D] = 0x00000000;
		palette_array[0x1E] = 0xFF000000;
		palette_array[0x1F] = 0xFF000000;
	}
	else {
		palette_array[0x1E] = 0xFF70D8F8;
		palette_array[0x1F] = 0xFF70D8F8;
	}

	//Convert 16-bit palette to 32-bit palette
	for (uint_fast16_t i = 0; i < 256; i++) {
		if ((i >= 0x08 && i < 0x10) || (i >= 0x18 && i < 0x20)) {
			continue;
		}
		uint_fast16_t c = RAM[0x3D00 + i] + (RAM[0x3E00 + i] << 8);
		palette_array[i] =
			0xFF000000 + (gammaRamp[c & 0x1F]) +
			(gammaRamp[(c >> 5) & 0x1F] << 8) +
			(gammaRamp[c >> 10] << 16);
	}
}

//Sprite Caching
void PreloadSPR() {
	//This makes palette
	ConvertPalette();

	uint_fast8_t i, index, temporaryPixelBuffer[0x10000];
	uint_fast32_t px;
	for (uint_fast16_t tile_t = 0; tile_t < 0x400; tile_t++) {
		memcpy(graphics_array, &VRAM[0xC000 + (tile_t << 5)], 32);
		px = ((tile_t & 0xF) << 3) + 7 + ((tile_t >> 4) << 10);
		for (index = 0; index < 16; index += 2) {
			for (i = 0; i < 8; i++) {
				temporaryPixelBuffer[px] = ((graphics_array[index] >> i) & 1) + (((graphics_array[1 + index] >> i) & 1) << 1) + (((graphics_array[16 + index] >> i) & 1) << 2) + (((graphics_array[17 + index] >> i) & 1) << 3);
				px--;
			}
			px += 136;
		}
	}

	//Create a surface, only needed once
	SDL_Surface* cached_spr_surf = SDL_CreateRGBSurface(0, 128, 256, 32,
		rmask, gmask, bmask, amask);
	for (uint_fast16_t e = 0; e < 256; e += 16) {
		SDL_memset(cached_spr_surf->pixels, 0, cached_spr_surf->h * cached_spr_surf->pitch);
		for (px = 0; px < 0x10000; px++) {
			if (temporaryPixelBuffer[px]) {
				*((Uint32*)(cached_spr_surf)->pixels + px) = palette_array[temporaryPixelBuffer[px] + e];
			}
		}
		ConvertSDLSurfaceToOpenGL(cached_spr_tilesGL[e >> 4], cached_spr_surf);
	}
	SDL_FreeSurface(cached_spr_surf);
}

//Layer 3 Caching
void PreloadL3() {
	//This makes palette
	ConvertPalette();
	uint_fast8_t temporaryPixelBuffer[8192];
	//Draw all L3 tiles
	uint_fast8_t i, index; uint_fast32_t px = 0;
	for (uint_fast8_t t = 0; t < 0x80; t++) {
		px = ((t & 0xF) << 3) + 7 + ((t >> 4) << 10);
		memcpy(graphics_array, &RAM[VRAM_Convert(0xB000) + (t << 4)], 16 * sizeof(uint_fast8_t));
		for (index = 0; index < 16; index += 2) {
			for (i = 0; i < 8; i++) {
				temporaryPixelBuffer[px] = ((graphics_array[index] >> i) & 1) + (((graphics_array[index + 1] >> i) & 1) << 1);
				px--;
			}
			px += 136;
		}
	}
	SDL_Surface* cached_l3_surf = SDL_CreateRGBSurface(0, 128, 64, 32,
		rmask, gmask, bmask, amask);
	for (uint_fast16_t e = 0; e < 32; e+=4) {
		SDL_memset(cached_l3_surf->pixels, 0, cached_l3_surf->h * cached_l3_surf->pitch);
		for (px = 0; px < 8192; px++) {
			if (temporaryPixelBuffer[px]) {
				*((Uint32*)(cached_l3_surf)->pixels + px) = palette_array[temporaryPixelBuffer[px] + e];
			}
		}
		ConvertSDLSurfaceToOpenGL(cached_l3_tilesGL[e >> 2], cached_l3_surf);
	}
	SDL_FreeSurface(cached_l3_surf);
}

//Load GFX/Asset file (cached)
class cachedAssetObject {
public:
	~cachedAssetObject() {
		delete data;
	}
	uint_fast8_t* data;
	long datasize;
};
unordered_map<string, cachedAssetObject*> cachedAssets;

void deleteAssetCache() {
	for (unordered_map<string, cachedAssetObject*>::iterator it = cachedAssets.begin(); it != cachedAssets.end(); ++it) {
		delete it->second;
	}
	cachedAssets.clear();
}

void loadAssetRAM(string file, int offset = 0, bool doMultiply = true, bool useCache = false) {
	if (doMultiply) {
		offset = offset << 12;
	}

	//precached.
	if (useCache) {
		auto entry = cachedAssets.find(file);
		if (entry != cachedAssets.end()) {
			cachedAssetObject* En = entry->second;
			memcpy(&RAM[VRAM_Location + offset], En->data, En->datasize);
			TriggerRAMSync();
			return;
		}
	}

	ifstream input(file, ios::binary | ios::ate);
	if (input.is_open()) {
		long fsize = long(input.tellg()); input.seekg(0);
		if (useCache) { //Let's put this into cache, usually from Lua
			cachedAssetObject* newD = new cachedAssetObject();
			newD->datasize = fsize;
			newD->data = new uint_fast8_t[fsize];
			input.read((char*)newD->data, fsize);
			memcpy(&RAM[VRAM_Location + offset], newD->data, fsize);
			cachedAssets.insert(make_pair(file, newD));
		}
		else { //Just put this into RAM normally.
			input.read((char*)&RAM[VRAM_Location + offset], fsize);
		}
		TriggerRAMSync();
	}
	input.close();
}

//OAM Definition
struct OAMTile {
	//pos
	int_fast16_t pos_x = 0;
	int_fast16_t pos_y = 0;
	uint_fast8_t tile = 0; //expanded by props
	uint_fast8_t bsize = 0x00; //YYYYXXXX
	uint_fast16_t props = 0; //--MAFSRR CCYXPPPP
	uint_fast8_t rotation = 0; //0-255, 64 = 90*
	uint_fast8_t scale_x = 0x20; //0x20 = 1x, 0x10 = 2x, 0x40 = 0.5x
	uint_fast8_t scale_y = 0x20;
};
vector<OAMTile> OAM_Tiles;

//Gamemode init
void GameInitialize() {
	loadAssetRAM("Graphics/exanimations.bin", 8);
	loadAssetRAM("Graphics/hud.bin", 11);
	memset(&RAM[VRAM_Convert(0xB800)], 0xFF, 0x800);

	global_frame_counter = 0;
	ingame_frame_counter = 0;

	OAM_Tiles.clear();

	memset(&RAM[0x6000], 0, 0x4000); //Clear OW/Free part of RAM
	TriggerRAMSync();
}