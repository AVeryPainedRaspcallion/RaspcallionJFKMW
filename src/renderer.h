#pragma once
//The Shitrenderer
//Made by da man himself

//Sorted OAM groups
vector<int> OAMGroupSorted[6];

//Player Drawing Macros
#define DRAW_GRABBED_SPRITE uint_fast8_t tile = uint_fast8_t(getRamValue(0x2F00 + CurrentPlayer.GRABBED_SPRITE, 1)); \
if (tile != 0) {\
	uint_fast8_t size = uint_fast8_t(((RAM[0x2E80 + CurrentPlayer.GRABBED_SPRITE] & 0x7F) >> 4) + (((RAM[0x2E80 + CurrentPlayer.GRABBED_SPRITE] & 0x7F) >> 4) << 4));\
	int_fast16_t x_position = int_fast16_t(double(CurrentPlayer.x + ((CurrentPlayer.climbing || CurrentPlayer.SKIDDING != 0) ? 0 : (CurrentPlayer.to_scale * -12.0))));\
	int_fast16_t y_position = int_fast16_t(double(CurrentPlayer.y - 1 - (CurrentPlayer.STATE > 0 ? 13.0 : 16.0) - (CurrentPlayer.CROUCH ? 2 : 0) * (CurrentPlayer.STATE > 0)));\
	uint_fast8_t pal = RAM[0x2E80 + CurrentPlayer.GRABBED_SPRITE] & 0xF;\
	draw_tile_custom(x_position - CameraX, INTERNAL_RESOLUTION_Y - 32 - y_position + CameraY, size, 0, tile, pal);\
}


//HUD Number Drawing functions
void draw_number_hex(uint_fast8_t pos_x, uint_fast8_t pos_y, uint_fast16_t number, int length) {
	for (int i = 0; i < length; i++) {
		VRAM[0xB800 + (-i * 2) + (pos_x * 2) + pos_y * 64] = (number >> (i << 2)) & 0xF;
		VRAM[0xB801 + (-i * 2) + (pos_x * 2) + pos_y * 64] = 6;
	}
}

void renderPlayers(bool D) {
	//Draw Player
	if (!(RAM[0x3F1F] & 2)) {
		for (int i = 0; i < Players.size(); i++) {
			MPlayer& CurrentPlayer = Players[i];
			if (CurrentPlayer.in_pipe == D) {

				//Some powerup rendering things..
				if (!CurrentPlayer.invisible) {
					//Cape
					if (CurrentPlayer.STATE == 2) {
						CreateSpriteCrop("Sprites/player/Power.png",
							(CurrentPlayer.to_scale * 8) + int(CurrentPlayer.x) - int(CameraX),
							INTERNAL_RESOLUTION_Y - 32 + ((CurrentPlayer.CROUCH && CurrentPlayer.CAPE_ST == -1) * 5) - int(CurrentPlayer.y - 1) + int(CameraY),
							CurrentPlayer.to_scale * -16, 32, (CurrentPlayer.CAPE_FRAME & 3) << 4, (CurrentPlayer.CAPE_ST + 1) << 5, 64, 128);
					}
				}

				//Grabbed shit (below, when not skidding)
				if (CurrentPlayer.GRABBED_SPRITE != 0xFF && CurrentPlayer.SKIDDING == 0) {
					DRAW_GRABBED_SPRITE
				}

				//Render player itself.
				if (!CurrentPlayer.invisible) {
					CreateSpriteCrop("Sprites/player/Skin" + to_string(CurrentPlayer.skin) + ".png",
						-8 + int(CurrentPlayer.x) - int(CameraX),
						INTERNAL_RESOLUTION_Y - 32 - int(CurrentPlayer.y - 1) + int(CameraY),
						int(CurrentPlayer.to_scale * (CurrentPlayer.SKIDDING != 0 ? -1 : 1)) * 32, 32,
						(CurrentPlayer.pose & 0xF) << 5, ((CurrentPlayer.pose >> 4) << 5) + CurrentPlayer.state_str() * 64, 512, 256, CurrentPlayer.INVINCIBILITY_FRAMES_STAR);
				}

				//Grabbed shit (on-top, when skidding)
				if (CurrentPlayer.GRABBED_SPRITE != 0xFF && CurrentPlayer.SKIDDING != 0) {
					DRAW_GRABBED_SPRITE
				}

				//Embedded
				int_fast16_t x_position = int_fast16_t(CurrentPlayer.x);
				int_fast16_t y_position = int_fast16_t(CurrentPlayer.y + 24);
				for (int e = 0; e < OAMGroupSorted[5].size(); e++) {
					OAMTile& T = OAM_Tiles[OAMGroupSorted[5][e]];
					if (T.rotation == i) {
						draw_tile_custom(T.pos_x + x_position - CameraX, -y_position - T.pos_y + CameraY + INTERNAL_RESOLUTION_Y, T.bsize, 0, T.tile + ((T.props & 192) << 2), T.props, T.scale_x, T.scale_y);
					}
				}

				if (CurrentPlayer.powerup_anim && (CurrentPlayer.powerup_anim >> 6) == 1) {
					draw_tile_custom(x_position - CameraX, INTERNAL_RESOLUTION_Y - y_position + CameraY, 0x11, 0, PlayerCapeSmokeFrames[(31 - (CurrentPlayer.powerup_anim & 0x3F)) / 7], 8);
				}
			}
		}
	}

}

