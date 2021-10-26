#pragma once

string GFX_Names[12] = {
	"gfx_1",
	"gfx_2",
	"gfx_3",
	"gfx_4",
	"gfx_5",
	"gfx_6",
	"gfx_7",
	"gfx_8",
	"sp_1",
	"sp_2",
	"sp_3",
	"sp_4"
};

int GFX_Locations[12] = {
	0,
	1,
	2,
	3,
	4,
	5,
	6,
	7,
	12,
	13,
	14,
	15
};

class LevelSprite {
public:
	uint_fast8_t number = 0;
	uint_fast16_t x_pos = 0;
	uint_fast16_t y_pos = 0;
	uint_fast8_t newstate = 0;
	uint_fast8_t dir = 0;

	void spawn() {
		spawnSpriteObj(number, newstate, x_pos, y_pos, dir);
	}
};

vector<LevelSprite> LevelSprites; //U know what they say. All toasters toast.

//This handles offscreen shit
void CheckSpritesInCam(int x_pos, int y_pos) {
	for (uint_fast16_t i = 0; i < LevelSprites.size(); i++) {
		bool can_spawn = false;
		LevelSprite& CSprite = LevelSprites[i];
		if (use_vertical_spawning) {
			int x = int(CSprite.x_pos);
			int y = int(CSprite.y_pos);
			can_spawn = y > (y_pos - spawn_bound_y) && y < (y_pos + spawn_bound_y) && x > (x_pos - spawn_bound_x) && x < (x_pos + spawn_bound_x);
		}
		else {
			int x = int(CSprite.x_pos);
			can_spawn = x > (x_pos - spawn_bound_x) && x < (x_pos + spawn_bound_x);
		}
		if (can_spawn) {
			CSprite.spawn();
			LevelSprites.erase(LevelSprites.begin() + i);
			i--;
		}
	}

	for (uint_fast8_t i = 0; i < 128; i++) {
		bool onscreen = RAM[0x2A80 + i] & 1;
		if (!onscreen) {
			if (use_vertical_spawning) {
				int x = RAM[0x2100 + i] + Sint8(RAM[0x2180 + i]) * 256;
				int y = RAM[0x2280 + i] + Sint8(RAM[0x2300 + i]) * 256;
				onscreen = y > (y_pos - spawn_bound_y) && y < (y_pos + spawn_bound_y) && x >(x_pos - spawn_bound_x) && x < (x_pos + spawn_bound_x);
			}
			else {
				int x = RAM[0x2100 + i] + Sint8(RAM[0x2180 + i]) * 256;
				onscreen = x > (x_pos - spawn_bound_x) && x < (x_pos + spawn_bound_x);
			}
		}
		if (onscreen && !(RAM[0x2A80 + i] & 2)) {
			RAM[0x2A80 + i] ^= 2;
		}
	}
}

class Level
{
public:
	Level() {}

	string current_level;
	double start_x = 16;
	double start_y = 16;
	int chunks = 0;
	unordered_map<string, uint_fast32_t> level_data;

	uint_fast32_t request_level_entry(string name) {
		auto entry = level_data.find(name);

		if (entry != level_data.end()) {
			return entry->second;
		}

		return 0;
	}

	void add_entry(string name, uint_fast32_t value) {
		level_data.insert(make_pair(name, value));
	}

	string LoadLevelData(string FILENAME)
	{
		ifstream file;
		file.open(FILENAME);
		if (file.is_open())
		{

			stringstream strStream;
			strStream << file.rdbuf(); //read the file
			string str = strStream.str(); //str holds the content of the file
			file.close();
			return str;
		}
		file.close();
		return "Error";
	}
	void LoadLevelFromFile(string FILENAME, uint_fast16_t num)
	{
		current_level = FILENAME;
		cout << green
			<< "[Level Manager] Loading " << FILENAME
			<< endl;
		LoadLevelFromString(LoadLevelData(FILENAME), num);
	}

