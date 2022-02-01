#pragma once
//ASM-related backend things.

//advanced ram writes
void writeToRam(uint_fast32_t pointer, uint_fast32_t value, uint_fast8_t size = 1) {
	if ((pointer+size) > RAM_Size) {
		return;
	}
	if (networking && !isClient) {
		if (pointer < RAM_OLD_SIZE) { RAM_decay_time[pointer] = min(255, level_ram_decay_time * PlayerAmount); }
		if (pointer >= 0x10000 && pointer < 0x20000) { RAM_decay_time_level[pointer & 0x7FFF] = min(255, level_ram_decay_time * PlayerAmount); }
	}
	for (uint_fast8_t i = 0; i < size; i++) {
		RAM[pointer + i] = uint_fast8_t(value >> (i << 3));
	}
	if (pointer == 0x420B) { ProcessDMA(); }
}

uint_fast32_t getRamValue(uint_fast32_t pointer, uint_fast8_t size = 1) {
	if ((pointer+size) > RAM_Size) { return -1; }
	uint_fast32_t temp = RAM[pointer];
	if (size > 1) {
		for (uint_fast8_t i = 1; i < size; i++) {
			temp += RAM[pointer + i] << (i << 3);
		}
	}
	return temp;
}

void resetImportantVariables() {
	for (int i = 0; i < 5; i++) {
		if (i != 2) {
			if (RAM[0x1DF9 + i] != 0) {
				audio_id[i] = RAM[0x1DF9 + i];
				audio_sync[i]++;
				RAM[0x1DF9 + i] = 0;
			}
		}
	}
}

//Networking RAM functions
#ifndef DISABLE_NETWORK
#define spr_ent_net 18
uint_fast16_t sprite_f_send[spr_ent_net] =
{
	0, //Status
	1, //Num
	2, //Xpos
	3, //Xpos
	5, //Ypos
	6, //Ypos
	8, //Xspd
	9, //Yspd
	10, //Xsize
	11, //Ysize
	12, //Flag 1
	13, //Dir
	14, //Inter
	16, //Flag 2
	17, //Flag 3
	28, //Prop
	29, //Prop
	30 //Prop
};

