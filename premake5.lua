workspace "JFKMarioWorld"
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
		links { "lua51", "SDL2", "SDL2_mixer", "SDL2_image", "opengl32", "winmm", "ws2_32" }
		defines { "_WIN32", "SFML_STATIC" }
		
	filter "platforms:Linux*" 
		system "linux"
		links { "dl", "luajit", "SDL2", "SDL2_mixer", "SDL2_image", "GL", "GLU", "sfml-system", "sfml-network" }
		linkgroups "On" -- if you don't use this then the linker breaks (blame gcc for being shit)
		defines { "__linux__" }
		
	filter "platforms:Mac*" 
		system "macosx"
		links { "OpenGL.framework", "luajit", "SDL2.framework", "SDL2_mixer.framework", "SDL2_image.framework", "sfml-system.framework", "sfml-network.framework" }
		linkgroups "On" -- if you don't use this then the linker breaks (blame clang? for being shit)
		defines { "__linux__", "__mac__" }
		
project "JFKMarioWorld"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	targetdir "bin/%{cfg.buildcfg}"
	includedirs { "libs", "libs/SDL/include", "libs/SDL_mixer", "libs/SDL_image", "libs/SFML/include", "libs/moonjit/src", "libs/lz4/lib" }
	libdirs { "libs" }

	files { "src/**.hpp", "src/**.h", "src/**.cpp", "src/**.c", "libs/lz4/lib/lz4.c" }
