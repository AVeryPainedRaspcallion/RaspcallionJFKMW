#pragma once

//The music that will be played
Mix_Music* music = NULL;

//The sound effects that will be used
Mix_Chunk* sfxPorts[0x300];
uint_fast16_t sound_table[4] = { 0x1DF9, 0x1DFC, 0x1DFA, 0x1DFD };
uint_fast8_t old_1dfb = 0xFF;

//Well, this is a neat storage format for sure.
#define MUSIC_SPC 0
#define MUSIC_OGG 1
#define MUSIC_MID 2
uint_fast8_t music_type = MUSIC_SPC;
uint_fast8_t new_music_type = MUSIC_SPC;
string Music_Check_Types[3] = { ".spc", ".ogg", ".mid" };

//Per-level sounds special class. Only replaces 0xF0 to 0xFF
class SpecialLevelSpecificSoundChunk {
public:
	//Data stuff
	Mix_Chunk* chunkPort = NULL;
	uint_fast8_t* snd_data_raw = nullptr;
	int snd_data_size;
	uint_fast8_t ID;
	bool databusy = false;

	//Delete Old Data from ram just incase
	void Unload() {
		if (snd_data_raw != nullptr) {
			delete[] snd_data_raw; snd_data_raw = nullptr;
		}
		snd_data_size = 0;
	}

#ifndef DISABLE_NETWORK
	//Load this chunk from net
	void LoadFromNet() {
		Unload();
		CurrentPacket >> snd_data_size;
		if (snd_data_size > 0 && snd_data_size <= CurrentPacket.getDataSize()) {
			snd_data_raw = new uint_fast8_t[snd_data_size];
			for (int i = 0; i < snd_data_size; i++) {
				CurrentPacket >> snd_data_raw[i];
			}
		}
	}

	//Send this chunk through net
	void SendNet() {
		while (databusy) {
			DATA_SAFETY_WAIT
		}
		CurrentPacket << snd_data_size;
		if (snd_data_size > 0) {
			CurrentPacket.append(snd_data_raw, snd_data_size);
		}
	}
#endif

	//From File
	int Load(uint_fast8_t N_ID) {
		ID = N_ID;
		string file1 = Modpack + "/levels/" + int_to_hex(getRamValue(0x010B, 2), false) + "/sound" + int_to_hex(ID, true) + ".ogg";
		if (networking) {
			databusy = true;
			Unload();
			ifstream input(file1, ios::in | ios::binary | ios::ate);
			if (input.is_open()) {
				snd_data_size = int(input.tellg()); snd_data_raw = new uint_fast8_t[snd_data_size];
				input.seekg(0, ios::beg); input.read((char*)snd_data_raw, snd_data_size); input.close();
			}
			databusy = false;
			return 1;
		}
		else {
			if (chunkPort != NULL) { Mix_FreeChunk(chunkPort); chunkPort = NULL; }
			chunkPort = Mix_LoadWAV(file1.c_str());
			if (chunkPort != NULL) {
				return 1;
			}
		}
		return 0;
	}

	//Load into chunk
	void ProcessData() {
		if (chunkPort != NULL) { Mix_FreeChunk(chunkPort); chunkPort = NULL; }
		if (snd_data_size > 0 && snd_data_raw != nullptr) {
			SDL_RWops* rw = SDL_RWFromMem(snd_data_raw, snd_data_size);
			chunkPort = Mix_LoadWAV_RW(rw, 0);
			SDL_FreeRW(rw);
		}
	}
};
SpecialLevelSpecificSoundChunk SpecChunksLevel[32];

string GetSongFile(uint_fast8_t ID) {
	string file1;
	if (gamemode == GAMEMODE_MAIN) {
		for (uint_fast8_t i = 0; i < 3; i++) {
			//Search Level Specific first, if we can.
			file1 = Modpack + "/levels/" + int_to_hex(getRamValue(0x010B, 2), false) + "/music" + int_to_hex(ID, true) + Music_Check_Types[i];
			if (is_file_exist(file1.c_str())) {
				music_type = i;
				return file1;
			}
		}
	}
	for (uint_fast8_t i = 0; i < 3; i++) {
		//Search for it in Music
		file1 = Modpack + "/music/" + int_to_hex(ID, true) + Music_Check_Types[i];
		if (is_file_exist(file1.c_str())) {
			music_type = i;
			return file1;
		}
	}
	music_type = MUSIC_OGG;
	return "Sounds/empty.ogg";
}

//Audio Device
SDL_AudioDeviceID audio_device;

//Audio Spec (For SPC Playbkck)
SDL_AudioSpec audio_spec;

//SPC
SNES_SPC* snes_spc;

char* music_data = NULL;
char* music_data_synced = NULL;
int music_data_size;
bool spc_playing = false;
bool spc_killed = false;
uint_fast8_t reading_music = 0;

//Terminate sound, will wait until all of the playing channels and music are terminated.
void Terminate_Music() {
	Mix_HaltChannel(-1);
	spc_playing = false; while (!spc_killed) { DATA_SAFETY_WAIT }
	old_1dfb = 0xFF;
	Mix_HaltMusic();
	while (Mix_PlayingMusic()) { DATA_SAFETY_WAIT }
	while (Mix_Playing(-1)) { DATA_SAFETY_WAIT }
}

