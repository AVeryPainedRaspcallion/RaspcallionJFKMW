#pragma once

int CurrentButtonPosition = 0;
void DrawButtonBar(bool isPressed, int icon, void* spwn, void* kill) {
	DestR = { CurrentButtonPosition, 6, 23, 22 }; \
	if (win_focus && mouse_l_p && CheckMouseInBounds(&DestR)) {
		if (isPressed) {
			if (kill != nullptr) {
				((void(*)(void)) kill)();
			}
		}
		else {
			if (spwn != nullptr) {
				((void(*)(void)) spwn)();
			}
		}
	}
	SrcR = { isPressed ? 25 : 2, 106, 23, 22 }; \
	SDL_RenderCopy(ren, UI_TEX, &SrcR, &DestR); \
	DestR = { CurrentButtonPosition + 3 + isPressed, 6 + 3 + isPressed, 16, 16 }; \
	SrcR = { icon * 16, 32, 16, 16 }; \
	SDL_RenderCopy(ren, UI_TEX, &SrcR, &DestR); \
	CurrentButtonPosition += 23;
}


void DrawLevelPart(int p, bool invert = false) {
	if (p >= 0 && p < levelLData.size()) {
		LevelObject& part = levelLData[p];

		SrcR = { int((part.tile & 0xF) << 4) + invert * 256, int((part.tile >> 4) << 4), 16, 16 };
		DestR = { (part.x * hudScale) - camOffsetX, h - 22 - ((part.y + 1) * hudScale) + camOffsetY, hudScale, hudScale };

		if (DestR.x >= (-part.size_x * hudScale) && DestR.y >= (-part.size_y * hudScale) && DestR.x < w && DestR.y < h) {
			for (int x = 0; x < part.size_x; x++) {
				for (int y = 0; y < part.size_y; y++) {
					SDL_RenderCopy(ren, MAP16_TEX, &SrcR, &DestR);
					DestR.y += hudScale;
				}
				DestR.y -= part.size_y * hudScale;
				DestR.x += hudScale;
			}
		}
	}
}

int specmod(int a, int b)
{
	return (a % b + b) % b;
}

