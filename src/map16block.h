#pragma once

//Defines
#define tile_1 0
#define tile_2 2
#define tile_3 4
#define tile_4 6
#define tile_palette_1 8
#define tile_palette_2 9
#define tile_flips 10
#define collision 11
#define BOUNCE_TIME 6

//Initialize map16 from file.
void LoadMap16File(string file) {
	ifstream input(file, ios::binary);
	if (!input.good()) {
		return;
	}
	cout << blue << "[MAP16] Loading " << file << endl;
	vector<unsigned char> buffer(istreambuf_iterator<char>(input), {});
	uint_fast8_t temp[16]; uint_fast8_t current_byte = 0;
	for (auto &v : buffer) {
		temp[current_byte] = uint_fast8_t(v); current_byte++;
		if (current_byte >= 16) {
			uint_fast16_t replace_p = MAP16_LOCATION + ((temp[1] + (temp[0] << 8)) << 4); //this is actually a thing.
			memcpy(&RAM[replace_p], &temp[2], 14);
			current_byte = 0;
		}
	}
	input.close();
}

//Macros
#define DRAW_SPARKLES_BLOCK(D) createParticle(0x7D, 0x00, 0x8, 2, x * 16, -20 + D + y * 16, 0, 0, 0, 0); createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -28 + D + y * 16, 0, 0, 0, -4); createParticle(0x7D, 0x00, 0x8, 2, 8 + x * 16, -20 + D + y * 16, 0, 0, 0, -8); createParticle(0x7D, 0x00, 0x8, 2, 4 + x * 16, -12 + D + y * 16, 0, 0, 0, -12);

//Bounce sprite handler, or just a block updater.
class block_timer
{
public:
	uint_fast16_t set_to = 0x025;
	uint_fast16_t x = 0;
	uint_fast16_t y = 0;
	uint_fast16_t time = 0xFF;
	bool has_spr = false;
	uint_fast8_t spr_tile = 0;
	uint_fast8_t pal_props = 0;
	double spr_x = 0;
	double spr_y = 0;
	double spr_sx = 0;
	double spr_sy = 0;
	void draw()
	{
		spr_sy -= Calculate_Speed(256);
		spr_x += spr_sx;
		spr_y += spr_sy;

		uint_fast16_t s_x = uint_fast16_t(spr_x);
		uint_fast16_t s_y = uint_fast16_t(spr_y);

		Create_OAMTile(spr_tile, 0x11, s_x, s_y, pal_props, 0);
	}
};

vector<block_timer> blocks_processing;


class map16blockhandler { //Map16 loaded block
public:
	uint_fast16_t tile;
	bool logic[8];

	//Get the map16 details of a tile.
	void get_map_16_details() {
		uint_fast8_t integer = RAM[MAP16_LOCATION + (tile << 4) + collision];
		logic[0] = integer & 0b10000000;
		logic[1] = integer & 0b01000000;
		logic[2] = integer & 0b00100000;
		logic[3] = integer & 0b00010000;
		logic[4] = integer & 0b00001000;
		logic[5] = integer & 0b00000100;
		logic[6] = integer & 0b00000010;
		logic[7] = integer & 0b00000001;

		if (tile >= 0x166 && tile <= 0x167) {
			bool solid = (RAM[0x14AF] > 0) == (tile == 0x167);
			logic[0] = solid; logic[1] = solid; logic[2] = solid; logic[3] = solid;
		}

		if (tile == 0x132 || tile == 0x2B) {
			bool solid = (RAM[0x14AD] > 0) == (tile == 0x2B);
			logic[0] = solid; logic[1] = solid; logic[2] = solid; logic[3] = solid;
		}
	}