void Sync_Server_RAM(bool compressed = false)
{
	if (!compressed) {
		CurrentPacket >> latest_sync;
		
		//LZ4 compression
		int CompressedSize = 0; CurrentPacket >> CompressedSize;
		for (int i = 0; i < CompressedSize; i++) {
			CurrentPacket >> RAM_compressed[i];
		}
		LZ4_decompress_safe((char*)RAM_compressed, (char*)RAM, CompressedSize, RAM_Size);

		//Set
		decompressHDMAnet();
		decompressDMAnet();
		DecompressOAM_Player();

		//Preload
		need_preload_sprites = true;
	}
	else {
		Uint16 entries, pointer;
		CurrentPacket >> entries;
		for (uint_fast16_t i = 0; i < entries; i++) {
			CurrentPacket >> pointer;
			CurrentPacket >> RAM[pointer];
		}
		CurrentPacket >> entries;
		for (uint_fast16_t i = 0; i < entries; i++) {
			CurrentPacket >> pointer;
			CurrentPacket >> RAM[ram_level_low + pointer];
			CurrentPacket >> RAM[ram_level_high + pointer];
		}

		//HDMA
		decompressHDMAnet();
		decompressDMAnet();

		//Receive Sound
		for (int i = 0; i < 5; i++) {
			if (i != 2) {
				uint_fast8_t new_i; CurrentPacket >> new_i;
				uint_fast8_t new_s; CurrentPacket >> new_s;
				if (new_s != audio_sync[i]) {
					audio_sync[i] = new_s;
					RAM[0x1DF9 + i] = new_i;
				}
			}
		}

		//receive Mode 7 stuff
		CurrentPacket >> RAM[0x36];
		CurrentPacket >> RAM[0x38];
		CurrentPacket >> RAM[0x39];

		//Receive flags, This is optimized
		uint_fast8_t flags; CurrentPacket >> flags;
		RAM[0x1411] = flags & 1;
		RAM[0x1412] = (flags >> 1) & 1;
		RAM[0x9D] = (flags >> 2) & 1;
		RAM[0x14AF] = (flags >> 3) & 1;
		RAM[0x85] = (flags >> 4) & 1;
		RAM[0x40] = flags >> 5;

		//Window flags
		CurrentPacket >> RAM[0x1B89];
		uint_fast8_t flags2; CurrentPacket >> flags2;
		RAM[0x3F1F] = flags2 & 0xF;
		RAM[0x1B88] = flags2 >> 4;

		//receive clear status & brightness flag
		CurrentPacket >> RAM[0x1493];

		//receive shake timer
		CurrentPacket >> RAM[0x1887];

		//receive pswitch timer
		CurrentPacket >> RAM[0x14AD];

		//receive grav & speed
		CurrentPacket >> RAM[0x7C];
		CurrentPacket >> RAM[0x7B];
		CurrentPacket >> RAM[0x7D];

		//receive darkness and mosaic
		CurrentPacket >> RAM[0x3F10];
		CurrentPacket >> RAM[0x3F11];
		CurrentPacket >> RAM[0x3F12];

		//Score
		CurrentPacket >> RAM[0x3F13];
		CurrentPacket >> RAM[0x3F14];
		CurrentPacket >> RAM[0x3F15];

		//recieve level start & Size
		CurrentPacket >> RAM[0x3F00];
		CurrentPacket >> RAM[0x3F01];
		CurrentPacket >> RAM[0x3F02];
		CurrentPacket >> RAM[0x3F03];
		CurrentPacket >> RAM[0x3F0B];
		CurrentPacket >> RAM[0x3F0C];
		CurrentPacket >> RAM[0x3F0D];
		CurrentPacket >> RAM[0x3F0E];
		CurrentPacket >> RAM[0x3F05];
		CurrentPacket >> RAM[0x3F1B];

		//Send L3 parameters
		if (RAM[0x3F1F] & 8) {
			CurrentPacket >> RAM[0x3F1C];
			CurrentPacket >> RAM[0x3F1D];
			CurrentPacket >> RAM[0x3F1E];
		}

		//Time Limit
		CurrentPacket >> RAM[0xF31];
		CurrentPacket >> RAM[0xF32];

		//Could be a loop once again
		CurrentPacket >> RAM[0x22];
		CurrentPacket >> RAM[0x23];
		CurrentPacket >> RAM[0x24];
		CurrentPacket >> RAM[0x25];
		CurrentPacket >> RAM[0x1462];
		CurrentPacket >> RAM[0x1463];
		CurrentPacket >> RAM[0x1464];
		CurrentPacket >> RAM[0x1465];
		CurrentPacket >> RAM[0x1466];
		CurrentPacket >> RAM[0x1467];
		CurrentPacket >> RAM[0x1468];
		CurrentPacket >> RAM[0x1469];

		//Decompress OAM
		DecompressOAM_Player();

		//Decompress sprite entries (Fuck do you mean )
		memset(&RAM[0x2000], 0, 0x80);
		uint_fast8_t spr_entries;
		CurrentPacket >> spr_entries;
		for (uint_fast8_t i = 0; i < spr_entries; i++) {
			uint_fast8_t p;
			CurrentPacket >> p;
			for (uint_fast16_t n = 0; n < spr_ent_net; n++) {
				CurrentPacket >> RAM[0x2000 + (sprite_f_send[n] << 7) + p];
			}
		}

		//Decompress T3
		memset(&RAM[VRAM_Convert(0xB800)], 0xFF, 0x800);

		uint_fast16_t T3_entries;
		CurrentPacket >> T3_entries;
		for (uint_fast16_t i = 0; i < T3_entries; i++) {
			uint_fast16_t p; uint_fast8_t t1; uint_fast8_t t2;
			CurrentPacket >> p;
			CurrentPacket >> t1; CurrentPacket >> t2;
			if (p < 0x800) {
				RAM[VRAM_Convert(0xB800) + p] = t1;
				RAM[VRAM_Convert(0xB801) + p] = t2;
			}
			else {
				break;
			}
		}

	}
}

