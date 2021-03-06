#pragma once
//TO-DO: Cleanup. Lua integration for JFKMW
bool lua_loaded = false;
lua_State* LUA_STATE;

void lua_print(string text) { cout << lua_color << "[Lua] " << text << endl; }
void addScore(uint_fast32_t Amount) { writeToRam(0x3F13, getRamValue(0x3F13, 3) + Amount, 3); }

//Lua functions to bind.
extern "C" {
	static int lua_write(lua_State* L) {
		lua_print((string)lua_tostring(L, 1)); return 0;
	}

	static int lua_write_ram(lua_State* L) {
		writeToRam((uint_fast32_t)lua_tointeger(L, 2), (uint_fast32_t)lua_tonumber(L, 1), (uint_fast8_t)lua_tointeger(L, 3)); return 0;
	}

	static int lua_addscore(lua_State* L) {
		addScore((uint_fast32_t)lua_tointeger(L, 1)); return 0;
	}

	static int lua_get_ram(lua_State* L) {
		lua_pushnumber(L, getRamValue((uint_fast32_t)lua_tointeger(L, 1), (uint_fast8_t)lua_tointeger(L, 2))); return 1;
	}

	static int lua_get_username(lua_State * L) {
		int p = ((int)lua_tointeger(L, 1))-1;
		if (p < 0 || p >= username_storage.size()) { lua_pushstring(L, ""); }
		else { lua_pushstring(L, username_storage[p].c_str()); }
		return 1;
	}

	static int lua_spawn_sprite(lua_State* L) {
		lua_pushnumber(L, spawnSpriteObj(
			(uint_fast8_t)lua_tointeger(L, 1), //num
			(uint_fast8_t)lua_tointeger(L, 2), //st
			(uint_fast16_t)lua_tonumber(L, 3), //x
			(uint_fast16_t)lua_tonumber(L, 4), //y
			(uint_fast8_t)lua_tointeger(L, 5) //dir
		));
		return 1;
	}

	/*
		START
		TO-DO: Deprecate (and remove eventually)
	*/
	uint_fast16_t convertOldProps(uint_fast16_t props) {
		return (props & 0xF) +
			((props & 0x10) << 2) +
			((props & 0x60) >> 1) +
			((props & 0x780) << 1);
	}

	static int createParticleHook(lua_State* L) {
		uint_fast8_t tile = (uint_fast8_t)lua_tointeger(L, 1);
		uint_fast8_t size = (uint_fast8_t)lua_tointeger(L, 2);
		uint_fast8_t prop = (uint_fast8_t)lua_tointeger(L, 3);
		uint_fast8_t anim_type = (uint_fast8_t)lua_tointeger(L, 4);
		double x = (double)lua_tonumber(L, 5);
		double y = (double)lua_tonumber(L, 6);
		double sx = (double)lua_tonumber(L, 7);
		double sy = (double)lua_tonumber(L, 8);
		double grav = (double)lua_tonumber(L, 9);
		int start_time = (int)lua_tointeger(L, 10);
		int delete_time = (int)lua_tointeger(L, 11);
		createParticle(tile, size, convertOldProps(prop), anim_type, x, y, sx, sy, grav, start_time, delete_time);
		return 0;
	}
	static int slideDeathHandler(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
		uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 2);
		uint_fast8_t flags = (uint_fast8_t)lua_tonumber(L, 3);
		int_fast16_t offset_x = (int_fast16_t)lua_tonumber(L, 4) + RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256;
		int_fast16_t offset_y = (int_fast16_t)lua_tonumber(L, 5) + RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256;
		double sy = Calculate_Speed(512);
		double sx = -int_fast8_t(RAM[0x2680 + sprite_index]) / (1.0 + double(RAM[0x85]) * 0.25);
		createParticle(tile, 0x11, convertOldProps(flags), 0xFF, offset_x, offset_y, sx, sy, Calculate_Speed(48 / (1 + RAM[0x85])), 0, 0, RAM[0x85] ? -1 : -4);
		createParticle(0x60, 0x11, convertOldProps(0x88), 5, offset_x, offset_y, 0, 0, 0);
		return 0;
	}
	static int spriteDeathParticle(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
		uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 2);
		uint_fast8_t flags = (uint_fast8_t)lua_tonumber(L, 3);
		uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 4);
		int_fast16_t offset_x = (int_fast16_t)lua_tonumber(L, 5) + RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256;
		int_fast16_t offset_y = (int_fast16_t)lua_tonumber(L, 6) + RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256;
		createParticle(tile, size, convertOldProps(flags), 0xFF, offset_x, offset_y, 0, 0, Calculate_Speed(48), 0, 0, -4);
		return 0;
	}
	static int draw_to_oam(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
		if (!(RAM[0x2A80 + sprite_index] & 2)) { return 0; }
		uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 2);
		uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 3);
		uint_fast8_t angle = (uint_fast8_t)lua_tonumber(L, 4);
		int_fast16_t offset_x = ((int_fast16_t)lua_tonumber(L, 5)) + RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256;
		int_fast16_t offset_y = ((int_fast16_t)lua_tonumber(L, 6)) + RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256;
		uint_fast16_t pal = (uint_fast16_t)lua_tonumber(L, 7);
		Create_OAMTile(tile, size, offset_x, offset_y, convertOldProps(pal), angle);
		return 0;
	}
	static int draw_to_oam_direct(lua_State* L) {
		uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 1);
		uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 2);
		uint_fast8_t angle = (uint_fast8_t)lua_tonumber(L, 3);
		int_fast16_t sprite_x_position = (int_fast16_t)lua_tonumber(L, 4);
		int_fast16_t sprite_y_position = (int_fast16_t)lua_tonumber(L, 5);
		uint_fast16_t pal = (uint_fast16_t)lua_tonumber(L, 6);
		Create_OAMTile(tile, size, sprite_x_position, sprite_y_position, convertOldProps(pal), angle);
		return 0;
	}
	/*
		END
		TO-DO: Deprecate (and remove eventually)
	*/
	static int pushOAM(lua_State* L) {
		int_fast16_t xpos = (int_fast16_t)lua_tonumber(L, 2);
		int_fast16_t ypos = (int_fast16_t)lua_tonumber(L, 3);
		if (!lua_isnil(L, 1)) {
			uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
			xpos += RAM[0x2100 + sprite_index] + int_fast8_t(RAM[0x2180 + sprite_index]) * 256;
			ypos += RAM[0x2280 + sprite_index] + int_fast8_t(RAM[0x2300 + sprite_index]) * 256;
		}
		uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 4);
		uint_fast8_t size = (uint_fast8_t)lua_tonumber(L, 5);
		uint_fast16_t props = (uint_fast16_t)lua_tonumber(L, 6);
		uint_fast8_t rot = lua_isnil(L, 7) ? 0 : (uint_fast8_t)lua_tonumber(L, 7);
		uint_fast8_t sx = lua_isnil(L, 8) ? 0x20 : (uint_fast8_t)lua_tonumber(L, 8);
		uint_fast8_t sy = lua_isnil(L, 9) ? 0x20 : (uint_fast8_t)lua_tonumber(L, 9);
		Create_OAMTile(tile, size, xpos, ypos, props, rot, sx, sy);
		return 0;
	}
	static int pushParticle(lua_State* L) {
		createParticle(
			(uint_fast8_t)lua_tointeger(L, 1), //Tile
			(uint_fast8_t)lua_tointeger(L, 2), //Size
			(uint_fast16_t)lua_tointeger(L, 3), //Props
			(uint_fast8_t)lua_tointeger(L, 4), //Anim Type
			(double)lua_tonumber(L, 5), //X
			(double)lua_tonumber(L, 6), //Y
			(double)lua_tonumber(L, 7), //SX
			(double)lua_tonumber(L, 8), //SY
			(double)lua_tonumber(L, 9), //GRAV
			lua_isnil(L, 10) ? 0 : (int)lua_tointeger(L, 10), //START_T
			lua_isnil(L, 11) ? 0 : (int)lua_tointeger(L, 11) //DELETE_T
		);
		return 0;
	}
	static int setSpriteX(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tointeger(L, 1); double newPosX = (double)lua_tonumber(L, 2); 
		RAM[0x2100 + sprite_index] = uint_fast8_t(newPosX);
		RAM[0x2180 + sprite_index] = uint_fast8_t(newPosX / 256.0) - (newPosX < 0);
		RAM[0x2200 + sprite_index] = uint_fast8_t(newPosX * 256.0);
		return 0;
	}
	static int setSpriteY(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tointeger(L, 1); double newPosY = (double)lua_tonumber(L, 2) + 16;
		RAM[0x2280 + sprite_index] = uint_fast8_t(newPosY);
		RAM[0x2300 + sprite_index] = uint_fast8_t(newPosY / 256.0) - (newPosY < 0);
		RAM[0x2380 + sprite_index] = uint_fast8_t(newPosY * 256.0);
		return 0;
	}
	static int drawtohud(lua_State* L) {
		uint_fast8_t tile = (uint_fast8_t)lua_tonumber(L, 1);
		uint_fast8_t prop = (uint_fast8_t)lua_tonumber(L, 2) & 0x0F;
		uint_fast8_t x = (uint_fast8_t)lua_tonumber(L, 3) & 0x1F;
		uint_fast8_t y = (uint_fast8_t)lua_tonumber(L, 4) & 0x1F;
		RAM[VRAM_Convert(0xB800) + ((x % 32) + (y * 32))*2] = tile;
		RAM[VRAM_Convert(0xB801) + ((x % 32) + (y * 32))*2] = prop;
		return 0;
	}
	static int clearStatusBar(lua_State* L) {
		memset(&RAM[VRAM_Convert(0xB800)], 0xFF, 0x800);
		return 0;
	}

	//Asset loading
	static int loadNewGFX(lua_State* L) {
		int gfx_file = (int)lua_tointeger(L, 1);
		int offset = (int)lua_tointeger(L, 2);
		loadAssetRAM(Modpack + "/graphics/GFX" + int_to_hex(gfx_file, true) + ".bin", offset, false, true);
		if (offset >= 0) {
			TriggerRAMSync();
		}
		return 0;
	}
	static int loadNewAsset(lua_State* L) {
		bool direct = (bool)lua_toboolean(L, 3);
		int offset = (int)lua_tointeger(L, 2);
		if (!direct) {
			int file = (int)lua_tointeger(L, 1);
			loadAssetRAM(Modpack + "/levels/" + int_to_hex(getRamValue(0x010B, 2), false) + "/asset" + int_to_hex(file, true) + ".bin", -0x20000 + offset, false, true);
		}
		else {
			loadAssetRAM(Modpack + "/lualibs/" + ((string)lua_tostring(L, 1)), -0x20000 + offset, false, true);
		}
		if (offset >= 0x20000) {
			TriggerRAMSync();
		}
		return 0;
	}

	//Require
	static int jfkmw_require(lua_State* L) {
		string file = Modpack + "/lualibs/" + string(lua_tostring(L, 1));
		std::ifstream fs(file);
		if (fs.good()) {
			std::ostringstream sstream; sstream << fs.rdbuf();
			const std::string str(sstream.str());
			luaL_dostring(L, str.c_str());
		}
		return 0;
	}

	//these all mess with players in a way
	static int killPlayer(lua_State* L) {
		uint_fast8_t plr = ((uint_fast8_t)lua_tointeger(L, 1)) - 1;
		player_netcommand[plr] = 0xf;
		return 0;
	}

	static int damagePlayer(lua_State* L) {
		uint_fast8_t plr = ((uint_fast8_t)lua_tointeger(L, 1)) - 1;
		player_netcommand[plr] = 0x1f;
		return 0;
	}

	static int setPlayerState(lua_State* L) {
		uint_fast8_t plr = ((uint_fast8_t)lua_tointeger(L, 1)) - 1;
		player_netcommand[plr] = 0xf + ((((int)lua_tointeger(L, 2)) + 2) << 4);
		return 0;
	}

	static int discordMessageHook(lua_State* L) {
		if (!networking) { return 0; }
		string text = (string)lua_tostring(L, 1); discord_message(text);
		return 0;
	}

	static int getPlayerX(lua_State* L) {
		uint_fast8_t plr = ((uint_fast8_t)lua_tointeger(L, 1)) - 1;
		lua_pushnumber(L, RAM[0x5000 + plr] + Sint8(RAM[0x5100 + plr]) * 256);
		return 1;
	}

	static int getPlayerY(lua_State* L) {
		uint_fast8_t plr = ((uint_fast8_t)lua_tointeger(L, 1)) - 1;
		lua_pushnumber(L, RAM[0x5200 + plr] + Sint8(RAM[0x5300 + plr]) * 256);
		return 1;
	}

	static int getSpriteX(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
		double sprite_x_position = RAM[0x2100 + sprite_index] + (int_fast8_t(RAM[0x2180 + sprite_index]) * 256) + double(RAM[0x2200 + sprite_index]) / 256.0;
		lua_pushnumber(L, sprite_x_position);
		return 1;
	}

	static int getSpriteY(lua_State* L) {
		uint_fast8_t sprite_index = (uint_fast8_t)lua_tonumber(L, 1);
		double sprite_y_position = RAM[0x2280 + sprite_index] + (int_fast8_t(RAM[0x2300 + sprite_index]) * 256) - 16 + double(RAM[0x2380 + sprite_index]) / 256.0;
		lua_pushnumber(L, sprite_y_position);
		return 1;
	}

	static int lua_checkbit(lua_State* L) {
		uint_fast32_t p = (uint_fast16_t)lua_tointeger(L, 1);
		uint_fast8_t s = (uint_fast8_t)lua_tointeger(L, 2);
		lua_pushboolean(L, bool((RAM[p] >> s) & 1));
		return 1;
	}

	static int lua_jfkmwostime(lua_State* L) {
		long int t = static_cast<long int> (time(NULL));
		lua_pushinteger(L, t);
		return 1;
	}

	static int lua_chartosmw(lua_State* L) {
		uint_fast8_t p = (uint_fast8_t)lua_tointeger(L, 1);
		lua_pushinteger(L, char_to_smw(p));
		return 1;
	}
}