void draw_number_dec(uint_fast8_t pos_x, uint_fast8_t pos_y, int number, uint_fast8_t off = 0, uint_fast8_t props = 6) {
	int length = int(to_string(number).length());
	for (int i = 0; i < length; i++) {
		VRAM[0xB800 + (-i * 2) + (pos_x * 2) + pos_y * 64] = off + (int(number / pow(10, i)) % 10);
		VRAM[0xB801 + (-i * 2) + (pos_x * 2) + pos_y * 64] = props;
	}
}

//OAM Renderer
void renderOamGroup(int priority) {
	if (drawSprites) {
		for (int i = 0; i < OAMGroupSorted[priority].size(); i++) {
			OAMTile& T = OAM_Tiles[OAMGroupSorted[priority][i]];
			draw_tile_custom(T.pos_x - CameraX, int_fast16_t(INTERNAL_RESOLUTION_Y) - T.pos_y - 32 + CameraY, T.bsize, T.rotation, T.tile + ((T.props & 192) << 2), T.props, T.scale_x, T.scale_y);
		}
	}
}

//Background renderers (optimized)
void drawBackground() {
	//Start drawing
	glBindTexture(GL_TEXTURE_2D, bg_texture_GL[0]);
	glEnable(GL_TEXTURE_2D);
	glColor4ub(255, 255, 255, 255);
	glBegin(GL_QUADS);
	//Render all scanlines
	int formula_x = int(double(CameraX) * (double(RAM[0x3F06]) / 16.0) + getRamValue(0x1466, 2));
	int formula_y = int(double(CameraY) * (double(RAM[0x3F07]) / 16.0) + getRamValue(0x1468, 2)) - 496;
	float wx = float((INTERNAL_RESOLUTION_X * RAM[0x38]) >> 5) / 512.f;
	float wy = 1.f / 512.f;
	for (int i = 0; i < INTERNAL_RESOLUTION_Y; i++) {
		int scanline = ((i * RAM[0x39]) >> 5) - INTERNAL_RESOLUTION_Y - formula_y;
		float xs = float(formula_x - hdmaLineData[scanline & 0x1FF][HDMA_L2_MODEX]) / 512.f;
		float ys = float(hdmaLineData[scanline & 0x1FF][HDMA_L2_MODEY] + scanline) / 512.f;
		glTexCoord2f(xs, ys); glVertex2i(0, i);
		glTexCoord2f(xs, ys + wy); glVertex2i(0, i+1);
		glTexCoord2f(xs + wx, ys + wy); glVertex2i(INTERNAL_RESOLUTION_X, i+1);
		glTexCoord2f(xs + wx, ys); glVertex2i(INTERNAL_RESOLUTION_X, i);
	}
	//Turn off mode
	glEnd();
	glDisable(GL_TEXTURE_2D);
}