	void LoadLevelFromString(string DLevel, uint_fast16_t num)
	{
		chunks = 0;
		lua_loaded = false;
		reset_map();

		cout << green
			<< "[Level Manager] Processing level string.."
			<< endl;

		string status;
		string line;

		bool finished_message = false;

		for (int msg = 0; msg < 16; msg++) {
			Messages[msg] = "";
		}


		chrono::high_resolution_clock::time_point START_CHECK = chrono::high_resolution_clock::now();
		stringstream str(DLevel); // string


		LevelSprites.clear();
		//int spr_index = 0;
		while (getline(str, line)) {
			if (line != "" || status.substr(0,7) == "message") {
				string CHECK = line.substr(0, 2);
				if (CHECK != "//")
				{
					if (line.substr(0, 1) == "[")
					{
						status = line.substr(1, line.length() - 2);
						cout << green << "[Level Manager] Status = " << status << endl;
						finished_message = false;

						//data.push_back();
						continue;
					}

					if(status == "level_config")
					{
						line.erase(remove_if(line.begin(), line.end(), ::isspace),
							line.end());
						auto delimiterPos = line.find("=");
						auto name = line.substr(0, delimiterPos);
						auto value = line.substr(delimiterPos + 1);

						if (name == "music") {
							RAM[0x1DFB] = stoi(value, nullptr, 16); continue;
						}
						if (name == "background") {
							RAM[0x3F05] = stoi(value, nullptr, 16); continue;
						}
						if (name == "size_x") {
							writeToRam(0x3F00, stoi(value), 2);
							mapWidth = stoi(value);
						}
						if (name == "size_y") {
							writeToRam(0x3F02, stoi(value), 2);
							mapHeight = stoi(value);
						}
						if (name == "vertical") {
							use_vertical_spawning = stoi(value);
						}
						add_entry(name, stoi(value));
					}

					if (!finished_message)
					{
						for (int msg = 1; msg <= 16; msg++) {
							if (status == ("message" + to_string(msg)))
							{
								if (line == "<ENDMSG>") { finished_message = true; continue; }
								else
								{
									line.resize(18, ' ');
									Messages[msg - 1] += line;
								}
							}
						}
					}

					if (status == "scripts") {
						line.erase(remove_if(line.begin(), line.end(), ::isspace),
							line.end());
						auto delimiterPos = line.find("=");
						auto name = line.substr(0, delimiterPos);
						auto value = line.substr(delimiterPos + 1);
						auto type = name.substr(name.length() - 3);
						cout << green << "[Level Manager] Loading Script " << name << " of type " << type << endl;
						if (type == "lua") {
							lua_loadfile(Modpack + "/levels/" + int_to_hex(num) + "/" + name);
							lua_loaded = true;
							lua_run_init();
						}
					}

					if (status == "level_data")
					{
						vector<string> v = split(line.c_str(), ',');
						if (v.size() == 5) //Format type 1.
						{
							chunks++;
							int tile = stoi(v[0], nullptr, 16);
							int x_start = stoi(v[1]);
							int y_start = stoi(v[2]);
							int x_end = stoi(v[3]);
							int y_end = stoi(v[4]);

							for (int x = x_start; x <= x_end; x++)
							{
								for (int y = y_start; y <= y_end; y++)
								{
									map16_handler.replace_map_tile(tile, x, y);
								}
							}
						}
						if(v.size() == 3) //Format type 2
						{
							chunks++;
							int tile = stoi(v[0], nullptr, 16);
							int x = stoi(v[1]);
							int y = stoi(v[2]);

							map16_handler.replace_map_tile(tile, x, y);
						}
					}

					if (status == "sprite_data")
					{
						vector<string> v = split(line.c_str(), ',');

						//to keep compatibility with old levels.. this is not supported though
						int offset = v.size() == 5 ? 1 : 0;
						LevelSprites.push_back(
							LevelSprite{
								uint_fast8_t(stoi(v[offset], nullptr, 16)),
								uint_fast16_t(stoi(v[offset+1])),
								uint_fast16_t(stoi(v[offset+2])),
								1,
								uint_fast8_t(stoi(v[offset+3]))
							}
						);
					}

				}
			}

		}

		chrono::high_resolution_clock::time_point CURRENT_CHECK = chrono::high_resolution_clock::now();
		cout << green
			<< "[Level Manager] Finished loading level, took " << int(chrono::duration_cast<chrono::duration<double>>(CURRENT_CHECK - START_CHECK).count() * 1000) << "ms, " << chunks << " chunks loaded."
			<< endl;
	}

