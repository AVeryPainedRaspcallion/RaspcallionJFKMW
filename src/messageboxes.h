#pragma once
/*
	0x1426 Message box trigger. 0 = None, 1 = Message 1, 2 = Message 2, and so on. Only up to 15 messages per level.
	0x1B88 Black window mode ----BBBM
	0x1B89 Black window size
*/

void ClearMessageBoxArea() {
	if ((RAM[0x1B88] >> 1) == 0) { //Messagebox
		for (int x = 0; x < 18; x++) {
			for (int y = 0; y < 9; y++) {
				RAM[VRAM_Convert(0xB98E) + (x << 1) + (y << 6)] = 0xFF;
				RAM[VRAM_Convert(0xB98F) + (x << 1) + (y << 6)] = 6;
			}
		}
	}
	else { //Retry prompt
		for (int x = 11; x <= 23; x++) {
			for (int y = 14; y <= 17; y++) {
				RAM[VRAM_Convert(0xB800) + (x << 1) + (y << 6)] = 0xFF;
				RAM[VRAM_Convert(0xB801) + (x << 1) + (y << 6)] = 6;
			}
		}
	}
}

void FadeMessageBox(int speed) {
	int actual_loops = (RAM[0x1B88] >> 1) ? 18 : 12;
	for (int i = 0; i < actual_loops; i++) {
		if (RAM[0x1B89] != (speed > 0 ? 255 : 0)) { RAM[0x1B89] += speed; }
		if (RAM[0x1B89] == 254) { ClearMessageBoxArea(); }
	}
}

void ProcessMessageBoxes() {
	//Display MessageBox
	if (RAM[0x1426] > 0 && RAM[0x1B88] != 1) {
		RAM[0x1B88] = 1;
		MessageBoxTimer = 60 * 4;
	}

	//Handle Size Changing
	if (RAM[0x1B88] & 1) {
		RAM[0x9D] = 0;
		FadeMessageBox(1);

		if ((RAM[0x1B88] >> 1) == 0) { //Messagebox
			if (RAM[0x1B89] == 255 && RAM[0x1426] > 0 && RAM[0x1426] < 17) {
				//Fully opened
				string MSG_Text = Messages[RAM[0x1426] - 1];
				for (int i = 0; i < MSG_Text.length(); i++)
				{
					uint_fast8_t new_l = char_to_smw(MSG_Text.at(i));
					RAM[VRAM_Convert(0xB98E) + ((i % 18) << 1) + ((i / 18) << 6)] = new_l;
					RAM[VRAM_Convert(0xB98F) + ((i % 18) << 1) + ((i / 18) << 6)] = 6;
				}
			}
		}
		else //Retry prompt
		{
			if (((pad_p[button_b] || pad_p[button_a]) || pad_p[button_y]) && RAM[0x1B89] <= 253) {
				RAM[0x1B89] = 254;
			}
			if (RAM[0x1B89] == 255) { 
				string MSG_Text = "  RETRY\n\n  EXIT";
				int x = 11;
				int y = 14;
				for (int i = 0; i < MSG_Text.length(); i++) {
					if (MSG_Text.at(i) == '\n') { y++; x = 10; }
					uint_fast8_t new_l = char_to_smw(MSG_Text.at(i));
					RAM[VRAM_Convert(0xB800) + (x << 1) + (y << 6)] = new_l;
					RAM[VRAM_Convert(0xB801) + (x << 1) + (y << 6)] = 6;
					x++;
				}

				y = !doRetry ? 16 : 14;
				if ((global_frame_counter & 0x1F) > 0x7) {
					RAM[VRAM_Convert(0xB800) + 22 + (y << 6)] = 0x7F;
					RAM[VRAM_Convert(0xB801) + 22 + (y << 6)] = 7;
				}

				if (pad_p[button_down] || pad_p[button_up]) {
					pad_p[button_down] = false;
					pad_p[button_up] = false;
					doRetry = !doRetry;
					RAM[0x1DFC] = 6;
				}
			}
		}

		if (RAM[0x1B89] == 255) {
			if (!networking) { //For singleplayer
				if ((pad_p[button_b] || pad_p[button_a]) || pad_p[button_y])
				{
					if ((RAM[0x1B88] >> 1) != 0) { //If it be a retry prompt
						RAM[0x1B88] = 2;
						RAM[0x1DFC] = 1;
						retryPromptOpen = false;
						if (!doRetry) {
							RAM[0x1DFB] = 9;
						}
					}
					else {
						RAM[0x1B88] = 0;
						RAM[0x1426] = 0;
					}
				}
			}
			else { //Multiplayer messageboxes are different
				MessageBoxTimer--;
				if (MessageBoxTimer <= 0) {
					RAM[0x1B88] = 0;
					RAM[0x1426] = 0;
				}
			}
		}
	}
	else {
		if (RAM[0x1B89] > 0) {
			FadeMessageBox(-1);
			if (RAM[0x1B89] == 0) {
				if ((RAM[0x1B88] >> 1) == 0) { //Messagebox
					RAM[0x9D] = 1;
				}
			}
		}
	}
}