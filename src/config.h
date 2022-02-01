#pragma once

//TO-DO: Cleanup
void load_configuration() {
	ifstream cFile("game_configuration.cfg");
	if (cFile.is_open()) {
		string line;
		while (getline(cFile, line)) {
			line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
			if (line[0] == '#' || line.empty()) { continue; }
			auto delimiterPos = line.find("=");
			string name = line.substr(0, delimiterPos);
			string value = line.substr(delimiterPos + 1);
			cout << cyan << "[CONFIG] Loading " << name << " = " << value << endl;
			if (name == "default_modpack") { LoadPack(value); }
			if (name == "resolution_x") { resolution_x = stoi(value); }
			if (name == "resolution_y") { resolution_y = stoi(value); }
			if (name == "fullscreen") { fullscreen = value == "true"; }
			if (name == "borderless_fullscreen") { borderless_fullscreen = value == "true" ? !fullscreen : false; }
			if (name == "smooth_camera") { smooth_camera = value == "true"; }
			if (name == "smooth_camera_speed") { smooth_camera_speed = double(stoi(value)); }
			if (name == "scale") { scale = stoi(value); if (scale > 0) { forced_scale = true; }}
			if (name == "audio_multichannel") { multichannel_sounds = value == "true"; }
			if (name == "integer_scaling") { integer_scaling = value == "true"; }
			if (name == "automatic_fps_cap") { automatic_fps_cap = value == "true"; }
			if (name == "spc_buffer_size") { spc_buffer_size = stoi(value); }
			if (name == "spc_interpolation") { spc_interpolation = uint_fast8_t(stoi(value)); }
			if (name == "sample_rate") { ogg_sample_rate = stoi(value); }
			if (name == "sfx_volume") { sfx_volume = max(0, min(MIX_MAX_VOLUME, stoi(value))); }
			if (name == "music_volume") { music_volume = max(0, min(MIX_MAX_VOLUME, stoi(value))); }
			if (name == "local_multiplayer") { local_multiplayer = value == "true"; pvp = !local_multiplayer; }
			if (name == "gamma_ramp" && value == "false") {
				for (int i = 0; i < 32; i++) {
					gammaRamp[i] = i << 3;
				}
			}
			if (name == "username") { username = value.substr(0, player_name_size); }
			if (name == "skin") { my_skin = uint_fast8_t(stoi(value)); }
			if (name == "port") { PORT = stoi(value); }
			if (value == "LeftShift") value = "Left Shift";
			if (value == "RightShift") value = "Right Shift";
			if (value == "LeftAlt") value = "Left Alt";
			if (value == "RightAlt") value = "Right Alt";
			if (value == "LeftCtrl") value = "Left Ctrl";
			const char *v = value.c_str();
			for (int i = 0; i < 18; i++) {
				if (name == button_configurations[i]) { input_settings[i] = SDL_GetScancodeFromName(value.c_str()); }
			}
			if (name == "debug") { debugging_enabled = value == "true"; }
			if (name == "use_mouse") { use_mouse = value == "true"; }
			if (name == "joystick_num") { controller = stoi(value); }
			if (name == "haptic_num") { haptic = stoi(value); }
			if (name == "discord_webhook") { discord_webhook = value; }
			if (name == "midi_patchset") { midi_patchset = value; }
			if (name == "use_retry") { useRetry = value == "true"; }
		}
	}
	cFile.close();
}