//SPC Audio Callback
void audio_callback(void* user_data, Uint8* raw_buffer, int bytes) {
	short* buffer = (short*)raw_buffer;
	int length = bytes / 2; // 2 bytes per sample for AUDIO_S16SYS
	double volmultiplier = double(Mix_VolumeMusic(-1)) / 64.0;
	if (spc_playing && !Mix_PausedMusic()) {
		spc_killed = false;
		spc_play(snes_spc, length, buffer);
		//Amplifier
		for (int i = 0; i < length; i++) {
			buffer[i] = short(min(32766, max(-32766, buffer[i] * volmultiplier)));
		}
	}
	else {
		spc_killed = true;
		memset(raw_buffer, 0, bytes);
	}
}

//Initialize Audio subsystem
void InitializeAudio() {
	//SF
	midi_patchset = midi_patchset == "NONE" ? "SMW.sf2" : midi_patchset;
	string soundfont = "Soundbanks/" + midi_patchset; Mix_SetSoundFonts(soundfont.c_str());

	//Initialize SDL_mixer
	if (Mix_OpenAudio(ogg_sample_rate, MIX_DEFAULT_FORMAT, 2, 1024) == -1) {
		cout << purple << "[Audio] Error : " << Mix_GetError() << endl;
		return;
	}
	cout << purple << "[Audio] Initialized mixer audio port (" << dec << ogg_sample_rate << "hz)." << endl;

	//Mixer sound ports
	Mix_AllocateChannels(32);

	//Initialize Audio Spec (For SPC)
	SDL_zero(audio_spec);
	audio_spec.freq = 32000;
	audio_spec.format = AUDIO_S16;
	audio_spec.channels = 2; audio_spec.samples = spc_buffer_size;
	audio_spec.callback = audio_callback;

	//Open Audio Device
	audio_device = SDL_OpenAudioDevice(
		NULL, 0, &audio_spec, NULL, 0);
	snes_spc = spc_new();
	cout << purple << "[Audio] Initialized audio spec and SPC playback (32000hz)." << endl;
	SDL_PauseAudioDevice(audio_device, 0);

	int snds = 0;
	for (uint_fast8_t i = 0; i < 3; i++) {
		for (uint_fast16_t sounds = 0; sounds < 0x100; sounds++) {
			string sfx_to_play = "Sounds/" + int_to_hex(sound_table[i]) + "/" + int_to_hex(sounds, true) + ".ogg";
			sfxPorts[sounds + (i << 8)] = Mix_LoadWAV(sfx_to_play.c_str());
			if (sfxPorts[sounds + (i << 8)] != NULL) {
				snds++;
			}
		}
	}
	cout << purple << "[Audio] Pre-loaded " << dec << snds << " sounds." << endl;
}


#ifndef DISABLE_NETWORK
void SendMusic() {
	cout << green << "[Network] Packing sound data.." << endl;

	uint_fast8_t musicTypeFlag = RAM[0x1DFB] == 0xFF ? 0xFF : music_type;
	CurrentPacket << music_latest_sync;
	CurrentPacket << musicTypeFlag;

	if (musicTypeFlag != 0xFF) {
		if (music_type == MUSIC_SPC) {
			int nCompressedSize = LZ4_compress_fast(music_data, (char*)RAM_compressed, 66048, RAM_Size, 1);
			CurrentPacket << nCompressedSize;
			CurrentPacket.append(RAM_compressed, nCompressedSize);
		}
		else {
			CurrentPacket << music_data_size;
			CurrentPacket.append(music_data, music_data_size);
		}
		for (uint_fast8_t i = 0; i < 32; i++) {
			SpecChunksLevel[i].SendNet();
		}
	}
}

void ReceiveMusic() {
	//Wait until an current music read is done.
	while (reading_music & 1) {
		DATA_SAFETY_WAIT
	}
	reading_music = 1;
	cout << green << "[Network] Downloading music/sounds from server.." << endl;
	CurrentPacket >> music_latest_sync;
	CurrentPacket >> new_music_type;
	if (new_music_type == 0xFF) {
		cout << green << "[Network] Received command to fade music." << endl;
		RAM[0x1DFB] = 0xFF;
		reading_music = 0;
	}
	else {
		RAM[0x1DFB] = 1;
		if (music_data != NULL) { delete[] music_data; }

		//Choose what type of decomp to use.
		if (new_music_type == MUSIC_SPC) {
			music_data = new char[66048];
			//LZ4 compression
			CurrentPacket >> music_data_size;
			for (int i = 0; i < music_data_size; i++) {
				CurrentPacket >> RAM_compressed[i];
			}
			LZ4_decompress_safe((char*)RAM_compressed, music_data, music_data_size, 66048);
			music_data_size = 66048;
		}
		else {
			CurrentPacket >> music_data_size;
			music_data = new char[music_data_size]; uint_fast8_t g;
			for (int i = 0; i < music_data_size; i++) {
				CurrentPacket >> g; music_data[i] = (char)g;
			}
		}
		int total_size_sounds = 0;
		for (uint_fast8_t i = 0; i < 32; i++) {
			SpecChunksLevel[i].LoadFromNet();
			total_size_sounds += SpecChunksLevel[i].snd_data_size;
		}
		cout << green << "[Network] Received sound data, " << dec << music_data_size / 1024 << "kb of music + " << dec << total_size_sounds / 1024 << "kb of sounds" << endl;
		reading_music = 2;
	}
}
#endif

