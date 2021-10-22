#pragma once

#define SPRITE_WINDOW_ID 1

//Scroll
int SpriteWindowScroll = 0;

//Process Map16 View
void ProcessSpriteSpawner() {
	CurrentWindow->sx = 408;
	CurrentWindow->sy = 256;
	CurrentWindow->name = "Sprite Select";
	CurrentWindow->hint = "Click on any sprite from the list here, then right click on the level to add it.";
	SpriteWindowScroll = 0;
}

//Map16 Viewer Interact
void InteractSpriteSpawner() {
	selectedSpriteSpawn = 0;
	for (int i = 0; i < installedSprites.size(); i++) {
		int by = 4 - SpriteWindowScroll + i * 14;
		int be = 4 - SpriteWindowScroll + (i + 1) * 14;
		if (mouse_y >= by && mouse_y < be) {
			selectedSpriteSpawn = installedSprites[i].num;
		}
	}
	selectedMap16Tile = 0x25;
	SelectedLevelPart = -1;
	SelectedSpriteObject = -1;
}

//Map16 viewer window
void RenderSpriteSpawner() {
	if (mouse_w_up) {
		SpriteWindowScroll = max(0, SpriteWindowScroll - 14);
	}
	if (mouse_w_down) {
		SpriteWindowScroll += 14;
	}
	for (int i = 0; i < installedSprites.size(); i++) {
		int by = 4 - SpriteWindowScroll + i * 14;
		if (by > -14 && by < 256) {
			DrawBox(CurrentWindow->mRenderer, 4, by, 400, 14);
			if (selectedSpriteSpawn == installedSprites[i].num) {
				DrawTextWindow(354, by + 2, "Selected");
			}
			DrawTextWindow(5, by + 2, int_to_hex(installedSprites[i].num, true) + "   " + installedSprites[i].name);
		}
	}
}

//Map16 Spawn
void SpawnSpriteSpawner() {
	SpawnWindow(SPRITE_WINDOW_ID, RenderSpriteSpawner, ProcessSpriteSpawner, InteractSpriteSpawner);
}

bool CheckSpriteWindowState() {
	for (int i = 0; i < Windows.size(); i++) {
		if (Windows[i].ID == SPRITE_WINDOW_ID) {
			return true;
		}
	}
	return false;
}

void KillSpriteWindow() {
	KillWindow(SPRITE_WINDOW_ID);
}