	void Initialize_Level()
	{
		Reset_BG();

		memset(&RAM[0x1462], 0, 8);
		memset(&RAM[0x22], 0, 4);
		memset(&player_netcommand, 0, 256);

		//Reset some vars
		RAM[0x14AF] = 0;
		RAM[0x7C] = 96;
		RAM[0x7B] = 0;
		RAM[0x7D] = 0;
		RAM[0x85] = 0;
		RAM[0x36] = 0;
		RAM[0x38] = 0x20;
		RAM[0x39] = 0x20;
		RAM[0x40] = 0;
		RAM[0x3F12] = 0;
		RAM[0x1493] = 0;
		RAM[0x1411] = 0;
		RAM[0x1887] = 0;
		RAM[0x1412] = 0;
		RAM[0x1426] = 0;
		RAM[0x1B89] = 0;
		RAM[0x1B88] = 0;
		RAM[0x1420] = 0;
		RAM[0x14AD] = 0;
		RAM[0x1490] = 0;
		RAM[0x0DDA] = 0;
		RAM[0x3F1B] = 0;
		RAM[0x3F1E] = 0;
		RAM[0x3F1F] = 0;
		doRetry = true;
		particles.clear();
		if (!midway_activated)
		{
			RAM[0x1420] = 0;
			RAM[0x0DBF] = 0;
		}

		ResetHDMA();
		ResetDMA();

		preloadSpriteCache();
	}

	void LoadLevel(uint_fast16_t num)
	{
		GameInitialize();
		level_data.clear();
		writeToRam(0x010B, num, 2);
		initialize_map16("Map16/global.jfkmap16");
		Initialize_Level();
		cout << green << "[Level Manager] Loading level " << int_to_hex(num) << endl;
		read_from_palette(Modpack + "/levels/" + int_to_hex(num) + "/level_palette.mw3");
		initialize_map16(Modpack + "/levels/" + int_to_hex(num) + "/level_map16.jfkmap16");
		LoadLevelFromFile(Modpack + "/levels/" + int_to_hex(num) + "/level_data.txt", num);

		deleteAssetCache();
		for (int i = 0; i < 12; i++)
		{
			loadAssetRAM("Graphics/GFX" + int_to_hex(request_level_entry(GFX_Names[i]), true) + ".bin", GFX_Locations[i]);
			loadAssetRAM(Modpack + "/levels/" + int_to_hex(num) + "/" + GFX_Names[i] + ".bin", GFX_Locations[i]);
		}

		//Level-specific sprites
		loadSpriteGroup(Modpack + "/levels/" + int_to_hex(num) + "/sprite");

		//BG & Sound
		for (uint_fast8_t i = 0; i < 16; i++) {
			BGObject[i].Load(i + 0xF0);
		}
		int Sounds = 0;
		for (uint_fast8_t i = 0; i < 32; i++) {
			Sounds += SpecChunksLevel[i].Load(i + 0xE0);
		}
		if (Sounds > 0) {
			cout << purple << "[Audio] Loaded " << Sounds << " custom sounds." << endl;
		}
		//Midway
		if (networking && !isClient)
		{
			midway_activated = false;
		}
		if (!midway_activated) //if is server or midway isn't activated, load the start pos
		{
			writeToRam(0x3F0B, request_level_entry("start_x") * 16, 2);
			writeToRam(0x3F0D, request_level_entry("start_y") * 16, 2);

			start_x = RAM[0x3F0B] + RAM[0x3F0C] * 256;
			start_y = RAM[0x3F0D] + RAM[0x3F0E] * 256;
		}
		writeToRam(0xF31, request_level_entry("time_limit") * 40, 2);
	}
};

Level LevelManager;


void load_level3f08() {
	if (getRamValue(0x3f08, 2) != 0) {
		gamemode = GAMEMODE_MAIN;
		if (getRamValue(0x3F08, 2) != getRamValue(0x010b, 2)) {
			midway_activated = false;
		}
		LevelManager.LoadLevel(uint_fast16_t(getRamValue(0x3f08, 2)));
		writeToRam(0x3f08, 0, 2);
	}
	else {
		writeToRam(0x1DFB, 0, 1);
	}
}