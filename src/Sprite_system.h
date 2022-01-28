#pragma once
/*
	Sprite system, RAM values documented in RAM map.lua
*/
class sprite_system
{
public:
	double x, y, x_size, y_size, xs, ys;
	lua_State* SPR_STATE[128];

	void process_sprite_logic(uint_fast8_t entry = 0)
	{
		RAM[0x2780 + entry] = 0;

		//New Pos calculation Formula
		x = RAM[0x2100 + entry] + (Sint8(RAM[0x2180 + entry]) * 256) + double(RAM[0x2200 + entry]) / 256.0;
		y = RAM[0x2280 + entry] + (Sint8(RAM[0x2300 + entry]) * 256) + double(RAM[0x2380 + entry]) / 256.0;
		x_size = RAM[0x2500 + entry];
		y_size = RAM[0x2580 + entry];

		bool IN_WT = RAM[0x85] != 0 || (WaterLevel > 0 && y < (WaterLevel - y_size));
		if (!IN_WT) { //buoyancy system
			uint_fast16_t check_x_1 = uint_fast16_t((x + x_size / 2.0) / 16.0);
			uint_fast16_t check_y_1 = uint_fast16_t((y + y_size / 2.0) / 16.0);
			IN_WT = map16_handler.get_tile(check_x_1, check_y_1) < 4;
		}

		if (RAM[0x2600 + entry] & 0b100000) //if gravity bit is on
		{
			int_fast8_t sprgravity = IN_WT ? -16 : -82;
			if (IN_WT) {
				RAM[0x2780 + entry] ^= 0b1000000;
				if (int_fast8_t(RAM[0x2480 + entry]) > 32) {
					RAM[0x2480 + entry] = 32;
				}
			}
			if (int_fast8_t(RAM[0x2480 + entry]) > sprgravity) {
				int grav = (RAM[0x2880 + entry] & 0b1000 ? 2 : 3) - (IN_WT * 2);
				RAM[0x2480 + entry] = max(sprgravity, int_fast8_t(RAM[0x2480 + entry] - grav));
			}
			else {
				RAM[0x2480 + entry] = sprgravity;
			}
		}

		if (RAM[0x2980 + entry]) {
			RAM[0x2980 + entry]--;
		}
		xs = double(double(int_fast8_t(RAM[0x2400 + entry])) * 16) / ((IN_WT && (RAM[0x2600 + entry] & 0b100000)) ? 384 : 256);
		ys = double(double(int_fast8_t(RAM[0x2480 + entry])) * 16) / 256.0;

		if (RAM[0x2600 + entry] & 0b1000000 && (RAM[0x2080 + entry] == 0x33 || !(RAM[0x2880 + entry] & 0b00010000)))
		{
			for (uint_fast8_t spr = 0; spr < 0x80; spr++)
			{
				if (spr != entry && RAM[0x2000 + spr])
				{
					double t_x = double(RAM[0x2100 + spr] + double(RAM[0x2180 + spr]) * 256.0) + double(RAM[0x2200 + spr]) / 256.0;
					double t_y = double(RAM[0x2280 + spr] + double(RAM[0x2300 + spr]) * 256.0) + double(RAM[0x2380 + spr]) / 256.0;

					double t_x_size = double(RAM[0x2500 + spr]);
					double t_y_size = double(RAM[0x2580 + spr]);

					if (
						x > (t_x - x_size) &&
						x < (t_x + t_x_size) &&
						y > (t_y - y_size) &&
						y < (t_y + t_y_size)
						)
					{
						//Fireball Kill Case
						if (RAM[0x2080 + entry] == 0x33) {
							if (RAM[0x2000 + spr] == 1 && RAM[0x2080 + spr] != 0x33 && !(RAM[0x2800 + spr] & 2)) {
								RAM[0x2B00 + entry] = 0x1;
								//Sprite fireball hit
								if (!(RAM[0x2880 + spr] & 0x80)) {
									RAM[0x2B00 + spr] = 0x2;
									RAM[0x2700 + spr] = 0xFF;
									RAM[0x1DF9] = 3;
								}
							}
						}
						else
						{
							//Logic
							if (RAM[0x2000 + entry] == 4 || (RAM[0x2000 + entry] == 2 && (abs(int_fast8_t(RAM[0x2400 + entry])) > 3 || abs(int_fast8_t(RAM[0x2480 + entry])) > 3)))
							{
								if (RAM[0x2000 + spr] != 3 && !(RAM[0x2600 + entry] & 0xF))
								{
									//2 handlers just incase
									RAM[0x2B00 + spr] = 0x1;
									RAM[0x2700 + spr] = 0xFF;
								}
							}
							else
							{
								if (RAM[0x2000 + entry] < 3 && !(RAM[0x2880 + spr] & 0b00010000) && RAM[0x2600 + entry] & 0b1000000 && RAM[0x2600 + spr] & 0b1000000)
								{
									if (RAM[0x2000 + spr] == 1 && !RAM[0x2980 + spr])
									{
										RAM[0x2680 + spr] *= -1;
										RAM[0x2400 + spr] *= -1;

										RAM[0x2980 + spr] = 16;
										RAM[0x2980 + entry] = 16;
									}
								}
							}
						}
					}
				}
			}
		}

		
		if (RAM[0x2600 + entry] & 0b1000000) { //if solid bit is on
			bool g = RAM[0x2000 + entry] == 2 || RAM[0x2000 + entry] == 4;
			if (!Move(xs, 0.0, x_size, y_size, g, entry)) {
				RAM[0x2780 + entry] ^= 0b00000001;
			}

			if (!Move(0.0, ys, x_size, y_size, g, entry)) {
				RAM[0x2780 + entry] ^= 0b00000010;
			}
		}
		else {
			x += xs;
			y += ys;
		}



		if (y < (-y_size-32) && !(RAM[0x2A80 + entry] & 1)) {
			for (uint_fast16_t i = 0; i < 32; i++) {
				RAM[0x2000 + entry + (i << 7)] = 0;
			}
			return;
		}
		Uint32 final_x = Uint32(x * 256.0);
		Uint32 final_y = Uint32(y * 256.0);
		RAM[0x2100 + entry] = final_x >> 8;
		RAM[0x2180 + entry] = final_x >> 16;
		RAM[0x2200 + entry] = final_x;
		RAM[0x2280 + entry] = final_y >> 8;
		RAM[0x2300 + entry] = final_y >> 16;
		RAM[0x2380 + entry] = final_y;
	}

