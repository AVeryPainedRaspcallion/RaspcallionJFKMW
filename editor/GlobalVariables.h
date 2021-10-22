#pragma once

//For use with inputbox.h
string GetStringInputBox(string text) {
	string Inputted = InputBox((char*)text.c_str(), (char*)"Solar Energy: Beyond Limits");
	return Inputted;
}

int GetValueInputBox(string text, int base = 10) {
	string Inputted = InputBox((char*)text.c_str(), (char*)"Solar Energy: Beyond Limits");
	int IntConversion = 0;
	safe_stringint(Inputted, &IntConversion, base);
	return IntConversion;
}

//Show message with win32
void ShowMessage(string caption) {
	MessageBoxA(NULL, caption.c_str(), "Solar Energy: Beyond Limits", MB_OK);
}

//Data
int hudScale = 32;
int camX, camY;
int camOffsetX, camOffsetY;
int mapWidth, mapHeight;
uint_fast8_t MAP16[0x4000];
string CurrentLevel = "1";
string CurrentPack;
string CurrentFile;
string CurrentOverworld;

SDL_Rect blockarea;
SDL_Rect blockareares;

//Some variables (move to global)
int MouseSelected = 0;
int MouseOffToWindowX = 0;
int MouseOffToWindowY = 0;

//Some selections
bool MouseResizing = false;
bool MouseResizingX = false;
bool MouseResizingY = false;
bool MouseDoResizeX = false;
bool MouseDoResizeY = false;
bool SnapSprites = false;
int SelectedLevelPart = -1;
int SelectedSpriteObject = -1;
uint_fast16_t selectedMap16Tile = 0x25;
uint_fast16_t selectedSpriteSpawn = 0x100;

//Level layout object, resizable
class LevelObject {
public:
	int x = 0;
	int y = 0;
	int size_x = 1;
	int size_y = 1;
	uint_fast16_t tile = 0;
};
vector<LevelObject> levelLData;

class SpriteObject {
public:
	int x = 0;
	int y = 0;
	int_fast8_t dir;
	uint_fast8_t num;
};
vector<SpriteObject> levelSData;

void cloneLevelInstance(int i) {
	if (i >= 0 && i < levelLData.size()) {
		LevelObject& part = levelLData[i];
		levelLData.push_back(LevelObject{ part.x, part.y, part.size_x, part.size_y, part.tile });
	}
}

void cloneSpriteInstance(int i) {
	if (i >= 0 && i < levelSData.size()) {
		SpriteObject& part = levelSData[i];
		levelSData.push_back(SpriteObject{ part.x, part.y, part.dir, part.num });
	}
}

void getPartRecs(int i) {
	if (i >= 0 && i < levelLData.size()) {
		LevelObject& part = levelLData[i];
		blockarea = { (part.x * hudScale) - camOffsetX, h - 22 - ((part.y + 1) * hudScale) + camOffsetY, part.size_x * hudScale, part.size_y * hudScale };
		blockareares = { blockarea.x + 4, blockarea.y + 4, blockarea.w - 8, blockarea.h - 8 };
	}
}

void getSpriteRecs(int i) {
	if (i >= 0 && i < levelSData.size()) {
		SpriteObject& spr = levelSData[i];
		blockarea = { ((spr.x * hudScale) >> 4) - camOffsetX, h - 22 - (((spr.y + 16) * hudScale) >> 4) + camOffsetY, hudScale, hudScale };
	}
}

//For manager, GFX names and lcoations.
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

//Installed Sprites
class installedSprite {
public:
	uint_fast8_t num;
	string name;
};
vector<installedSprite> installedSprites;

void InitInstalledSprites() {
	cout << yellow << "[SE] Loading sprite lists.." << endl;
	installedSprites.clear();
	ifstream FE(path + "Editor/spritelist.txt");
	if (FE.is_open()) {
		string line;
		int i = 0;
		while (getline(FE, line)) {
			auto delimiterPos = line.find("=");
			auto name = line.substr(0, delimiterPos);
			auto value = line.substr(delimiterPos + 1);
			installedSprites.push_back({ uint_fast8_t(stoi(name, nullptr, 16)), value });
		}
	}
	FE.close();
	cout << "[SE] Loaded " << installedSprites.size() << "/256 sprites from spritelist.txt" << white << endl;

	for (int i = 0; i < 0x100; i++) {
		bool found = false;
		for (int e = 0; e < installedSprites.size(); e++) {
			if (installedSprites[e].num == i) {
				found = true;
				break;
			}
		}
		if (!found) {
			installedSprites.push_back({ uint_fast8_t(i), "Unknown" });
		}
	}
}

//Video
uint_fast8_t VRAM[0x10000];
uint_fast32_t palette_array[256];
uint_fast8_t palette_data[512];
uint_fast8_t gammaRamp[32];

