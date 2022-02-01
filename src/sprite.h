#pragma once

//Texture storage variables
unordered_map<string, GL_Texture> Textures_GL;
GL_Texture bg_texture_GL[2];
uint_fast16_t curr_bg[2] = { 0xFFFF, 0xFFFF };
uint_fast8_t bg_load_called = 0;
void Reset_BG() {
	curr_bg[0] = 0xFFFF; curr_bg[1] = 0xFFFF;
}
//Convert Surface to SMW/SNES Gamma Ramp
void GammaSurface(SDL_Surface* loadedSurface) {
	if (loadedSurface != nullptr && loadedSurface->format->BytesPerPixel >= 3) {
		for (int x = 0; x < loadedSurface->w; x++) {
			for (int y = 0; y < loadedSurface->h; y++) {
				Uint8* p_screen = (Uint8*)loadedSurface->pixels;
				p_screen += (y * loadedSurface->w + x) * loadedSurface->format->BytesPerPixel;
				p_screen[0] = gammaRamp[p_screen[0] >> 3]; p_screen[1] = gammaRamp[p_screen[1] >> 3]; p_screen[2] = gammaRamp[p_screen[2] >> 3];
			}
		}
	}
}

SDL_Surface* loadSurface(string pathD) {
	SDL_Surface* loadedSurface = IMG_Load(pathD.c_str());
	if (loadedSurface != nullptr) {
		GammaSurface(loadedSurface); return loadedSurface;
	}
	else {
		return nullptr;
	}
}

GL_Texture loadSDLTextureGL(string file) {
	auto entry = Textures_GL.find(file);
	if (entry != Textures_GL.end()) {
		return entry->second;
	}
	SDL_Surface* s = loadSurface(file); if (s == nullptr) { return 0; }
	GL_Texture Tex; glGenTextures(1, &Tex);
	ConvertSDLSurfaceToOpenGL(Tex, s);
	cout << purple_int << "[OpenGL] Loaded texture " << file << " (ID: " << Tex << ")" << endl;
	SDL_FreeSurface(s); Textures_GL.insert(make_pair(file, Tex));
	return Tex;
}

void PreloadAllTextures() {
	cout << purple_int << "[TexManager] Preloading all textures." << endl;
	string fPath = "Sprites";
	for (const auto& entry : fs::recursive_directory_iterator(fPath)) {
		string impath = string(entry.path().u8string());
		replace(impath.begin(), impath.end(), '\\', '/'); // replace all 'x' to 'y'
		loadSDLTextureGL(impath);
	}
	cout << purple_int << "[TexManager] Loaded " << Textures_GL.size() << " textures." << endl;
}

void CreateSprite(string sprite, int x, int y, int size_x, int size_y) {
	DestR.x = x + (size_x < 0 ? abs(size_x) : 0);
	DestR.y = y;
	DestR.w = size_x; DestR.h = size_y;
	RenderCopyOpenGL(&DestR, loadSDLTextureGL(sprite));
}

void CreateSpriteCrop(string sprite, int x, int y, int size_x, int size_y, int src_x, int src_y, int src_w, int src_h, uint_fast8_t rainbow_t = 0)
{
	DestR = { x, y, 0, 0 };
	SrcR = { src_x, src_y, abs(size_x), abs(size_y) };
	uint_fast8_t rc = 255; uint_fast8_t gc = 255; uint_fast8_t bc = 255;
	if (rainbow_t) {
		uint_fast8_t divider = rainbow_t < 0x20 ? 4 : 2; uint_fast8_t counter = (ingame_frame_counter / divider) % 3;
		rc = (!counter) * 255; gc = (counter == 1) * 255; bc = (counter == 2) * 255;
	}
	DestR.x = x + (size_x < 0 ? abs(size_x) : 0);
	DestR.w = size_x; DestR.h = size_y;
	RenderCopyOpenGLEx(&SrcR, &DestR, loadSDLTextureGL(sprite), src_w, src_h, 0.f, rc, gc, bc);
}

class CustomBGObject {
public:
	//Storage
	SDL_Surface* Surface_Temp = NULL;
	uint_fast8_t ID;
	uint_fast8_t* png_data_raw = nullptr;
	int png_data_size;
	bool databusy = false;

