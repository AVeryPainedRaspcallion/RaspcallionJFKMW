#pragma once

//Add chat line
void Add_Chat(string c) {
	//This is only used in the client to push the chat back. This is a very rough implementation.
	for (int i = 5; i >= 1; i--) {
		Curr_ChatString[i] = Curr_ChatString[i - 1];
		Time_ChatString[i] = Time_ChatString[i - 1];
	}
	Curr_ChatString[0] = c;
	Time_ChatString[0] = chat_onscreen_timer;
}

void Send_Chat(string c) {
	Curr_PChatString = c;
	Time_ChatString[0] = chat_onscreen_timer;
}

//Serverside chat handler
void Chat_ServerSide() {
	for (int i = 0; i < Players.size(); i++)
	{
		MPlayer& CurrPlayer = Players[i];
		if (CurrPlayer.current_chat != CurrPlayer.old_chat) {
			//Update curr pchatstring to send later
			Curr_PChatString = CurrPlayer.curr_chat_string;
			Curr_PChatString = Curr_PChatString.substr(0, min(64, int(Curr_PChatString.length())));
			Time_ChatString[0] = chat_onscreen_timer;

			//Update strings and output to console
			CurrPlayer.old_chat = CurrPlayer.current_chat;
			cout << lua_color << "[Chat S] " << Curr_PChatString << endl;
			discord_message(Curr_PChatString);
			lua_on_chatted(CurrPlayer.curr_chat_string, i+1);
		}
	}
	
	//Reset the chatstring after a while lol
	if (Time_ChatString[0] > 0 && networking) {
		Time_ChatString[0]--;
		if (!Time_ChatString[0]) {
			Curr_PChatString = "";
		}
	}
}

void Chat_ClientSide() {
	//Add a new line to chat. We have to check that it's different than hte previous one, and also, make sure it's not blank.
	if (Old_ChatString != Curr_PChatString && Curr_PChatString != "") {
		Add_Chat(Curr_PChatString);
		Old_ChatString = Curr_PChatString;

		cout << lua_color << "[Chat C] " << Curr_PChatString << endl;
	}

	//Pinnacle of shitcode
	for (int i = 0; i < 6; i++) {
		if (Time_ChatString[i] > 0) {
			Time_ChatString[i]--;
			if (!Time_ChatString[i]) {
				Curr_ChatString[i] = "";
			}
		}
	}

	//If we are chatting
	if (Chatting) {
		//Letters
		for (int i = 0x41; i <= 0x5A; i++) {
			if (getKey(i)) {
				char new_l = i + (state[SDL_SCANCODE_LSHIFT] ? 0x0 : 0x20);
				Typing_In_Chat += new_l;
			}
		}
		//Numbers
		for (int i = 0x30; i <= 0x39; i++) {
			if (getKey(i)) {
				Typing_In_Chat += state[SDL_SCANCODE_LSHIFT] ? Number_Caps[i - 0x30] : ((i - 0x30) + '0');
			}
		}
		//Spaces
		if (getKey(0x20)) {
			Typing_In_Chat += " ";
		}
		//Special chars 21 2E 2C 2D 3C 3E
		if (getKey(VK_OEM_PERIOD)) { Typing_In_Chat += "."; }
		if (getKey(VK_OEM_COMMA)) { Typing_In_Chat += ","; }
		if (getKey(VK_OEM_MINUS)) { Typing_In_Chat += "-"; }
		if (getKey(VK_OEM_PLUS)) { Typing_In_Chat += "+"; }
		//Delete last chat
		if (getKey(0x08) && Typing_In_Chat.size() > 0) { Typing_In_Chat.pop_back(); }
		//Typing in chat limiter
		Typing_In_Chat = Typing_In_Chat.substr(0, min(49, int(Typing_In_Chat.length())));
	}

	//Da status :flushed:
	bool stat = Chatting ? state[SDL_SCANCODE_RETURN] : state[input_settings[10]];
	if (stat != pressed_start) {
		pressed_start = stat;
		if (stat) {
			Chatting = !Chatting;
			if (Chatting == false) {
				if (gamemode != GAMEMODE_TITLE) {
					MPlayer& LocalPlayer = GetPlayerByNumber(SelfPlayerNumber);
					LocalPlayer.Chat(Typing_In_Chat);
				}
			}
			else {
#ifdef _WIN32
				memset(KeyStates, 1, sizeof(KeyStates));
#endif
			}
			Raw_PChatString = Typing_In_Chat;
			Typing_In_Chat = "";
		}
	}
}

void ProcessChat() {
	if (!isClient) {
		Chat_ServerSide();
	}
	if (isClient || !networking) {
		Chat_ClientSide();
	}
}

void Chat_Prerender() {
	//Render chat
	if ((Chatting && gamemode != GAMEMODE_TITLE) || Time_ChatString[0] > 0) {
		memset(&VRAM[0xBC00], 0xFF, 896); //Clear bottom
	}
	bool stat = (state[input_settings[9]]) || BUTTONS_GAMEPAD[8];
	if (stat != pressed_select) {
		pressed_select = stat;
		if (stat) {
			RAM[0x1DFC] = 0x15;
			showing_player_list = !showing_player_list;
		}
	}
}

void Chat_Render()
{
	//Render chat
	if ((Chatting && gamemode != GAMEMODE_TITLE) || Time_ChatString[0] > 0) {
		int y = 27;
		string Curr_Typing = (Typing_In_Chat + ((global_frame_counter % 20) > 10 ? "_" : ""));
		int Typing_Len = int(Curr_Typing.length());
		if (Chatting) {
			y = (Typing_Len > 31) ? 25 : 26;
			for (int i = 0; i < Curr_Typing.length(); i++) {
				int x = (i + 1) << 3;
				draw8x8_tile_2bpp(x & 0xFF, (y << 3) + ((x >> 8) << 3), char_to_smw(Curr_Typing.at(i)), 6);
			}
		}
		for (int cc = 0; cc < 6; cc++) {
			string C_String = Curr_ChatString[cc];
			int C_len = int(Curr_ChatString[cc].length());
			y -= ((C_len > 31) ? 2 : 1);
			for (int i = 0; i < C_String.length(); i++) {
				int x = (i + 1) << 3;
				draw8x8_tile_2bpp(x & 0xFF, (y << 3) + ((x >> 8) << 3), char_to_smw(C_String.at(i)), 6);
			}
		}
	}
	else {
		//Player list
		if (showing_player_list) {
			int y = 26;
			for (int i = 0; i < Players.size(); i++)
			{
				MPlayer& CurrentPlayer = Players[i];
				draw8x8_tile_2bpp(8, y << 3, i + 1, 6);
				draw8x8_tile_2bpp(14, y << 3, 0x3C, 6);

				for (int i = 0; i < player_name_size; i++) {
					draw8x8_tile_2bpp(24 + (i << 3), y << 3, char_to_smw(CurrentPlayer.player_name_cut[i]), 6);
				}

				//KO's
				string WC = to_string(CurrentPlayer.KO_counter) + " KO's " + to_string(CurrentPlayer.WO_counter) + " WO's";
				int x = 240;
				for (int i = int(WC.length()-1); i >= 0; i--) {
					draw8x8_tile_2bpp(x, y << 3, char_to_smw(WC[i]), 6);
					x -= 8;
				}
				y--;
			}
		}
	}
}