void SoundLoop()
{
	if (!networking || isClient) {
		if (!networking) {
			//Singleplayer music handling.
			if (RAM[0x1DFB] != old_1dfb && RAM[0x1DFB] != 0xFF) {
				Terminate_Music(); old_1dfb = RAM[0x1DFB];
				string SNG = GetSongFile(old_1dfb);
				if (music_type == MUSIC_SPC) {
					ifstream input(SNG.c_str(), ios::binary);
					char* mDataBuffer = new char[66048]; input.read(mDataBuffer, 66048);
					spc_load_spc(snes_spc, mDataBuffer, 66048); delete[] mDataBuffer;
					input.close();
					spc_playing = true;
				}
				else {
					music = Mix_LoadMUS(SNG.c_str());
					if (music == NULL) {
						cout << purple << "[Audio] Failed to change music : " << Mix_GetError() << endl;
					}
					else {
						Mix_PlayMusic(music, -1);
					}
				}
				cout << purple << "[Audio] Playing song 0x" << hex << int_to_hex(old_1dfb, true) << dec << endl;
			}
		}
		else {
			if (!players_synced) {
				if (!Mix_PausedMusic()) { Mix_PauseMusic(); }
			}
			else {
				if (Mix_PausedMusic()) { Mix_ResumeMusic(); }
			}
			if (reading_music == 2) {
				reading_music = 3;
				music_type = new_music_type;
				Terminate_Music();

				//Prepare new music buffer (this is because using music_data normally would cause crashes, because SDL sucks at handling this situation)
				if (music_data_synced != NULL) {
					delete[] music_data_synced;
				}
				music_data_synced = new char[music_data_size];
				memcpy(music_data_synced, music_data, music_data_size);

				//Setup
				if (music_type != MUSIC_SPC) {
					SDL_RWops* rw = SDL_RWFromMem(music_data_synced, music_data_size);
					music = Mix_LoadMUSType_RW(rw, music_type == MUSIC_OGG ? MUS_OGG : MUS_MID, 0);
					if (music == NULL) {
						cout << purple << "[Audio] Failed to change music : " << Mix_GetError() << endl;
					}
					else {
						Mix_PlayMusic(music, -1);
					}
				}
				else {
					spc_load_spc(snes_spc, music_data_synced, 66048); spc_playing = true;
				}

				//Sound effects
				for (uint_fast8_t i = 0; i < 32; i++) {
					SpecChunksLevel[i].ProcessData();
				}

				//Finalized
				cout << purple << "[Audio] Loaded music/sounds from server." << endl;
				reading_music = 0;
			}
		}
		Mix_Volume(-1, sfx_volume);
		Mix_VolumeMusic(RAM[0x1DFB] == 0xFF ? max(0, Mix_VolumeMusic(-1) - 1) : music_volume);
		for (uint_fast8_t i = 0; i < 4; i++) {
			uint_fast16_t RAM_P = sound_table[i];
			if (RAM[RAM_P] != 0) {
				//Audiochunk find
				uint_fast16_t MixChunk = RAM[RAM_P] + ((i % 3) << 8);
				Mix_Chunk* Port = sfxPorts[MixChunk];

				if ((MixChunk & 0xFF) >= 0xE0 || i == 3) {
					SpecialLevelSpecificSoundChunk& P = SpecChunksLevel[MixChunk & 0x1F];
					if (P.chunkPort != NULL) { Port = P.chunkPort; }
				}
				if (Port != NULL) {
					if (Mix_PlayChannel(multichannel_sounds || i == 3 ? -1 : (31 - i), Port, 0) == -1) {
						cout << purple << "[Audio] Port " << dec << (i + 1) << " Error : " << Mix_GetError() << endl;
					}
				}
				else {
					cout << purple << "[Audio] Port " << dec << (i + 1) << " Error : Doesn't exist" << endl;
				}
				RAM[RAM_P] = 0;
			}
		}

	}
#ifndef DISABLE_NETWORK
	else {
		//Music additions for serverside
		if (RAM[0x1DFB] != old_1dfb) {
			old_1dfb = RAM[0x1DFB];
			string file_picked = GetSongFile(old_1dfb);
			ifstream input(file_picked, ios::in | ios::binary | ios::ate);
			if (input.is_open()) {
				delete[] music_data;
				music_data_size = int(input.tellg());
				music_data = new char[music_data_size];
				music_latest_sync++;
				input.seekg(0, ios::beg);
				input.read(music_data, music_data_size);
				input.close();
				cout << purple << "[Audio] Loaded " << file_picked << endl;
			}
			else
			{
				cout << purple << "[Audio] Failed to load " << file_picked << endl;
			}
		}
	}
#endif
}
