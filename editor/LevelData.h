#pragma once

class LevelManager
{
public:
	std::string status;
	std::string line;
	std::string scripts;
	std::string messages;

	//Config
	std::unordered_map<std::string, uint_fast32_t> config;
	uint_fast32_t request_level_entry(std::string name) {
		auto entry = config.find(name);
		if (entry != config.end()) {
			return entry->second;
		}
		return 0;
	}
	void edit_level_entry(std::string name, uint_fast32_t value) {
		auto entry = config.find(name);
		if (entry != config.end()) {
			entry->second = value;
		}
	}
	void add_entry(std::string name, uint_fast32_t value) {
		config.insert(std::make_pair(name, value));
	}

	//Assets
	void ReloadAssets() {
		for (int i = 0; i < 8; i++) {
			LoadAssetIntoVRAM("Graphics/GFX" + int_to_hex(request_level_entry(GFX_Names[i]), true) + ".bin", GFX_Locations[i]);
			LoadAssetIntoVRAM(CurrentPack + "/levels/" + CurrentLevel + "/" + GFX_Names[i] + ".bin", GFX_Locations[i]);
		}
		load_background(request_level_entry("background"));
	}

	void LoadLevelFromString(std::string DLevel)
	{
		int Curr_Sprite = 0;

		config.clear();
		scripts = "";
		messages = "";

		levelSData.clear();
		levelLData.clear();

		istringstream str(DLevel); // std::string
		while (getline(str, line)) {
			if (line != "" || status.substr(0, 7) == "message")
			{
				if (line.substr(0, 1) == "[")
				{
					status = line.substr(1, line.length() - 2);
					if (status.substr(0, 7) == "message")
					{
						messages = messages + "[" + status + "]\n";
					}
					continue;
				}

				string CHECK = line.substr(0, 2);
				//if this isnt a comment.
				if (CHECK != "//") {
					if (status == "scripts") {
						scripts = scripts + line + "\n";
					}
					for (int i = 1; i <= 15; i++) {
						if (status == ("message" + to_string(i)))
						{
							messages = messages + line + "\n";
						}
					}
					if (status == "sprite_data") {
						vector<string> v = split(line.c_str(), ',');
						int offset = v.size() == 5 ? 1 : 0;
						levelSData.push_back(SpriteObject{
							stoi(v[offset + 1]),
							stoi(v[offset + 2]),
							int_fast8_t(stoi(v[offset + 3])),
							uint_fast8_t(stoi(v[offset], nullptr, 16))
							});
					}
					if (status == "level_config") {
						line.erase(remove_if(line.begin(), line.end(), ::isspace),
							line.end());
						auto delimiterPos = line.find("=");
						auto name = line.substr(0, delimiterPos);
						auto value = line.substr(delimiterPos + 1);

						if (name == "music" || name == "background") {
							add_entry(name, stoi(value, nullptr, 16));
						}
						else {
							add_entry(name, stoi(value));
						}
					}
					if (status == "level_data" && line != status) {
						vector<string> v = split(line.c_str(), ',');
						if (v.size() == 5) {
							int x = stoi(v[1]);
							int y = stoi(v[2]);
							int w = stoi(v[3]);
							int h = stoi(v[4]);
							levelLData.push_back(LevelObject{ x, h, w - x + 1, h - y + 1, uint_fast16_t(stoi(v[0], nullptr, 16)) });
						}
						if (v.size() == 3) {
							levelLData.push_back(LevelObject{stoi(v[1]), stoi(v[2]), 1, 1, uint_fast16_t(stoi(v[0], nullptr, 16)) });
						}
					}
				}
			}
		}
	}
};

LevelManager lManager;

//File loading
void LoadLevelFromFile(std::string FILENAME) {
	CurrentFile = FILENAME;

	size_t found = FILENAME.find_last_of("/");
	string pathf2 = FILENAME.substr(0, found);
	size_t found2 = pathf2.find_last_of("/");
	string pathf3 = FILENAME.substr(0, found2);
	size_t found3 = pathf3.find_last_of("/");
	string pathf4 = FILENAME.substr(0, found3);
	size_t found4 = pathf4.find_last_of("/");
	CurrentLevel = FILENAME.substr(found2 + 1, found - found2 - 1);
	CurrentPack = "Packs/" + FILENAME.substr(found4 + 1, found3 - found4 - 1);

	std::ifstream t(FILENAME);
	std::string str((std::istreambuf_iterator<char>(t)),
		std::istreambuf_iterator<char>());

	lManager.LoadLevelFromString(str);
	lManager.ReloadAssets();

	//Init M16
	InitializeMap16();

	cout << yellow << "[SE] Loaded level " << CurrentLevel << " (" << CurrentPack << ") from " << CurrentFile << white << endl;
}

