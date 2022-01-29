#pragma once

void handleTransitions() {
	//Transitions
	uint_fast8_t transition_type = RAM[0x1493] > 0 ? 3 : RAM[0x3F11];
	uint_fast8_t transition_speed = transition_type == 3 ? 7 : 1;
	//Transition speed
	if (!(global_frame_counter & transition_speed) && !isClient)
	{
		uint_fast8_t mosaic_val = RAM[0x3F10] >> 4;
		uint_fast8_t bright_val = RAM[0x3F10] & 0xF;
		if (transition_type > 0 && transition_type != 2) {
			if (bright_val < 0xF) { bright_val++; }
			else {
				//these 2 are special transition modes since they interact with the OW
				if (transition_type == 5) { overworld.Initialize(); }
				if (transition_type == 6) { overworld.Warp(); }
				RAM[0x3F11] = 0;
			}
		}
		if (transition_type == 2) {
			if (bright_val > 0) { bright_val--; }
			if (mosaic_val > 0) { mosaic_val--; }
			if (mosaic_val == 0 && bright_val == 0) {
				RAM[0x9D] = 1;
				RAM[0x3F11] = 0;
			}
		}
		if (transition_type == 1 || transition_type == 4) {
			RAM[0x9D] = 0;
			if (mosaic_val < 0xF) { mosaic_val++; }
			else {
				RAM[0x3F11] = 0;
				if (transition_type == 4) {
					RAM[0x1DFB] = 0;
					load_level3f08();
				}
			}
		}
		RAM[0x3F10] = bright_val + (mosaic_val << 4);
	}
}