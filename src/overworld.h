#pragma once
/*
	JFK Mario World Overworld Implementation

	Should improve later but for now it's fine.
*/

class overworldSystem
{
public:
	double WarpDestinationX, WarpDestinationY;

	//Some variables
	double CamSX, CamSY;
	bool FreeCam = false;
	bool ResetCam = false;

	double overworldSavedX, overworldSavedY;

	bool initializedPosition = false;

	//Yes, the overworld depends on the player. This is to be able to add MP support.
	MPlayer* OverworldPlayer;

	uint_fast8_t current_map = 0;
	uint_fast8_t old_map = 0xFF;

	string level_strings[0x100];

	uint_fast16_t ind_offset[8] = {
			 0,  0x400,
		0x0800, 0x0C00,
		0x1000, 0x1400,
		0x1800, 0x1C00
	};

	//Config
	unordered_map<string, int> ow_config;
	int get_config_entry(string name) {
		auto entry = ow_config.find(name);
		if (entry != ow_config.end()) {
			return entry->second;
		}
		return 0;
	}

	void add_entry(string name, int value) {
		ow_config.insert(make_pair(name, value));
	}

	//Warp
	void Warp() {
		for (uint_fast8_t i = 0; i < Mario.size(); i++) {
			OverworldPlayer = &Mario[i];
			OverworldPlayer->x = WarpDestinationX;
			OverworldPlayer->y = WarpDestinationY;
			OverworldPlayer->server_position_sync_x = WarpDestinationX;
			OverworldPlayer->server_position_sync_y = WarpDestinationY;
			OverworldPlayer->server_position_sync_s++;
		}
		overworldSavedX = WarpDestinationX;
		overworldSavedY = WarpDestinationY;
		ingame_frame_counter = 0;
		global_frame_counter = 0;
	}

	
	//Check if we are standing on a tile
	bool StandingOnTile() {
		int TilePosX = int(OverworldPlayer->x) >> 4;
		int TilePosY = int(OverworldPlayer->y + 32) >> 4;

		uint_fast8_t tile = Get_Tile(TilePosX, TilePosY);

		return (tile >= 0x58 && tile <= 0x6D) && !(int(OverworldPlayer->x) & 0xF) && !(int(OverworldPlayer->y) & 0xF);
	}

	//Get the tile at x,y
	uint_fast8_t Get_Tile(uint_fast8_t x, uint_fast8_t y) {
		return RAM[0xA000 + x + (y << 5)];
	}

	//Get the level number at x,y
	uint_fast8_t Get_Level(uint_fast8_t x, uint_fast8_t y) {
		return RAM[0xA800 + x + (y << 5)];
	}

	//Calculate Submap
	void CalculateSubmap(bool c)
	{
		if (OverworldPlayer->y < 512) {
			current_map = 0;
		}
		else {
			current_map = 1 + ((OverworldPlayer->y > 688) * 2) + ((OverworldPlayer->y > 856) * 2) + (OverworldPlayer->x > 256);
			OverworldPlayer->CAMERA_X = (OverworldPlayer->x > 256) * 256;
			OverworldPlayer->CAMERA_Y = 512;
		}

		//Load OW Palette
		if (current_map != old_map && c) {
			old_map = current_map;
			read_from_palette(Modpack + "/Map" + to_string(int(current_map)) + ".mw3");
		}
	}

