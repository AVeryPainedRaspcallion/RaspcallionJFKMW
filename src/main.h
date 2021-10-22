#pragma once

void player_code() {
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) != 0) {
		cout << red << "[SDL] SDL audio/video initialization failed. SDL Error: " << SDL_GetError() << endl; return;
	}
	if (SDL_Init(SDL_INIT_EVENTS) != 0) {
		cout << red << "[SDL] SDL event initialization failed. SDL Error: " << SDL_GetError() << endl; return;
	}
	if (controller > -1) {
		if (SDL_Init(SDL_INIT_GAMECONTROLLER) != 0) {
			cout << red << "[SDL] Controller init failed. SDL Error: " << SDL_GetError() << endl; return;
		}
	}
	if (haptic > -1) {
		if (SDL_Init(SDL_INIT_HAPTIC) != 0) {
			cout << red << "[SDL] Haptic initialization failed. SDL Error: " << SDL_GetError() << endl; return;
		}
	}

	//Load SDL part
	screen(resolution_x, resolution_y);
	init_input();
	init_audio();
	PreloadAllTextures();

	if (!testing_level.length()) {
		InitTitlescreen();
	}

	while (true) {
		//Cleanup
		Reset_BG();
		LevelSprites.clear();
		particles.clear();

		disconnected = false;
		PlayerAmount = 0; SelfPlayerNumber = 0; CheckForPlayers();
		quit = false;

		midway_activated = false;
		global_frame_counter = 0;

		//Testing mode!
		if (testing_level.length()) {
			gamemode = GAMEMODE_MAIN; LevelManager.LoadLevel(stoi(testing_level, nullptr, 16));
		}

		//Gamemode load checks
		if (gamemode == GAMEMODE_MAIN || gamemode == GAMEMODE_ATTEMPTCONNECTION) {
			if (networking) {
				isClient = true;
#ifndef DISABLE_NETWORK
				if (!ConnectClient()) {
					cout << red << "[Network] Failed to connect. Falling back to normal mode." << endl; last_status = "Failed to connect.";
					InitTitlescreen();
					continue;
				}
#else
				cout << red << "[Network] Multiplayer is not supported in this build!" << endl; last_status = "Not supported.";
				InitTitlescreen();
				continue;
#endif
			}
		}

		//Initialize Players
		if (!networking) {
			PlayerAmount = 1; SelfPlayerNumber = 0;
			if (local_multiplayer) {
				for (int i = 0; i < 4; i++) {
					if (gGameController[i]) { PlayerAmount++; }
				}
			}
			CheckForPlayers();
		}

		//Title
		if (gamemode == GAMEMODE_TITLE) {
			InitTitlescreen();
			LevelManager.LoadLevel(0xFFFF);
		}
		
		//OW
		if (gamemode == GAMEMODE_OVERWORLD) {
			last_status = "";
			if (!networking) {
				overworld.Initialize();
			}
		}

#ifndef DISABLE_NETWORK
		//Initialize Multiplayer Client
		if (networking) {
			thread = new sf::Thread(&NetWorkLoop); thread->launch();
			gamemode = GAMEMODE_MAIN;
		}
#endif
		//Player Init Wait
		cout << yellow << "[JFKMW] Waiting for player..." << endl;
		while (Mario.size() == 0) {
			DATA_SAFETY_WAIT
		}

		//Main Loop
		while (!done()) {
			WAIT_READ_COMPLETE
			doing_write = true;
			cls();
			chrono::high_resolution_clock::time_point t1 = chrono::high_resolution_clock::now();
			if (!gGameController) {
				check_input();
			}
			game_loop_code(); SoundLoop();
			render();
			doing_write = false;

			chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
			total_time_ticks = chrono::duration_cast<chrono::duration<double>>(t2 - t1);
			redraw();

			if (disconnected) {
				quit = true; cout << red << "[Network] Disconnected." << endl; last_status = "Disconnected from server"; break;
			}
		}
		doing_read = false; doing_write = false;

		//We quit the game, deinitialize network if needed
#ifndef DISABLE_NETWORK
		if (networking && isClient) { isClient = false; networking = false; socketG.disconnect(); CurrentPacket.clear(); gamemode = GAMEMODE_OVERWORLD; thread->terminate(); }
#endif
		Terminate_Music();

		//return to old gamemode
		if (gamemode == GAMEMODE_TITLE || testing_level != "") {
			return;
		}
		if (gamemode == GAMEMODE_OVERWORLD) {
			gamemode = GAMEMODE_TITLE;
		}
		if (gamemode == GAMEMODE_MAIN) {
			gamemode = (testing_level != "" ? GAMEMODE_MAIN : GAMEMODE_TITLE);
		}

		cout << yellow << "[JFKMW] Redoing loop" << endl;
	}

}