//functions end
void lua_connect_functions(lua_State* L) {
	luaL_openlibs(L);

	//Functions
	lua_pushcfunction(L, lua_write); lua_setglobal(L, "jfkmwPrint");
	lua_pushcfunction(L, lua_write_ram); lua_setglobal(L, "asmWrite");
	lua_pushcfunction(L, lua_spawn_sprite); lua_setglobal(L, "spawnSprite");
	lua_pushcfunction(L, jfkmw_require); lua_setglobal(L, "require");
	lua_pushcfunction(L, drawtohud); lua_setglobal(L, "drawToHud");
	lua_pushcfunction(L, killPlayer); lua_setglobal(L, "killPlayer");
	lua_pushcfunction(L, damagePlayer); lua_setglobal(L, "damagePlayer");
	lua_pushcfunction(L, setPlayerState); lua_setglobal(L, "setPlayerState");
	lua_pushcfunction(L, discordMessageHook); lua_setglobal(L, "discordMessage");
	lua_pushcfunction(L, lua_jfkmwostime); lua_setglobal(L, "jfkmwOSTime");
	lua_pushcfunction(L, setSpriteX); lua_setglobal(L, "setSpriteX");
	lua_pushcfunction(L, setSpriteY); lua_setglobal(L, "setSpriteY");
	lua_pushcfunction(L, loadNewGFX); lua_setglobal(L, "loadNewGFX");
	lua_pushcfunction(L, loadNewAsset); lua_setglobal(L, "loadLevelAsset");
	lua_pushcfunction(L, clearStatusBar); lua_setglobal(L, "clearStatusBar");
	lua_pushcfunction(L, lua_addscore); lua_setglobal(L, "addScore");

	//New OAM functions
	lua_pushcfunction(L, pushOAM); lua_setglobal(L, "pushOAM");
	lua_pushcfunction(L, pushParticle); lua_setglobal(L, "pushParticle");

	//Deprecated
	lua_pushcfunction(L, createParticleHook); lua_setglobal(L, "createParticle");
	lua_pushcfunction(L, slideDeathHandler); lua_setglobal(L, "deathBySlide");
	lua_pushcfunction(L, spriteDeathParticle); lua_setglobal(L, "deathByJump");
	lua_pushcfunction(L, draw_to_oam); lua_setglobal(L, "drawOam");
	lua_pushcfunction(L, draw_to_oam_direct); lua_setglobal(L, "drawOamDirect");

	//More functions
	lua_register(L, "asmRead", lua_get_ram);
	lua_register(L, "charToSmw", lua_chartosmw);
	lua_register(L, "getPlayerX", getPlayerX);
	lua_register(L, "getPlayerY", getPlayerY);
	lua_register(L, "getSpriteX", getSpriteX);
	lua_register(L, "getSpriteY", getSpriteY);
	lua_register(L, "asmCheckBit", lua_checkbit);
	lua_register(L, "getPlayerUsername", lua_get_username);
}