	/*
		Shitty Movement Code
	*/
	bool Move(double xMove, double yMove, double x_size, double y_size, bool kickedgrabbed, uint_fast8_t entry)
	{
		bool finna_return = true;
		double NewPositionX = x + xMove;
		double NewPositionY = y + yMove;

		int_fast16_t PosXBlock = int_fast16_t(NewPositionX / 16); 
		int_fast16_t PosYBlock = int_fast16_t(NewPositionY / 16);

		int_fast8_t TotalBlocksCollisionCheckSpriteX = int_fast8_t(x_size / 16.0) + 2;
		int_fast8_t TotalBlocksCollisionCheckSpriteY = int_fast8_t(y_size / 16.0) + 2;

		int_fast16_t startX = max(0, PosXBlock - 1);
		int_fast16_t startY = max(0, PosYBlock - 1);

		for (int_fast16_t xB = startX; xB < PosXBlock + TotalBlocksCollisionCheckSpriteX; xB++)
		{
			for (int_fast16_t yB = startY; yB < PosYBlock + TotalBlocksCollisionCheckSpriteY; yB++)
			{
				map16_handler.update_map_tile(xB, yB);
				uint_fast8_t new_s = map16_handler.get_slope();
				double f_h = map16_handler.ground_y(NewPositionX + (x_size / 2.0) - (xB * 16));
				double BelowBlock = double(yB * 16) + (f_h - 16.0) - y_size;
				double AboveBlock = double(yB * 16) + f_h;
				double RightBlock = double(xB * 16) + 16.0;
				double LeftBlock = double(xB * 16) - x_size;

				bool checkLeft = map16_handler.logic[3];
				bool checkRight = map16_handler.logic[2];
				bool checkBottom = map16_handler.logic[1];
				bool checkTop = map16_handler.logic[0];

				if (NewPositionX < RightBlock && NewPositionX > LeftBlock && NewPositionY < AboveBlock && NewPositionY > BelowBlock)
				{
					if (xMove < 0.0 && checkRight && NewPositionX < RightBlock && x >= RightBlock)
					{
						NewPositionX = RightBlock;
						finna_return = false;
						RAM[0x2780 + entry] ^= 0b00000100;
						if (kickedgrabbed)
						{
							map16_handler.process_block(xB, yB, bottom);
						}
					}
					if (xMove > 0.0 && checkLeft && NewPositionX > LeftBlock && x <= LeftBlock)
					{
						NewPositionX = LeftBlock;
						finna_return = false;
						RAM[0x2780 + entry] ^= 0b00001000;
						if (kickedgrabbed)
						{
							map16_handler.process_block(xB, yB, bottom);
						}
					}
					if (((checkTop && yMove < 0.0) || (new_s && yMove != 0.0)) && NewPositionY < AboveBlock && y >= (AboveBlock - 2.0 - abs(xs))) {
						NewPositionY = AboveBlock;
						finna_return = false;
						RAM[0x2780 + entry] ^= 0b00010000;
					}
					if (yMove > 0.0 && checkBottom && NewPositionY > BelowBlock && y <= BelowBlock) {
						NewPositionY = BelowBlock;
						finna_return = false;

						RAM[0x2780 + entry] ^= 0b00100000;
						if (kickedgrabbed)
						{
							map16_handler.process_block(xB, yB, bottom);
						}
					}

				}
			}
		}

		x = NewPositionX;
		y = NewPositionY;
		return finna_return;
	}

