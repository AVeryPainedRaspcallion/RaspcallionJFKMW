#pragma once

void init_input() {
	if (controller < 0) {
		return;
	}
	if (SDL_NumJoysticks() < 1) {
		cout << cyan << "[SDL] No controllers are plugged in." << endl;
	}
	else {
		cout << cyan << "[SDL] There's " << SDL_NumJoysticks() << " controllers plugged in, and " << SDL_NumHaptics() << " haptics detected for every controller! Trying to open controller " << controller << " and haptic device " << haptic << "." << endl;
		//Load joystick 1 and 2
		for (int i = 0; i < 4; i++) {
			gGameController[i] = SDL_GameControllerOpen(controller + i);
			if (gGameController[i] == NULL) {
				cout << cyan << "[SDL] Controller " << (controller + i) << " error : " << SDL_GetError() << endl;
			}
			else {
				cout << cyan << "[SDL] Controller " << (controller + i) << " is plugged in." << endl;
			}
		}
		//Load haptic
		if (haptic >= 0)
		{
			haptic_device = SDL_HapticOpen(haptic);
			if (haptic_device == NULL) {
				cout << cyan << "[SDL] Haptic " << haptic << " error : " << SDL_GetError() << endl;
			}
			else {
				cout << cyan << "[SDL] Haptic " << haptic << " has been connected." << endl;
				if (SDL_HapticRumbleInit(haptic_device) != 0) {
					cout << cyan << "[SDL] Haptic " << haptic << " rumble init error : " << SDL_GetError() << endl;
				}
			}
		}
	}
}

void FetchControllerInput(int ControllerPicked = 0) {
	if ((ControllerPicked >= 4 || ControllerPicked < 0) || !gGameController[ControllerPicked]) {
		for (uint_fast8_t i = 0; i < 10; i++) {
			BUTTONS_GAMEPAD[i] = false;
		}
		return;
	}
	BUTTONS_GAMEPAD[0] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_DPAD_UP) || (SDL_GameControllerGetAxis(gGameController[ControllerPicked], SDL_CONTROLLER_AXIS_LEFTY) < -24576);
	BUTTONS_GAMEPAD[1] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_DPAD_RIGHT) || (SDL_GameControllerGetAxis(gGameController[ControllerPicked], SDL_CONTROLLER_AXIS_LEFTX) > 24576);
	BUTTONS_GAMEPAD[2] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_DPAD_DOWN) || (SDL_GameControllerGetAxis(gGameController[ControllerPicked], SDL_CONTROLLER_AXIS_LEFTY) > 24576);
	BUTTONS_GAMEPAD[3] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_DPAD_LEFT) || (SDL_GameControllerGetAxis(gGameController[ControllerPicked], SDL_CONTROLLER_AXIS_LEFTX) < -24576);

	BUTTONS_GAMEPAD[4] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_A);
	BUTTONS_GAMEPAD[5] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_B);
	BUTTONS_GAMEPAD[6] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_X);
	BUTTONS_GAMEPAD[7] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_Y);

	BUTTONS_GAMEPAD[8] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_START);
	BUTTONS_GAMEPAD[9] = SDL_GameControllerGetButton(gGameController[ControllerPicked], SDL_CONTROLLER_BUTTON_BACK);
}

void vibrate_controller(double intensity, int time) {
	if (haptic_device != NULL) {
		SDL_HapticRumblePlay(haptic_device, float(min(1.0, intensity)), time);
	}
}

