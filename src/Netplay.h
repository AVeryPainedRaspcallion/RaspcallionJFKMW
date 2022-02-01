#pragma once

//Packet headers
#define Header_UpdatePlayerData 0x80
#define Header_GlobalUpdate 0x81
#define Header_ConnectData 0x83
#define Header_MusicData 0x84
#define Header_FailedToConnect 0x85
#define Header_AttemptJoin 0xFF

//Connection Validating
bool validated_connection = false;

//Packets and network status
sf::Socket::Status last_network_status;
sf::Socket::Status receiveWithTimeout(GNetSocket& socket, sf::Packet& packet, sf::Time timeout) {
	last_network_status = sf::Socket::NotReady;

	sf::SocketSelector selector; selector.add(socket);
	if (selector.wait(timeout)) {
		last_network_status = socket.receive(packet);
	}
	return last_network_status;
}

//Update Player List
uint_fast8_t GetAmountOfPlayers() {
	PlayerAmount = uint_fast8_t(clients.size()); CheckForPlayers();
	return PlayerAmount;
}

//Prepare new packet
void PreparePacket(uint8_t header) {
	CurrentPacket.clear(); CurrentPacket << header;
	CurrentPacket_header = header;
}

//Send all player data
void pack_player_data(uint_fast8_t skip = 0) {
	if (!isClient) {
		CurrentPacket << PlayerAmount;
		for (uint_fast8_t i = 0; i < Players.size(); i++) {
			MPlayer& CurrentPlayer = Players[i];
			if (i != skip) {
				CurrentPlayer.NetPackVariables(true);
			}
			else {
				CurrentPlayer.NetPackSpecificVariables();
			}
		}
	}
	else {
		MPlayer& CurrentPlayer = GetPlayerByNumber(SelfPlayerNumber);
		CurrentPacket << SelfPlayerNumber;
		CurrentPacket << latest_sync;
		CurrentPacket << music_latest_sync;
		CurrentPacket << CurrentPlayer.server_position_sync_c;
		CurrentPacket << CurrentPlayer.current_chat;
		CurrentPacket << CurrentPlayer.curr_chat_string;
		CurrentPlayer.NetPackVariables(false);
	}
}

//Handle client disconnection (or disconnect them)
void HandleDisconnection(GNetSocket* ToSend = nullptr) {
	if (ToSend != nullptr) {
		cout << blue << "[Server] " << ToSend->username << " (" << ToSend->getRemoteAddress() << ") has disconnected." << endl;
		if (find(clients.begin(), clients.end(), ToSend) != clients.end()) {
			clients.erase(remove(clients.begin(), clients.end(), ToSend));

			discord_message("**" + ToSend->username + " just disconnected, There are " + to_string(clients.size()) + " players connected.**");
			Send_Chat(ToSend->username + " left");

			selector.remove(*ToSend);
			ToSend->disconnect();
			last_network_status = sf::Socket::Error;
			delete ToSend;
		}
		else
		{
			ToSend->disconnect();
			last_network_status = sf::Socket::Error;
		}
	}
}

//Send packet to socket, direct.
void sendPacketSocket(GNetSocket* ToSend = nullptr, bool blocking = false) {
	ToSend->setBlocking(blocking);
	if (ToSend->send(CurrentPacket) == sf::Socket::Disconnected) {
		if (!isClient) {
			HandleDisconnection(ToSend);
		}
		else {
			disconnected = true;
		}
	}
	ToSend->setBlocking(true);
}

//Send packet handler.
void SendPacket(GNetSocket* ToSend = nullptr, bool blocking = false) {
	if (!isClient) {
		data_size_now += int(CurrentPacket.getDataSize());
		if (ToSend != nullptr) {
			sendPacketSocket(ToSend, blocking);
		}
		else {
			for (int i = 0; i < clients.size(); ++i) {
				sendPacketSocket(clients[i], blocking);
			}
		}
	}
	else {
		sendPacketSocket(&socketG, blocking);
	}
}

