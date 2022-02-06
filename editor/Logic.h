#pragma once

//General Logic
int GetClickedOn(bool m) {
	if (mouse_l_p) {
		//Toolbar click
		if (m) {
			return 0;
		}

		//Detect Sprite Click
		for (int i = 0; i < levelSData.size(); i++) {
			getSpriteRecs(i);
			if (CheckMouseInBounds(&blockarea)) {
				MouseSelected = i;
				MouseResizing = false;
				MouseOffToWindowX = mouse_x - blockarea.x;
				MouseOffToWindowY = mouse_y - blockarea.y;
				return 3;
			}
		}

		//Detect Level Click
		if (SelectedLevelPart >= 0 && SelectedLevelPart < levelLData.size()) {
			getPartRecs(SelectedLevelPart);
			if (CheckMouseInBounds(&blockarea) && !CheckMouseInBounds(&blockareares)) {
				MouseDoResizeX = abs(mouse_x - (blockarea.x + blockarea.w / 2)) >= (-8 + blockarea.w / 2);
				MouseDoResizeY = abs(mouse_y - (blockarea.y + blockarea.h / 2)) >= (-8 + blockarea.h / 2);

				MouseResizingX = mouse_x > (blockarea.x + blockarea.w / 2);
				MouseResizingY = mouse_y > (blockarea.y + blockarea.h / 2);
				MouseSelected = SelectedLevelPart;
				MouseResizing = true;
				return 2;
			}
		}

		for (int i = (levelLData.size() - 1); i >= 0; i--) {
			getPartRecs(i);
			if (CheckMouseInBounds(&blockarea)) {
				MouseSelected = i;
				MouseOffToWindowX = mouse_x - blockarea.x;
				MouseOffToWindowY = mouse_y - blockarea.y;
				MouseOffToWindowX /= hudScale; MouseOffToWindowX *= hudScale;
				MouseOffToWindowY /= hudScale; MouseOffToWindowY *= hudScale;
				return 1;
			}
		}
	}
	return mouse_l_p ? 0xFF : 0;
}

//Reset
void ResetCameraPosition() {
	camX = 0;
	camY = 0;
}

