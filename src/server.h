#pragma once

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD CEvent) {
	if (CEvent == CTRL_CLOSE_EVENT) {
		discord_message("Server stopped.");
	}
	return TRUE;
}
#endif

//Commands
bool processing_cmd_queue = false;
void cinLoop() {
	//Commands
	std::string new_cmd;
	while (!quit && getline(std::cin, new_cmd)) {
		if (new_cmd != "") {
			while (processing_cmd_queue) { DATA_SAFETY_WAIT }
			server_command_queue.push_back(new_cmd);
		}
	}
}

void server_code() {
#ifdef _WIN32
	if(SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleHandler, TRUE) == FALSE) {
		cout << red << "[JFKMW] Console hooks are not working." << endl;
	}
#endif
	SDL_Init(SDL_INIT_TIMER | SDL_INIT_EVENTS);
	cout << yellow << "[JFKMW] Starting up a server." << endl;

	//OW
	overworld.Initialize();

	//Network stuff
	isClient = false;
	networking = true;

	//Data and threads
	data_size_current = 0;
	thread = new sf::Thread(&NetworkLoop); thread->launch();
	thread_alt = new sf::Thread(&cinLoop); thread_alt->launch();

	//network print toggle
	bool DisablePrints = false;
	while (!quit) {
		WAIT_READ_COMPLETE("server loop wait")

		//Start
		doing_write = true;

		//Stats
		START_CHECK = chrono::high_resolution_clock::now();
		GameLoop(); SoundLoop();
		CURRENT_CHECK = chrono::high_resolution_clock::now();
		total_time_ticks = chrono::duration_cast<chrono::duration<double>>(CURRENT_CHECK - START_CHECK);

		//Network stats
		if (!(global_frame_counter % 60) && clients.size() > 0) {
			if (DisablePrints) {
				cout << green << "[Network] latest loop : "
					<< dec << total_time_ticks.count() * 1000.0 << " ms. "
					<< (double(data_size_current) / 1024.0) << "kb/s in, " << (double(data_size_now) / 1024.0) << "kb/s out" << endl;
			}
			data_size_current = 0;
			data_size_now = 0;
		}

		processing_cmd_queue = true;
		//Commands
		for (int i = 0; i < server_command_queue.size(); i++) {
			string command = server_command_queue[i];
			vector<string> cmd_data = split(command, ' ');
			if (command == "help") {
				cout << yellow << "Server commands:\n";
				cout << "changelevel/level <ID> - change level\n";
				cout << "sync - force sync game state\n";
				cout << "pack <NAME> - switch modpack\n";
				cout << "fullsync - force full sync of game state, including music\n";
				cout << "stats - toggle network stats\n";
				cout << "editmem <RAM> <VALUE> - edit memory\n";
				cout << "pvp - toggle pvp\n";
				cout << "dump - dump ram and level\n";
				cout << "reload - reload Lua\n";
				cout << "overworld - return to overworld\n";
				cout << "list - see player list\n";
				cout << "close - stop server\n";
				cout << "kick/ban <PLAYER> - moderation features" << endl;
			}
			if (command == "sync") {
				RAM[0x3F11] = 0;
				RAM[0x3F10] = 0;
				RAM[0x9D] = 1;
				TriggerRAMSync();
				cout << green << "[Network] Syncing RAM to other players.." << endl;
			}
			if (command == "stats") {
				DisablePrints = !DisablePrints;
				if (!DisablePrints) {
					cout << green << "[Network] Disabled Network latest loop prints." << endl;
				}
				else {
					cout << green << "[Network] Enabled Network latest loop prints." << endl;
				}
			}
			if (command == "fullsync") {
				cout << green << "[Network] Syncing music to other players.." << endl;
				music_latest_sync++;
			}
			if (cmd_data[0] == "editmem" && cmd_data.size() > 2) {
				RAM[safe_stoi(cmd_data[1], 16) & 0x1FFFF] = uint_fast8_t(safe_stoi(cmd_data[2], 16));
			}
			if (command == "pvp") {
				pvp = !pvp;
				string msg = pvp ? "PVP is now enabled" : "PVP is now off";
				cout << green << "[Network] " << msg << endl;
				Send_Chat(msg); discord_message(msg);
			}
			if (command == "close") {
				quit = true;
			}
			if (command == "dump") {
				dump_ram();
				dump_level_data();
			}
			if (command == "overworld") {
				RAM[0x3F11] = 5;
			}
			if (command == "reload") {
				cout << blue << "[Network] Reloading lua" << endl;
				lua_loadfile(last_lua_file);
				lua_run_init();
			}
			if (command == "list") {
				cout << cyan << "List of players (" << clients.size() << "): " << endl;
				for (int i = 0; i < clients.size(); i++) {
					cout << to_string(i + 1) << ". " << clients[i]->username << " (" << clients[i]->getRemoteAddress() << ")" << endl;
				}
			}
			//multi-arg commands
			if (cmd_data.size() > 1) {
				if (cmd_data[0] == "changelevel" || cmd_data[0] == "level") {
					writeToRam(0x3f08, safe_stoi(cmd_data[1], 16), 2); RAM[0x3F11] = 4;
					discord_message("Switched to level " + cmd_data[1]);
				}
				if (cmd_data[0] == "pack") {
					LoadPack(cmd_data[1]);
					RAM[0x3F11] = 5;
				}
				if (cmd_data[0] == "kick") {
					int plr = safe_stoi(cmd_data[1], 10);
					if (plr > 0 && plr < (clients.size() + 1)) {
						GNetSocket* t = clients[plr - 1];
						PreparePacket(Header_FailedToConnect);
						CurrentPacket << "Kicked.";
						SendPacket(t);
					}
					else {
						cout << red << "Not valid." << endl;
					}
				}
				if (cmd_data[0] == "ban") {
					int plr = safe_stoi(cmd_data[1], 10);
					if (plr > 0 && plr < (clients.size() + 1)) {
						GNetSocket* t = clients[plr - 1];
						bans.push_back(t->getRemoteAddress());
						PreparePacket(Header_FailedToConnect);
						CurrentPacket << "Banned from server.";
						SendPacket(t);
					}
					else {
						cout << red << "Not valid." << endl;
					}
				}
			}
		}
		server_command_queue.clear();
		processing_cmd_queue = false;

		//Sleeping
		doing_write = false;

		CAP_FPS60
	}

	//Finish
	thread->terminate();
	thread_alt->terminate();
}