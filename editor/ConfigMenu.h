#pragma once

#define CONFIG_WINDOW_ID 2

//Process Map16 View
void ProcessConfigWindow() {
	CurrentWindow->sx = 408;
	CurrentWindow->sy = 256;
	CurrentWindow->name = "Level Config Editor";
	CurrentWindow->hint = "Click on parameters to edit them.";
	SpriteWindowScroll = 0;
}

//Map16 Viewer Interact
void InteractConfigWindow() {
	unordered_map<string, uint_fast32_t>::iterator it;
	int i = 0;
	int x = 0;
	for (it = lManager.config.begin(); it != lManager.config.end(); it++)
	{
		DestR = { x + 70, 4 + i * 16, 40, 14 };
		if (CheckMouseInBounds(&DestR)) {
			bool nHex = it->first == "background" || it->first == "music";
			uint_fast32_t NewValue = GetValueInputBox("Input new value for " + it->first, nHex ? 16 : 10);
			it->second = NewValue;
			lManager.ReloadAssets();
			InitializeMap16();
		}
		i++;
		if (i == 14) {
			i = 0; x += 75 + 50;
		}
	}
}

//Map16 viewer window
void RenderConfigWindow() {
	unordered_map<string, uint_fast32_t>::iterator it;
	int i = 0;
	int x = 0;
	for (it = lManager.config.begin(); it != lManager.config.end(); it++)
	{
		DrawTextWindow(x + 5, 4 + i * 16, it->first + " = ");
		DrawBox(CurrentWindow->mRenderer, x + 70, 4 + i * 16, 40, 14);

		bool nHex = it->first == "background" || it->first == "music";
		DrawTextWindow(x + 75, 4 + i * 16, nHex ? int_to_hex(it->second) : to_string(it->second));

		i++;
		if (i == 14) {
			i = 0; x += 75 + 50;
		}
	}
}

//Map16 Spawn
void SpawnConfigWindow() {
	SpawnWindow(CONFIG_WINDOW_ID, RenderConfigWindow, ProcessConfigWindow, InteractConfigWindow);
}

bool CheckConfigWindowState() {
	for (int i = 0; i < Windows.size(); i++) {
		if (Windows[i].ID == CONFIG_WINDOW_ID) {
			return true;
		}
	}
	return false;
}

void KillConfigWindow() {
	KillWindow(CONFIG_WINDOW_ID);
}