	//What to do when a sprite errors
	void sprite_error(int index, string err) {
		lua_print("Error (from sprite slot " + to_string(index) + " id 0x" + int_to_hex(RAM[0x2080 + index], true) + "): " + err);
		RAM[0x2000 + index] = 0;
	}

	//Call function of sprite
	void call_lua_function_sprite(int index, const char* func) {
		lua_getglobal(SPR_STATE[index], func);
		if (!lua_isnil(SPR_STATE[index], -1)) {
			lua_pushinteger(SPR_STATE[index], index);
			int ret = lua_pcall(SPR_STATE[index], 1, 0, 0); // run script
			if (ret != 0) { sprite_error(index, string(func) + "(): " + string(lua_tostring(SPR_STATE[index], -1))); }
		}
		else {
			sprite_error(index, "function " + string(func) + " does not exist");
		}
	}

	//Initialize Lua for sprite
	void init_sprite_lua(int index, int type) {
		if (SPR_STATE[index]) { lua_close(SPR_STATE[index]); }
		SPR_STATE[index] = luaL_newstate();
		lua_connect_functions(SPR_STATE[index]);
		int ret = luaL_dostring(SPR_STATE[index], SPR_CODE[type].c_str());
		if (ret != 0) {
			sprite_error(index, "Failed to load: " + string(lua_tostring(SPR_STATE[index], -1)));
			return;
		}
		call_lua_function_sprite(index, "Init");
	}

	//Process all sprites.
	void process_all_sprites() {
		for (uint_fast8_t i = 0; i < 128; i++) {
			if (RAM[0x2000 + i] != 0) { //If sprite exists..
				//Check if sprite hasn't been initialized, if so, initialize it
				if (!RAM[0x2F80 + i]) {
					init_sprite_lua(int(i), RAM[0x2080 + i]);
					RAM[0x2F80 + i] = 1;
				}
				else {
					//Run offscreen logic for already initialized sprites.
					if (!(RAM[0x2A80 + i] & 2)) { //Don't process offscreen, useless.
						if (RAM[0x2A80 + i] & 4) {
							RAM[0x2000 + i] = 0; //Destroy the sprite, if it's supposed to despawn offscreen.
						}
						continue;
					}
					process_sprite_logic(i);
					if (!RAM[0x2000 + i]) {
						continue;
					}
					call_lua_function_sprite(i, "Main");
				}
			}
		}
	}
};


sprite_system Sprites;