#define checkArea(i, x, y) (i >= x || i <= y)
void Push_Server_RAM(bool compress = false) {
	if (!compress) {
		CurrentPacket << latest_sync;

		//LZ4 Compression
		int nCompressedSize = LZ4_compress_fast((char*)RAM, (char*)RAM_compressed, RAM_Size, RAM_Size, 1);
		CurrentPacket << nCompressedSize;
		CurrentPacket.append(RAM_compressed, nCompressedSize);

		memset(&RAM_decay_time_level, 0, LEVEL_DECAY_SIZE);

		//Set
		sendHDMAnet();
		sendDMANet();
		CompressOAM_Server();
	}
	else {
		//TO-DO: Optimize
		Uint16 entries = 0; int index = 0;
		for (Uint16 i = 0; i < RAM_OLD_SIZE; i++) {
			if (!((checkArea(i, 0x3000, 0x3CFF) || checkArea(i, 0, 0xFF)) || (checkArea(i, 0x5000, 0x5FFF) || checkArea(i, 0x2000, 0x2FFF))) && RAM_decay_time[i]) {
				RAM_decay_time[i]--;
				*((Uint16*)&RAM_compressed[index]) = i;
				RAM_compressed[index + 2] = RAM[i];
				entries++; index += 3;
			}
		}
		CurrentPacket << entries;
		CurrentPacket.append(RAM_compressed, index);

		entries = 0; index = 0;
		for (uint_fast16_t i = 0; i < LEVEL_DECAY_SIZE; i++) {
			if (RAM_decay_time_level[i]) {
				RAM_decay_time_level[i]--;
				*((Uint16*)&RAM_compressed[index]) = i;
				RAM_compressed[index + 2] = RAM[ram_level_low + i];
				RAM_compressed[index + 3] = RAM[ram_level_high + i];
				entries++; index += 4;
			}
		}
		CurrentPacket << entries;
		CurrentPacket.append(RAM_compressed, index);

		//HDMA & DMA
		sendHDMAnet();
		sendDMANet();

		//Send Sound
		for (int i = 0; i < 5; i++) {
			if (i != 2) {
				CurrentPacket << audio_id[i];
				CurrentPacket << audio_sync[i];
			}
		}

		//Send Mode 7 stuff
		CurrentPacket << RAM[0x36];
		CurrentPacket << RAM[0x38];
		CurrentPacket << RAM[0x39];

		//Send some important flags, This is optimized
		uint_fast8_t flags = 0;
		flags += (RAM[0x1411] & 1); //Screen locked X
		flags += (RAM[0x1412] & 1) << 1; //Screen locked Y
		flags += (RAM[0x9D] & 1) << 2; //Global Pause Flag
		flags += (RAM[0x14AF] & 1) << 3; //On/Off
		flags += (RAM[0x85] & 1) << 4; //Water
		flags += (RAM[0x40] & 7) << 5; //SDL Related stuff
		CurrentPacket << flags;

		//Window flags
		CurrentPacket << RAM[0x1B89];
		uint_fast8_t flags2 = (RAM[0x3F1F] & 0xF) + (RAM[0x1B88] << 4); CurrentPacket << flags2;

		//Send level clear status
		CurrentPacket << RAM[0x1493];

		//Send shake timer
		CurrentPacket << RAM[0x1887];

		//receive pswitch timer
		CurrentPacket << RAM[0x14AD];

		//Send grav & Speed
		CurrentPacket << RAM[0x7C];
		CurrentPacket << RAM[0x7B];
		CurrentPacket << RAM[0x7D];

		//Send darkness and mosaic
		CurrentPacket << RAM[0x3F10];
		CurrentPacket << RAM[0x3F11];
		CurrentPacket << RAM[0x3F12];

		//Score
		CurrentPacket << RAM[0x3F13];
		CurrentPacket << RAM[0x3F14];
		CurrentPacket << RAM[0x3F15];

		//Send level start & Size
		CurrentPacket << RAM[0x3F00];
		CurrentPacket << RAM[0x3F01];
		CurrentPacket << RAM[0x3F02];
		CurrentPacket << RAM[0x3F03];
		CurrentPacket << RAM[0x3F0B];
		CurrentPacket << RAM[0x3F0C];
		CurrentPacket << RAM[0x3F0D];
		CurrentPacket << RAM[0x3F0E];
		CurrentPacket << RAM[0x3F05];
		CurrentPacket << RAM[0x3F1B];

		//Send L3 parameters
		if (RAM[0x3F1F] & 8)
		{
			CurrentPacket << RAM[0x3F1C];
			CurrentPacket << RAM[0x3F1D];
			CurrentPacket << RAM[0x3F1E];
		}

		//Time Limit
		CurrentPacket << RAM[0xF31];
		CurrentPacket << RAM[0xF32];

		//Could put these in a loop but I don't care right now
		CurrentPacket << RAM[0x22];
		CurrentPacket << RAM[0x23];
		CurrentPacket << RAM[0x24];
		CurrentPacket << RAM[0x25];
		CurrentPacket << RAM[0x1462];
		CurrentPacket << RAM[0x1463];
		CurrentPacket << RAM[0x1464];
		CurrentPacket << RAM[0x1465];
		CurrentPacket << RAM[0x1466];
		CurrentPacket << RAM[0x1467];
		CurrentPacket << RAM[0x1468];
		CurrentPacket << RAM[0x1469];

		//OAM
		CompressOAM_Server();

		//Compress sprite entries
		uint_fast8_t spr_entries = 0;
		for (uint_fast8_t i = 0; i < 0x80; i++) {
			if (RAM[0x2000 + i] != 0 && RAM[0x2A80 + i] & 2) { //Sprites exist and is onscreen
				spr_entries++;
			}
		}
		CurrentPacket << spr_entries;
		for (uint_fast8_t i = 0; i < 0x80; i++) {
			if (RAM[0x2000 + i] != 0 && RAM[0x2A80 + i] & 2) {
				CurrentPacket << i;
				for (uint_fast16_t n = 0; n < spr_ent_net; n++) {
					CurrentPacket << RAM[0x2000 + (sprite_f_send[n] << 7) + i];
				}
			}
		}

		
		//Compress T3
		uint_fast16_t T3_entries = 0;
		for (uint_fast16_t T3_loop = 0; T3_loop < 0x800; T3_loop += 2) {
			if (RAM[VRAM_Convert(0xB800) + T3_loop] < MAX_L3_TILES) { //This tile exists
				T3_entries++;
			}
		}
		CurrentPacket << T3_entries;
		for (uint_fast16_t T3_loop = 0; T3_loop < 0x800; T3_loop += 2) {
			if (RAM[VRAM_Convert(0xB800) + T3_loop] < MAX_L3_TILES) { //This tile exists
				CurrentPacket << T3_loop;
				CurrentPacket << RAM[VRAM_Convert(0xB800) + T3_loop];
				CurrentPacket << RAM[VRAM_Convert(0xB801) + T3_loop];
			}
		}	
	}
}
#endif