	//Unload PNG_DATA_RAW
	void Unload()
	{
		if (Surface_Temp != NULL) {
			SDL_FreeSurface(Surface_Temp);
			Surface_Temp = NULL;
		}
		if (png_data_raw != nullptr) {
			delete[] png_data_raw; png_data_raw = nullptr;
		}
		png_data_size = 0;
	}

#ifndef DISABLE_NETWORK
	//Load this chunk from net
	void LoadFromNet() {
		Unload();
		CurrentPacket >> png_data_size;
		if (png_data_size > 0 && png_data_size <= CurrentPacket.getDataSize()) {
			png_data_raw = new uint_fast8_t[png_data_size];
			for (int i = 0; i < png_data_size; i++) {
				CurrentPacket >> png_data_raw[i];
			}
		}
	}

	//Send this chunk through net
	void SendNet() {
		//yeah we wouldn't want to send a unfinished texture
		while (databusy) { DATA_SAFETY_WAIT }
		CurrentPacket << png_data_size;
		if (png_data_size > 0) { CurrentPacket.append(png_data_raw, png_data_size); }
	}
#endif

	//Tex
	void LoadTexture() {
		if (networking && png_data_raw != nullptr && png_data_size > 0) {
			SDL_RWops* rw = SDL_RWFromMem(png_data_raw, png_data_size); Surface_Temp = IMG_Load_RW(rw, 0); SDL_FreeRW(rw);
		}
		if (Surface_Temp) {
			GammaSurface(Surface_Temp); ConvertSDLSurfaceToOpenGL(cached_bg_sprites[ID & 0xF], Surface_Temp);
		}
	}

	//Load
	void Load(uint_fast8_t N_ID) {
		ID = N_ID;
		string file1 = Modpack + "/levels/" + int_to_hex(getRamValue(0x010B, 2), false) + "/Background" + int_to_hex(ID, true) + ".png";
		if (networking) {
			databusy = true;
			Unload();
			ifstream input(file1, ios::in | ios::binary | ios::ate);
			if (input.is_open()) {
				png_data_size = int(input.tellg()); png_data_raw = new uint_fast8_t[png_data_size];
				input.seekg(0, ios::beg); input.read((char*)png_data_raw, png_data_size); input.close();
				cout << yellow << "[BG] Loaded per-level background " << int_to_hex(ID, true) << endl;
			}
			databusy = false;
		}
		else {
			Unload();
			Surface_Temp = IMG_Load(file1.c_str());
			if (Surface_Temp) {
				cout << yellow << "[BG] Loaded per-level background " << int_to_hex(ID, true) << endl;
				LoadTexture();
			}
		}
	}
};
CustomBGObject BGObject[16];

void ProcessBG(uint_fast16_t R, uint_fast8_t I) {
	if (RAM[R] != curr_bg[I] || RAM[R] >= 0xF0) {
		curr_bg[I] = RAM[R];
		if (curr_bg[I] >= 0xF0) {
			bg_texture_GL[I] = cached_bg_sprites[curr_bg[I] - 0xF0];
		}
		else {
			bg_texture_GL[I] = loadSDLTextureGL("Sprites/backgrounds/Background" + int_to_hex(curr_bg[I], true) + ".png");
		}
	}
}

void loadBG() {
	if (bg_load_called == 2) {
		bg_load_called = 3;
		for (uint_fast8_t i = 0; i < 16; i++) {
			BGObject[i].LoadTexture();
		}
		bg_load_called = 0;
	}
	ProcessBG(0x3F05, 0);
	ProcessBG(0x3F1E, 1);
}

//Networking-related.
#ifndef DISABLE_NETWORK
void SendBackgrounds() {
	for (int i = 0; i < 16; i++) {
		BGObject[i].SendNet();
	}
}

void ReceiveBackgrounds() {
	while (bg_load_called & 1) {
		DATA_SAFETY_WAIT
	}
	bg_load_called = 1;
	for (int i = 0; i < 16; i++) {
		BGObject[i].LoadFromNet();
	}
	bg_load_called = 2;
}
#endif