//Behaviour when a packet is received.
void ReceivePacket(GNetSocket &whoSentThis, bool for_validating = false) {
	CurrentPacket >> CurrentPacket_header;
	data_size_current += int(CurrentPacket.getDataSize());

	//Very simple version validation system.
	if (for_validating) {
		if (CurrentPacket_header == Header_AttemptJoin && CurrentPacket.getDataSize() < 64) { //Why would the verification packet be bigger than 64 bytes? It's only username and checksum so.
			cout << blue << "[Client] Receiving verification.." << endl;
			string validation;
			CurrentPacket >> username;
			CurrentPacket >> validation;
			if (validation == GAME_VERSION) {
				cout << blue << "[Client] " << username << " has passed verification." << endl;
				validated_connection = true;
				return;
			}
			cout << blue << "[Client] " << username << " failed verification. Outdated version or invalid chk? V/C = " << validation << endl;
			latest_error = "Outdated version (use " + GAME_VERSION + ")";
		}
		return;
	}

	//SERVER BEHAVIOUR
	if (!isClient)
	{
		//Player only sends things to update their data, so they shouldn't send stuff that big
		if (CurrentPacket.getDataSize() < player_expected_packet_size || CurrentPacket.getDataSize() > (player_expected_packet_size + 70)) {
			cout << blue << "[Network] Something's weird, " << whoSentThis.username << " sent a packet that wasn't in the correct size range, " + to_string(player_expected_packet_size) + " bytes (" << dec << CurrentPacket.getDataSize() << " bytes) Disconnecting!" << endl;
			PreparePacket(Header_FailedToConnect);
			CurrentPacket << "Invalid data sent.";
			SendPacket(&whoSentThis);
			HandleDisconnection(&whoSentThis);
			return;
		}

		if (CurrentPacket_header == Header_UpdatePlayerData) {
			uint_fast8_t PlrNum;
			CurrentPacket >> PlrNum;
			CurrentPacket >> whoSentThis.latest_sync_p;
			CurrentPacket >> whoSentThis.music_latest_sync_p;

			if (PlrNum >= clients.size()) {
				return;
			}
			
			PlayerAmount = uint_fast8_t(clients.size());
			MPlayer& CurrentPlayer = GetPlayerByNumber(PlrNum);
			CurrentPacket >> CurrentPlayer.server_position_sync_c;
			CurrentPacket >> CurrentPlayer.current_chat;
			CurrentPacket >> CurrentPlayer.curr_chat_string;
			CurrentPlayer.NetUnpackVariables(false);
		}
	}
	else
	//CLIENT BEHAVIOUR
	{
		WAIT_READ_COMPLETE("Packet receive 0x" + int_to_hex(CurrentPacket_header, true))
		doing_read = true;
		if (CurrentPacket_header == Header_MusicData) {
			ReceiveMusic(); Sync_Server_RAM(false);
		}

		if (CurrentPacket_header == Header_GlobalUpdate) {
			//Receive basic response data, gamemode, ping, etc.
			uint_fast8_t bflags1 = 0; CurrentPacket >> bflags1;
			players_synced = bflags1 & 0x80; gamemode = bflags1 & 0x7F;
			int_fast32_t timestamp; CurrentPacket >> timestamp; latest_server_response = int_fast32_t(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count()) - timestamp;
			CurrentPacket >> SelfPlayerNumber;
			CurrentPacket >> player_netcommand[SelfPlayerNumber];
			CurrentPacket >> PlayerAmount;

			//Player list update in case
			CheckForPlayers();
			for (uint_fast8_t i = 0; i < Players.size(); i++) {
				MPlayer& CurrentPlayer = Players[i];
				if (i != SelfPlayerNumber) {
					CurrentPlayer.NetUnpackVariables(true);
				}
				else {
					CurrentPlayer.NetUnpackSpecificVariables();
				}
			}

			//Sync Variables
			CurrentPacket >> recent_big_change;
			Sync_Server_RAM(!recent_big_change);
			if (recent_big_change && gamemode == GAMEMODE_MAIN) { ReceiveBackgrounds(); }

			//Receive Last Chat String
			CurrentPacket >> Curr_PChatString;
		}

		if (CurrentPacket_header == Header_ConnectData) {
			CurrentPacket >> gamemode;
			CurrentPacket >> PlayerAmount; //Update Plr Amount
			CheckForPlayers(); Sync_Server_RAM(false); ReceiveMusic();
			if (gamemode == GAMEMODE_MAIN) { ReceiveBackgrounds(); }
			cout << blue << "[Client] Received connection data." << endl;
			validated_connection = true;
		}

		if (CurrentPacket_header == Header_FailedToConnect) {
			CurrentPacket >> latest_error;
			last_status = "Disconnected.";
			cout << red << "[Network] Received disconnection reason from server: " << latest_error << endl;
			disconnected = true;
		}
		doing_read = false;
	}
}