//Textures
SDL_Surface* MAP16_SURF;
SDL_Texture* MAP16_TEX;
SDL_Surface* BG_SURF;
SDL_Texture* BG_TEX;
SDL_Surface* UI_SURF;
SDL_Texture* UI_TEX;
SDL_Surface* SPR_SURF;
SDL_Texture* SPR_TEX;

//Assets and functions
void LoadAssetIntoVRAM(string file, int location) {
	location <<= 12;
	ifstream infile(path + file, ios::binary | ios::ate);
	long fsize = long(infile.tellg()); infile.seekg(0);
	infile.read((char*)&VRAM[location], fsize);
	infile.close();
}

//Palette
void LoadPaletteFile(string file) {
	for (int i = 0; i < 32; i++) {
		gammaRamp[i] = i << 3;
	}
	ifstream infile(path + file, ios::binary);
	infile.read((char*)palette_data, 0x200);
	infile.close();

	for (uint_fast16_t i = 0; i < 256; i++) {
		uint_fast16_t c = palette_data[i << 1] + (palette_data[(i << 1) + 1] << 8);
		palette_array[i] =
			0xFF000000 + (gammaRamp[c & 0x1F]) +
			(gammaRamp[(c >> 5) & 0x1F] << 8) +
			(gammaRamp[c >> 10] << 16);
	}
}

//It's how you can see riki2321 tiles in your house!
uint_fast8_t graphics_array[32];
void draw8x8_tile(int_fast16_t x, int_fast16_t y, uint_fast16_t tile, uint_fast8_t palette) {
	palette = palette << 4;	tile = tile << 5;
	uint_fast8_t color1 = 0;
	uint_fast8_t i = 0;
	uint_fast8_t index = 0;
	memcpy(graphics_array, &VRAM[tile], 32);
	for (index = 0; index < 8; index++) {
		uint_fast8_t ind = index << 1;
		for (i = 0; i < 8; i++) {
			uint_fast16_t y_p = y + index;
			uint_fast16_t x_p = 7 - i + x;
			color1 =
				((graphics_array[0 + ind] >> i) & 1) +
				(((graphics_array[1 + ind] >> i) & 1) << 1) +
				(((graphics_array[16 + ind] >> i) & 1) << 2) +
				(((graphics_array[17 + ind] >> i) & 1) << 3)
				;

			if (color1 != 0) {
				Uint32* p_screen = (Uint32*)(MAP16_SURF)->pixels + ((y_p << 9) + x_p);
				Uint32* p_screen2 = (Uint32*)(MAP16_SURF)->pixels + ((y_p << 9) + x_p + 256);

				*p_screen = palette_array[color1 + palette];
				*p_screen2 = palette_array[color1 + palette] ^ 0xFFFFFF;
			}
		}
	}
}

//File existance check.
bool is_file_exist(const char* fileName) {
	ifstream infile(fileName);
	return infile.good();
}

//Background loading
void load_background(uint_fast8_t num) {
	if (BG_TEX) { SDL_DestroyTexture(BG_TEX); BG_TEX = NULL; }
	if (BG_SURF) { SDL_FreeSurface(BG_SURF); BG_SURF = NULL; }

	string toload = path + "Sprites/backgrounds/Background" + int_to_hex(num, true) + ".png";
	string toload_alt = path + CurrentPack + "/levels/" + CurrentLevel + "/Background" + int_to_hex(num, true) + ".png";
	if (is_file_exist(toload_alt.c_str())) {
		toload = toload_alt;
	}
	BG_SURF = IMG_Load(toload.c_str());
	BG_TEX = SDL_CreateTextureFromSurface(ren, BG_SURF);
}

