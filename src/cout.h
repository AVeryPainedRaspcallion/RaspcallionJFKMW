#pragma once

#ifdef _WIN32
inline ostream& blue(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, FOREGROUND_BLUE
		| FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& red(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& green(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& yellow(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& white(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
	return s;
}

inline ostream& cyan(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_GREEN | FOREGROUND_BLUE);
	return s;
}

inline ostream& purple(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_BLUE);
	return s;
}

inline ostream& purple_int(ostream &s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout,
		FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	return s;
}

inline ostream& lua_color(ostream& s) {
	HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hStdout, 9);
	return s;
}

#define endl white << "\n"  //replacing "endl"
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