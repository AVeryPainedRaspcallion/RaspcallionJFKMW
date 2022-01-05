#pragma once

////////////////////////////////////////////////////////////////////////////////
//BASIC SCREEN FUNCTIONS////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
HWND sdl_window;

static HMENU hFile;
static HMENU hInput;
static HMENU hSound;
static HMENU hVideo;
static HMENU hHelp;
static HMENU hMenuBar;

//Function which retrieves the address/Handle of an SDL window
//Also retrieves the specific subsystem used by SDL to create that window which is platform specific (Windows, MAC OS x, IOS, etc...)
void getSDLWinHandle() {
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(win, &wmInfo);
	sdl_window = wmInfo.info.win.window;
}

void ActivateMenu() {
	hMenuBar = CreateMenu();
	hFile = CreateMenu();
	hInput = CreateMenu();
	hSound = CreateMenu();
	hVideo = CreateMenu();
	hHelp = CreateMenu();

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"File");
	AppendMenu(hFile, MF_STRING, 1, L"Reload Configuration");
	AppendMenu(hFile, MF_STRING, 2, L"Exit");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hInput, L"Input");
	AppendMenu(hInput, MF_STRING, 3, L"Use config for now LOL");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hSound, L"Sound");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hVideo, L"Video");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hHelp, L"Help");
	AppendMenu(hHelp, MF_STRING, 4, L"About");

	SetMenu(sdl_window, hMenuBar);
}
#endif

