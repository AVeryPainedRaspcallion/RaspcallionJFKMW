#pragma once

class WindowInstanceEditor {
public:
	int sx = 0;
	int sy = 0;

	void* RenderCode;
	void* ProcessCode;
	void* MouseInteractCode;

	bool canBeResized = false;

	int ID = 0;

	string name = "Test Window Instance";
	string hint = "";

	SDL_Window* mWindow;
	SDL_Renderer* mRenderer;
	HWND hwnd;
	HWND statb;

	//Cache
	unordered_map<SDL_Surface*, SDL_Texture*> mTextureCache;

	//Cleanup
	void Cleanup() {
		unordered_map<SDL_Surface*, SDL_Texture*>::iterator it;
		for (it = mTextureCache.begin(); it != mTextureCache.end(); it++) {
			if (it->second != NULL) {
				SDL_DestroyTexture(it->second);
			}
		}
		SDL_DestroyWindow(mWindow);
		SDL_DestroyRenderer(mRenderer);
	}

	//Transfer from ren
	SDL_Texture* RequestTextureTransfer(SDL_Surface* trans) {

		auto entry = mTextureCache.find(trans);
		if (entry != mTextureCache.end()) {
			return entry->second;
		}

		//Return
		SDL_Texture* newTex = SDL_CreateTextureFromSurface(mRenderer, trans);
		mTextureCache.insert(make_pair(trans, newTex));
		return newTex;
	}

	//Yeah
	void InitS() {
		//Set style to just close button
		DWORD style = GetWindowLong(hwnd, GWL_STYLE);
		style &= ~WS_MINIMIZEBOX;
		SetWindowLong(hwnd, GWL_STYLE, style);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}
};
vector<WindowInstanceEditor> Windows;
WindowInstanceEditor* CurrentWindow;

void SpawnWindow(int ID, void* RenderC, void* ProcC, void* InteC) {
	for (int i = 0; i < Windows.size(); i++) {
		//Officer, I can't spawn the same window!
		if (Windows[i].ID == ID) {
			return;
		}
	}
	WindowInstanceEditor New_WND;
	New_WND.ID = ID;
	New_WND.RenderCode = RenderC;
	New_WND.ProcessCode = ProcC;

	//Run proc once
	CurrentWindow = &New_WND;
	if (New_WND.ProcessCode != nullptr) {
		((void(*)(void)) New_WND.ProcessCode)();
	}

	New_WND.MouseInteractCode = InteC;

	//Prepare window and renderer
	int flags1 = New_WND.canBeResized ? SDL_WINDOW_RESIZABLE : 0;
	New_WND.mWindow = SDL_CreateWindow(New_WND.name.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, New_WND.sx, New_WND.sy, SDL_WINDOW_SHOWN | SDL_WINDOW_SKIP_TASKBAR | flags1);
	New_WND.mRenderer = SDL_CreateRenderer(New_WND.mWindow, rendering_device, SDL_RENDERER_ACCELERATED);
	SDL_SetRenderDrawBlendMode(New_WND.mRenderer, SDL_BLENDMODE_BLEND);

	//Add to list
	Windows.push_back(New_WND);

	//Get Window Handle
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(New_WND.mWindow, &wmInfo);
	New_WND.hwnd = wmInfo.info.win.window;
	New_WND.InitS();

	if (New_WND.hint != "") {
		New_WND.statb = CreateStatusWindowA(WS_CHILD | WS_VISIBLE, New_WND.hint.c_str(), New_WND.hwnd, 9000);
	}
}


void KillWindow(int ID) {
	for (int i = 0; i < Windows.size(); i++) {
		if (Windows[i].ID == ID) {
			Windows[i].Cleanup();
			Windows.erase(Windows.begin() + i);
			return;
		}
	}
}