	//Update Map Tile
	void update_map_tile(uint_fast16_t x, uint_fast16_t y) {
		if (x >= mapWidth || y >= mapHeight) {
			tile = 0x25;
		}
		else {
			uint_fast16_t index = x + y * mapWidth;
			tile = (RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8)) & 0x3FF;
		}
		get_map_16_details();
	}

	//Replace a map tile with anything using x/y coordinates.
	void replace_map_tile(uint16_t tile, uint_fast16_t x, uint_fast16_t y) {
		if (x >= mapWidth || y >= mapHeight) {
			return;
		}
		uint_fast16_t index = x + y * mapWidth;
		RAM[ram_level_low + index] = uint_fast8_t(tile); RAM[ram_level_high + index] = tile >> 8;

		RAM_decay_time_level[index] = level_ram_decay_time * PlayerAmount;
	}

	//Get ground height.
	double ground_y(double x_relative) {
		if (tile == 0x1AA || tile == 0x1AB) { //45* slope Right
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return x_relative;
		}
		if (tile == 0x1AF || tile == 0x1B0) { //45* slope Left
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 16.0 - x_relative;
		}
		if (tile == 0x196) { //23* slope Right P1
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return x_relative / 2;
		}
		if (tile == 0x19B) { //23* slope Right P2
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 8.0 + x_relative / 2;
		}
		if (tile == 0x1A0) { //23* slope Left P1
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 16.0 - x_relative / 2;
		}
		if (tile == 0x1A5) { //23* slope Left P2
			if (x_relative < 0 || x_relative > 16) { return -9999; }
			return 8.0 - x_relative / 2;
		}
		return 16.0;
	}

	//Check if a block is solid.
	bool check_solid(uint_fast16_t x, uint_fast16_t y) {
		update_map_tile(x, y);
		return (logic[0] || logic[1]) || (logic[2] || logic[3]);
	}

	//Check if a block is climbable.
	bool check_climbable(uint_fast16_t x, uint_fast16_t y) {
		uint_fast16_t tile = get_tile(x, y);
		return tile >= 6 && tile <= 0xF;
	}

	//Ground collision height checks.
	double ground_s() {
		if ((tile == 0x1AA || tile == 0x1AB) || (tile == 0x1AF || tile == 0x1B0)) { //45* slope Right/Left
			return 16.0;
		}
		if ((tile == 0x1A0 || tile == 0x1A5) || (tile == 0x196 || tile == 0x19B)) { //23* slope Right/Left
			return 16.0;
		}
		return 15.0;
	}
	
	//Check if a tile is a slope.
	uint_fast8_t get_slope() {
		//45
		if (tile == 0x1AA || tile == 0x1AB) { return 1; }
		if (tile == 0x1AF || tile == 0x1B0) { return 2; }
		//23
		if (tile == 0x196) { return 3; }
		if (tile == 0x19B) { return 4; }
		//87? Needs clarifying
		if (tile == 0x1A0) { return 5; }
		if (tile == 0x1A5) { return 6; }
		return 0;
	}

	//Block hit process
	void process_block(uint_fast16_t x, uint_fast16_t y, uint8_t side, bool pressing_y = false, bool shatter = false, uint_fast8_t state = 0) {
		if (x >= mapWidth || y >= mapHeight) {
			return;
		}
		if (!isClient) {
			uint_fast16_t index = x + y * mapWidth;
			uint_fast16_t t = RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
			if (t == 0x11E && side == bottom) {
				blocks_processing.push_back(block_timer{ 0x48, x, y, BOUNCE_TIME, true, 0x40, 0x8, double(x * 16), double(y * 16) - 17.0, 0.0, 4.0 });
				blocks_processing.push_back(block_timer{ 0x11E, x, y, 0x100+4 });
				replace_map_tile(0xFF, x, y);
			}
			if (t == 0x11E && shatter) {
				replace_map_tile(0x25, x, y);
				RAM[0x1DFC] = 7;
				for (int x_p = 0; x_p < 2; x_p++) {
					for (int y_p = 0; y_p < 2; y_p++) {
						createParticle(0x3C, 0x00, 0x8, 1, (x * 16) + x_p * 8, -24 + (y * 16) + y_p * 8, ((x_p * 2) - 1), 2 + y_p * 2, Calculate_Speed(64), int(rand() % 16));
					}
				}
			}
			if (t == 0x0124 && side == bottom) {
				replace_map_tile(0xFF, x, y);
			}

			if (side == bottom) {
				//Turn blocks
				if (t >= 0x0117 && t <= 0x119) {
					uint_fast8_t powerup = t == 0x119 ? 0x76 : (state == 0 ? 0x74 : (0x75 + (t - 0x117) * 2));
					uint_fast8_t spr = spawnSpriteObj(powerup, 5, x * 16, 2 + y * 16, 1);
					RAM[0x2A00 + spr] = 2; RAM[0x1DFC] = 2;
				}
				if (t == 0x11A) {
					uint_fast8_t spr = spawnSpriteObj(0x79, 5, x * 16, 8 + y * 16, 1); RAM[0x1DFC] = 2;
				}
				if (t == 0x11D) {
					uint_fast8_t spr = spawnSpriteObj(0x3E, 5, x * 16, 8 + y * 16, 1); RAM[0x2480 + spr] = 0x20; RAM[0x1DFC] = 2;
				}
				//Turn block handler
				if (t >= 0x117 && t <= 0x11D) {
					blocks_processing.push_back(block_timer{ 0x132, x, y, BOUNCE_TIME, true, 0x40, 0x8, double(x * 16), double(y * 16) - 17.0, 0.0, 4.0 });
					replace_map_tile(0xFF, x, y);
				}
				//Question blocks
				if (t >= 0x011F && t <= 0x121) {
					uint_fast8_t powerup = t == 0x121 ? 0x76 : (state == 0 ? 0x74 : (0x75 + (t - 0x11F) * 2));
					uint_fast8_t spr = spawnSpriteObj(powerup, 5, x * 16, 2 + y * 16, 1);
					RAM[0x2A00 + spr] = 2; RAM[0x1DFC] = 2;
				}
				//Question block handler
				if (t >= 0x11F && t <= 0x128) {
					blocks_processing.push_back(block_timer{ 0x132, x, y, BOUNCE_TIME, true, 0x2A, 0x8, double(x * 16), double(y * 16) - 17.0, 0.0, 4.0 });
					replace_map_tile(0xFF, x, y);
				}
				//ON/OFF handler
				if (t == 0x0112) {
					RAM[0x14AF] = !RAM[0x14AF]; RAM[0x1DF9] = 0xB;
					blocks_processing.push_back(block_timer{ 0x112, x, y, BOUNCE_TIME, true, 0xCE, 0xB, double(x * 16), double(y * 16) - 17.0, 0.0, 4.0 });
					replace_map_tile(0xFF, x, y);
				}
			}
			//Midway
			if (t == 0x0038) {
				replace_map_tile(0x0025, x, y);
				RAM[0x1DF9] = 5;
				writeToRam(0x3F0B, x * 16, 2);
				writeToRam(0x3F0D, y * 16, 2);
				DRAW_SPARKLES_BLOCK(0)
				midway_activated = true;
			}

			//Coins
			if ((t == 0x002B && !RAM[0x14AD]) || (t == 0x0132 && RAM[0x14AD])) {
				replace_map_tile(0x0025, x, y);
				RAM[0x1DFC] = 1;
				RAM[0x0DBF]++;
				DRAW_SPARKLES_BLOCK(0)
			}

			//Dragon Coins (both parts)
			if (t == 0x002D) {
				replace_map_tile(0x0025, x, y);
				replace_map_tile(0x0025, x, y - 1);
				DRAW_SPARKLES_BLOCK(-8)
			}
			if (t == 0x002E) {
				replace_map_tile(0x0025, x, y);
				replace_map_tile(0x0025, x, y + 1);
				DRAW_SPARKLES_BLOCK(8)
			}
			if (t == 0x002D || t == 0x002E) {
				RAM[0x1DF9] = 0x1C;
				RAM[0x1420]++;
			}

			//Grabblock obtain
			if (t == 0x012E && pressing_y) {
				replace_map_tile(0x0025, x, y);
				x *= 16;
				y *= 16;
				spawned_grabbable = spawnSpriteObj(0x53, 2, x, y, 0);
			}
		}

	}

	//Get a tile on the map using x/y coordinates.
	uint_fast16_t get_tile(uint_fast16_t x, uint_fast16_t y) {
		if (x >= mapWidth || y >= mapHeight) { return 0x25; }
		uint_fast16_t index = x + y * mapWidth;
		return RAM[ram_level_low + index] + (RAM[ram_level_high + index] << 8);
	}

	//Process bounce sprites/blocks.
	void process_global() {
		for (int i = 0; i < blocks_processing.size(); i++) {
			block_timer& b = blocks_processing[i];
			if (b.has_spr) {
				b.draw();
			}
			b.time--;
			if (b.time == 0) {
				replace_map_tile(b.set_to, b.x, b.y);
				blocks_processing.erase(blocks_processing.begin() + i);
				i--;
			}
		}
	}
};

map16blockhandler map16_handler;

void reset_map() {
	memset(&RAM[0x5000], 0, 0x400);
	memset(&RAM[ram_level_low], 0x25, LEVEL_SIZE);
	memset(&RAM[ram_level_high], 0x00, LEVEL_SIZE);
	memset(&RAM[0x2000], 0x00, 0x1000);

	use_vertical_spawning = false;
	blocks_processing.clear();
}