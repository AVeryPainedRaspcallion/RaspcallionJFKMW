#pragma once

#define MAP16_WINDOW_ID 0

//Scroll
int Map16_Scroll = 0;

//Process Map16 View
void ProcessMap16View() {
	CurrentWindow->sx = 256+4;
	CurrentWindow->sy = 256+4;
	CurrentWindow->canBeResized = true;
	CurrentWindow->name = "16x16 Tile Selector";
	CurrentWindow->hint = "";
}

//Map16 Viewer Interact
void InteractMap16View() {
	if (mouse_x >= 2 && mouse_x <= 258 && mouse_y >= 2 && mouse_y <= (CurrentWindow->sy-2)) {
		int T1 = ((mouse_x - 2) >> 4) & 0xF;
		int T2 = ((mouse_y - 2 + Map16_Scroll) >> 4) << 4;
		selectedMap16Tile = T1 + T2;
		selectedSpriteSpawn = 0x100;
		SelectedLevelPart = -1;
		SelectedSpriteObject = -1;
	}
}

//Map16 viewer window
void RenderMap16View() {
	SDL_GetWindowSize(CurrentWindow->mWindow, &CurrentWindow->sx, &CurrentWindow->sy);

	if (mouse_w_up) { Map16_Scroll -= 16; }
	if (mouse_w_down) { Map16_Scroll += 16; }
	Map16_Scroll = min(1024 - CurrentWindow->sy - 4, max(0, Map16_Scroll));

	SrcR = { 254, 0, 2, 128 };
	DestR = { 0, 0, 260, CurrentWindow->sy };
	SDL_RenderCopy(CurrentWindow->mRenderer, CurrentWindow->RequestTextureTransfer(UI_SURF), &SrcR, &DestR);

	DrawFrameBox(CurrentWindow->mRenderer, 0, 0, 260, CurrentWindow->sy);

	SrcR = { 0, Map16_Scroll, 256, CurrentWindow->sy-4 };
	DestR = { 2, 2, 256, CurrentWindow->sy-4 };
	SDL_RenderCopy(CurrentWindow->mRenderer, CurrentWindow->RequestTextureTransfer(MAP16_SURF), &SrcR, &DestR);

	//Selected Tile
	if (selectedMap16Tile != 0x25) {
		int x = (selectedMap16Tile & 0xF) << 4;
		int y = (selectedMap16Tile >> 4) << 4;
		SrcR = { 256 + x, y, 16, 16 };
		DestR = { -254 + SrcR.x, SrcR.y + 2 - Map16_Scroll, 16, 16 };
		SDL_RenderCopy(CurrentWindow->mRenderer, CurrentWindow->RequestTextureTransfer(MAP16_SURF), &SrcR, &DestR);
	}
}

//Map16 Spawn
void SpawnMap16Window() {
	SpawnWindow(MAP16_WINDOW_ID, RenderMap16View, ProcessMap16View, InteractMap16View);
}

bool CheckMap16WindowStatus() {
	for (int i = 0; i < Windows.size(); i++) {
		if (Windows[i].ID == MAP16_WINDOW_ID) {
			return true;
		}
	}
	return false;
}

void KillMap16Window() {
	KillWindow(MAP16_WINDOW_ID);
}