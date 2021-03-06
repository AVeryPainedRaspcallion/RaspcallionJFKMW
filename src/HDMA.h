#pragma once

void ResetHDMA()
{
	//Disable all HDMA channels.
	RAM[0x420C] = 0;
}

#ifndef DISABLE_NETWORK
void sendHDMAnet() {
	CurrentPacket << RAM[0x420C];
	CurrentPacket << RAM[0x420B];

	//Compress all data in actually enabled HDMA channels.
	for (uint_fast8_t c = 0; c < 8; c++)
	{
		uint_fast8_t channel = c << 4;
		bool enabled = (RAM[0x420C] >> c) & 1;

		if (enabled) //This HDMA channel is enabled
		{
			//Send mode and reg
			CurrentPacket << RAM[0x4300 + channel];
			CurrentPacket << RAM[0x4301 + channel];

			//Send data size?
			CurrentPacket << RAM[0x4302 + channel];
			CurrentPacket << RAM[0x4303 + channel];
			CurrentPacket << RAM[0x4304 + channel];

			//Compress
			uint_fast32_t bank = RAM[0x4302 + channel] + (RAM[0x4303 + channel] << 8) + (RAM[0x4304 + channel] << 16);
			int nCompressedSize = LZ4_compress_fast((char*)&RAM[bank], (char*)RAM_compressed, hdma_size[c] + 1, RAM_Size, 1);
			CurrentPacket << nCompressedSize;
			CurrentPacket.append(RAM_compressed, nCompressedSize);
		}
	}
}

void decompressHDMAnet() {
	CurrentPacket >> RAM[0x420C];
	CurrentPacket >> RAM[0x420B];

	//Decompress all data in actually enabled HDMA channels.
	for (uint_fast8_t c = 0; c < 8; c++)
	{
		uint_fast8_t channel = c << 4;
		bool enabled = (RAM[0x420C] >> c) & 1;
		if (enabled) //This HDMA channel is enabled
		{
			//Receive mode and reg
			CurrentPacket >> RAM[0x4300 + channel];
			CurrentPacket >> RAM[0x4301 + channel];

			//Receive data size?
			CurrentPacket >> RAM[0x4302 + channel];
			CurrentPacket >> RAM[0x4303 + channel];
			CurrentPacket >> RAM[0x4304 + channel];

			//Uncompress
			uint_fast32_t bank = RAM[0x4302 + channel] + (RAM[0x4303 + channel] << 8) + (RAM[0x4304 + channel] << 16);
			int CompressedSize; CurrentPacket >> CompressedSize;
			for (int i = 0; i < CompressedSize; i++) {
				CurrentPacket >> RAM_compressed[i];
			}
			LZ4_decompress_safe((char*)RAM_compressed, (char*)&RAM[bank], CompressedSize, RAM_Size);
		}
	}
}
#endif

void ProcessHDMA()
{
	//Reset Horizontal Tables
	for (int i = 0; i < 4; i++) {
		hdmaModeEnabled[i] = false;
	}
	memset(hdmaLineData, 0, sizeof(hdmaLineData));

	for (uint_fast8_t c = 0; c < 8; c++)
	{
		hdma_size[c] = 0;

		uint_fast8_t channel = c << 4;
		if ((RAM[0x420C] >> c) & 1) { //This HDMA channel is enabled
			uint_fast8_t mode = RAM[0x4300 + channel];
			uint_fast8_t size = mode + 1;
			uint_fast8_t reg = RAM[0x4301 + channel];
			uint_fast32_t bank = RAM[0x4302 + channel] + (RAM[0x4303 + channel] << 8) + (RAM[0x4304 + channel] << 16);
			uint_fast16_t i = 0;
			uint_fast16_t scanline = 0;

			if (reg >= 0x0D && reg <= 0x10) { hdmaModeEnabled[reg - 0xD] = true; }
			if (reg >= 0x26 && reg <= 0x29) { hdmaModeEnabled[reg - 0x22] = true; }
			if (reg == 0x32) { hdmaModeEnabled[HDMA_FIXEDCOLORDATA] = true; }
			while (true) {
				uint_fast16_t scanlines = RAM[bank + i];
				if (scanlines == 0) {
					hdma_size[c]++;
					break;
				}
				else {
					for (uint_fast16_t l = 0; l < scanlines; l++) {
						int value = RAM[bank + 1 + i] + (size > 2 ? (RAM[bank + 2 + i] << 8) : 0);
						if (value >= 0x8000) { value = -(0x10000 - value); }
						if (scanline < 512) {
							if (reg >= 0x0D && reg <= 0x10) { hdmaLineData[scanline][reg - 0xD] = int_fast16_t(value); }
							if (reg >= 0x26 && reg <= 0x29) { hdmaLineData[scanline][reg - 0x22] = int_fast16_t(value); }
							if (reg == 0x32) { hdmaLineData[scanline][HDMA_FIXEDCOLORDATA] = int_fast16_t(value & 0xFF); }
						}
						scanline++;
					}
					i += size;
					hdma_size[c] += size;
				}
			}

		}
	}
}