//Receive all packets with a queue.
void ReceiveAllPackets(GNetSocket& socket, bool slower = false, bool for_validating = false) {
	validated_connection = false;
	while (receiveWithTimeout(socket, CurrentPacket, sf::milliseconds(slower ? 2000 : 1)) != sf::Socket::NotReady) {
		//Failed
		if (last_network_status == sf::Socket::Disconnected || last_network_status == sf::Socket::Error) {
			if (!isClient) { HandleDisconnection(&socket); }
			else { disconnected = true; }
			return;
		}
		ReceivePacket(socket, for_validating);
		if (validated_connection) { return; }
	}
}

//Pending connection handler.
void PendingConnection() {
	GNetSocket* client = new GNetSocket;
	if (listener.accept(*client) == sf::Socket::Done) {
		uint_fast8_t NewPlayerNumber = GetAmountOfPlayers();
		username = "Unknown";

		cout << blue << "[Server] " << client->getRemoteAddress() << " (assigned to Player " << int(NewPlayerNumber) << ") is trying to connect.. " << endl;

		for (int i = 0; i < bans.size(); i++) {
			if (client->getRemoteAddress() == bans[i]) {
				cout << red << "[Network] Client is banned." << endl;
				PreparePacket(Header_FailedToConnect);
				CurrentPacket << "You are banned from this server.";
				SendPacket(client);
				client->disconnect();
				delete client;
				return;
			}
		}

		latest_error = "Invalid information sent";

		//Wait for the player to send in verification data
		ReceiveAllPackets(*client, true, true);

		//Wait a few moments
		sf::sleep(sf::milliseconds(1000));

		//Validation might have succeeded by now
		if (validated_connection) {
			validated_connection = false;

			// Add the new client to the clients list
			client->username = username;

			clients.push_back(client); selector.add(*client); GetAmountOfPlayers();

			PreparePacket(Header_ConnectData);
			CurrentPacket << gamemode; CurrentPacket << PlayerAmount;
			Push_Server_RAM(false); SendMusic();
			if (gamemode == GAMEMODE_MAIN) { SendBackgrounds(); }
			SendPacket(client, true);

			//Wait a few moments
			sf::sleep(sf::milliseconds(1000));

			//Send msg to discord
			discord_message("**" + username + " has joined, There are " + to_string(clients.size()) + " players connected.**");
			Send_Chat(username + " joined");
		}
		else {
			//This is a failed connection
			cout << red << "[Network] Client timed out or sent invalid information. Disconnecting." << endl;
			PreparePacket(Header_FailedToConnect);
			CurrentPacket << latest_error;
			SendPacket(client);

			client->disconnect();

			delete client;
		}
	}
	else {
		// Error, we won't get a new connection, delete the socket
		delete client;
	}
}

