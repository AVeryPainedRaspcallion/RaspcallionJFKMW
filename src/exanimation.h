#pragma once
/*
	This should only be done in the client, if there's need for, a better implementation will be done later.
	Why it's not done on the server : It would be laggy
*/

void ProcessGraphicAnimations()
{
	if (!networking || (networking && isClient))
	{
		if (gamemode == GAMEMODE_OVERWORLD) {
			//Animate water (To-do dedicate own exanimation)
			memcpy(&RAM[VRAM_Location + (0x75 * 32)], &RAM[VRAM_Location + 0x2000 + (((RAM[0x14] >> 3) & 7) << 5)], 32);
			memcpy(&RAM[VRAM_Location + (0x7D * 32)], &RAM[VRAM_Location + 0x8000 + (0x68 * 32) + (((RAM[0x14] >> 3) & 7) << 5)], 32);
			memcpy(&RAM[VRAM_Location + (0x76 * 32)], &RAM[VRAM_Location + 0x2000 + (0x08 * 32) + (((RAM[0x14] >> 3) & 7) << 5)], 32);
		}
		else {
			//Question block (Corrected)
			memcpy(&RAM[VRAM_Location + (32 * 0x60)], &RAM[VRAM_Location + 0x8000 + (0xC0 * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);

			//Brown block & Coin
			uint_fast16_t block_location = RAM[0x14AD] > 0 ? 0x6C : 0x58;
			uint_fast16_t coin_location = RAM[0x14AD] > 0 ? 0x58 : 0x6C;
			memcpy(&RAM[VRAM_Location + (block_location << 5)], &RAM[VRAM_Location + 0x8000 + (0xB4 * 32)], 128);
			memcpy(&RAM[VRAM_Location + (coin_location << 5)], &RAM[VRAM_Location + 0x8000 + (0xCC * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);

			//Flippin turnblock
			memcpy(&RAM[VRAM_Location + (32 * 0xEA)], &RAM[VRAM_Location + 0x8000 + (0xC8 * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);

			//Muncher
			memcpy(&RAM[VRAM_Location + (32 * 0x5C)], &RAM[VRAM_Location + 0x8000 + ((0xF8 + 0x40) * 32) + (((RAM[0x14] >> 3) & 1) << 7)], 128);

			//Water
			memcpy(&RAM[VRAM_Location + (32 * 0x70)], &RAM[VRAM_Location + 0x8000 + (0x100 * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);

			//Lava
			memcpy(&RAM[VRAM_Location + (32 * 0x4C)], &RAM[VRAM_Location + 0x8000 + (0x8C * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);

			//On/Off Switch
			memcpy(&RAM[VRAM_Location + (32 * 0xDA)], &RAM[VRAM_Location + 0x8000 + ((0xA0 + (RAM[0x14AF] << 4)) * 32)], 128);

			//That one line thing idk
			memcpy(&RAM[VRAM_Location + (32 * 0x68)], &RAM[VRAM_Location + 0x8000 + ((0xC0 + 0x80) * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);

			//Berray
			memcpy(&RAM[VRAM_Location + (32 * 0x80)], &RAM[VRAM_Location + 0xA900 + (((RAM[0x14] >> 3) & 3) << 9)], 64);
			memcpy(&RAM[VRAM_Location + (32 * 0x90)], &RAM[VRAM_Location + 0xA940 + (((RAM[0x14] >> 3) & 3) << 9)], 64);

			//On/Off Switch Blocks
			memcpy(&RAM[VRAM_Location + (32 * 0xAC)], &RAM[VRAM_Location + 0x8000 + ((0xC8 + (RAM[0x14AF] << 5)) * 32)], 128);
			memcpy(&RAM[VRAM_Location + (32 * 0xCC)], &RAM[VRAM_Location + 0x8000 + ((0xE8 - (RAM[0x14AF] << 5)) * 32)], 128);

			//Midpoint
			memcpy(&RAM[VRAM_Location + (32 * 0x74)], &RAM[VRAM_Location + 0x8000 + (0x104 * 32) + (((RAM[0x14] >> 3) & 3) << 9)], 128);
		}
	}
}