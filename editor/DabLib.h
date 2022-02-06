#pragma once

//it's shitcode, but it's way faster than using a stringstream for sure
string int2hexcache[16] = { "0","1","2","3","4","5","6","7","8","9","A","B","C","D","E","F" };
string int_to_hex(int T, bool add_0 = false) {
	string STR = "";
	if (T < 16 && add_0) {
		STR = "0" + int2hexcache[T & 0xF];
	}
	else {
		if (T <= 0) {
			return "0";
		}
		while (T > 0) {
			STR = int2hexcache[T & 0xF] + STR; T >>= 4;
		}
	}
	return STR;
}

//Path. For testing, i made this directly go to my folder path on debug builds
string path = "";
#if defined(_WIN32)
void GetExecutableDirectory()
{
	char buffer[MAX_PATH];
	GetModuleFileNameA(NULL, buffer, MAX_PATH);
	std::string::size_type pos = std::string(buffer).find_last_of("\\/");

	path = std::string(buffer).substr(0, pos) + "\\";
	replace(path.begin(), path.end(), '\\', '/'); // replace all 'x' to 'y'
}
#else
void GetExecutableDirectory()
{
	path = "";
}
#endif

vector<string> split(const string& s, char delim) {
	vector<string> result;
	stringstream ss(s);
	string item;

	while (getline(ss, item, delim)) {
		result.push_back(item);
	}

	return result;
}
//Video/SDL Related stuff. These will be configurable later. So keep this in mind
SDL_Window* win;
SDL_Renderer* ren;
SDL_Event event = { 0 };

//internal window size, set later by the program!
int w, h;

//You can modify these
bool v_sync = true;
bool renderer_accelerated = true;
int rendering_device = 0;
bool win_focus = false;
bool quit = false;

//FPS Calculations
chrono::duration<double> total_time_ticks;

//Rendering
SDL_Rect SrcR;
SDL_Rect DestR;

//Input
const Uint8* keyboardState = SDL_GetKeyboardState(NULL);
bool mouse_w_up = false;
bool mouse_w_down = false;
bool mouse_w_press = false;
bool mouse_l = false;
bool mouse_r = false;
bool mouse_l_p = false;
bool mouse_r_p = false;
bool mouse_l_d = false;
bool mouse_r_d = false;
bool del_press = false;
int mouse_x = 0;
int mouse_y = 0;

//Some inlines for console text colors.
#if defined(_WIN32)
inline ostream& blue(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
		| FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& red(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& green(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& yellow(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& white(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return s;
}

inline ostream& cyan(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_BLUE);
	return s;
}

inline ostream& purple(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_BLUE);
	return s;
}

inline ostream& purple_int(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& lua_color(ostream& s)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, 9);
	return s;
}

struct color {
	color(WORD attribute) :m_color(attribute) {};
	WORD m_color;
};

template <class _Elem, class _Traits>
basic_ostream<_Elem, _Traits>&
operator<<(basic_ostream<_Elem, _Traits>& i, color& c)
{
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, c.m_color);
	return i;
}
#else
string blue = "";
string red = "";
string green = "";
string yellow = "";
string white = "";
string cyan = "";
string purple = "";
string purple_int = "";
string lua_color = "";
#endif

//Win32
HWND sdl_window;

//Namespace variables/Defines
static HMENU hFile;
static HMENU hEdit;
static HMENU hMenuBar;


//Function which retrieves the address/Handle of an SDL window
//Also retrieves the specific subsystem used by SDL to create that window which is platform specific (Windows, MAC OS x, IOS, etc...)
void getSDLWinHandle()
{
	SDL_SysWMinfo wmInfo;
	SDL_VERSION(&wmInfo.version);
	SDL_GetWindowWMInfo(win, &wmInfo);

	sdl_window = wmInfo.info.win.window;
}

void ActivateMenu()
{
	hMenuBar = CreateMenu();
	hFile = CreateMenu();
	hEdit = CreateMenu();


	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hFile, L"File");
	AppendMenu(hFile, MF_STRING, 1, L"Open Level");
	AppendMenu(hFile, MF_STRING, 2, L"Save Level to Folder");
	AppendMenu(hFile, MF_SEPARATOR, 3, L"");
	AppendMenu(hFile, MF_STRING, 4, L"Exit");

	AppendMenu(hMenuBar, MF_POPUP, (UINT_PTR)hEdit, L"Edit");
	AppendMenu(hEdit, MF_STRING, 5, L"Delete");

	SetMenu(sdl_window, hMenuBar);
}

//Logic Related
int frame_counter = 0;

void screen(int width, int height)
{
	w = width;
	h = height;

	//Delete old window if it exists
	cout << cyan << "[SDL] Creating window" << endl;
	if (win) { SDL_DestroyWindow(win); }

	//Create window
	win = SDL_CreateWindow("Solar Energy: Beyond Limits", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);
	if (win == NULL) { cout << cyan << "[SDL] Window error: " << SDL_GetError() << white << endl; SDL_Quit(); exit(1); }

	//Create renderer
	cout << cyan << "[SDL] Window created, initializing renderer..." << endl;
	ren = SDL_CreateRenderer(win, rendering_device, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	if (ren == NULL) { cout << cyan << "[SDL] Renderer error: " << SDL_GetError() << white << endl; SDL_Quit(); exit(1); }

	//Initialize Window
	cout << cyan << "[SDL] Initialized window of " << dec << width << "x" << height << " resolution" << white << endl;
	cout << cyan << "[SDL] Current video card : " << SDL_GetCurrentVideoDriver() << white << endl;

	//Render Draw Blend Mode
	SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

	//Event state, activate menus.
	SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);
	getSDLWinHandle();
	ActivateMenu();
}

void cls()
{
	SDL_SetRenderDrawColor(ren, 0, 0, 0, 255);
	SDL_RenderClear(ren);
}

void redraw()
{
	SDL_RenderPresent(ren);
}

bool CheckMouseInBounds(SDL_Rect* Rect) {
	if (Rect == nullptr) {
		return false;
	}
	return mouse_x >= Rect->x && mouse_y >= Rect->y && mouse_x <= (Rect->x + Rect->w) && mouse_y <= (Rect->y + Rect->h);
}

//Safe 
int safe_stoi(std::string str, int base = 10) {
	try { return stoi(str, nullptr, base); }
	catch (...) { return 0; }
}

// wrapping std::stoi because it may throw an exception
int safe_stringint(std::string str, int* p_value, int base = 10) {
	try {
		*p_value = std::stoi(str, nullptr, base);
		return 0;
	}

	catch (const std::invalid_argument& ia) {
		cout << red << "[STRING2INT] Invalid argument: " << ia.what() << white << endl;
		return -1;
	}

	catch (const std::out_of_range& oor) {
		cout << red << "[STRING2INT] Out of Range error: " << oor.what() << white << endl;
		return -2;
	}

	catch (const std::exception& e)
	{
		cout << red << "[STRING2INT] Undefined error: " << e.what() << white << endl;
		return -3;
	}
}

void replaceAll(string& str, const string& from, const string& to) {
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != string::npos) {
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
	}
}
