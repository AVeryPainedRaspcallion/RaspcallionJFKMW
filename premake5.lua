workspace "JFKMW"
    configurations { "Debug", "Release" }
	
	filter "action:vs*"
		platforms { "Win64" }
		prelinkcommands { 'call "$(VSInstallDir)VC\\Auxiliary\\Build\\vcvars64.bat" && cd "$(ProjectDir)libs\\moonjit\\src" && msvcbuild static && copy /y "$(ProjectDir)libs\\moonjit\\src\\lua51.lib" "$(ProjectDir)libs"' }
		
	filter "action:gmake*"
		platforms { "Linux64", "Mac64" }
		prelinkcommands { 'cd libs/moonjit/src; make XCFLAGS+=-DLUAJIT_ENABLE_LUA52COMPAT BUILDMODE=static; mv libluajit.a ../..' }
				
	filter "platforms:*64"
		architecture "x86_64"
		
	filter "Debug"
		defines { "DEBUG" }
		symbols "On"
	
	filter { "configurations:Debug", "system:windows" }
		links { "sfml-system-s-d", "sfml-network-s-d" }

	filter "Release"
		defines { "NDEBUG" }
		optimize "Speed"
		
	filter { "configurations:Release", "system:windows" }
		links { "sfml-system-s", "sfml-network-s" }
	
	filter "platforms:Win*" 
		system "windows"
		links { "lua51", "SDL2", "SDL2_mixer", "SDL2_image", "glew32", "glu32", "opengl32", "winmm", "ws2_32" }
		defines { "_WIN32", "SFML_STATIC" }
		
	filter "platforms:Linux*" 
		system "linux"
		links { "dl", "luajit", "SDL2", "SDL2_mixer", "SDL2_image", "GL", "GLU", "GLEW", "sfml-system", "sfml-network" }
		linkgroups "On" -- if you don't use this then the linker breaks (blame gcc for being shit)
		defines { "__linux__" }
		
project "JFKMW"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"
	includedirs { "libs", "libs/SDL/include", "libs/SDL_mixer", "libs/SDL_image", "libs/glew-2.1.0/include", "libs/SFML/include", "libs/moonjit/src", "libs/lz4/lib" }
	libdirs { "libs" }

	files { "src/**.hpp", "src/**.h", "src/**.cpp", "src/**.c", "libs/lz4/lib/lz4.c" }

project "JFKMWLVEditor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"
	includedirs { "libs", "libs/SDL/include", "libs/SDL_image" }
	libdirs { "libs" }

	files { "editor/**.hpp", "editor/**.h", "editor/**.cpp", "editor/**.c" }