//Main Process
void MainProcess() {
	mapWidth = lManager.request_level_entry("size_x");
	mapHeight = lManager.request_level_entry("size_y");

	if (keyboardState[SDL_SCANCODE_LCTRL]) {
		if (mouse_w_up) {
			hudScale = min(64, hudScale + 4);
		}
		if (mouse_w_down) {
			hudScale = max(16, hudScale - 4);
		}
	}

	if (!del_press) {
		del_press = keyboardState[SDL_SCANCODE_DELETE] || keyboardState[SDL_SCANCODE_BACKSPACE];
	}

	camX += (keyboardState[SDL_SCANCODE_RIGHT] - keyboardState[SDL_SCANCODE_LEFT]) * (1 + keyboardState[SDL_SCANCODE_LSHIFT]) * 3;
	camY += (keyboardState[SDL_SCANCODE_UP] - keyboardState[SDL_SCANCODE_DOWN])* (1 + keyboardState[SDL_SCANCODE_LSHIFT]) * 3;

	camOffsetX = (camX * hudScale) >> 4;
	camOffsetY = (camY * hudScale) >> 4;

	//If we have to detect interactions
	if(win_focus) {
		MouseResizing = MouseResizing ? mouse_l : false;
		bool mValid = mouse_y <= 32 || mouse_y >= (h - 22);
		int ClickedOn = GetClickedOn(mValid);
		if(ClickedOn) {
			SelectedLevelPart = -1;
			SelectedSpriteObject = -1;
		}
		switch (ClickedOn) {
		case 1:
			cloneLevelInstance(MouseSelected);
			levelLData.erase(levelLData.begin() + MouseSelected);
			SelectedLevelPart = levelLData.size() - 1;
			break;
		case 2: //Window title (start drag)
			SelectedLevelPart = MouseSelected;
			break;
		case 3:
			SelectedSpriteObject = MouseSelected;
			break;
		default:
			break;
		}

		//Sprite Selection
		if (SelectedSpriteObject >= 0) {
			if (SelectedSpriteObject >= levelSData.size()) {
				SelectedSpriteObject = -1;
			}
			else {
				SelectedLevelPart = -1;
				selectedMap16Tile = 0x25;
				if (del_press) {
					levelSData.erase(levelSData.begin() + SelectedSpriteObject);
					SelectedSpriteObject = -1;
				}
				else {
					if (!mValid) {
						//Move/Clone
						if (mouse_r_p) {
							cloneSpriteInstance(SelectedSpriteObject);
							SelectedSpriteObject = levelSData.size() - 1;
							MouseOffToWindowX = hudScale / 2;
							MouseOffToWindowY = hudScale / 2;
						} 
						SpriteObject& part = levelSData[SelectedSpriteObject];
						if (mouse_l || mouse_r) {
							if (SnapSprites) {
								MouseOffToWindowX = 0;
								MouseOffToWindowY = 0;
							}
							part.x = ((mouse_x - MouseOffToWindowX + camOffsetX) * 16) / hudScale;
							part.y = (((h - 22 - mouse_y + MouseOffToWindowY + camOffsetY) * 16) / hudScale) - 16;

							if (SnapSprites) {
								part.y += 16;
								part.x >>= 4; part.x <<= 4;
								part.y >>= 4; part.y <<= 4;
							}
						}
						part.dir += mouse_w_up - mouse_w_down;
					}
				}
			}
		}
		else {
			if (selectedSpriteSpawn != 0x100 && mouse_r_p) {
				MouseOffToWindowX = hudScale / 2;
				MouseOffToWindowY = hudScale / 2;
				levelSData.push_back(SpriteObject{ ((mouse_x - MouseOffToWindowX + camOffsetX) * 16) / hudScale, (((h - 22 - mouse_y + MouseOffToWindowY + camOffsetY) * 16) / hudScale) - 16, 1, uint_fast8_t(selectedSpriteSpawn) });
				SelectedSpriteObject = levelSData.size() - 1;
			}
		}

		//Selection
		if (SelectedLevelPart >= 0) {
			if (SelectedLevelPart >= levelLData.size()) {
				SelectedLevelPart = -1;
			}
			else {
				selectedSpriteSpawn = 0x100;
				//Delete
				if (del_press) {
					levelLData.erase(levelLData.begin() + SelectedLevelPart);
					SelectedLevelPart = -1;
				}
				else {
					if (!mValid) {
						if (MouseResizing) {
							//Resize
							LevelObject& part = levelLData[SelectedLevelPart];
							getPartRecs(SelectedLevelPart);

							//X Axis
							if (MouseDoResizeX) {
								if (MouseResizingX) {
									if (mouse_x > (blockarea.x + blockarea.w)) {
										part.size_x++;
									}
									if (mouse_x <= (blockarea.x + blockarea.w - hudScale)) {
										if (part.size_x > 1) {
											part.size_x--;
										}
									}
								}
								else {
									if (mouse_x < blockarea.x) {
										part.size_x++;
										part.x--;
									}
									if (mouse_x > (blockarea.x + hudScale)) {
										if (part.size_x > 1) {
											part.size_x--;
											part.x++;
										}
									}
								}
							}

							//Y Axis
							if (MouseDoResizeY) {
								if (MouseResizingY) {
									if (mouse_y > (blockarea.y + blockarea.h)) {
										part.size_y++;
									}
									if (mouse_y <= (blockarea.y + blockarea.h - hudScale)) {
										if (part.size_y > 1) {
											part.size_y--;
										}
									}
								}
								else {
									if (mouse_y < blockarea.y) {
										part.size_y++;
										part.y++;
									}
									if (mouse_y > (blockarea.y + hudScale)) {
										if (part.size_y > 1) {
											part.size_y--;
											part.y--;
										}
									}
								}
							}
						}
						else {
							//Move/Clone
							if (mouse_r_p) {
								cloneLevelInstance(SelectedLevelPart);
								SelectedLevelPart = levelLData.size() - 1;
								MouseOffToWindowX = 0;
								MouseOffToWindowY = 0;
							}
							LevelObject& part = levelLData[SelectedLevelPart];
							if (mouse_l || mouse_r) {
								part.x = ((mouse_x - MouseOffToWindowX + camOffsetX) / hudScale);
								part.y = ((h - 22 - mouse_y + MouseOffToWindowY + camOffsetY) / hudScale);
							}
						}
					}
				}
			}
		}
		else {
			if (mouse_r_p && selectedMap16Tile != 0x25) {
				MouseOffToWindowX = 0;
				MouseOffToWindowY = 0;
				levelLData.push_back(LevelObject{ (mouse_x + camOffsetX) / hudScale, (h - 22 - mouse_y + camOffsetY) / hudScale, 1, 1, selectedMap16Tile });
				SelectedLevelPart = levelLData.size() - 1;
			}
		}
	}
	else {
		MouseResizing = false;
		for (int i = 0; i < Windows.size(); i++) {
			WindowInstanceEditor& WND = Windows[i];
			CurrentWindow = &WND;
			if (SDL_GetWindowFlags(WND.mWindow) & SDL_WINDOW_INPUT_FOCUS) {
				if (mouse_l_p) {
					if (WND.MouseInteractCode != nullptr) {
						((void(*)(void)) WND.MouseInteractCode)();
					}
				}
			}
		}
	}
}