//Initialize UI
void InitializeUI() {
	string str = path + "Editor/UI.png";
	UI_SURF = IMG_Load(str.c_str());
	UI_TEX = SDL_CreateTextureFromSurface(ren, UI_SURF);
	str = path + "Editor/SPRITES.png";

	SDL_Surface* SPR_SURF_T = SDL_CreateRGBSurface(0, 1024, 512, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
	SPR_SURF = IMG_Load(str.c_str());
	SrcR = { 0, 0, 512, 512 };
	SDL_BlitSurface(SPR_SURF, &SrcR, SPR_SURF_T, &SrcR);

	for (int x = 512; x < 1024; x++) {
		for (int y = 0; y < 512; y++) {
			*((Uint32*)SPR_SURF_T->pixels + (x + y * 1024)) = *((Uint32*)SPR_SURF_T->pixels + (x - 512 + y * 1024)) ^ 0xFFFFFF;
		}
	}
	SPR_TEX = SDL_CreateTextureFromSurface(ren, SPR_SURF_T);
	SDL_FreeSurface(SPR_SURF);
	SDL_FreeSurface(SPR_SURF_T);
}

//Load MAP16 File
void LoadMap16File(string file) {
	ifstream input(path + file, ios::binary);
	if (!input.good()) { return; }
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
	uint8_t temp[16];
	int current_byte = 0;
	for (auto& v : buffer) {
		temp[current_byte] = uint8_t(v);
		current_byte++;
		if (current_byte >= 16) {
			uint16_t replace_p = (temp[1] + (temp[0] << 8)) & 0x3FF; //this is actually a thing.
			memcpy(&MAP16[replace_p << 4], &temp[0], 16);
			current_byte = 0;
		}
	}
	input.close();
}

//Render map16 texture
void InitializeMap16()
{
	if (MAP16_TEX) { SDL_DestroyTexture(MAP16_TEX); MAP16_TEX = NULL; }
	if (MAP16_SURF) { SDL_FreeSurface(MAP16_SURF); MAP16_SURF = NULL; }

	cout << "[SE] Loading assets..." << endl;
	MAP16_SURF = SDL_CreateRGBSurface(0, 512, 1024, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);

	//kys.. kys bro
	memset(&MAP16, 0, 0x4000 * sizeof(uint_fast8_t));
	LoadAssetIntoVRAM("Graphics/exanimations.bin", 8);
	LoadMap16File("Map16/global.jfkmap16");
	LoadMap16File(CurrentPack + "/levels/" + CurrentLevel + "/level_map16.jfkmap16");
	LoadPaletteFile(CurrentPack + "/levels/" + CurrentLevel + "/level_palette.mw3");

	//Brown block & Coin
	uint_fast16_t block_location = 0x58; //RAM[0x14AD] > 0 ? 0x6C : 0x58;
	uint_fast16_t coin_location = 0x6C; //RAM[0x14AD] > 0 ? 0x58 : 0x6C;
	memcpy(&VRAM[(block_location << 5)], &VRAM[0x8000 + (0xB4 * 32)], 128);
	memcpy(&VRAM[(coin_location << 5)], &VRAM[0x8000 + (0xCC * 32) + (((0 >> 3) & 3) << 9)], 128);
	memcpy(&VRAM[(32 * 0x60)], &VRAM[0x8000 + (0xC0 * 32) + (((0 >> 3) & 3) << 9)], 128);
	memcpy(&VRAM[(32 * 0xEA)], &VRAM[0x8000 + (0xC8 * 32) + (((0 >> 3) & 3) << 9)], 128);
	memcpy(&VRAM[(32 * 0x5C)], &VRAM[0x8000 + ((0xF8 + 0x40) * 32) + (((0 >> 3) & 1) << 7)], 128);
	memcpy(&VRAM[(32 * 0x70)], &VRAM[0x8000 + (0x100 * 32) + (((0 >> 3) & 3) << 9)], 128);
	memcpy(&VRAM[(32 * 0x4C)], &VRAM[0x8000 + (0x8C * 32) + (((0 >> 3) & 3) << 9)], 128);
	memcpy(&VRAM[(32 * 0xDA)], &VRAM[0x8000 + ((0xA0 + (0 << 4)) * 32)], 128);
	memcpy(&VRAM[(32 * 0x68)], &VRAM[0x8000 + ((0xC0 + 0x80) * 32) + (((0 >> 3) & 3) << 9)], 128);
	memcpy(&VRAM[(32 * 0x80)], &VRAM[0xA900 + (((0 >> 3) & 3) << 9)], 64);
	memcpy(&VRAM[(32 * 0x90)], &VRAM[0xA940 + (((0 >> 3) & 3) << 9)], 64);
	memcpy(&VRAM[(32 * 0xAC)], &VRAM[0x8000 + ((0xC8 + (0 << 5)) * 32)], 128);
	memcpy(&VRAM[(32 * 0xCC)], &VRAM[0x8000 + ((0xE8 - (0 << 5)) * 32)], 128);
	memcpy(&VRAM[(32 * 0x74)], &VRAM[0x8000 + (0x104 * 32) + (((0 >> 3) & 3) << 9)], 128);
	for (int tile = 0; tile < 1024; tile++) {
		int TileOffset = tile << 4;
		int_fast16_t X = ((tile & 0xF) << 4);
		int_fast16_t Y = (tile >> 4) << 4;
		uint_fast16_t T1 = MAP16[TileOffset + 3] + (MAP16[TileOffset + 2] << 8);
		uint_fast16_t T2 = MAP16[TileOffset + 5] + (MAP16[TileOffset + 4] << 8);
		uint_fast16_t T3 = MAP16[TileOffset + 7] + (MAP16[TileOffset + 6] << 8);
		uint_fast16_t T4 = MAP16[TileOffset + 9] + (MAP16[TileOffset + 8] << 8);
		uint_fast8_t P1 = MAP16[TileOffset + 10];
		uint_fast8_t P2 = MAP16[TileOffset + 11];
		draw8x8_tile(X, Y, T1, P1 >> 4);
		draw8x8_tile(X + 8, Y, T2, P1 & 0xF);
		draw8x8_tile(X, Y + 8, T3, P2 >> 4);
		draw8x8_tile(X + 8, Y + 8, T4, P2 & 0xF);
	}
	MAP16_TEX = SDL_CreateTextureFromSurface(ren, MAP16_SURF);
}