//Natural Loop
#define S_TO_CLIENT_STOP_CHECK if (clients.size() < 1 || (last_network_status == sf::Socket::Error || last_network_status == sf::Socket::Disconnected)) { break; }
void Server_To_Clients() {
	bool players_synced_new = true;
	GetAmountOfPlayers();
	if (clients.size() > 0) {
		for (uint_fast8_t i = 0; i < clients.size(); i++) {
			GNetSocket& client = *clients[i];

			//Receive all packets from client. This halts the thread.
			ReceiveAllPackets(client);

			//Safety
			S_TO_CLIENT_STOP_CHECK

			//Start read. We're sending in stuff from the game, so we have to wait until the game thread is not doing anything.
			WAIT_READ_COMPLETE("Server To Client Loop")
			doing_read = true;

			//Send basic response data
			PreparePacket(Header_GlobalUpdate);
			uint_fast8_t bflags1 = (players_synced << 7) + (gamemode & 0x7F);
			CurrentPacket << bflags1;
			CurrentPacket << int_fast32_t(chrono::duration_cast<chrono::milliseconds>(chrono::system_clock::now().time_since_epoch()).count());
			CurrentPacket << i;
			CurrentPacket << player_netcommand[i];

			//Send player data
			pack_player_data(i);

			//DATA SYNC
			bool d_change = client.latest_sync_p != latest_sync;
			if (d_change) {
				players_synced_new = false;

				//try to sync more aggresively if there's more tries and it's still not synced
				int max_sync_time = 30 + min(8, client.sync_tries) * 10;
				//unable to sync timer
				if (client.last_send_timer > max_sync_time) {
					client.sync_tries++;
					cout << red << "[Network] Tried to sync " << client.username << ", no change shown. trying again (" << client.sync_tries << " tries)" << endl;
					client.last_send_timer = 0;
					Send_Chat("Failed to sync " + client.username + " (" + to_string(client.sync_tries) + ")");
				}
				//last send timer check, if 0 send an actual packet containing all synced data
				if (client.last_send_timer > 0) {
					d_change = false;
				}
				//last send timer
				client.last_send_timer++;
			}
			else {
				//it's properly synced, we just set sync tries and timer to 0.
				client.last_send_timer = 0;
				client.sync_tries = 0;
			}

			//Pack backgrounds
			CurrentPacket << d_change;
			Push_Server_RAM(!d_change);
			if (d_change && gamemode == GAMEMODE_MAIN) {
				SendBackgrounds();
			}
			CurrentPacket << Curr_PChatString;
			SendPacket(&client, d_change);

			//Safety
			S_TO_CLIENT_STOP_CHECK

			//MUSIC SYNC
			d_change = client.music_latest_sync_p != music_latest_sync;
			if (d_change) {
				players_synced_new = false;

				//try to sync more aggresively if there's more tries and it's still not synced
				int max_sync_time = 60 + min(3, client.music_sync_tries) * 60;
				//unable to sync timer
				if (client.last_music_send_timer > max_sync_time) {
					client.music_sync_tries++;
					cout << red << "[Network] Tried to sync " << client.username << " music state, no change shown. trying again (" << client.music_sync_tries << " tries)" << endl;
					client.last_music_send_timer = 0;
				}
				//last send timer check, if 0 send an actual packet containing all synced data
				if (client.last_music_send_timer > 0) {
					d_change = false;
				}
				//last send timer
				client.last_music_send_timer++;
			}
			else {
				//it's properly synced, we just set sync tries and timer to 0.
				client.last_music_send_timer = 0;
				client.music_sync_tries = 0;
			}
			if (d_change) {
				PreparePacket(Header_MusicData);
				SendMusic();
				Push_Server_RAM(false);
				SendPacket(&client, true);

				//Safety
				S_TO_CLIENT_STOP_CHECK
			}

			//By this point, our reads are complete.
			doing_read = false;
		}

		//Reset
		resetImportantVariables();
		if (recent_big_change) {
			recent_big_change = false;
			cout << green << "[Network] Sent game RAM sync." << endl;
		}
	}
	players_synced = players_synced_new;
	doing_read = false;
}

//Network thread
sf::Clock netPclock;
void NetCapSpeed60() {
	auto elapsedMillisecondsExpected = 1000 / 60; // in milliseconds
	auto elapsedMilliseconds = netPclock.getElapsedTime().asMilliseconds();
	if (elapsedMilliseconds < elapsedMillisecondsExpected) { sf::sleep(sf::milliseconds(elapsedMillisecondsExpected - elapsedMilliseconds)); }
	netPclock.restart();
}
void NetworkLoop() {
	if (!isClient) {
		listener.listen(PORT); selector.add(listener);
		cout << blue << "[Server] Server is running on port " << dec << PORT << endl;

		//Discord
		discord_message("There's a JFKMW server version " + GAME_VERSION + " being hosted, the ip is **" + sf::IpAddress::getPublicAddress(sf::seconds(5.f)).toString() + "**, port is " + to_string(PORT));

		//Endless loop
		while (!quit) {
			//Test listener for pending connection.
			if (selector.wait(sf::milliseconds(4)) && selector.isReady(listener)) { PendingConnection(); }
			//Server loop
			Server_To_Clients();
			//Wait
			NetCapSpeed60();
		}
	}
	else {
		selector.add(socketG);

		//Very basic loop
		cout << blue << "[Network] Connected to server. " << int(PlayerAmount) << " player(s) connected." << endl;
		while (!quit && !disconnected) {
			ReceiveAllPackets(socketG);
			PreparePacket(Header_UpdatePlayerData); pack_player_data(); SendPacket();
			NetCapSpeed60();
		}
	}
}

//Prepare & launch connection.
bool ConnectClient(void) {
	if (socketG.connect(ip, PORT) == sf::Socket::Done) {
		cout << blue << "[Network] Found server.. sending punch packet" << endl;

		//We send the punch packet, asking the server if we can join.
		PreparePacket(Header_AttemptJoin);
		CurrentPacket << username; CurrentPacket << GAME_VERSION;
		SendPacket();

		//Server might take a while to respond, so we wait a few seconds, then receive packets again.
		cout << blue << "[Network] Waiting for connection data.." << endl;
		ReceiveAllPackets(socketG, true);
		if (!validated_connection) { return false; }
		validated_connection = false;
		if (disconnected) { return false; }

		//We have joined succesfully.
		CheckForPlayers();
		cout << blue << "[Network] Connected to " << ip << ":" << dec <<  PORT << endl;
		return true;
	}
	else {
		return false;
	}
}