//WProcess
bool ProgramEnded()
{
	del_press = false;

	uint_fast32_t m_state = SDL_GetGlobalMouseState(&mouse_x, &mouse_y);
	mouse_l = m_state & SDL_BUTTON(SDL_BUTTON_LEFT);
	mouse_r = m_state & SDL_BUTTON(SDL_BUTTON_RIGHT);
	mouse_w_press = m_state & SDL_BUTTON(SDL_BUTTON_MIDDLE);

	keyboardState = SDL_GetKeyboardState(NULL);
	mouse_w_up = false;
	mouse_w_down = false;

	win_focus = SDL_GetWindowFlags(win) & SDL_WINDOW_INPUT_FOCUS;
	SDL_GetWindowSize(win, &w, &h);
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_WINDOWEVENT) {
			if (event.window.event == SDL_WINDOWEVENT_CLOSE) {
				if (SDL_GetWindowID(win) == event.window.windowID) {
					return true;
				}
				for (int i = 0; i < Windows.size(); i++) {
					if (SDL_GetWindowID(Windows[i].mWindow) == event.window.windowID) {
						Windows[i].Cleanup();
						Windows.erase(Windows.begin() + i);
						break;
					}
				}
			}
			if (event.window.event == SDL_WINDOWEVENT_FOCUS_GAINED) {
				win_focus = SDL_GetWindowID(win) == event.window.windowID;
			}
		}
		if (event.type == SDL_MOUSEWHEEL) {
			mouse_w_up = event.wheel.y > 0;
			mouse_w_down = event.wheel.y < 0;
		}
		//WPARAM
		if (event.type == SDL_SYSWMEVENT) {
			if (event.syswm.msg->msg.win.msg == WM_COMMAND) {
				switch (LOWORD(event.syswm.msg->msg.win.wParam)) {
				case 1:
					LoadFile();
					break;
				case 2:
					SaveFile();
					break;
				case 4:
					return true;
				case 5:
					del_press = true; break;
				default:
					break;
				}
			}
		}
	}
	int w_x = 0;
	int w_y = 0;
	if (win_focus) {
		SDL_GetWindowPosition(win, &w_x, &w_y);
	}
	else {
		for (int i = 0; i < Windows.size(); i++) {
			WindowInstanceEditor& WND = Windows[i];
			if (SDL_GetWindowFlags(WND.mWindow) & SDL_WINDOW_INPUT_FOCUS) {
				SDL_GetWindowPosition(WND.mWindow, &w_x, &w_y);
				break;
			}
		}
	}
	mouse_x -= w_x;
	mouse_y -= w_y;

	//Pressed
	mouse_l_p = false;
	if (mouse_l != mouse_l_d) {
		mouse_l_d = mouse_l;
		if (mouse_l) {
			mouse_l_p = true;
		}
	}
	mouse_r_p = false;
	if (mouse_r != mouse_r_d) {
		mouse_r_d = mouse_r;
		if (mouse_r) {
			mouse_r_p = true;
		}
	}

	//State
	frame_counter++;
	return false;
}

void ChangeSpriteSnap()
{
	SnapSprites = !SnapSprites;
}