//The screen function: sets up the window for 32-bit color graphics.
//Creates a graphical screen of width*height pixels in 32-bit color.
//Set fullscreen to false for a window, or to true for fullscreen output
//text is the caption or title of the window
//also inits SDL therefore you MUST call screen before any other InstantCG or SDL functions
void screen(int width, int height)
{
	cout << cyan << "[SDL] Creating window" << endl;
	if (win) { SDL_DestroyWindow(win); }
	w = width;
#ifdef _WIN32
	h = height + (fullscreen || borderless_fullscreen ? 0 : 20);
#else
	h = height;
#endif

	int flags = SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL;
	if (fullscreen) {
		flags |= SDL_WINDOW_FULLSCREEN;
	}
	else {
		flags |= SDL_WINDOW_RESIZABLE;
		if (borderless_fullscreen)
		{
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
		}
	}

	win = SDL_CreateWindow("RaspcallionJFKMW Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, flags);
	if (win == NULL) { cout << cyan << "[SDL] Window error: " << SDL_GetError() << endl; SDL_Quit(); exit(1); }
	cout << cyan << "[SDL] Window created, initializing renderer..." << endl;

	InitializeOpenGLViewport();

	cout << cyan << "[SDL] Initialized window of " << dec << width << "x" << height << " resolution." << endl;
	cout << cyan << "[SDL] Current video card : " << SDL_GetCurrentVideoDriver() << endl;
	screen_s_l1 = SDL_CreateRGBSurface(0, int_res_x + 16, int_res_y + 16, 32, rmask, gmask, bmask, amask);
	screen_s_l1_temp = SDL_CreateRGBSurface(0, int_res_x + 16, int_res_y + 16, 32, rmask, gmask, bmask, amask);
	SDL_SetSurfaceBlendMode(screen_s_l1, SDL_BLENDMODE_NONE);
	SDL_SetSurfaceBlendMode(screen_s_l1_temp, SDL_BLENDMODE_NONE);
	sur_res_x = int_res_x + 16;

#ifdef _WIN32
	//Enable WinAPI Events Processing
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	if (!fullscreen && !borderless_fullscreen) {
		getSDLWinHandle();
		ActivateMenu();
	}
#endif
}

void PrepareRendering() {
	if (!fullscreen) {
		SDL_GetWindowSize(win, &resolution_x, &resolution_y);
		w = resolution_x;
		h = resolution_y;
	}
	
	//Automatically set scale of game.
	if (!forced_scale) {
		scale = integer_scaling ? max(1, int(resolution_y / int_res_y)) : max(1.0, double(resolution_y) / double(int_res_y));
	}
	sp_offset_x = (w / 2) - int(double(int_res_x / 2) * scale);
	sp_offset_y = (h / 2) - int(double(int_res_y / 2) * scale);

}

void cls() {
	OpenGLClear();
	uint_fast16_t c = RAM[0x3D00] + (RAM[0x3E00] << 8);
	Ren_SetDrawColor(gammaRamp[c & 0x1F], gammaRamp[(c >> 5) & 0x1F], gammaRamp[(c >> 10) & 0x1F], 255);
	Ren_FillRect(nullptr);
}

void DrawMouse() {
	if (!use_mouse) {
		Ren_SetDrawColor(255, 255, 255, 127);
		DestR = { sp_offset_x + int(double(mouse_x) * scale), sp_offset_y + int(double(mouse_y) * scale), int(scale), int(scale) }; Ren_FillRect(&DestR);
	}
}

void redraw() {
	DrawMouse();
	OpenGLRedraw();
}

bool done() {
	//INPUT MOUSE WHEEL
	if (!networking && !gGameController[0]) {
		mouse_w_up = false;
		mouse_w_down = false;
	}

	if (automatic_fps_cap) {
		if (Uint32(1000 / 60) > (SDL_GetTicks() - automatic_fps_cap_start)) {
			SDL_Delay((1000 / 60) - (SDL_GetTicks() - automatic_fps_cap_start));
		}
		automatic_fps_cap_start = SDL_GetTicks();
	}

	//SDL EVENTS
	while (SDL_PollEvent(&event)) {
		//QUIT
		if (event.type == SDL_QUIT) {
			testing_level = "";
			return true;
		}
		//WHEEL
		if (event.type == SDL_MOUSEWHEEL) {
			mouse_w_up = event.wheel.y > 0;
			mouse_w_down = event.wheel.y < 0;
		}
		//WPARAM
#if defined (_WIN32)
		if (event.type == SDL_SYSWMEVENT) {
			if (event.syswm.msg->msg.win.msg == WM_COMMAND) {
				switch (LOWORD(event.syswm.msg->msg.win.wParam)) {
				case 1:
					load_configuration();
					break;
				case 2:
					return true;
					break;
				case 4:
					string str = "JFKMW has been a ongoing 6 year effort to create a fun experience for everyone, with lots of modability and options.\n\nThanks to all contributors, programmers, and level designers.\n\n\nJFKMW, Made by the JFKMW Team, originally programmed by a good friend covered in white feathers.\n\nVersion " + GAME_VERSION;
					std::wstring stemp = std::wstring(str.begin(), str.end());
					LPCWSTR sw = stemp.c_str();
					MessageBox(NULL, sw, L"About", MB_OK);
					break;
				}
			}
		}
#endif
	}
	return quit;
}

void read_from_palette(string file) {
	ifstream input(file, ios::binary);
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
	uint_fast16_t curr = 0x7A00;
	for (auto &v : buffer) {
		if (curr < 0x7C00) {
			RAM[(curr >> 1) + ((curr & 1) << 8)] = uint_fast8_t(v);
		}
		curr++;
	}
	input.close();
}

//4bpp tile drawing functions
void draw8x8_tile(int_fast16_t x, int_fast16_t y, uint_fast16_t tile, uint_fast8_t palette) {
	x += 7; palette = palette << 4;	tile = tile << 5;
	uint_fast8_t color1, i, index;
	memcpy(graphics_array, &VRAM[tile], 32);

	for (index = 0; index < 16; index+=2) {
		for (i = 0; i < 8; i++) {
			color1 =
				((graphics_array[index] >> i) & 1) +
				(((graphics_array[1 + index] >> i) & 1) << 1) +
				(((graphics_array[16 + index] >> i) & 1) << 2) +
				(((graphics_array[17 + index] >> i) & 1) << 3);

			if (color1 != 0) {
				*((Uint32*)screen_s_l1->pixels + (x + y * sur_res_x)) = palette_array[color1 + palette];
			}
			x--;
		}
		x += 8; y++;
	}
}

void draw8x8_tile_f(int_fast16_t x, int_fast16_t y, uint_fast16_t tile, uint_fast8_t palette, bool flipx, bool flipy) {
	x += (!flipx) * 7; y += flipy * 7; palette = palette << 4;	tile = tile << 5;
	uint_fast8_t color1, i, index;
	memcpy(graphics_array, &VRAM[tile], 32);
	for (index = 0; index < 16; index+=2) {
		for (i = 0; i < 8; i++) {
			color1 =
				((graphics_array[0 + index] >> i) & 1) +
				(((graphics_array[1 + index] >> i) & 1) << 1) +
				(((graphics_array[16 + index] >> i) & 1) << 2) +
				(((graphics_array[17 + index] >> i) & 1) << 3);

			if (color1 != 0) {
				*((Uint32*)screen_s_l1->pixels + (x + y * sur_res_x)) = palette_array[color1 + palette];
			}
			if (flipx) { x++; }
			else { x--; }
		}
		if (flipx) { x -= 8; } else { x += 8; }
		if (flipy) { y--; } else { y++; }
	}
}

//2bpp
void draw8x8_tile_2bpp(int_fast16_t x, int_fast16_t y, uint_fast8_t tile, uint_fast16_t palette_offs) {
	if (tile > 0x7f) {
		return;
	}
	DestR.x = x + (int_res_x - 256) / 2;
	DestR.y = y + (show_full_screen ? (int_res_y - 224) / 2 : 0);
	DestR.w = 8;
	DestR.h = 8;

	SrcR.x = (tile & 0xF) << 3;
	SrcR.y = ((tile >> 4) << 3);
	SrcR.w = 8; SrcR.h = 8;
	RenderCopyOpenGLEx(&SrcR, &DestR, cached_l3_tilesGL[palette_offs & 0x7], 128, 64);
}

//OAM Drawer
void draw_tile_custom(int_fast16_t x, int_fast16_t y, uint_fast8_t size, uint_fast8_t rot, uint_fast16_t tile, uint_fast16_t props, uint_fast8_t sx = 0x20, uint_fast8_t sy = 0x20) {
	uint_fast8_t size_x = (size & 0xF) + 1;
	uint_fast8_t size_y = (size >> 4) + 1;
	DestR.x = x; DestR.y = y;
	DestR.w = size_x << 3; DestR.h = size_y << 3;
	if (DestR.x > -DestR.w && DestR.x < int(int_res_x) && DestR.y > -DestR.h && DestR.y < int(int_res_y)) {
		SrcR.x = (tile << 3) & 0x7F;
		SrcR.y = (tile >> 4) << 3;
		SrcR.w = DestR.w;
		SrcR.h = DestR.h;
		if (props & 0x10) { DestR.x += DestR.w; DestR.w *= -1; }
		if (props & 0x20) { DestR.y += DestR.h; DestR.h *= -1; }
		if (props & 0x2000) {
			DestR.w *= 0x20; DestR.w /= max(1, sx);
			DestR.h *= 0x20; DestR.h /= max(1, sy);
		}
		if (props & 0x1000) { glBlendFunc(GL_ONE, GL_ONE); }
		RenderCopyOpenGLEx(&SrcR, &DestR, cached_spr_tilesGL[props & 0xF], 128, 512, (float(rot) / 256.f) * 360.f);
		if (props & 0x1000) { glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA); }
	}
}