//LUA General
string last_lua_file;
void lua_loadfile(string file)
{
	if (LUA_STATE) {
		lua_close(LUA_STATE); LUA_STATE = NULL;
	}

	last_lua_file = file; LUA_STATE = luaL_newstate();
	lua_connect_functions(LUA_STATE);

	int ret = luaL_dofile(LUA_STATE, file.c_str());
	if (ret != 0) {
		lua_print("Error occurred when calling luaL_dofile()");
		lua_print("Error: " + string(lua_tostring(LUA_STATE, -1)));
		lua_close(LUA_STATE);
		LUA_STATE = NULL;
		return;
	}
	//main connectors back to JFKMW.
	lua_print("loaded " + file);
}

void lua_run_init() {
	if (LUA_STATE != NULL) {
		lua_getglobal(LUA_STATE, "Init"); lua_pcall(LUA_STATE, 0, 0, 0);
		lua_getglobal(LUA_STATE, "Main"); lua_pcall(LUA_STATE, 0, 0, 0);
	}
}

void lua_run_main() {
	if (LUA_STATE != NULL) {
		lua_getglobal(LUA_STATE, "Main");
		int ret = lua_pcall(LUA_STATE, 0, 0, 0);
		if (ret != 0) {
			lua_print("Error (from level lua): " + string(lua_tostring(LUA_STATE, -1)));
			if (LUA_STATE) {
				lua_close(LUA_STATE);
				LUA_STATE = NULL;
			}
		}
	}
}

void lua_on_chatted(string message, int plr = 0) {
	if (gamemode != GAMEMODE_MAIN || LUA_STATE == NULL) {
		return;
	}
	lua_getglobal(LUA_STATE, "OnChatted");
	if (!lua_isnil(LUA_STATE, -1)) {
		lua_pushstring(LUA_STATE, message.c_str());
		lua_pushinteger(LUA_STATE, plr);
		lua_pcall(LUA_STATE, 2, 0, 0);
	}
}