void check_input() {
	state = SDL_GetKeyboardState(NULL);

	if (state[SDL_SCANCODE_ESCAPE] && (gamemode == GAMEMODE_MAIN || gamemode == GAMEMODE_OVERWORLD)) {
		if (!networking) {
			quit = true;
		}
	}
	if (use_mouse || !gGameController[0]) {
		//If we're using keyboard we're also gonna use mouse I guess
		SDL_GetMouseState(&mouse_x, &mouse_y);

		mouse_x -= sp_offset_x; mouse_y -= sp_offset_y;

		mouse_x = int(double(mouse_x) / scale);
		mouse_y = int(double(mouse_y) / scale);

		mouse_x = min(INTERNAL_RESOLUTION_X, max(0, mouse_x));
		mouse_y = min(INTERNAL_RESOLUTION_Y, max(0, mouse_y));

		uint_fast32_t m_state = SDL_GetMouseState(NULL, NULL);
		mouse_down = m_state & SDL_BUTTON(SDL_BUTTON_LEFT);
		mouse_down_r = m_state & SDL_BUTTON(SDL_BUTTON_RIGHT);
	}
	else {
		//If we're using a controller we obviously aren't gonna use the controller and mouse at the same time, so..
		int_fast16_t AxisX = SDL_GameControllerGetAxis(gGameController[0], SDL_CONTROLLER_AXIS_RIGHTX);
		int_fast16_t AxisY = SDL_GameControllerGetAxis(gGameController[0], SDL_CONTROLLER_AXIS_RIGHTY);
		controller_mouse_x += double(abs(AxisX) > 4096 ? AxisX : 0) / 6000.0;
		controller_mouse_y += double(abs(AxisY) > 4096 ? AxisY : 0) / 6000.0;

		controller_mouse_x = min(INTERNAL_RESOLUTION_X, max(0.0, controller_mouse_x));
		controller_mouse_y = min(INTERNAL_RESOLUTION_Y, max(0.0, controller_mouse_y));

		mouse_down = SDL_GameControllerGetAxis(gGameController[0], SDL_CONTROLLER_AXIS_TRIGGERRIGHT) > 24576;
		mouse_down_r = SDL_GameControllerGetAxis(gGameController[0], SDL_CONTROLLER_AXIS_TRIGGERLEFT) > 24576;

		mouse_w_up = false;
		mouse_w_down = false;

		bool stat = SDL_GameControllerGetButton(gGameController[0], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
		if (stat != left_st_pr) {
			left_st_pr = stat;
			if (stat) {
				mouse_w_up = true;
			}
		}

		stat = SDL_GameControllerGetButton(gGameController[0], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
		if (stat != right_st_pr) {
			right_st_pr = stat;
			if (stat) {
				mouse_w_down = true;
			}
		}

		mouse_x = int(controller_mouse_x);
		mouse_y = int(controller_mouse_y);

	}

	if (special_input_disabled) {
		return;
	}

	if (!Chatting && RAM[0x3F11] == 0) {
		if (local_multiplayer) {
			for (uint_fast8_t i = 0; i < 10; i++) {
				BUTTONS_GAMEPAD[i] = false;
			}
		}
		else {
			FetchControllerInput(controller);
		}
		s_pad[button_y] = (state[input_settings[0]] || state[input_settings[3]]) || (BUTTONS_GAMEPAD[7] || BUTTONS_GAMEPAD[6]);
		s_pad[button_b] = state[input_settings[1]] || BUTTONS_GAMEPAD[4];
		s_pad[button_a] = state[input_settings[2]] || BUTTONS_GAMEPAD[5];
		s_pad[button_left] = state[input_settings[4]] || BUTTONS_GAMEPAD[3];
		s_pad[button_right] = state[input_settings[5]] || BUTTONS_GAMEPAD[1];
		s_pad[button_down] = state[input_settings[6]] || BUTTONS_GAMEPAD[2];
		s_pad[button_up] = state[input_settings[7]] || BUTTONS_GAMEPAD[0];
		s_pad[button_start] = state[input_settings[8]] || BUTTONS_GAMEPAD[9];

		for (uint_fast8_t i = 0; i < total_inputs; i++) {
			pad_p[i] = false;
			if (pad_s[i] != s_pad[i]) {
				pad_s[i] = s_pad[i];
				if (s_pad[i]) {
					pad_p[i] = true;
				}
			}
		}
	}
	else {
		for (uint_fast8_t i = 0; i < total_inputs; i++) {
			pad_p[i] = false;
			pad_s[i] = false;
			s_pad[i] = false;
		}
	}
}

#ifdef OTHER_INPUT_METHOD
uint_fast32_t next_type = 0;
auto typemap = new unordered_map<int, uint_fast32_t>();
#endif

#ifdef _WIN32
bool KeyStates[0x100];
#endif

bool getKey(int what_want)
{
#ifdef _WIN32
	bool stat;
	if (what_want == 0x08) {
		stat = GetAsyncKeyState(what_want) & 0x7FFF;
	}
	else {
		stat = GetKeyState(what_want) & 0x80;
	}
	if (!(SDL_GetWindowFlags(win) & SDL_WINDOW_INPUT_FOCUS)) {
		return false;
	}
	if (KeyStates[what_want] != stat) {
		KeyStates[what_want] = stat;
		if (stat) {
			return true;
		}
	}
	return false;
#endif

#ifdef OTHER_INPUT_METHOD
	if (global_frame_counter > next_type &&
		((what_want >= 0x30 && what_want <= 0x46) || what_want == 0xBE ||
			what_want == 0x08) &&
		(typemap->find(what_want) == typemap->end() ||
			typemap->at(what_want) < global_frame_counter)) {
		next_type = global_frame_counter + 10;
		auto r = false;
		switch (what_want)
		{
		case 0x30:
			r = state[SDL_SCANCODE_0];
			break;
		case 0x31:
			r = state[SDL_SCANCODE_1];
			break;
		case 0x32:
			r = state[SDL_SCANCODE_2];
			break;
		case 0x33:
			r = state[SDL_SCANCODE_3];
			break;
		case 0x34:
			r = state[SDL_SCANCODE_4];
			break;
		case 0x35:
			r = state[SDL_SCANCODE_5];
			break;
		case 0x36:
			r = state[SDL_SCANCODE_6];
			break;
		case 0x37:
			r = state[SDL_SCANCODE_7];
			break;
		case 0x38:
			r = state[SDL_SCANCODE_8];
			break;
		case 0x39:
			r = state[SDL_SCANCODE_9];
			break;
		case 0x41:
			r = state[SDL_SCANCODE_A];
			break;
		case 0x42:
			r = state[SDL_SCANCODE_B];
			break;
		case 0x43:
			r = state[SDL_SCANCODE_C];
			break;
		case 0x44:
			r = state[SDL_SCANCODE_D];
			break;
		case 0x45:
			r = state[SDL_SCANCODE_E];
			break;
		case 0x46:
			r = state[SDL_SCANCODE_F];
			break;
		case 0xBE:
			r = state[SDL_SCANCODE_PERIOD];
			break;
		case 0x08:
			r = state[SDL_SCANCODE_BACKSPACE];
			break;
		}
		if (!r) {
			next_type = 0;
		}
		else if (what_want != 0x08) {
			typemap->erase(what_want);
			typemap->emplace(what_want, global_frame_counter + 30);
		}
		return r;
	}
#endif
	return false;
}
