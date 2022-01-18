#pragma once
//Titlescreen, simple handler.
bool pickingmode = false;
bool in_type = false;
bool type_step = false;
int choice = 0;

void InitTitlescreen() {
	gamemode = GAMEMODE_TITLE;
	pickingmode = false; in_type = false; choice = 0; type_step = false;
	networking = false; isClient = false; disconnected = false;
	RAM[0x1DFB] = 0xC;
}

void ProcessTitlescreen() {
	if (RAM[0x3F11]) { return; }
	if (pickingmode) {
		RAM[0x9D] = 0;
		if (in_type) {
			if (Typing_In_Chat == " ") {
				Typing_In_Chat = "";
			}
			//We sent it
			if (!Chatting) {
				if (Raw_PChatString == "") {
					type_step = false; in_type = false;
				}
				else {
					switch (choice) {
					case 1:
						//Multiplayer server joining
						if (!type_step) {
							RAM[0x1DFC] = 1;
							Chatting = true;
							type_step = true;
							ip = Raw_PChatString;
						}
						else {
							PORT = safe_stoi(Raw_PChatString, 10);
							quit = true;
							gamemode = GAMEMODE_ATTEMPTCONNECTION;
							networking = true;
						}
						break;
					case 3:
						networking = false; isClient = false;
						Chatting = false; in_type = false;
						LoadPack(Raw_PChatString);
						LevelManager.LoadLevel(0xFFFF);
						old_1dfb = 0; //Force sound restart
						pickingmode = false;
						break;
					default:
						//Level picking
						Chatting = false;
						gamemode = GAMEMODE_MAIN;
						LevelManager.LoadLevel(safe_stoi(Raw_PChatString, 16));
						break;
					}
				}
			}
		}
		else {
			if (pad_p[button_up] || pad_p[button_down]) {
				RAM[0x1DFC] = 6;
				choice += pad_p[button_down] - pad_p[button_up];
				choice = (choice) & 3;
			}

			if (pad_p[button_b]) {
				RAM[0x1DFC] = 1;
				if (choice == 0) {
					RAM[0x3F11] = 5;
				}
				else {
					Chatting = true;
					in_type = true;
					type_step = false;
				}
			}
		}
	}
	else {
		Chatting = false;
		RAM[0x9D] = global_frame_counter < 180 ? 0 : 1;
	}
	if (pad_p[button_b]) {
		if (!pickingmode) {
			global_frame_counter = 400;
			choice = 0;
			pickingmode = true;
		}
	}
}

//Could be macros
void DrawTitlestring(string t, int y, int off = 128) {
	for (int i = 0; i < t.length(); i++) {
		uint_fast8_t new_l = char_to_smw(t[i]);
		draw8x8_tile_2bpp(off - ((int_fast16_t(t.length()) / 2) * 8) + i * 8, y, new_l, 6);
	}
}

void DrawTitlestringNotCenter(string t, int x, int y) {
	for (int i = 0; i < t.length(); i++) {
		uint_fast8_t new_l = char_to_smw(t[i]);
		draw8x8_tile_2bpp(x + i * 8, y, new_l, 6);
	}
}

//Rendering
void RenderTitlescreen() {
	int circle_size = max(0, int(global_frame_counter) - 120) * 4;
	Ren_SetDrawColor(0, 0, 0, 255);

	//Windows
	DestR.w = 256;
	DestR.h = 112;
	DestR.x = 0;
	DestR.y = -circle_size;
	Ren_FillRect(&DestR);

	DestR.y = 112 + circle_size;
	Ren_FillRect(&DestR);

	DestR.w = 128;
	DestR.h = 224;
	DestR.y = 0;
	DestR.x = -circle_size;
	Ren_FillRect(&DestR);

	DestR.x = 128 + circle_size;
	Ren_FillRect(&DestR);

	//Circle
	CreateSprite("Sprites/ui/window.png", 128 - circle_size, 112 - circle_size, circle_size * 2, circle_size * 2);

	DrawTitlestring("JFKMW " + GAME_VERSION + " (" + Modpack + ")", 224 - 32, 128);

	if (pickingmode) {
		if (in_type) {
			if (!networking) {
				//Draw hint
				switch (choice) {
				case 1:
					DrawTitlestring(type_step ? "Server port" : "Input the IP address", 168 - 48);
					break;
				case 3:
					DrawTitlestring("Input Pack (Default)", 168 - 48);
					break;
				default:
					DrawTitlestring("Input Level Num", 168 - 48);
				}
				DrawTitlestringNotCenter(Typing_In_Chat + ((global_frame_counter % 64) > 32 ? "_" : ""), 48, 168 - 32);
			}
			else {
				DrawTitlestring("Attempting connection", 168 - 48);
				DrawTitlestring("Please wait...", 168 - 32);
			}
		}
		else {
			DrawTitlestringNotCenter("OPTION A ...SINGLEPLAYER", 40, 168 - 48);
			DrawTitlestringNotCenter("OPTION B ...MULTIPLAYER", 40, 168 - 32);
			DrawTitlestringNotCenter("OPTION C ...LEVEL DEBUG", 40, 168 - 16);
			DrawTitlestringNotCenter("OPTION D ...CHANGE PACK", 40, 168);
			if ((global_frame_counter & 0x1F) > 0x7) { draw8x8_tile_2bpp(32, 120 + choice * 16, 0x3B, 0); }
		}
	}
	else {
		DrawTitlestring(last_status, 160);
		DrawTitlestring(latest_error, 168);
	}
	CreateSprite(Modpack + "/title.png", 0, 0, 256, 224);
	Ren_SetDrawColor(0, 0, 0, (max(0, 255 - max(0, int(global_frame_counter) - 45) * 4) >> 3) << 3);
	Ren_FillRect(NULL);
}