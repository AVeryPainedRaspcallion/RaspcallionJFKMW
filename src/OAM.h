#pragma once

//OAM Structure
struct OAMTile {
	uint_fast8_t tile = 0;
	uint_fast8_t size = 0;
	int_fast16_t posx = 0;
	int_fast16_t posy = 0;
	uint_fast16_t props = 0;
	uint_fast8_t rot = 0;
};

vector<OAMTile> OAM_Tiles;
int CurrentSlot = 0;
void Clear_OAM() {
	CurrentSlot = 0;
}

void Create_OAMTile(uint_fast8_t tile, uint_fast8_t size, int_fast16_t posx, int_fast16_t posy, uint_fast16_t props, uint_fast8_t rot) {
	if (CurrentSlot >= OAM_Tiles.size()) {
		//Create a new tile if we ran out of OAM slots..
		OAM_Tiles.push_back(OAMTile{ tile, size, posx, posy, props, rot });
	}
	else {
		//just reuse an old tile. This is way faster...
		OAMTile& T = OAM_Tiles[CurrentSlot];
		T.tile = tile;
		T.size = size;
		T.posx = posx;
		T.posy = posy;
		T.props = props;
		T.rot = rot;
	}
	CurrentSlot++;
}

#ifndef DISABLE_NETWORK
//These are used for syncing OAM, this compresses the data on the serverside and puts it in the packet.
void CompressOAM_Server() {
	int CurrIndex = 0;
	uint_fast16_t OAM_Tiles_amount_compressed = uint_fast16_t(min(7280, OAM_Tiles.size()));
	for (uint_fast16_t i = 0; i < OAM_Tiles_amount_compressed; i++) {
		OAMTile& T = OAM_Tiles[i]; uint_fast16_t X = uint_fast16_t(T.posx); uint_fast16_t Y = uint_fast16_t(T.posy);
		RAM_compressed[CurrIndex++] = T.tile;
		RAM_compressed[CurrIndex++] = T.size;
		RAM_compressed[CurrIndex++] = X;
		RAM_compressed[CurrIndex++] = X >> 8;
		RAM_compressed[CurrIndex++] = Y;
		RAM_compressed[CurrIndex++] = Y >> 8;
		RAM_compressed[CurrIndex++] = T.props;
		RAM_compressed[CurrIndex++] = T.props >> 8;
		RAM_compressed[CurrIndex++] = T.rot;
	}

	//LZ4 Compression
	int OAM_data_compressed_size = LZ4_compress_fast((char*)RAM_compressed, (char*)OAM_data_comp, OAM_Tiles_amount_compressed * 9, 0x10000, 1);
	CurrentPacket << OAM_data_compressed_size;
	CurrentPacket << OAM_Tiles_amount_compressed;
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
		T.size = OAM_data_comp[CurrIndex+1];
		T.posx = OAM_data_comp[CurrIndex + 2] + Sint8(OAM_data_comp[CurrIndex + 3]) * 256;
		T.posy = OAM_data_comp[CurrIndex + 4] + Sint8(OAM_data_comp[CurrIndex + 5]) * 256;
		T.props = OAM_data_comp[CurrIndex + 6] + (OAM_data_comp[CurrIndex + 7] << 8);
		T.rot = OAM_data_comp[CurrIndex + 8];
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