void drawLayer3Background() {
	//Blending
	if (RAM[0x3F1B] & 2) { glBlendFunc(GL_ONE, GL_ONE); }
	if (RAM[0x3F1B] & 4) { glBlendFunc(GL_DST_COLOR, GL_ZERO); }

	//Assign 2nd
	glBindTexture(GL_TEXTURE_2D, bg_texture_GL[1]);
	glEnable(GL_TEXTURE_2D);
	glColor4ub(255, 255, 255, 255);
	glBegin(GL_QUADS);

	float wx = float(INTERNAL_RESOLUTION_X) / 512.f; float wy = float(INTERNAL_RESOLUTION_Y) / 512.f;
	float xs = float(double(CameraX) * (double(RAM[0x3F1C]) / 16.0) + getRamValue(0x22, 2)) / 512.f;
	float ys = float(double(CameraY) * (double(RAM[0x3F1D]) / 16.0) + getRamValue(0x24, 2) - 272) / -512.f;

	glTexCoord2f(xs, ys); glVertex2i(0, 0);
	glTexCoord2f(xs, ys + wy); glVertex2i(0, INTERNAL_RESOLUTION_Y);
	glTexCoord2f(xs + wx, ys + wy); glVertex2i(INTERNAL_RESOLUTION_X, INTERNAL_RESOLUTION_Y);
	glTexCoord2f(xs + wx, ys); glVertex2i(INTERNAL_RESOLUTION_X, 0);

	//Turn off mode
	glEnd();
	glDisable(GL_TEXTURE_2D);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void handleRenderingForPlayer(int player) {
	CheckForPlayers();
	if (Players.size() < player) {
		return;
	}

	//Sort OAM
	for (int i = 0; i < 6; i++) { if (OAMGroupSorted[i].size() > 0) { OAMGroupSorted[i].clear(); } }
	for (int i = 0; i < OAM_Tiles.size(); i++) {
		OAMGroupSorted[OAM_Tiles[i].props & 0x800 ? 5 : min(4, (OAM_Tiles[i].props >> 8) & 7)].push_back(i);
	}

	//Transition and shit
	uint_fast8_t transition_type = RAM[0x1493] > 0 ? 3 : RAM[0x3F11];
	uint_fast8_t mosaic_val = RAM[0x3F10] >> 4;
	uint_fast8_t bright_val = RAM[0x3F10] & 0xF;

	MPlayer& LocalPlayer = GetPlayerByNumber(player);
	LocalPlayer.ProcessCamera();
	double CAM_X = LocalPlayer.CAMERA_X;
	double CAM_Y = LocalPlayer.CAMERA_Y;

	//Local MP
	if (local_multiplayer) {
		double totalDiv = 1;
		for (int i = 1; i < Players.size(); i++) {
			MPlayer& PlayerMP = GetPlayerByNumber(i);
			PlayerMP.ProcessCamera();
			CAM_X += PlayerMP.CAMERA_X;
			CAM_Y += PlayerMP.CAMERA_Y;

			totalDiv += 1;
		}
		CAM_X /= totalDiv; CAM_Y /= totalDiv;
	}

	//Camera
	CameraX = max(0, min(
		int_fast16_t(CAM_X - (int_fast16_t(INTERNAL_RESOLUTION_X >> 1) - 8)),
		-int_fast16_t(INTERNAL_RESOLUTION_X) + int_fast16_t(mapWidth) * 16
	));
	CameraY = max(0, min(
		-int_fast16_t(INTERNAL_RESOLUTION_Y) + int_fast16_t(mapHeight) * 16,
		int_fast16_t(CAM_Y - int_fast16_t(INTERNAL_RESOLUTION_Y >> 1))
	));

	if (RAM[0x1887] > 0) {
		CameraY += (((global_frame_counter >> 1) & 1) ? 2 : -2) * (global_frame_counter & 1);
	}

	int_fast16_t offsetX = int_fast16_t(CameraX >> 4);
	uint_fast8_t offsetXPixel = uint_fast8_t(CameraX) & 0xF;
	int_fast16_t offsetY = int_fast16_t(CameraY >> 4);
	uint_fast8_t offsetYPixel = uint_fast8_t(CameraY) & 0xF;

	//Load BG data
	loadBG();

	//Draw BG
	if (drawBg) {
		if (!(RAM[0x40] & 1)) {
			drawBackground();
		}
		if (!(RAM[0x3F1B] & 1) && RAM[0x3F1F] & 8) {
			drawLayer3Background();
		}
	}

	//Draw OAM (under)
	renderOamGroup(2);
	renderOamGroup(3);

	//Render Players (in pipes)
	renderPlayers(true);

	//L1
	if (drawL1) {
		// Start rendering, by locking surface.
		SDL_LockSurface(screen_s_l1);
		SDL_memset(screen_s_l1->pixels, 0, screen_s_l1->h * screen_s_l1->pitch);

		//Draw scenery
		uint_fast8_t int_b_x = uint_fast8_t(INTERNAL_RESOLUTION_X >> 4) + 1;
		uint_fast8_t int_b_y = uint_fast8_t(INTERNAL_RESOLUTION_Y >> 4) + 1;
		uint_fast8_t block_palette;
		uint_fast16_t tile; uint_fast16_t entry; uint_fast16_t block_index; uint_fast8_t index; uint_fast8_t flip;
		for (uint_fast8_t x = 0; x < int_b_x; x++) {
			for (uint_fast8_t y = 0; y < int_b_y; y++) {
				tile = map16_handler.get_tile(x + offsetX, y + offsetY) & 0x3FF;
				if (tile != 0x25) {
					entry = MAP16_LOCATION + (tile << 4);
					for (uint_fast8_t i = 0; i < 4; i++) {
						block_index = RAM[entry + 1 + (i << 1)] + (RAM[entry + (i << 1)] << 8);
						index = RAM[entry + tile_palette_2 - (i <= 1)];
						flip = (RAM[entry + tile_flips] >> (i << 1)) & 3;

						if (tile >= 0x133 && tile <= 0x136) {
							block_palette = pipe_colors[((x + offsetX) >> 4) & 3];
						}
						else {
							block_palette = index >> ((i & 1) << 2) & 0xF;
						}
						if (block_index != 0xF8) {
							if (flip) {
								draw8x8_tile_f(
									((i << 3) & 0xF) + (x << 4),
									INTERNAL_RESOLUTION_Y + ((i >> 1) << 3) - (y << 4),
									block_index, block_palette,
									flip & 1, flip & 2
								);
							}
							else {
								draw8x8_tile(
									((i << 3) & 0xF) + (x << 4),
									INTERNAL_RESOLUTION_Y + ((i >> 1) << 3) - (y << 4),
									block_index, block_palette
								);
							}
						}
					}
				}
			}
		}

		//End rendering
		SDL_UnlockSurface(screen_s_l1);
		//We can now draw the screen finished product.

		//Mosaic Algorithm
		MosaicScreenSL1(mosaic_val);

		//Rendermode
		if (RAM[0x40] & 2) { glBlendFunc(GL_ONE, GL_ONE); }
		if (RAM[0x40] & 4) { glBlendFunc(GL_DST_COLOR, GL_ZERO); }
		convertL1Tex();

		if (hdmaModeEnabled[HDMA_L1_MODEX] || hdmaModeEnabled[HDMA_L1_MODEY]) {
			SrcR.x = 0;
			SrcR.w = INTERNAL_RESOLUTION_X_P16;
			SrcR.h = 1;
			DestR.w = INTERNAL_RESOLUTION_X_P16;
			DestR.h = 1;
			for (int i = 0; i < int(INTERNAL_RESOLUTION_Y_P16); i++) {
				int hdma_scan = (256 - CameraY + i + offsetYPixel) & 0x1FF;
				SrcR.y = (i + (hdmaLineData[hdma_scan][HDMA_L1_MODEY]));

				DestR.y = ((i + -16 + offsetYPixel));
				DestR.x = (-offsetXPixel + hdmaLineData[hdma_scan][HDMA_L1_MODEX]);
				RenderCopyOpenGLEx(&SrcR, &DestR, screen_t_l1GL, INTERNAL_RESOLUTION_X_P16, INTERNAL_RESOLUTION_Y_P16);
			}
		}
		else {
			SrcR.x = 0;
			SrcR.y = 0;
			SrcR.w = INTERNAL_RESOLUTION_X_P16;
			SrcR.h = INTERNAL_RESOLUTION_Y_P16;

			DestR.x = -offsetXPixel;
			DestR.y = -16 + offsetYPixel;
			DestR.w = INTERNAL_RESOLUTION_X_P16;
			DestR.h = INTERNAL_RESOLUTION_Y_P16;
			RenderCopyOpenGLEx(&SrcR, &DestR, screen_t_l1GL, INTERNAL_RESOLUTION_X_P16, INTERNAL_RESOLUTION_Y_P16);
		}
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}


	//Draw screen darkening (Level Clear)
	if (transition_type == 3) {
		Ren_SetDrawColor(0, 0, 0, bright_val == 0xF ? 255 : (bright_val << 4));
		Ren_FillRect(nullptr);
	}


	//Draw OAM (under)
	renderOamGroup(0);

	//Draw players
	renderPlayers(false);

	//Draw OAM (priority)
	renderOamGroup(1);

	//L3 (priority)
	if (drawBg) {
		if (RAM[0x40] & 1) {
			drawBackground();
		}
		if (RAM[0x3F1B] & 1 && RAM[0x3F1F] & 8) {
			drawLayer3Background();
		}
	}

	//Water
	if (WaterLevel > 0) {
		CreateSprite("Sprites/backgrounds/BackgroundWater.png",
			-int((global_frame_counter + CameraX) & 0xF), INTERNAL_RESOLUTION_Y - (int(WaterLevel) - CameraY),
			512, 512);
	}


	if (gamemode == GAMEMODE_MAIN) {
		//Normal hud
		if (!hudMode && !(RAM[0x3F1F] & 1))
		{
			//Status bar code here
			for (int i = 0; i < 5; i++) {
				VRAM[0xB804 + (i * 2) + 128] = LocalPlayer.skin >= 2 ? char_to_smw(LocalPlayer.player_name_cut[i]) : (0x40 + i + LocalPlayer.skin * 5);
				VRAM[0xB805 + (i * 2) + 128] = LocalPlayer.skin >= 2 ? 7 : 3;
			}

			//WO's
			VRAM[0xB806 + 192] = 0x26;
			VRAM[0xB807 + 192] = 0x6;
			draw_number_dec(5, 3, max(0, 99 - LocalPlayer.WO_counter));

			//Dragon coins
			for (uint_fast8_t d_c = 0; d_c < RAM[0x1420]; d_c++)
			{
				VRAM[0xB800 + ((d_c + 8) * 2) + 128] = 0x2F;
				VRAM[0xB801 + ((d_c + 8) * 2) + 128] = 0x7;
			}
			VRAM[0xB800 + (9 * 2) + 192] = 0x6A;
			VRAM[0xB801 + (9 * 2) + 192] = 0x3;
			VRAM[0xB800 + (10 * 2) + 192] = 0x26;
			VRAM[0xB801 + (10 * 2) + 192] = 0x6;

			//Bonus stars
			draw_number_dec(13, 2, 0, 0x6B);
			draw_number_dec(13, 3, 0, 0x75);

			//Coins
			draw_number_dec(29, 2, RAM[0x0DBF] % 100);

			VRAM[0xB800 + (25 * 2) + 128] = 0x2F;
			VRAM[0xB801 + (25 * 2) + 128] = 0x7;
			VRAM[0xB800 + (26 * 2) + 128] = 0x26;
			VRAM[0xB801 + (26 * 2) + 128] = 0x6;

			//Score
			draw_number_dec(29, 3, getRamValue(0x3F13, 3));


			//Reserve

			/* Top */
			VRAM[0xB800 + (14 * 2) + 64] = 0x30;
			VRAM[0xB801 + (14 * 2) + 64] = 0x3;
			VRAM[0xB800 + (15 * 2) + 64] = 0x31;
			VRAM[0xB801 + (15 * 2) + 64] = 0x3;
			VRAM[0xB800 + (16 * 2) + 64] = 0x31;
			VRAM[0xB801 + (16 * 2) + 64] = 0x3;
			VRAM[0xB800 + (17 * 2) + 64] = 0x32;
			VRAM[0xB801 + (17 * 2) + 64] = 0x3;

			/* Middle */
			VRAM[0xB800 + (14 * 2) + 128] = 0x33;
			VRAM[0xB801 + (14 * 2) + 128] = 0x3;
			VRAM[0xB800 + (17 * 2) + 128] = 0x34;
			VRAM[0xB801 + (17 * 2) + 128] = 0x3;
			VRAM[0xB800 + (14 * 2) + 192] = 0x33;
			VRAM[0xB801 + (14 * 2) + 192] = 0x3;
			VRAM[0xB800 + (17 * 2) + 192] = 0x34;
			VRAM[0xB801 + (17 * 2) + 192] = 0x3;

			/* Bottom */
			VRAM[0xB800 + (14 * 2) + 256] = 0x35;
			VRAM[0xB801 + (14 * 2) + 256] = 0x3;
			VRAM[0xB800 + (15 * 2) + 256] = 0x36;
			VRAM[0xB801 + (15 * 2) + 256] = 0x3;
			VRAM[0xB800 + (16 * 2) + 256] = 0x36;
			VRAM[0xB801 + (16 * 2) + 256] = 0x3;
			VRAM[0xB800 + (17 * 2) + 256] = 0x37;
			VRAM[0xB801 + (17 * 2) + 256] = 0x3;

			//Time
			VRAM[0xB800 + (19 * 2) + 128] = 0x2E;
			VRAM[0xB801 + (19 * 2) + 128] = 0x7;
			VRAM[0xB800 + (20 * 2) + 128] = 0x3F;
			VRAM[0xB801 + (20 * 2) + 128] = 0x7;
			VRAM[0xB800 + (21 * 2) + 128] = 0x4F;
			VRAM[0xB801 + (21 * 2) + 128] = 0x7;

			//Change later
			draw_number_dec(21, 3, getRamValue(0xF31, 2) / 40, 0, 7);

			//KB & Ping
			if (networking)
			{
				VRAM[0xB800 + (12 * 2) + 192] = 0x14;	VRAM[0xB801 + (12 * 2) + 192] = 6;
				VRAM[0xB800 + (13 * 2) + 192] = 0x0B;	VRAM[0xB801 + (13 * 2) + 192] = 6;
				draw_number_dec(11, 3, data_size_now / 1024);

				VRAM[0xB800 + (12 * 2) + 128] = 0x16;	VRAM[0xB801 + (12 * 2) + 128] = 6;
				VRAM[0xB800 + (13 * 2) + 128] = 0x1C;	VRAM[0xB801 + (13 * 2) + 128] = 6;
				draw_number_dec(11, 2, ((abs(latest_server_response) % 3600) % 1000) / 3);
			}
		}
		/*
			Debug hud
		*/
		if (hudMode == 1)
		{
			//Status bar code here
			//Status bar code here
			for (int i = 0; i < 5; i++) {
				VRAM[0xB804 + (i * 2) + 128] = LocalPlayer.skin >= 2 ? char_to_smw(LocalPlayer.player_name_cut[i]) : (0x40 + i + LocalPlayer.skin * 5);
				VRAM[0xB805 + (i * 2) + 128] = LocalPlayer.skin >= 2 ? 7 : 3;
			}

			//WO's
			VRAM[0xB806 + 192] = 0x26;
			VRAM[0xB807 + 192] = 0x6;
			draw_number_dec(5, 3, LocalPlayer.WO_counter);

			//Player X/Y
			draw_number_hex(21, 2, int(LocalPlayer.x), 4);
			draw_number_hex(26, 2, int(LocalPlayer.y), 4);

			//Player Speed X/Y
			draw_number_hex(16, 2, uint_fast16_t(LocalPlayer.Y_SPEED * 256.0), 4);
			draw_number_hex(11, 2, uint_fast16_t(LocalPlayer.X_SPEED * 256.0), 4);

			//Networking symbols
			VRAM[0xB800 + 20 + 192] = networking ? 0x17 : 0x15; VRAM[0xB801 + 20 + 192] = 6;
			VRAM[0xB800 + 18 + 192] = 0x3A; VRAM[0xB801 + 18 + 192] = 6;
			VRAM[0xB800 + 16 + 192] = isClient ? 0xC : 0x1C; VRAM[0xB801 + 16 + 192] = 6;

			//FCounter
			draw_number_hex(29, 2, RAM[0x14], 2);

			//Ping
			if (networking) {
				VRAM[0xB800 + 56 + 192] = 0x16;	VRAM[0xB801 + 56 + 192] = 6;
				VRAM[0xB800 + 58 + 192] = 0x1C;	VRAM[0xB801 + 58 + 192] = 6;
				draw_number_dec(27, 3, ((abs(latest_server_response) % 3600) % 1000) / 3);
			}
			else {
				int time = int(total_time_ticks.count() * 10000.0);
				VRAM[0xB800 + 58 + 192] = 0x1D; VRAM[0xB801 + 58 + 192] = 6;
				VRAM[0xB800 + 56 + 192] = time % 10; VRAM[0xB801 + 56 + 192] = 6;
				VRAM[0xB800 + 54 + 192] = 0x24; VRAM[0xB801 + 54 + 192] = 6;
				draw_number_dec(26, 3, time / 10);
			}

			//FPS
			VRAM[0xB800 + 44 + 192] = 0xF;	VRAM[0xB801 + 44 + 192] = 6;
			VRAM[0xB800 + 46 + 192] = 0x19;	VRAM[0xB801 + 46 + 192] = 6;
			VRAM[0xB800 + 48 + 192] = 0x1C;	VRAM[0xB801 + 48 + 192] = 6;
			draw_number_dec(21, 3, int(1000.0 / (total_time_ticks.count() * 1000.0)));

			//KB
			//VRAM[0xB800 + 26 + 192] = 0x24;	VRAM[0xB801 + 26 + 192] = 6;
			VRAM[0xB800 + 30 + 192] = 0x14;	VRAM[0xB801 + 30 + 192] = 6;
			VRAM[0xB800 + 32 + 192] = 0x0B;	VRAM[0xB801 + 32 + 192] = 6;
			draw_number_dec(14, 3, data_size_now / 1024);
		}

		Chat_Prerender();

		if (!(RAM[0x3F1F] & 2)) {
			//Draw L3 player names
			for (int i = 0; i < Players.size(); i++) {
				MPlayer& CurrentPlayer = Players[i]; 
				if (!CurrentPlayer.PlayerControlled && CurrentPlayer.x > (CameraX - camBoundX) && CurrentPlayer.y > (CameraY - camBoundY) && CurrentPlayer.x < (CameraX + INTERNAL_RESOLUTION_X + camBoundX) && CurrentPlayer.y < (CameraY + INTERNAL_RESOLUTION_Y + camBoundY)) {
					for (int i = 0; i < 5; i++) {
						uint_fast8_t new_l = char_to_smw(CurrentPlayer.player_name_cut[i]);
						draw8x8_tile_2bpp(-12 + int(CurrentPlayer.x) - int(CameraX) + i * 8, 224 - int(CurrentPlayer.y + (CurrentPlayer.STATE ? 40 : 32)) + int(CameraY), new_l, 6);
					}
				}
			}
		}
	}


	//Draw screen cover effects
	uint_fast8_t CoverAm = RAM[0x3F12] & 0xF;
	if (CoverAm) {
		uint_fast8_t C_Index = RAM[0x3F12] >> 4;
		Ren_SetDrawColor(palette_array[C_Index], palette_array[C_Index] >> 8, palette_array[C_Index] >> 16, CoverAm == 15 ? 255 : (CoverAm << 4));
		Ren_FillRect(nullptr);
	}

	//Draw windows
	if (RAM[0x3F1B] & 8) { glBlendFunc(GL_ONE, GL_ONE); }
	for (int window = 0; window < 2; window++) {
		//Window is enabled
		if (RAM[0x3F1B] & (16 << window)) {
			//Window uses HDMA
			DestR = { 0, 0, INTERNAL_RESOLUTION_X, 1 };
			for (int i = 0; i < INTERNAL_RESOLUTION_Y; i++) {
				if (RAM[0x3F1B] & (64 << window)) {
					int scanline = (i + 256 - CameraY) & 0x1FF;
					int_fast16_t x1 = min(hdmaLineData[scanline][HDMA_WIN1_R + window * 2], hdmaLineData[scanline][HDMA_WIN1_L + window * 2]) - CameraX;
					int_fast16_t x2 = max(hdmaLineData[scanline][HDMA_WIN1_R + window * 2], hdmaLineData[scanline][HDMA_WIN1_L + window * 2]) - CameraX;
					DestR.x = x1; DestR.w = x2 - x1;
				}
				if (DestR.w > 0) {
					uint_fast8_t C_Index = hdmaModeEnabled[HDMA_FIXEDCOLORDATA] ? hdmaLineData[i][HDMA_FIXEDCOLORDATA] : 0x10 + (window << 4);
					Ren_SetDrawColor(palette_array[C_Index], palette_array[C_Index] >> 8, palette_array[C_Index] >> 16, 255);
					Ren_FillRect(&DestR);
				}
				DestR.y++;
			}
		}
	}
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	//Titlescreen
	if (gamemode == GAMEMODE_TITLE || gamemode == GAMEMODE_ATTEMPTCONNECTION) {
		RenderTitlescreen();
	}

	//Draw windows (Messagebox, retry, etc)
	if (RAM[0x1B89] > 0) {
		double Window_Size_X = (RAM[0x1B88] >> 1) ? 146.0 : 160.0;
		double Window_Size_Y = (RAM[0x1B88] >> 1) ? 56.0 : 80.0;
		int DestinationX = 128;
		int DestinationY = (RAM[0x1B88] >> 1) ? 122 : 80;

		Window_Size_X = Window_Size_X * ((1.0 + double(RAM[0x1B89])) / 256.0);
		Window_Size_Y = Window_Size_Y * ((1.0 + double(RAM[0x1B89])) / 256.0);

		DestR.x = DestinationX - int(Window_Size_X / 2.0);
		DestR.y = DestinationY - int(Window_Size_Y / 2.0);
		DestR.w = int(Window_Size_X);
		DestR.h = int(Window_Size_Y);
		Ren_SetDrawColor(0, 0, 0, 255);
		Ren_FillRect(&DestR);
	}

	//Render screen-relative was previously a call to renderoamgroup
	if (drawSprites) {
		for (int i = 0; i < OAMGroupSorted[4].size(); i++) {
			OAMTile& T = OAM_Tiles[OAMGroupSorted[4][i]];
			draw_tile_custom(T.pos_x, T.pos_y, T.bsize, T.rotation, T.tile + ((T.props & 192) << 2), T.props, T.scale_x, T.scale_y);
		}
	}

	//Draw L3
	for (uint_fast16_t t3_x = 0; t3_x < 32; t3_x++) {
		for (uint_fast16_t t3_y = 0; t3_y < 28; t3_y++) {
			if (VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)] < MAX_L3_TILES) {
				draw8x8_tile_2bpp(t3_x << 3, t3_y << 3, VRAM[0xB800 + (t3_x << 1) + (t3_y << 6)], VRAM[0xB801 + (t3_x << 1) + (t3_y << 6)]);
			}
		}
	}

	//Draw Reserve
	if (gamemode == GAMEMODE_MAIN && !(RAM[0x3F1F] & 2)) {
		if (LocalPlayer.reserve_item != 0) {
			uint_fast8_t gfx_t = reserve_graphics_tile[(LocalPlayer.reserve_item - 1) * 2];
			uint_fast8_t gfx_p = reserve_graphics_tile[1 + (LocalPlayer.reserve_item - 1) * 2];
			draw_tile_custom((INTERNAL_RESOLUTION_X / 2) - 8, 16, 0x11, 0, gfx_t, gfx_p);
		}
		
	}

	//Debug CPU meter
	if (drawDiag) {
		draw_tile_custom(0, int((total_time_ticks.count() * 1000.0) * 16.0), 0, 0, 0x31, 0xA);
	}

	//Draw screen darkening (Fades)
	if (transition_type != 3) {
		Ren_SetDrawColor(0, 0, 0, bright_val == 0xF ? 255 : (bright_val << 4));
		Ren_FillRect(nullptr);
	}

	//Chat is absolute, and will be rendered last.
	Chat_Render();
}

void render()
{
	PrepareRendering();

	//Convert 16bit palette to 32bit palette (for speed)
	ConvertPalette();
	memcpy(VRAM, &RAM[VRAM_Location], VRAM_Size * sizeof(uint_fast8_t));

	//Asset preload
	if (need_preload_sprites) {
		need_preload_sprites = false;
		PreloadSPR();
		PreloadL3();
	}

	//Overworld rendering
	if (gamemode == GAMEMODE_OVERWORLD) {
		overworld.Render();
		return;
	}

	handleRenderingForPlayer(SelfPlayerNumber);
}