void Render() {
	/*
		Render BG
	*/
	int inc = hudScale << 5;
	for (int x = -specmod(camOffsetX >> 1, inc); x < w; x += inc) {
		for (int y = h - inc + specmod(camOffsetY >> 1, inc); y > -inc; y -= inc) {
			DestR = { x, y, inc, inc };
			SDL_RenderCopy(ren, BG_TEX, NULL, &DestR);
		}
	}

	/*
		Render level
	*/
	for (int i = 0; i < levelLData.size(); i++) {
		DrawLevelPart(i);
	}
	if (SelectedLevelPart > -1) {
		DrawLevelPart(SelectedLevelPart, true);
		getPartRecs(SelectedLevelPart);

		SDL_SetRenderDrawColor(ren, 0, 255, 0, (MouseResizing || (CheckMouseInBounds(&blockarea) && !CheckMouseInBounds(&blockareares))) && win_focus ? 192 : 64);
		SDL_RenderDrawRect(ren, &blockarea);
		for (int i = 0; i < 3; i++) {
			blockarea.x++;
			blockarea.y++;
			blockarea.w -= 2;
			blockarea.h -= 2;
			SDL_RenderDrawRect(ren, &blockarea);
		}
	}

	/*
		Render sprites
	*/
	for (int i = 0; i < levelSData.size(); i++) {
		SpriteObject& spr = levelSData[i];
		DestR = { ((spr.x * hudScale) >> 4) - camOffsetX, h - 22 - (((spr.y + 32) * hudScale) >> 4) + camOffsetY, hudScale << 1, hudScale << 1 };
		
		if (DestR.x >= -32 && DestR.y >= -32 && DestR.x < w && DestR.y < h) {
			SrcR = { (i == SelectedSpriteObject ? 512 : 0) + ((spr.num & 0xF) << 5), (spr.num >> 4) << 5, 32, 32 };
			SDL_RenderCopy(ren, SPR_TEX, &SrcR, &DestR);
			if (i == SelectedSpriteObject) {
				DrawTextWindow(DestR.x + 4, DestR.y + DestR.h + 4, "v, " + to_string(int(spr.dir)), false, 255, true);
			}
		}
	}

	/*
		Render Level Entrance
	*/
	DestR = { int(lManager.request_level_entry("start_x") * hudScale) - camOffsetX, h - 22 - int((lManager.request_level_entry("start_y") + 1) * hudScale) + camOffsetY, hudScale, hudScale };
	SDL_SetRenderDrawColor(ren, 255, 255, 0, 192);
	SDL_RenderDrawRect(ren, &DestR);

	/*
		Level Border
	*/
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 230);

	DestR = {0, min(0, camOffsetY - 22 - mapHeight * hudScale), w, h};
	SDL_RenderFillRect(ren, &DestR);


	int sy = max(0, h - abs(min(0, camOffsetY - 22)));
	DestR.h = min(mapHeight * hudScale, sy);
	DestR.y = max(0, DestR.y + h);
	DestR.x = max(0, -camOffsetX + mapWidth * hudScale);
	SDL_RenderFillRect(ren, &DestR);

	DestR.x = 0;
	DestR.w = abs(min(0, camOffsetX));
	SDL_RenderFillRect(ren, &DestR);

	DestR.y = max(0, h + min(0, camOffsetY - 22));
	DestR.h = h;
	DestR.w = w;
	SDL_RenderFillRect(ren, &DestR);

	/*
		Render program top bar with tools and buttons
	*/
	DestR = { 0, 0, w, 32 };
	SrcR = { 0, 96, 2, 32 };
	SDL_RenderCopy(ren, UI_TEX, &SrcR, &DestR);

	/*
		Button bars!
	*/

	CurrentButtonPosition = 8;
	DrawButtonBar(false, 0, &LoadFile, nullptr);
	DrawButtonBar(false, 1, &SaveFile, nullptr);
	DrawButtonBar(false, 8, &CreateNewLevel, nullptr);
	CurrentButtonPosition += 8;
	DrawButtonBar(CheckMap16WindowStatus(), 2, &SpawnMap16Window, &KillMap16Window);
	DrawButtonBar(CheckSpriteWindowState(), 4, &SpawnSpriteSpawner, &KillSpriteWindow);
	DrawButtonBar(CheckConfigWindowState(), 9, &SpawnConfigWindow, &KillConfigWindow);
	CurrentButtonPosition += 8;
	DrawButtonBar(SnapSprites, 3, &ChangeSpriteSnap, &ChangeSpriteSnap);
	DrawButtonBar(false, 6, &ResetCameraPosition, nullptr);
	CurrentButtonPosition += 8;
	DrawButtonBar(false, 7, &PlayTest, nullptr);

	/*
		Bottom status bar
	*/
	DestR = { 0, h-22, w, 22 };
	SrcR = { 48, 106, 2, 22 };
	SDL_RenderCopy(ren, UI_TEX, &SrcR, &DestR);

	DrawBox(ren, 0, h - 20, w / 4, 20);
	DrawBox(ren, (w / 4) + 2, h - 20, w - (w / 4) - 2, 20);

	/*
		Render most important UI aspect, the windows that allow us to edit shit
	*/
	for (int i = 0; i < Windows.size(); i++) {
		WindowInstanceEditor& WND = Windows[i];
		CurrentWindow = &WND;

		SDL_SetRenderDrawColor(WND.mRenderer, 240, 240, 240, 255);
		SDL_RenderClear(WND.mRenderer);

		//Yeah
		if (WND.RenderCode != nullptr) {
			((void(*)(void)) WND.RenderCode)();
		}

		SDL_RenderPresent(WND.mRenderer);
	}
}