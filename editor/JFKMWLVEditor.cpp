#if defined(_WIN32)
#pragma comment (lib, "comctl32")
#include <Windows.h>
#include "CommCtrl.h"
#include "psapi.h"
#endif
#include <cstdint>
#include <cmath>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <fstream>
#include <iomanip>
#include <algorithm>
#include <unordered_map>
#include <ctime>
#include <cstdio>
#include <stdio.h>
#include <cstdlib>
#include <ratio>
#include <chrono>
#include <ctype.h>
#include <cstring>
#include <cassert>
#include <locale>
#include <codecvt>
#include <filesystem>
#include "InputBox.h"


#include <SDL.h>
#include <SDL_syswm.h>
#include <SDL_image.h>
#undef main

//Namespaces
using namespace std;
namespace fs = std::filesystem;

//Global library for wrapping SDL Functions easily (we call it dablib). we do a little dabbing
#include "DabLib.h"

//Global
#include "GlobalVariables.h"
#include "LevelData.h"

//Render
#include "WindowInstance.h"
#include "RendererFunctions.h"

//Tools
#include "Map16View.h"
#include "SpriteSpawner.h"
#include "ConfigMenu.h"

//Logic
#include "Logic.h"
#include "Renderer.h"

//main shit
int main()
{
    SDL_SetHint("SDL_HINT_MOUSE_FOCUS_CLICKTHROUGH", "1");

    //Get directory (path), do not remove or loading will break
    GetExecutableDirectory();
    screen(1280, 720);

    //Prepare
    InitializeUI();
    InitInstalledSprites();

    //WIN LOOP
    while (!ProgramEnded()) {
        //do logic stuff here
        MainProcess();

        //clear screen
        cls();

        //render stuff here
        Render();

        //send buffer to screen
        redraw();
    }
    //END
    return 1;
}