//Export
string ExportLevel() {
	//Level Config
	string FinalizedLevel = "[level_config]\n";
	unordered_map<std::string, uint_fast32_t>::iterator it;
	for (it = lManager.config.begin(); it != lManager.config.end(); it++) {
		bool nHex = it->first == "background" || it->first == "music";
		FinalizedLevel += it->first + " = " + (nHex ? int_to_hex(it->second) : to_string(it->second)) + "\n";
	}
	FinalizedLevel += "\n";

	//Scripts and messages
	FinalizedLevel += lManager.messages + "[scripts]\n" + lManager.scripts;

	//Sprites
	FinalizedLevel += "\n[sprite_data]\n";
	for (int i = 0; i < levelSData.size(); i++) {
		SpriteObject& sObj = levelSData[i];
		FinalizedLevel += int_to_hex(sObj.num, true) + "," +
			to_string(sObj.x) + "," +
			to_string(sObj.y) + "," +
			to_string(sObj.dir) + "\n";
	}

	//Layout Data
	FinalizedLevel += "\n[level_data]\n";
	for (int i = 0; i < levelLData.size(); i++) {
		LevelObject& lObj = levelLData[i];
		if (lObj.size_x == 1 && lObj.size_y == 1) {
			FinalizedLevel +=
				int_to_hex(lObj.tile) + "," +
				to_string(lObj.x) + "," +
				to_string(lObj.y) + "\n";
		}
		else {
			FinalizedLevel +=
				int_to_hex(lObj.tile) + "," +
				to_string(lObj.x) + "," +
				to_string(lObj.y - lObj.size_y + 1) + "," +
				to_string(lObj.size_x + lObj.x - 1) + "," +
				to_string(lObj.y) + "\n";
		}

	}
	cout << green << "[SE] Succesfully exported level, " << levelLData.size() << " chunks, and " << levelSData.size() << " sprites." << white << endl;
	return FinalizedLevel;
}

//Load File
void LoadFile() {
	wchar_t filename[MAX_PATH];

	OPENFILENAME ofn;
	ZeroMemory(&filename, sizeof(filename)); ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn); ofn.hwndOwner = NULL; ofn.lpstrFilter = L"Level file.\0*.txt\0";
	ofn.lpstrFile = filename; ofn.nMaxFile = MAX_PATH; ofn.lpstrTitle = L"Load a level .txt file.";
	ofn.Flags = OFN_FILEMUSTEXIST;

	if (!GetOpenFileName(&ofn)) {
		MessageBoxA(NULL, "Yo ass\nCatastrophic failure", "Open", MB_OK | MB_ICONEXCLAMATION);
	}
	else {
		wstring ws(filename);
		string str(ws.begin(), ws.end());
		replaceAll(str, "\\", "/");
		LoadLevelFromFile(str);
	}
}

//Save File
bool SaveFile() {
	bool status = false;
	std::ofstream out(CurrentFile);
	if (out.is_open()) {
		status = true;
		out << ExportLevel();
	}
	else {
		ShowMessage("Failed to save level.");
	}
	out.close();
	return status;
}

//Play Test Level
void PlayTest() {
	if (SaveFile()) {
		string command = "start \"\" /d \"" + path + "\" \"JFKMarioWorld.exe\" -l " + CurrentLevel + "," + CurrentPack.substr(6);
		cout << yellow << command << white << endl;
		system(command.c_str());
	}
}

//Create New Level
void CreateNewLevel() {
	string NEWPACK = GetStringInputBox("Pack name, leave blank for default.");
	if (NEWPACK == "") {
		NEWPACK = "default";
	}
	NEWPACK = "Packs/" + NEWPACK;
	if (!fs::is_directory(path + NEWPACK)) {
		ShowMessage(NEWPACK + " does not exist!");
		return;
	}
	CurrentPack = NEWPACK;

	//Pass 1, ID.
	int ID = GetValueInputBox("New Level ID, something like 1, F4, D5, 105, 114, etc.", 16);
	if (ID <= 0 || ID > 65535) {
		ShowMessage("Cannot create a level with the ID " + int_to_hex(ID) + ".");
		return;
	}

	string FOLDER = path + CurrentPack + "/levels/" + int_to_hex(ID);
	if (fs::is_directory(FOLDER)) {
		ShowMessage("Level " + int_to_hex(ID) + " already exists.");
		return;
	}
	string FILENAME = FOLDER + "/level_data.txt";

	//Pass 2, level size.
	int newX = GetValueInputBox("Level size X");
	int newY = GetValueInputBox("Level size Y");
	if (newX < 16 || newY < 14) {
		ShowMessage("Level is too small.");
		return;
	}
	if ((newX * newY) > 32768) {
		ShowMessage("Size X * Size Y is over 32768! Trying to create a level like this would corrupt the memory ingame and cause glitches.");
		return;
	}

	//Pass 3, template files.
	int templatechoose = GetValueInputBox("Template (choose a number, or from installed folder):\n - 1 is a grass level.\n - 2 is a cave level.\n - 3 is a mushroom platforms level.\n - 4 is a castle level.");

	string TemplateFile = path + "Editor/Templates/template" + to_string(templatechoose) + ".txt";
	string LuaFile = path + "Editor/Templates/template" + to_string(templatechoose) + ".lua";
	string PaletteFile = path + "Editor/Templates/template" + to_string(templatechoose) + ".mw3";
	string Map16File = path + "Editor/Templates/template" + to_string(templatechoose) + ".jfkmap16";

	ifstream temp(TemplateFile);
	if (!temp.is_open()) {
		ShowMessage("Could not find template.");
		temp.close();
		return;
	}

	//Read this into a stringstream buffer.
	stringstream buffer;
	buffer << temp.rdbuf();
	temp.close();
	
	std::string NewLevelFile = "[level_config]\n" + buffer.str() + "\nsize_x = " + to_string(newX) + "\nsize_y = " + to_string(newY) + "\n\n[scripts]\nlevel_lua.lua = main";
	
	
	fs::create_directories(FOLDER);
	fs::copy(LuaFile, FOLDER + "/level_lua.lua"); // copy file
	fs::copy(PaletteFile, FOLDER + "/level_palette.mw3"); // copy file

	ifstream test(Map16File);
	if (test.is_open()) {
		test.close();
		fs::copy(Map16File, FOLDER + "/level_map16.jfkmap16"); // copy file
	}
	else {
		test.close();
	}

	//We did it
	ofstream out(FILENAME);
	out << NewLevelFile;
	out.close();

	//Level was created with success, go ahead!
	LoadLevelFromFile(FILENAME);
}