#pragma once

//This handles the whole loop code for 1 frame of the game.
void GameLoop() {
	//Get input
	if (!networking || isClient) {
		CheckInput();
	}

	//Username storages
	if (!isClient) {
		if (networking) {
#ifndef DISABLE_NETWORK
			username_storage.resize(clients.size());
			for (int i = 0; i < clients.size(); i++) {
				username_storage[i] = clients[i]->username;
			}
#endif
		}
		else {
			if (username_storage.empty()) {
				username_storage.resize(1);
			}
			username_storage[0] = username;
		}
	}

	//DMA Start
	DMAStartFrame();
	uint_fast16_t count = uint_fast16_t(min(65535, int(total_time_ticks.count() * 3584.0)));
	RAM[0x4207] = count & 0xFF; RAM[0x4209] = count / 256;

	//Check if we can process
#ifndef DISABLE_NETWORK
	if (networking) {
		if (isClient && !(global_frame_counter % 60)) {
			data_size_now = data_size_current; data_size_current = 0;
		}
		if (clients.size() == 0 && !isClient) { //Server is empty, no point in wasting time processing.
			return;
		}
	}
#endif

	//Prepare players
	CheckForPlayers();
	for (uint_fast8_t i = 0; i < Players.size(); i++) {
		MPlayer& CurrPlayer = Players[i];
		CurrPlayer.player_index = i;
		if (!isClient && networking) { CurrPlayer.PlayerControlled = false; }
		else {
			CurrPlayer.PlayerControlled = networking ? (i == SelfPlayerNumber) : true; //Only control myself
		}
		if (CurrPlayer.PlayerControlled) {
			CurrPlayer.skin = local_multiplayer ? i : my_skin;
		}
	}

	//TO-DO: Implement 0x100 properly
	show_full_screen = gamemode == GAMEMODE_OVERWORLD || gamemode == GAMEMODE_TITLE;

	//Players synced wait
	if (players_synced || !networking) {
		global_frame_counter++;

		if (gamemode == GAMEMODE_OVERWORLD) {
			//Main overworld loop.
			if (Players.size() == 0) {
				return;
			}
			overworld.Process();

			if (!isClient) {
				if (global_frame_counter <= 5) {
					RAM[0x9D] = 0; RAM[0x3F10] = 0x0F; RAM[0x3F11] = 2;
				}
				else {
					handleTransitions();
				}
			}

			if (gamemode != GAMEMODE_OVERWORLD) { return; }
		}

		if (gamemode == GAMEMODE_MAIN || gamemode == GAMEMODE_TITLE) {
			if (gamemode == GAMEMODE_TITLE) {
				ProcessTitlescreen();
			}
			//Main gamemode loop.
			WaterLevel = (RAM[0x3F1F] & 8) ? 0 : getRamValue(0x24, 2);

			if (RAM[0x9D]) {
				ingame_frame_counter++;
			}

			if (!isClient) {
				if (global_frame_counter <= 5) {
					RAM[0x9D] = 0; RAM[0x3F10] = 0xFF; RAM[0x3F11] = 2;
				}
				else {
					handleTransitions();
				}
				if (gamemode == GAMEMODE_OVERWORLD) { return; }

				//Level Clear
				if (RAM[0x1493] > 0) {
					memset(&RAM[0x2000], 0, 0x80);
					if ((global_frame_counter % 3) == 0) {
						RAM[0x1493]--;
					}
					if (!RAM[0x1493]) {
						RAM[0x3F11] = 5;
					}
				}
			}

			//Earthquake
			if (RAM[0x1887] > 0) {
				if (!isClient) { RAM[0x1887]--; }
				if (!networking || isClient) { VibrateController(1.0, 32); }
			}

			//Music restore
			if (!isClient) {
				//PSwitch
				if (RAM[0x9D] && RAM[0x14AD] > 0 && !(ingame_frame_counter & 3)) {
					RAM[0x14AD]--;
					if (RAM[0x14AD] == 0x1E) {
						RAM[0x1DFC] = 0x24;
					}
					if (RAM[0x14AD] == 0) {
						if (RAM[0x0DDA] != 0 && RAM[0x1DFB] == 0xB) {
							RAM[0x1DFB] = RAM[0x0DDA];
						}
					}
				}

				//Star
				if (RAM[0x9D] && RAM[0x1490] > 0 && !(ingame_frame_counter & 3)) {
					RAM[0x1490]--;
					if (RAM[0x1490] == 0) {
						if (RAM[0x0DDA] != 0 && RAM[0x1DFB] == 0xA) {
							RAM[0x1DFB] = RAM[0x0DDA];
						}
					}
				}
			}

			mapWidth = max(1, RAM[0x3F00] + (RAM[0x3F01] << 8));
			mapHeight = max(1, RAM[0x3F02] + (RAM[0x3F03] << 8));

			RAM[0x3F0A] = networking;

			LevelManager.start_x = RAM[0x3F0B] + RAM[0x3F0C] * 256;
			LevelManager.start_y = RAM[0x3F0D] + RAM[0x3F0E] * 256;

			uint_fast8_t LAST_9D = RAM[0x9D];
			if (LAST_9D) {
				if (!isClient) {
					for (uint_fast16_t i = 0; i < 256; i++) {
						if (player_netcommand[i] & 0xF) {
							player_netcommand[i]--;
						}
					}
				}
				if (!isClient || !networking) { //if we are the server or we are playing locally...
					RAM[0x300A] = 0;
					Clear_OAM();
					memset(&RAM[0x2780], 0, 0x80); //Clear sprite coll flags
					for (uint_fast8_t i = 0; i < 128; i++) {
						RAM[0x2A80 + i] = RAM[0x2A80 + i] & 0xFD; //I am not sure if this is correct.. lol
					}

					uint_fast32_t time = getRamValue(0xF31, 2);
					if (time > 0) {
						time--;
						if (time == 0) {
							RAM[0x1DFB] = 9; memset(&player_netcommand, 0xF, 256);
						}
						writeToRam(0xF31, time, 2);
					}
				}
			}

			//Process all players
			int camera_total_x = 0; int camera_total_y = 0;
			for (uint_fast8_t i = 0; i < Players.size(); i++) {
				MPlayer& CurrPlayer = Players[i];
				CurrPlayer.player_index = i;
				if (CurrPlayer.PlayerControlled) {
					CurrPlayer.mouse_x = mouse_x + CameraX;
					CurrPlayer.mouse_y = (INTERNAL_RESOLUTION_Y - mouse_y) + CameraY;
					CurrPlayer.mouse_state[0] = mouse_down_l;
					CurrPlayer.mouse_state[1] = mouse_down_r;
					CurrPlayer.mouse_state[2] = mouse_w_up;
					CurrPlayer.mouse_state[3] = mouse_w_down;
					for (int i = 0; i < player_name_size; i++) {
						if (i >= username.length()) {
							CurrPlayer.player_name_cut[i] = ' ';
						}
						else {
							CurrPlayer.player_name_cut[i] = username.at(i);
						}
					}

					if (player_netcommand[i] & 0xF) {
						uint_fast8_t command = player_netcommand[i] >> 4;
						if (command == 0) { CurrPlayer.Die(); }
						if (command == 1) { CurrPlayer.Hurt(); }
						//Set State (Direct, note, this is not entirely for much states, and it's just reserved. might be buggy)
						if (command >= 2 && command < 13) {
							CurrPlayer.STATE = command - 2;
						}
						//Set State (Animated)
						if (command >= 13) {
							uint_fast8_t NEW_ST = command - 12;
							if (CurrPlayer.STATE != NEW_ST && (CurrPlayer.STATE == 0 || NEW_ST != 1)) {
								CurrPlayer.powerup_anim = 31 + ((NEW_ST - 1) << 6);
								if (!networking) { RAM[0x9D] = 0; player_netcommand[i] = 0; }
							}
							CurrPlayer.STATE = NEW_ST;
						}
					}
				}

				CurrPlayer.Process();

				camera_total_x += max(0, int(CurrPlayer.CAMERA_X - 120.0));
				camera_total_y += max(0, int(CurrPlayer.CAMERA_Y - 104.0));

				bool DoSync = (networking && !isClient) ? (CurrPlayer.server_position_sync_s == CurrPlayer.server_position_sync_c) : true;
				uint_fast16_t x_r = uint_fast16_t(DoSync ? CurrPlayer.x : CurrPlayer.server_position_sync_x);
				uint_fast16_t y_r = uint_fast16_t(DoSync ? CurrPlayer.y : CurrPlayer.server_position_sync_y);
				uint_fast8_t x_s_r = uint_fast8_t(CurrPlayer.X_SPEED * 16.0);
				uint_fast8_t y_s_r = uint_fast8_t(CurrPlayer.Y_SPEED * 16.0);

				uint_fast16_t m_state_1 = (CurrPlayer.mouse_x & 0x3FFF) + (CurrPlayer.mouse_state[0] << 15) + (CurrPlayer.mouse_state[2] << 14);
				uint_fast16_t m_state_2 = (CurrPlayer.mouse_y & 0x3FFF) + (CurrPlayer.mouse_state[1] << 15) + (CurrPlayer.mouse_state[3] << 14);

				uint_fast8_t result = uint_fast8_t(CurrPlayer.CROUCH) << 7;
				for (uint_fast8_t i = 0; i < 7; i++) {
					result += CurrPlayer.p_pad[i] << i;
				}

				RAM[0x5000 + i] = x_r;
				RAM[0x5100 + i] = x_r >> 8;
				RAM[0x5200 + i] = y_r;
				RAM[0x5300 + i] = y_r >> 8;
				RAM[0x5400 + i] = x_s_r;
				RAM[0x5500 + i] = y_s_r;
				RAM[0x5600 + i] = CurrPlayer.KO_counter;
				RAM[0x5700 + i] = CurrPlayer.WO_counter;
				RAM[0x5800 + i] = CurrPlayer.STATE;
				RAM[0x5900 + i] = CurrPlayer.DEAD;
				RAM[0x5A00 + i] = m_state_1;
				RAM[0x5B00 + i] = m_state_1 >> 8;
				RAM[0x5C00 + i] = m_state_2;
				RAM[0x5D00 + i] = m_state_2 >> 8;
				RAM[0x5E00 + i] = result;
				RAM[0x5F00 + i] = CurrPlayer.in_pipe;

				if (LAST_9D && !isClient && RAM[0x1493] == 0) {
					CheckSpritesInCam(int(max(128.0, CurrPlayer.CAMERA_X)), int(max(112.0, CurrPlayer.CAMERA_Y)));
				}
			}

			//Player amount for Lua
			RAM[0x3F0F] = uint_fast8_t(Players.size());

			PlayerInteraction();

			if (!isClient) {
				if (RAM[0x1411] != 0) { writeToRam(0x1462, uint_fast32_t(camera_total_x / max(1, int(Players.size()))), 2); }
				if (RAM[0x1412] != 0) { writeToRam(0x1464, uint_fast32_t(camera_total_y / max(1, int(Players.size()))), 2); }

				if (LAST_9D) {
					Sprites.process_all_sprites(); //we're processing sprites. we're either the server or a player in local mode.
					if (lua_loaded) { lua_run_main(); }
					map16_handler.process_global();
					processParticles();
				}
				ProcessMessageBoxes();
			}
		}
	}

	ProcessGraphicAnimations();
	ProcessChat();
	ProcessHDMA();

	RAM[0x13] = global_frame_counter & 0xFF; RAM[0x14] = ingame_frame_counter & 0xFF;

	//Finish OAM if we're the server or playing singleplayer.
	if (!isClient || !networking) { Finish_OAM(); }
	if (debugging_enabled) { debugging_functions(); }
}