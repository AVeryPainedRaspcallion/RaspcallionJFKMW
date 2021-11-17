#pragma once

//OAM Structure
int CurrentSlot = 0;
void Clear_OAM() {
	CurrentSlot = 0;
}

void Create_OAMTile(uint_fast8_t tile, uint_fast8_t size, int_fast16_t posx, int_fast16_t posy, uint_fast16_t props, uint_fast8_t rot = 0, uint_fast8_t sx = 0x2, uint_fast8_t sy = 0x20) {
	if (CurrentSlot >= OAM_Tiles.size()) {
		//Create a new tile if we ran out of OAM slots..
		OAM_Tiles.push_back(OAMTile{ posx, posy, tile, size, props, rot, sx, sy });
	}
	else {
		//just reuse an old tile. This is way faster...
		OAMTile& T = OAM_Tiles[CurrentSlot];
		T.tile = tile;
		T.bsize = size;
		T.pos_x = posx;
		T.pos_y = posy;
		T.props = props;
		T.rotation = rot;
		T.scale_x = sx;
		T.scale_y = sy;
	}
	CurrentSlot++;
}

#ifndef DISABLE_NETWORK
//These are used for syncing OAM, this compresses the data on the serverside and puts it in the packet.
void CompressOAM_Server() {
	int CurrIndex = 0;
	uint_fast16_t OAM_Tiles_sent = 0;
	uint_fast16_t OAM_Tiles_amount_compressed = uint_fast16_t(min(0xFFFF, OAM_Tiles.size()));
	while(OAM_Tiles_sent < OAM_Tiles_amount_compressed) {
		//We don't want to pass the assigned buffer.
		if (CurrIndex >= 0xFFF0) { break; }
		OAMTile& T = OAM_Tiles[OAM_Tiles_sent]; uint_fast16_t X = uint_fast16_t(T.pos_x); uint_fast16_t Y = uint_fast16_t(T.pos_y);
		RAM_compressed[CurrIndex++] = T.tile;
		RAM_compressed[CurrIndex++] = T.bsize;
		RAM_compressed[CurrIndex++] = X; RAM_compressed[CurrIndex++] = X >> 8;
		RAM_compressed[CurrIndex++] = Y; RAM_compressed[CurrIndex++] = Y >> 8;
		RAM_compressed[CurrIndex++] = T.rotation;
		RAM_compressed[CurrIndex++] = T.props;
		RAM_compressed[CurrIndex++] = T.props >> 8;
		if (T.props & 0x2000) { //Scaling enabled
			RAM_compressed[CurrIndex++] = T.scale_x;
			RAM_compressed[CurrIndex++] = T.scale_y;
		}
		OAM_Tiles_sent++;
	}

	//LZ4 Compression
	int OAM_data_compressed_size = LZ4_compress_fast((char*)RAM_compressed, (char*)OAM_data_comp, CurrIndex, 0x10000, 1);
	CurrentPacket << OAM_data_compressed_size;
	CurrentPacket << OAM_Tiles_sent;
	CurrentPacket.append(OAM_data_comp, OAM_data_compressed_size);
}

//This decompresses the data on the client side.
void DecompressOAM_Player() {
	//Decompress data first
	uint_fast16_t OAM_Tiles_amount_compressed;
	int CompressedSize;
	CurrentPacket >> CompressedSize;
	CurrentPacket >> OAM_Tiles_amount_compressed;
	for (int i = 0; i < CompressedSize; i++) {
		CurrentPacket >> RAM_compressed[i];
	}
	LZ4_decompress_safe((char*)RAM_compressed, (char*)OAM_data_comp, CompressedSize, 0x10000);

	//Fetch data
	int CurrIndex = 0;
	if (OAM_Tiles.size() != OAM_Tiles_amount_compressed) {
		OAM_Tiles.resize(OAM_Tiles_amount_compressed);
	}
	for (uint_fast16_t i = 0; i < OAM_Tiles_amount_compressed; i++) {
		OAMTile& T = OAM_Tiles[i];
		T.tile = OAM_data_comp[CurrIndex];
		T.bsize = OAM_data_comp[CurrIndex + 1];
		T.pos_x = OAM_data_comp[CurrIndex + 2] + Sint8(OAM_data_comp[CurrIndex + 3]) * 256;
		T.pos_y = OAM_data_comp[CurrIndex + 4] + Sint8(OAM_data_comp[CurrIndex + 5]) * 256;
		T.rotation = OAM_data_comp[CurrIndex + 6];
		T.props = OAM_data_comp[CurrIndex + 7] + (OAM_data_comp[CurrIndex + 8] << 8);
		if (T.props & 0x2000) {
			T.scale_x = OAM_data_comp[CurrIndex + 9];
			T.scale_y = OAM_data_comp[CurrIndex + 10];
			CurrIndex += 2;
		}
		CurrIndex += 9;
	}
}
#endif

//Resize OAM buffer if it's complete.
void Finish_OAM() {
	if (CurrentSlot != OAM_Tiles.size()) {
		OAM_Tiles.resize(CurrentSlot);
	}
}