	//Initialize overworld
	void Initialize()
	{
		game_init();

		//Reset camera
		FreeCam = false;
		ResetCam = false;

		//Free up memory that was probably used in the level
		LevelSprites.clear();
		particles.clear();

		//Configuration
		ow_config.clear();
		ifstream ConfigFile(Modpack + "/OverworldConfig.txt");
		if (ConfigFile.is_open())
		{
			string line;
			while (getline(ConfigFile, line)) {
				if (line != "" && line.substr(0, 2) != "//") {
					line.erase(remove_if(line.begin(), line.end(), ::isspace),
						line.end());
					auto delimiterPos = line.find("=");
					string name = line.substr(0, delimiterPos);
					string value = line.substr(delimiterPos + 1);
					add_entry(name, stoi(value, nullptr, 16));
				}
			}
		}
		ConfigFile.close();


		if (!ow_has_been_loaded) {
			ow_has_been_loaded = true;
			overworldSavedX = get_config_entry("start_x");
			overworldSavedY = get_config_entry("start_y");
		}

		//Tell to reinitialize positions
		initializedPosition = false;
		if (!isClient) {
			for (uint_fast8_t i = 0; i < Mario.size(); i++) {
				Mario[i].ow_pos_inited = false;
			}
		}
		
		//Forced level config entry
		if (get_config_entry("forced_level") != 0) {
			//Set to overworld
			writeToRam(0x3F08, get_config_entry("forced_level"), 2);
			load_level3f08();
			return;
		}

		//Set to overworld
		gamemode = GAMEMODE_OVERWORLD;
		old_map = 0xFF;

		//Load exanimation file, and clear out hud memory.
		loadAssetRAM("Graphics/hud.bin", 11);
		memset(&RAM[VRAM_Convert(0xB800)], 0xFF, 0x800);

		//Clear overworld L1 and L2
		memset(&RAM[0xA000], 0x00, 0x1000);
		memset(&RAM[0x6000], 0x00, 0x4000);

		//Reset counters
		ingame_frame_counter = 0;
		global_frame_counter = 0;

		//Clear OAM
		Clear_OAM();

		//Load OW Data (With swag)
		ifstream infile(Modpack + "/MainOW_L1.bin", ios::binary);
		infile.read((char*)&RAM[0xA000], 0x800);
		infile.close();

		ifstream infile2(Modpack + "/MainOW_L2.bin", ios::binary);
		infile2.read((char*)&RAM[0x6000], 0x4000);
		infile2.close();

		ifstream infile3(Modpack + "/MainOW_Levels.bin", ios::binary);
		infile3.read((char*)&RAM[0xA800], 0x800);
		infile3.close();

		//Initialize level names
		ifstream LevelNames(Modpack + "/LevelNames.txt");
		if (LevelNames.is_open())
		{
			string line;
			int i = 0;
			while (getline(LevelNames, line)) {
				
				auto delimiterPos = line.find("=");
				auto name = line.substr(0, delimiterPos);
				auto value = line.substr(delimiterPos + 1);
				level_strings[stoi(name, nullptr, 16)] = value;
			}
		}
		LevelNames.close();

		for (int i = 0; i < 9; i++) {
			loadAssetRAM(Modpack + "/OW_GFX" + int_to_hex(i, true) + ".bin", i);
		}

		need_preload_sprites = true;

		//Loaded it
		cout << yellow << "[JFKMW] Loaded overworld from modpack " << Modpack << endl;
	}

