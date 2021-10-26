#pragma once
class VRAM_DMA {
public:
	uint_fast16_t size; uint_fast32_t part;
	uint_fast8_t* data;
	VRAM_DMA(uint_fast16_t s, uint_fast32_t p) {
		size = s;
		part = p;
		data = new uint_fast8_t[size];
	}
};
vector<VRAM_DMA> DMAsProcessed;

void ResetDMA() {
	//Disable all DMA channels.
	RAM[0x420B] = 0;
}

void DMAStartFrame() {
	if (DMAsProcessed.size() > 0) {
		for (int i = 0; i < DMAsProcessed.size(); i++) {
			delete DMAsProcessed[i].data;
		}
		DMAsProcessed.clear();
	}
}

#ifndef DISABLE_NETWORK
void sendDMANet() {
	uint_fast16_t all_dmas = uint_fast16_t(DMAsProcessed.size());
	CurrentPacket << all_dmas;
	for (uint_fast16_t i = 0; i < all_dmas; i++)
	{
		CurrentPacket << DMAsProcessed[i].part;

		//LZ4 Compression
		int nCompressedSize = LZ4_compress_fast((char*)DMAsProcessed[i].data, (char*)RAM_compressed, DMAsProcessed[i].size, RAM_Size, 1);
		CurrentPacket << nCompressedSize;
		CurrentPacket.append(RAM_compressed, nCompressedSize);
	}
}

void decompressDMAnet() {
	uint_fast16_t all_dmas;
	CurrentPacket >> all_dmas;
	for (uint_fast16_t i = 0; i < all_dmas; i++)
	{
		uint_fast32_t dma_part; CurrentPacket >> dma_part;

		//LZ4 Compression
		int CompressedSize = 0; CurrentPacket >> CompressedSize;
		for (int i = 0; i < CompressedSize; i++) {
			CurrentPacket >> RAM_compressed[i];
		}
		LZ4_decompress_safe((char*)RAM_compressed, (char*)&RAM[dma_part], CompressedSize, RAM_Size);
		if (dma_part >= 0x2C000) {
			need_preload_sprites = true;
		}
	}
}
#endif

void ProcessDMA() {
	for (uint_fast8_t c = 0; c < 8; c++) {
		uint_fast8_t channel = c << 4;
		if ((RAM[0x420B] >> c) & 1) { //This DMA channel is enabled
			uint_fast8_t mode = RAM[0x4300 + channel];
			uint_fast8_t reg = RAM[0x4301 + channel];
			uint_fast32_t bank = RAM[0x4302 + channel] + (RAM[0x4303 + channel] << 8) + (RAM[0x4304 + channel] << 16);
			bank = (bank - 0x7E0000) & 0x1FFFF;
			uint_fast16_t size = RAM[0x4305 + channel] + (RAM[0x4306 + channel] << 8);
			//43x8 43x9 (unused?) are now used as $2116, since $2116 is used by sprite ram in the engine
			uint_fast32_t part = RAM[0x4308 + channel] + (RAM[0x4309 + channel] << 8);

			//DMA Vram $2118 and $2119
			if (mode == 1 && reg == 0x18) {
				memcpy(&RAM[0x20000 + (part << 1)], &RAM[bank], size);

				//create dma processed object
				VRAM_DMA newDma = VRAM_DMA(size, 0x20000 + (part << 1));
				memcpy(newDma.data, &RAM[bank], size);
				DMAsProcessed.push_back(newDma);
			}

			//RAM DMA
			if (mode == 0 && reg == 0x80) {
				uint_fast32_t destination = ((RAM[0x4281] + (RAM[0x4282] << 8) + (RAM[0x4283] << 16)) - 0x7E0000) & 0x1FFFF;
				memcpy(&RAM[destination], &RAM[bank], size);

				//create dma processed object
				VRAM_DMA newDma = VRAM_DMA(size, destination);
				memcpy(newDma.data, &RAM[bank], size);
				DMAsProcessed.push_back(newDma);
			}
		}
	}
	RAM[0x420B] = 0;
}

//CC-DMA implementation. TO-DO: Cleanup and recode
void CCDMA_Bitmap4BPP(int src, int dest, int width, int height, bool packed) {
	int odest = dest;
	memset(&RAM[dest], 0, width * height * 32);
	uint_fast8_t row = 0;
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			for (int bp = 0; bp < 4; bp++) {
				for (int r = 0; r < 8; r++) {
					if (!packed) {
						row = ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width)] >> bp) & 1) << 7;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 1] >> bp) & 1) << 6;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 2] >> bp) & 1) << 5;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 3] >> bp) & 1) << 4;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 4] >> bp) & 1) << 3;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 5] >> bp) & 1) << 2;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 6] >> bp) & 1) << 1;
						row |= ((RAM[src + (x * 8) + (y * 64 * width) + (r * 8 * width) + 7] >> bp) & 1);
					}
					else {
						row = ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width)] >> (bp + 4)) & 1) << 7;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width)] >> bp) & 1) << 6;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width) + 1] >> (bp + 4)) & 1) << 5;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width) + 1] >> bp) & 1) << 4;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width) + 2] >> (bp + 4)) & 1) << 3;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width) + 2] >> bp) & 1) << 2;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width) + 3] >> (bp + 4)) & 1) << 1;
						row |= ((RAM[src + (x * 4) + (y * 64 * width) + (r * 8 * width) + 3] >> bp) & 1);
					}
					RAM[dest + ((bp >> 1) * 16) + (bp & 1) + (r * 2)] = row;
				}
			}
			dest += 32;
		}
	}
	if (odest >= 0x20000) {
		//create dma processed object
		VRAM_DMA newDma = VRAM_DMA(width * height * 32, odest);
		memcpy(newDma.data, &RAM[odest], newDma.size);
		DMAsProcessed.push_back(newDma);
		if (odest >= 0x2C000) {
			need_preload_sprites = true;
		}
	}
}