	//Draw ow tile (Shitty)
	void drawL1Tile(uint_fast8_t tile, int x, int y)
	{
		if (tile >= 0x66 && tile <= 0x6D)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int e = 0; e < 2; e++)
				{
					draw8x8_tile_f(
						x + i * 8,
						y + e * 8,
						0x186, ((tile >> 1) & 1) ? 6 : 7,
						i == 1,
						e == 0
					);
				}
			}
		}

		if (tile == 0x58)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int e = 0; e < 2; e++)
				{
					draw8x8_tile(
						x + i * 8,
						y + e * 8,
						0x188 + i + (e << 4),
						5
					);
				}
			}
		}


		if (tile == 0x5B)
		{
			for (int i = 0; i < 2; i++)
			{
				for (int e = 0; e < 2; e++)
				{
					draw8x8_tile(
						x + i * 8,
						y + e * 8,
						0x18C + i + (e << 4),
						5
					);
				}
			}
		}
	}

	//Render override
	void Render() {
		if (OverworldPlayer == NULL) {
			//Darken the screen globally
			Ren_SetDrawColor(0, 0, 0, 255);
			Ren_FillRect(nullptr);
			return;
		}

		//Player pos
		int TilePosX = int(OverworldPlayer->x) >> 4;
		int TilePosY = int(OverworldPlayer->y + 32) >> 4;
			 
		//Initialize some variables
		DestR = { 0, 0, 256, 224 };
		SrcR = { 0, 0, 256, 224 };
		uint_fast8_t mosaic_val = RAM[0x3F10] >> 4;
		uint_fast8_t bright_val = RAM[0x3F10] & 0xF;

		//Convert 16-bit palette to 32-bit palette
		ConvertPalette();

		//Lock surface for drawing
		SDL_LockSurface(screen_s_l1);
		SDL_memset(screen_s_l1->pixels, 0, screen_s_l1->h * screen_s_l1->pitch);

		//Layer 2
		if (drawBg) {
			uint_fast16_t Cam_PX_Off = ((int(OverworldPlayer->CAMERA_X) >> 4) << 1);
			uint_fast16_t Cam_PY_Off = ((int(OverworldPlayer->CAMERA_Y) >> 4) << 1);
			//Draw scenery
			for (uint_fast16_t x = 0; x < 30; x++) {
				for (uint_fast16_t y = 0; y < 23; y++) {
					uint_fast16_t px = x + Cam_PX_Off;
					uint_fast16_t py = y + Cam_PY_Off;
					uint_fast16_t ind = (px & 0x1F) + ((py & 0x1F) << 5) + ind_offset[((px >> 5) + ((py >> 5) << 1)) & 7];
					uint_fast16_t ent = ind << 1;
					uint_fast8_t tile = RAM[0x6000 + ent];
					uint_fast8_t props = RAM[0x6001 + ent];
					draw8x8_tile_f(
						16 + (x << 3),
						40 + (y << 3),
						tile, (props >> 2) & 7,
						props & 0x40, props & 0x80
					);

				}
			}
		}

		if (drawL1) {
			//Draw Layer 1
			for (uint_fast16_t x = 0; x < 15; x++) {
				for (uint_fast16_t y = 0; y < 12; y++) {
					uint_fast16_t px = x + (int(OverworldPlayer->CAMERA_X) >> 4);
					uint_fast16_t py = 2 + y + (int(OverworldPlayer->CAMERA_Y) >> 4);
					uint_fast16_t ind = px + (py << 5);

					uint_fast8_t tile = RAM[0xA000 + ind];
					if (tile != 0) {
						drawL1Tile(tile, 16 + (x << 4), 40 + (y << 4));
					}
				}
			}
		}

		//Draw screen s l1
		SDL_UnlockSurface(screen_s_l1);
		//We can now draw the screen finished product.

		//Mosaic Algo (Speed++++++)
		MosaicScreenSL1(mosaic_val);

		convertL1Tex();

		DestR.x = ((int_res_x - 256) / 2) - (int(OverworldPlayer->CAMERA_X) & 0xF);
		DestR.y = ((int_res_y - 224) / 2) - (int(OverworldPlayer->CAMERA_Y) & 0xF);
		DestR.w = int_res_x + 16;
		DestR.h = int_res_y + 16;
		SrcR.x = 0;
		SrcR.y = 0;
		SrcR.w = int_res_x + 16;
		SrcR.h = int_res_y + 16;

		RenderCopyOpenGLEx(&SrcR, &DestR, screen_t_l1GL, int_res_x + 16, int_res_y + 16);

		//Render all of em
		for (int i = 0; i < Mario.size(); i++) {
			uint_fast8_t SPR = Mario[i].X_SPEED != 0 ? (SPR = Mario[i].X_SPEED > 0 ? 1 : 3) : 0;

			//Player pos checks
			uint_fast8_t plr_tile = Get_Tile(int(Mario[i].x) >> 4, int(Mario[i].y + 32) >> 4);
			Mario[i].IN_WT = ((plr_tile >= 0x28 && plr_tile <= 0x3E) || (plr_tile >= 0x6A && plr_tile <= 0x6D)) || plr_tile == 0x50;
			Mario[i].climbing = plr_tile == 0x3F;

			if (Mario[i].Y_SPEED < 0) { SPR = 2; }
			if (RAM[0x3F11] == 4) { SPR = 4; }
			if (Mario[i].climbing) { SPR = 5; }

			//Walking mario on the OW border
			CreateSpriteCrop("Sprites/mario/Skin" + to_string(Mario[i].skin) + ".png",
				((int_res_x - 256) / 2) + 16 + int(Mario[i].x - OverworldPlayer->CAMERA_X),
				((int_res_y - 224) / 2) + (Mario[i].IN_WT ? 33 : 27) + int(Mario[i].y - OverworldPlayer->CAMERA_Y),
				16, (Mario[i].IN_WT ? 18 : 24),
				(((ingame_frame_counter >> 3) & 3) << 4) + (SPR << 6), 192, 512, 256, 0);

			//Water thing
			if (Mario[i].IN_WT) {
				CreateSprite("Sprites/ui/OW_Water_" + to_string((ingame_frame_counter >> 3) & 1) + ".png", DestR.x, DestR.y + 15, 16, 4);
			}
		}

		//Draw border and HUD
		if (hudMode < 2) {
			//The border
			CreateSprite("Sprites/ui/OWBorder.png",
				((int_res_x - 256) / 2),
				((int_res_y - 224) / 2),
				256,
				224);
			if (FreeCam && (ingame_frame_counter & 0x1F) < 0x18 && !ResetCam) {
				CreateSprite("Sprites/ui/OWBorderArrows.png",
					((int_res_x - 256) / 2),
					((int_res_y - 224) / 2),
					256,
					224);
			}
			//Walking mario on the OW border
			CreateSpriteCrop("Sprites/mario/Skin" + to_string(my_skin) + ".png",
				((int_res_x - 256) / 2) + 24,
				((int_res_y - 224) / 2) + 6,
				32, 32,
				(1 + ((ingame_frame_counter >> 3) % 3)) << 5, 64, 512, 256, 0);

			//Text
			if (OverworldPlayer->ow_level_name != "") {
				string text = OverworldPlayer->ow_level_name;
				int y = 31 - (int(text.length())/20) * 8;
				for (int i = 0; i < text.length(); i++) {
					draw8x8_tile_2bpp((11 + (i % 19)) << 3, y + (i / 19) * 8, char_to_smw(text.at(i)), 7);
				}
			}

			//Lives
			draw8x8_tile_2bpp(64, 30, 0, 6);
		}

		//Draw diagnostics
		if (hudMode == 1) {
			int fps = int(1.0 / (total_time_ticks.count() / 1.0));
			int texty = 34;
			int textx = 0;
			string t1 = to_string(int(OverworldPlayer->x)) + ", " + to_string(int(OverworldPlayer->y)) + " submap " + to_string(int(current_map)) + "\n" + to_string(fps) + " fps";
			for (int i = 0; i < t1.length(); i++) {
				if (t1.at(i) == '\n') {
					texty -= 8; textx = 0;
				}
				draw8x8_tile_2bpp(1 + ((2 + textx) << 3), 224 - texty, char_to_smw(t1.at(i)), 6);
				textx++;
			}
		}

		//Prepare chat
		Chat_Prerender();

		//Draw L3
		for (uint_fast16_t t3_x = 0; t3_x < 32; t3_x++) {
			for (uint_fast16_t t3_y = 0; t3_y < 28; t3_y++) {
				if (VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)] < MAX_L3_TILES) {
					draw8x8_tile_2bpp(t3_x << 3, t3_y << 3, VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)], VRAM[0xB801 + (t3_x << 1) + (t3_y << 6)]);
				}
			}
		}

		//Darken the screen globally
		Ren_SetDrawColor(0, 0, 0, bright_val == 0xF ? 255 : (bright_val << 4));
		Ren_FillRect(nullptr);

		//Chat
		Chat_Render();
	}
	
	//Process OW
	void Process()
	{
		//For player
		RAM[0x1493] = 0;

		//Clientside (Movement) Behaviour
		if (!networking || isClient) {
			//For player
			OverworldPlayer = &get_mario(SelfPlayerNumber);
			OverworldPlayer->getInput();
			CalculateSubmap(false);

			//Position
			if (!initializedPosition && !networking) {
				initializedPosition = true;
				OverworldPlayer->x = overworldSavedX;
				OverworldPlayer->y = overworldSavedY;
				old_map = 0xFF;
			}

			if (!RAM[0x3F11]) {
				//Cam toggle
				if (pad_p[button_start] && current_map == 0) {
					if (FreeCam) {
						if (!ResetCam) {
							ResetCam = true;
							double gotoX = OverworldPlayer->x - (120.0 - 16.0);
							double gotoY = OverworldPlayer->y - (104.0 - 24.0);
							gotoX = min(287.0, max(0.0, gotoX));
							gotoY = min(343.0, max(0.0, gotoY));

							double ang = atan2(gotoY - OverworldPlayer->CAMERA_Y, gotoX - OverworldPlayer->CAMERA_X);
							CamSX = cos(ang) * 5.0;
							CamSY = sin(ang) * 5.0;
						}
					}
					else {
						FreeCam = true;
						ResetCam = false;
					}
				}

				if (current_map != 0) {
					FreeCam = false;
				}

				//Modes
				if (!FreeCam) {
					//Movement

					int TilePosX = int(OverworldPlayer->x) >> 4;
					int TilePosY = int(OverworldPlayer->y + 32) >> 4;

					if (StandingOnTile()) {
						//Snap
						OverworldPlayer->x = int(OverworldPlayer->x);
						OverworldPlayer->y = int(OverworldPlayer->y);
						OverworldPlayer->X_SPEED = pad_p[button_right] - pad_p[button_left];
						OverworldPlayer->Y_SPEED = pad_p[button_down] - pad_p[button_up];

						if (OverworldPlayer->X_SPEED != 0 && Get_Tile(uint_fast8_t(TilePosX + OverworldPlayer->X_SPEED), TilePosY) == 0x0) {
							OverworldPlayer->X_SPEED = 0;
						}
						if (OverworldPlayer->Y_SPEED != 0 && Get_Tile(uint_fast8_t(TilePosX), uint_fast8_t(TilePosY + OverworldPlayer->Y_SPEED)) == 0x0) {
							OverworldPlayer->Y_SPEED = 0;
						}
					}

					//Special Movement
					TilePosX = int(OverworldPlayer->x + 8) >> 4;
					TilePosY = int(OverworldPlayer->y + 40) >> 4;
					uint_fast8_t Tile = Get_Tile(uint_fast8_t(TilePosX), TilePosY);

					//TO-DO clean this mess up
					//Diagonal 22.5* Left Up
					if (Tile == 0x16 || Tile == 0x15) {
						OverworldPlayer->y += OverworldPlayer->X_SPEED / 4.0;
					}
					if ((Tile == 0x1F || Tile == 0x17) || Tile == 0x14) {
						OverworldPlayer->y += OverworldPlayer->X_SPEED / 2.0;
					}

					//Diagonal 22.5* Right Up
					if ((Tile == 0x3 || Tile == 0x2B) || Tile == 0x5) {
						OverworldPlayer->y -= OverworldPlayer->X_SPEED / 4.0;
					}
					if ((Tile == 0x7 || Tile == 0x11) || Tile == 0x1) {
						OverworldPlayer->y -= OverworldPlayer->X_SPEED / 2.0;
					}

					//45* 1.
					if (Tile == 0x19 || Tile == 0x37) {
						if (OverworldPlayer->X_SPEED < 0) {
							OverworldPlayer->X_SPEED = 0; OverworldPlayer->Y_SPEED = -1;
						}
						if (OverworldPlayer->Y_SPEED > 0) {
							OverworldPlayer->X_SPEED = 1; OverworldPlayer->Y_SPEED = 0;
						}
						OverworldPlayer->x += OverworldPlayer->Y_SPEED / 2.0;
						OverworldPlayer->y += OverworldPlayer->X_SPEED;
					}

					//45* 2.
					if (Tile == 0xA || Tile == 0x2E) {
						if (OverworldPlayer->X_SPEED < 0) {
							OverworldPlayer->X_SPEED = 0; OverworldPlayer->Y_SPEED = 1;
						}
						if (OverworldPlayer->Y_SPEED < 0) {
							OverworldPlayer->X_SPEED = 1; OverworldPlayer->Y_SPEED = 0;
						}
						OverworldPlayer->x -= OverworldPlayer->Y_SPEED / 1.5;
						OverworldPlayer->y -= OverworldPlayer->X_SPEED / 1.5;
					}

					//45* 3.
					if (Tile == 0x9 || Tile == 0x2D) {
						if (OverworldPlayer->X_SPEED > 0) {
							OverworldPlayer->X_SPEED = 0; OverworldPlayer->Y_SPEED = -1;
						}
						if (OverworldPlayer->Y_SPEED > 0) {
							OverworldPlayer->X_SPEED = -1; OverworldPlayer->Y_SPEED = 0;
						}
						OverworldPlayer->x -= OverworldPlayer->Y_SPEED / 1.25;
						OverworldPlayer->y -= OverworldPlayer->X_SPEED / 1.25;
					}

					//22.5* Bottom-Left
					if (Tile == 0x12 || Tile == 0x50 || Tile == 0xF) {
						OverworldPlayer->x -= OverworldPlayer->Y_SPEED / 4.0;
					}

					//22.5* Bottom-Right
					if (Tile == 0x1E || Tile == 0x20) {
						OverworldPlayer->x += OverworldPlayer->Y_SPEED / 4.0;
					}
					if (Tile >= 0x1B && Tile <= 0x1D) {
						OverworldPlayer->x += OverworldPlayer->Y_SPEED / 2.0;
					}
					if (!OverworldPlayer->climbing || (ingame_frame_counter & 1)) {
						OverworldPlayer->x += OverworldPlayer->X_SPEED;
						OverworldPlayer->y += OverworldPlayer->Y_SPEED;
					}

					//Camera IG
					if (current_map == 0) {
						OverworldPlayer->CAMERA_X = OverworldPlayer->x - (120.0 - 16.0);
						OverworldPlayer->CAMERA_Y = OverworldPlayer->y - (104.0 - 24.0);
					}
				}
				else
				{
					//Free-cam mode
					if (!ResetCam) {
						OverworldPlayer->CAMERA_X += (s_pad[button_right] - s_pad[button_left]) * 2;
						OverworldPlayer->CAMERA_Y += (s_pad[button_down] - s_pad[button_up]) * 2;
					}
					else {
						OverworldPlayer->CAMERA_X += CamSX;
						OverworldPlayer->CAMERA_Y += CamSY;
						double gotoX = OverworldPlayer->x - (120.0 - 16.0);
						double gotoY = OverworldPlayer->y - (104.0 - 24.0);
						gotoX = min(287.0, max(0.0, gotoX));
						gotoY = min(343.0, max(0.0, gotoY));

						double dist = sqrt(pow(gotoX - OverworldPlayer->CAMERA_X, 2) + pow(gotoY - OverworldPlayer->CAMERA_Y, 2));
						if (dist < 8)
						{
							ResetCam = false;
							FreeCam = false;
							OverworldPlayer->CAMERA_X = gotoX;
							OverworldPlayer->CAMERA_Y = gotoY;
						}
					}
				}
				if (current_map == 0) {
					OverworldPlayer->CAMERA_X = min(287.0, max(0.0, OverworldPlayer->CAMERA_X));
					OverworldPlayer->CAMERA_Y = min(343.0, max(0.0, OverworldPlayer->CAMERA_Y));
				}
			}
		}

		//Serverside behaviour
		if (!isClient) {
			for (uint_fast8_t i = 0; i < Mario.size(); i++) {
				//Player
				OverworldPlayer = &Mario[i];
				if (!OverworldPlayer->ow_pos_inited && networking) {
					OverworldPlayer->x = overworldSavedX;
					OverworldPlayer->y = overworldSavedY;
					OverworldPlayer->server_position_sync_x = overworldSavedX;
					OverworldPlayer->server_position_sync_y = overworldSavedY;
					OverworldPlayer->server_position_sync_s++;
					OverworldPlayer->ow_pos_inited = true;
				}
				//Submap/Music calculation
				if (i == 0) {
					CalculateSubmap(true);
				}
				//Enter level
				int TilePosX = int(OverworldPlayer->x) >> 4;
				int TilePosY = int(OverworldPlayer->y + 32) >> 4;
				if (StandingOnTile()) {
					if (Get_Level(TilePosX, TilePosY) != OverworldPlayer->ow_old_level) {
						OverworldPlayer->ow_old_level = Get_Level(TilePosX, TilePosY);
						if (Get_Tile(TilePosX, TilePosY) != 0x5B) {
							RAM[0x1DFC] = 0x23;
						}
					}
					OverworldPlayer->ow_level_name = level_strings[OverworldPlayer->ow_old_level];

					if (((OverworldPlayer->p_pad[button_b] || OverworldPlayer->p_pad[button_a]) || OverworldPlayer->p_pad[button_y]) && !RAM[0x3F10])
					{
						//Position is not synced, don't bother.
						if (networking) {
							if (OverworldPlayer->server_position_sync_s != OverworldPlayer->server_position_sync_c) {
								continue;
							}
						}

						//When you enter a level, the overworld position is saved for later.
						overworldSavedX = OverworldPlayer->x;
						overworldSavedY = OverworldPlayer->y;

						//Tile check
						if (Get_Tile(TilePosX, TilePosY) == 0x5B) {
							WarpDestinationX = OverworldPlayer->x;
							WarpDestinationY = OverworldPlayer->y;
							for (int p_x = 0; p_x < 32; p_x++) {
								for (int p_y = 0; p_y < 64; p_y++) {
									if (p_x != TilePosX && p_y != TilePosY && Get_Tile(p_x, p_y) == 0x5B) {
										if (Get_Level(p_x, p_y) == Get_Level(TilePosX, TilePosY)) {
											WarpDestinationX = p_x * 16;
											WarpDestinationY = -32 + p_y * 16;
										}
									}
								}
							}
							RAM[0x3F11] = 6; RAM[0x3F10] = 0;
						}
						else {
							writeToRam(0x3F08, Get_Level(TilePosX, TilePosY), 2);
							RAM[0x3F11] = 4; RAM[0x3F10] = 0;
						}
					}
				}
				else {
					OverworldPlayer->ow_level_name = "";
				}
			}
		}

		//Music
		RAM[0x1DFB] = 0xF0 + current_map;

		if (global_frame_counter == 5) {
			//Fade in
			RAM[0x3F10] = 0x0F; RAM[0x3F11] = 2;
		}

		//Increease frmcounter
		if (!RAM[0x3F11]) {
			ingame_frame_counter++;
		}
	}
};

overworldSystem overworld;