#pragma once

#define P_METER_REQUIRED 0x70

class MPlayer
{
public:
	int_fast8_t SKIDDING = 0;
	int_fast8_t WALKING_DIR = 1;

	bool CROUCH = false;
	uint_fast8_t INVINCIBILITY_FRAMES = 0;
	uint_fast8_t INVINCIBILITY_FRAMES_STAR = 0;

	bool invisible = false;
	bool ON_FL = false; //on floor
	bool IN_WT = false; //on water
	bool OLD_WT = false; //old on water status
	bool SLIDING = false;
	uint_fast8_t SLOPE_TYPE = 0;
	/*
		0 : None
		1 : Right 45*
		2 : Left 45*
	*/
	uint_fast8_t P_METER = 0;
	uint_fast8_t reserve_item = 0;

	double X_SPEED = 0.0;
	double Y_SPEED = 0.0;

	double LAST_X_SPEED_ON_FL = 0.0;

	double CAMERA_X = 0.0;
	double CAMERA_Y = 0.0;
	double FRM = 0;
	int_fast8_t to_scale = 1;
	uint_fast8_t fire_anim_timer = 0;

	//Powerup defines
	//Cape
	uint_fast8_t cape_spin_timer = 0;
	uint_fast8_t cape_ascent_timer = 0;
	uint_fast8_t cape_state = 0;
	double CAPE_CLOTH_Y = 0.0;
	uint_fast8_t CAPE_FRAME = 0;
	int CAPE_ST = 0;

	//Collision rleated, positions
	double height = 14.0;
	double x = 16.0;
	double y = 16.0;

	uint_fast8_t player_index = 0;
	uint_fast8_t skin = 0;
	uint_fast8_t STATE = 0;

	uint_fast8_t score_streak = 0;
	uint_fast8_t WO_counter = 0;
	uint_fast8_t KO_counter = 0;

	uint_fast16_t mouse_x = 0;
	uint_fast16_t mouse_y = 0;

	//TTTXXXXX, its ((powerup_anim & 0x1F) >> 2) & 1
	uint_fast8_t powerup_anim = 0;
	
	//left, right, mousewup, mousewdown
	bool mouse_state[4];

	char player_name_cut[player_name_size] = "       ";

	uint_fast8_t GRABBED_SPRITE = 0xFF; //A sprite index from 0 to 7F

	bool DEAD = false;
	bool OLD_DEAD = false;
	bool pressed_y = false;
	bool old_y = false;

	bool old_s = false;

	bool in_pipe = false;
	bool jump_is_spin = false; //If a jump is a spinjump
	bool climbing = false;

	uint_fast8_t enemyjump_cooldown = 0;

	int_fast8_t pipe_speed_x = 0;
	int_fast8_t pipe_speed_y = 0;
	int_fast16_t DEATH_TIMER = 0;

	bool PlayerControlled = false;

	bool p_pad[total_inputs];
	bool wasJumping = false; //debounce for jump

	bool CAN_SPRINT = false;
	uint_fast8_t pose = 0;

	//Chat strings (THIS IS ONLY SENT. NOT RECEIVED!)
	string curr_chat_string = "";

	uint_fast16_t current_chat = 0;
	uint_fast16_t old_chat = 0;

	//Server position syncs
	uint_fast8_t server_position_sync_c = 0;
	uint_fast8_t server_position_sync_s = 0;
	double server_position_sync_x = 0;
	double server_position_sync_y = 0;

	//Overworld
	bool ow_pos_inited = false;
	string ow_level_name;
	uint_fast8_t ow_old_level;

	//On object creation
	MPlayer(double newX = 0.0, double newY = 0.0) {
		x = newX; server_position_sync_x = x;
		y = newY; server_position_sync_y = y;
		server_position_sync_s++;

		CAMERA_X = x;
		CAMERA_Y = y;
	}

	//Send a chat message.
	void Chat(string new_c) {
		if (new_c.length() > 0)
		{
			curr_chat_string = "<";
			curr_chat_string += username;
			curr_chat_string += "> " + new_c;
			current_chat++;
		}
	}

	//Use this to kill the player.
	void Die() {
		if (PlayerControlled) {
			if (!DEAD) {
				X_SPEED = 0;
				Y_SPEED = 0;
				STATE = 0;
				RAM[0x1DFC] = 100;
				if (WO_counter != 255) {
					WO_counter++;
				}
				
				if (!((networking || local_multiplayer) || gamemode == GAMEMODE_TITLE)) {
					RAM[0x9D] = 0;

					//Retry incorporation
					if (useRetry) {
						retryPromptOpen = true;
						RAM[0x1B88] = 3;
					}
					else {
						RAM[0x1DFB] = 9;
					}
				}
				DEATH_TIMER = y < -16.0 ? 150 : 32;
				DEAD = true;
			}
		}
	}

	//Use this to hurt the player.
	bool Hurt() {
		if (INVINCIBILITY_FRAMES_STAR > 0) {
			return false;
		}
		if (!DEAD && INVINCIBILITY_FRAMES == 0) {
			if (STATE != 0) {
				STATE = 0;
				powerup_anim = 0xDF;
				INVINCIBILITY_FRAMES = 90;
				RAM[0x1DF9] = 4;
				if (!networking) { RAM[0x9D] = 0; }
			}
			else {
				Die();
				return true;
			}
		}
		return false;
	}

	//Clear/reset player variables to respawn them.
	void Respawn(bool resetState = true) {
		if (local_multiplayer && resetState) {
			x = getRamValue(0x1462, 2) + 112;
			y = getRamValue(0x1464, 2) + 300;
		}
		else {
			x = LevelManager.start_x;
			y = LevelManager.start_y;
		}
		server_position_sync_x = x;
		server_position_sync_y = y;
		server_position_sync_s++; powerup_anim = 0;
		INVINCIBILITY_FRAMES = (networking || gamemode == GAMEMODE_TITLE) ? 120 : 0; INVINCIBILITY_FRAMES_STAR = 0;
		DEATH_TIMER = 0; P_METER = 0;
		DEAD = false; in_pipe = false; jump_is_spin = false;
		X_SPEED = 0; Y_SPEED = 0;

		if (resetState) { STATE = 0; }

		ResetCamera();

		cape_state = 0;
		cape_ascent_timer = 0;
		cape_spin_timer = 0;
	}

	//Get Controller buttons currently held down. Format: byetUDLR.
	//b = A or B; y = X or Y; e = select; t = Start; U = up; D = down; L = left, R = right.
	void getInput() {
		if (RAM[0x1493] > 0) {
			for (int inputs = 0; inputs < total_inputs; inputs++) {
				p_pad[inputs] = false;
			}
		}
		else {
			//TO-DO: Modularize local multiplayer behaviour.
			if (local_multiplayer && player_index > 0) {
				FetchControllerInput(player_index - 1);

				p_pad[button_y] = BUTTONS_GAMEPAD[7] || BUTTONS_GAMEPAD[6];
				p_pad[button_b] = BUTTONS_GAMEPAD[4];
				p_pad[button_a] = BUTTONS_GAMEPAD[5];
				p_pad[button_left] = BUTTONS_GAMEPAD[3];
				p_pad[button_right] = BUTTONS_GAMEPAD[1];
				p_pad[button_down] = BUTTONS_GAMEPAD[2];
				p_pad[button_up] = BUTTONS_GAMEPAD[0];
				p_pad[button_start] = BUTTONS_GAMEPAD[9];
				return;
			}
			if (PlayerControlled) {
				for (int inputs = 0; inputs < total_inputs; inputs++) {
					p_pad[inputs] = s_pad[inputs];
				}
			}
		}
	}

	//Called when an enemy is jumped on. Gives the player score.
	void Enemy_Jump() {
		if (enemyjump_cooldown) { return; }
		enemyjump_cooldown = 8;

		score_streak++;
		if (!isClient) {
			addScore(uint_fast32_t(score_streak) * 200);
		}

		createParticle(0x44, 0x11, 0x8, 5, x, y - 24.0, 0, 0, 0);
		
		if (!jump_is_spin) {
			if (p_pad[button_b] || p_pad[button_a]) {
				Y_SPEED = Calculate_Speed(1408);
			}
			else {
				Y_SPEED = Calculate_Speed(800);
			}
		}
	}

	//Called when the player jumps out of water, gives them a boost.
	void JumpOutOfWater() {
		Y_SPEED = Calculate_Speed(0x500 + (STATE > 0) * 0x80);
		jump_is_spin = p_pad[button_a];
	}

	//Called when a enemy is spinjumped on.
	void Enemy_Jump_Spin(bool block_snd = false) {
		if (enemyjump_cooldown) { return; }
		enemyjump_cooldown = 8;

		//Hitspark
		createParticle(0x44, 0x11, 0x8, 5, x, y - 24.0, 0, 0, 0);

		if (!block_snd) {
			RAM[0x1DF9] = 2;
		}
		if (p_pad[button_b] || p_pad[button_a]) {
			Y_SPEED = Calculate_Speed(1408);
		}
		else {
			Y_SPEED = Calculate_Speed(800);
		}
	}

	//Called while the player is dead.
	int DeathProcess() {
		if (retryPromptOpen && !networking) {
			pose = POSE_DEAD;
			return 1;
		}
		x += X_SPEED;
		y += Y_SPEED;

		if ((networking || local_multiplayer) || gamemode == GAMEMODE_TITLE) {
			double Bound = RAM[0x1412] ? (max(0.0, CAMERA_Y - 128.0) - 16.0) : (double(getRamValue(0x1464, 2)) - 64.0);
			if (y < Bound || DEATH_TIMER < -360) { //death failsafe to respawn after 6 seconds
				Respawn();
			}
		}
		else {
			if (useRetry) {
				writeToRam(0x3F08, getRamValue(0x010B, 2), 2);
				if (doRetry) {
					RAM[0x3F11] = 4;
				}
			}
			if (y < -496.0) {
				RAM[0x3F11] = 5;
			}
		}

		DEATH_TIMER--;
		if (DEATH_TIMER) {
			if (!DEATH_TIMER && y > -14.0) {
				Y_SPEED = Calculate_Speed(1280.0);
			}
		}
		if (DEATH_TIMER <= 0) {
			if (!(global_frame_counter % 5)) {
				to_scale *= -1;
			}
			Y_SPEED = max(-5.0, Y_SPEED - Calculate_Speed(48.0));
		}
		else {
			X_SPEED = 0.0;
			Y_SPEED = 0.0;
		}

		pose = POSE_DEAD;
		return 1;
	}

	//Called to process grabbed objects.
	void ProcessGrabbed() {
		if (GRABBED_SPRITE != 0xFF && !isClient) {
			if (RAM[0x2000 + GRABBED_SPRITE] != 0 && RAM[0x2000 + GRABBED_SPRITE] != 4) {
				RAM[0x2000 + GRABBED_SPRITE] = 0x03;
			}
			if (RAM[0x2000 + GRABBED_SPRITE] == 0 || RAM[0x2000 + GRABBED_SPRITE] == 4) {
				GRABBED_SPRITE = 0xFF;
			}
			if (GRABBED_SPRITE != 0xFF) {
				uint_fast16_t x_position = uint_fast16_t(double(x + (climbing ? 0 : (to_scale * -16.0))));
				uint_fast16_t y_position = uint_fast16_t(double(y - (STATE > 0 ? 13.0 : 16.0)) + 17.0);
				if (RAM[0x2000 + GRABBED_SPRITE] == 3) {
					RAM[0x2100 + GRABBED_SPRITE] = uint_fast8_t(x_position & 0xFF);
					RAM[0x2180 + GRABBED_SPRITE] = uint_fast8_t(x_position >> 8);
					RAM[0x2200 + GRABBED_SPRITE] = 0x00;

					RAM[0x2280 + GRABBED_SPRITE] = uint_fast8_t(y_position & 0xFF);
					RAM[0x2300 + GRABBED_SPRITE] = uint_fast8_t(y_position >> 8);
					RAM[0x2380 + GRABBED_SPRITE] = 0x00;

					RAM[0x2480 + GRABBED_SPRITE] = 0;
					RAM[0x2400 + GRABBED_SPRITE] = 0;
				}

				if (!in_pipe && (!p_pad[button_y] || DEAD)) {
					RAM[0x2E00 + GRABBED_SPRITE] = 0x08;
					RAM[0x2780 + GRABBED_SPRITE] = 0;

					RAM[0x2000 + GRABBED_SPRITE] = 0x02;

					RAM[0x2480 + GRABBED_SPRITE] = 0;
					RAM[0x2400 + GRABBED_SPRITE] = uint_fast8_t(int_fast8_t(to_scale * -4)) + uint_fast8_t(int_fast8_t(X_SPEED * 16.0));

					if (p_pad[button_up]) {
						RAM[0x2480 + GRABBED_SPRITE] = 0x70;
						RAM[0x2400 + GRABBED_SPRITE] = uint_fast8_t(int_fast8_t(X_SPEED * 8.0));
						writeToRam(0x1DF9, 0x3, 1);
					}

					if (!p_pad[button_up] && !p_pad[button_down] && RAM[0x2080 + GRABBED_SPRITE] != 0x3E) {
						RAM[0x2680 + GRABBED_SPRITE] = int_fast8_t(to_scale);
						RAM[0x2000 + GRABBED_SPRITE] = 0x04;
						RAM[0x2480 + GRABBED_SPRITE] = 0x00;

						RAM[0x2400 + GRABBED_SPRITE] = uint_fast8_t(int_fast8_t((-0x2E * to_scale) + (X_SPEED * 8.0)));
						writeToRam(0x1DF9, 0x3, 1);
					}
					if (!p_pad[button_down] && RAM[0x2080 + GRABBED_SPRITE] != 0x3E) {
						//Hitspark
						createParticle(0x44, 0x11, 0x8, 5, x_position, y_position - 16.0, 0, 0, 0);
					}
					GRABBED_SPRITE = 0xFF;
				}
			}
		}
	}

	//CAPE & CAPE CLOTH SIMULATION
	void Cape_Cloth_Simulation()
	{
		if (STATE != 2) {
			CAPE_CLOTH_Y = 0.0;
			CAPE_FRAME = 0;
			CAPE_ST = 0;
		}
		else {
			double CAPE_NEW_Y = ON_FL ? (1.0 - (X_SPEED != 0.0)) : (Y_SPEED > 0.0 ? !CAN_SPRINT : -1.0);
			CAPE_CLOTH_Y += (CAPE_NEW_Y - CAPE_CLOTH_Y) / 10.0;
			int CAPE_ST_N = cape_spin_timer ? 2 : int(round(CAPE_CLOTH_Y));
			if (CAPE_ST_N != CAPE_ST) {
				CAPE_ST = CAPE_ST_N;
				CAPE_FRAME = 0;
			}
			if (CAPE_ST == 2) { CAPE_FRAME = global_frame_counter >> 1; }
			else
			{
				if (!(global_frame_counter & 3)) {
					if (CAPE_ST == 1) {
						CAPE_FRAME = min(2, CAPE_FRAME + 1);
					}
					else {
						CAPE_FRAME++;
					}
				}
			}
		}
	}

	void ProcessCape() {
		if (STATE == 2) {
			if (!isClient) {
				if (jump_is_spin && Y_SPEED > 0) { cape_spin_timer = 2; }
				if (cape_spin_timer) { cape_spin_timer--; }
				if (pressed_y) {
					RAM[0x1DFC] = 0x04;
					cape_spin_timer = 16;
				}
			}
			if (cape_ascent_timer > 0 && !IN_WT && !CROUCH) {
				bool holding_jump = p_pad[button_a] || p_pad[button_b];
				if (jump_is_spin)
				{
					cape_ascent_timer--;
					if (holding_jump) { Y_SPEED = max(Calculate_Speed(0x380), Y_SPEED - Calculate_Speed(0x30)); }
				}
				else
				{
					if (holding_jump) {
						Y_SPEED = max(Calculate_Speed(0x380), Y_SPEED - Calculate_Speed(0x30));
					}
					else {
						Y_SPEED = max(0.0, Y_SPEED - Calculate_Speed(0x60 / (1 + holding_jump)));
					}
					if (!cape_ascent_timer || Y_SPEED <= 0) {
						cape_ascent_timer = 0;
						//Put the cape set state code here.. Otherwise nothing
					}
					else {
						cape_ascent_timer--;
					}
				}
			}
			else {
				//Deault slow fall logic
				cape_ascent_timer = 0;
				if (p_pad[button_b] || p_pad[button_a]) {
					Y_SPEED = max(-1.0, Y_SPEED);
				}
			}
		}
		else {
			cape_state = 0;
			cape_ascent_timer = 0;
			cape_spin_timer = 0;
		}
	}

	//FIREBALLS
	void ProcessFireballs() {
		if (!isClient) {
			if (STATE == 3 && !in_pipe && !SLIDING && GRABBED_SPRITE == 0xFF && !CROUCH && !climbing) {
				if (fire_anim_timer) { fire_anim_timer--; }
				if (pressed_y) {
					fire_anim_timer = 10;
					uint_fast8_t s = spawnSpriteObj(0x33, 0x1, uint_fast16_t(double(x + to_scale * -11.0)), uint_fast16_t(double(y + 9.0)), to_scale);
					RAM[0x2E00 + s] = player_index; RAM[0x2480 + s] = 0xD8;
					RAM[0x1DFC] = 6;
				}
				if (jump_is_spin && !(ingame_frame_counter % 16) && !ON_FL) {
					uint_fast8_t rand_dir = (ingame_frame_counter >> 4) & 1;
					uint_fast8_t s = spawnSpriteObj(0x33, 0x1, uint_fast16_t(double(x + 8 + rand_dir * -16.0)),  uint_fast16_t(double(y + 9.0)), 0xFF + (rand_dir << 1));
					RAM[0x2E00 + s] = player_index; RAM[0x2480 + s] = 0xD8;
					RAM[0x1DFC] = 6;
				}
			}
			else {
				fire_anim_timer = 0;
			}
		}
	}

	//RESERVE
	void ProcessReserve() {
		if (!isClient) {
			if (p_pad[button_start] != old_s) {
				old_s = p_pad[button_start];
				if (old_s && reserve_item != 0) {
					uint_fast8_t s = spawnSpriteObj(reserve_lookup[reserve_item], 0x1, uint_fast16_t(max(0.0, CAMERA_X - 120.0) + 120.0), uint_fast16_t(max(0.0, CAMERA_Y - 112.0) + 176.0), 0);
					RAM[0x2A00 + s] = 1; RAM[0x1DFC] = 0xC;
					reserve_item = 0;
				}
			}
		}
	}

	//Pipe enter state.
	void EnterPipe(int_fast8_t xe, int_fast8_t ye) {
		in_pipe = true; pipe_speed_x = xe; pipe_speed_y = ye; writeToRam(0x1DF9, 0x4, 1);
	}

	//Process in-pipe state.
	void ProcessPipes() {
		y += pipe_speed_y * 3;
		x += pipe_speed_x * 3;
		uint_fast16_t check_y_1 = uint_fast16_t((y + height - 1) / 16.0);
		uint_fast16_t check_y_2 = uint_fast16_t((y + 1) / 16.0);
		uint_fast16_t check_x_1 = uint_fast16_t((x) / 16.0);
		uint_fast16_t check_x_2 = uint_fast16_t((x + 16.0) / 16.0);
		CROUCH = false;
		X_SPEED = 0;
		Y_SPEED = 0;
		if (
			!map16_handler.check_solid(check_x_1, check_y_1) && !map16_handler.check_solid(check_x_2, check_y_2)
			) {
			in_pipe = false;
			pipe_speed_x = 0;
			pipe_speed_y = 0;
			writeToRam(0x1DF9, 0x4, 1);
		}
	}

	//Movement and collision.
	bool Move(double xMove, double yMove, bool do_change = false) {
		height = (STATE > 0 && (!CROUCH || jump_is_spin)) ? 28.0 : 14.0;

		double NewPositionX = x + xMove;
		double NewPositionY = y + yMove;
		bool willreturn = true;
		int_fast16_t PosXBlock = int_fast16_t(NewPositionX / 16);
		int_fast16_t PosYBlock = int_fast16_t(NewPositionY / 16);
		int_fast16_t startX = max(0, PosXBlock - TotalBlocksCollisionCheck);
		int_fast16_t startY = max(0, PosYBlock - TotalBlocksCollisionCheck);

		for (uint_fast8_t sprite = 0; sprite < 128; sprite++) {
			if (RAM[0x2000 + sprite] != 0) {
				double sprite_x = double(RAM[0x2100 + sprite] + int(int_fast8_t(RAM[0x2180 + sprite])) * 256) + double(RAM[0x2200 + sprite]) / 256.0;
				double sprite_y = double(RAM[0x2280 + sprite] + int(int_fast8_t(RAM[0x2300 + sprite])) * 256) + double(RAM[0x2380 + sprite]) / 256.0;
				double sprite_x_size = double(RAM[0x2500 + sprite]);
				double sprite_y_size = double(RAM[0x2580 + sprite]);

				double BelowSprite = sprite_y - height;
				double AboveSprite = sprite_y + sprite_y_size;
				double RightSprite = sprite_x + sprite_x_size - 2.0;
				double LeftSprite = sprite_x - 14.0;

				bool checkRight = RAM[0x2600 + sprite] & 0b1;
				bool checkLeft = RAM[0x2600 + sprite] & 0b10;
				bool checkBottom = RAM[0x2600 + sprite] & 0b100;
				bool checkTop = RAM[0x2600 + sprite] & 0b1000;

				bool results[4] = { false,false,false,false };
				if (NewPositionX < RightSprite && NewPositionX > LeftSprite && NewPositionY < AboveSprite && NewPositionY > BelowSprite) {
					if (RAM[0x2600 + sprite] & 0b10000000) { //sprite is solid
						if (RAM[0x2080 + sprite] == 0x33) {
							if (RAM[0x2E00 + sprite] != player_index && pvp) {
								Hurt();
								if (!isClient) {
									RAM[0x2B00 + sprite] = 1;
								}
							}
						}
						else {
							if (RAM[0x2600 + sprite] & 0b10000) { //can jump on sprite
								if (INVINCIBILITY_FRAMES_STAR > 0) {
									results[2] = true;
									RAM[0x2B00 + sprite] = 0x2;
									RAM[0x2700 + sprite] = 0xFF;
								}
								else {
									if (NewPositionY > (AboveSprite - bounds_y)) {
										//NewPositionY += 1;
										if (jump_is_spin) {
											if (RAM[0x2880 + sprite] & 1) {
												if (RAM[0x2880 + sprite] & 2) {
													RAM[0x2A00 + sprite] = 0x01;
												}
												else {
													RAM[0x2000 + sprite] = 0;
												}

												//High jumps
												if (RAM[0x2880 + sprite] & 0x20) {
													if (p_pad[button_b] || p_pad[button_a]) {
														Y_SPEED = Calculate_Speed(1280);
													}
													else {
														Y_SPEED = Calculate_Speed(512);
													}
												}
												else {
													Y_SPEED = Calculate_Speed(128);
												}
												writeToRam(0x1DF9, 0x8, 1);
											}
											else {

												if (RAM[0x2880 + sprite] & 0x40) {
													results[2] = true;
												}
												Enemy_Jump_Spin(RAM[0x2880 + sprite] & 0x40);
											}
										}
										else {
											
											Enemy_Jump();

											//Chuck-like jumps
											if (RAM[0x2800 + sprite] & 1) {
												RAM[0x1DF9] = 2;
												X_SPEED = (x - sprite_x) > 0 ? 1.5 : -1.5;
											}

											results[2] = true;
										}
									}
									else {
										if (SLIDING) {
											if (RAM[0x2880 + sprite] & 0b100000) {
												results[2] = true;
											}
											else {
												RAM[0x2B00 + sprite] = 1;
											}
										}
										else {
											Hurt();
										}
									}
								}
							}
							else { //we cant jump on this sprite
								if (INVINCIBILITY_FRAMES_STAR > 0) {
									results[2] = true;
									RAM[0x2B00 + sprite] = 0x2;
									RAM[0x2700 + sprite] = 0xFF;
								}
								else {
									if (jump_is_spin && NewPositionY > (AboveSprite - bounds_y)) {
										if (RAM[0x2880 + sprite] & 0x40) {
											Hurt();
										}
										else {
											Enemy_Jump_Spin();
										}
									}
									else {
										Hurt();
									}
								}
							}
						}
					}

					if (do_change && (checkTop && checkBottom && checkRight && checkLeft)) { //Push out of bounding if in area.
						if (NewPositionY < AboveSprite && NewPositionY > BelowSprite) {
							if (NewPositionX < RightSprite && NewPositionX > RightSprite - bounds_x) {
								x = RightSprite;
							}
							if (NewPositionX > LeftSprite && NewPositionX < LeftSprite + bounds_x) {
								x = LeftSprite;
							}
						}
					}
					else {
						if (checkRight && xMove < 0.0 && NewPositionX < RightSprite && x >= RightSprite) {
							NewPositionX = RightSprite;
							willreturn = false;
							results[0] = true;
						}
						if (checkLeft && xMove > 0.0 && NewPositionX > LeftSprite && x <= LeftSprite) {
							NewPositionX = LeftSprite;
							willreturn = false;
							results[1] = true;
						}
					}
					double F = double(Sint8(RAM[0x2480 + sprite])) / 16.0;
					if (checkTop && yMove < 0.0 && NewPositionY < AboveSprite && y >= (AboveSprite - (Y_SPEED <= 0 ? 4.0 : 1.0) - abs(F))) {
						willreturn = false;
						if (Y_SPEED <= 0) {
							results[2] = true;
						}
						xMove = double(Sint8(RAM[0x2400 + sprite])) / 16.0;
						NewPositionY = AboveSprite + F;
						NewPositionX += xMove;
						do_change = false;
					}
					if (checkBottom && yMove > 0.0 && NewPositionY > BelowSprite && y <= BelowSprite) {
						NewPositionY = BelowSprite;
						willreturn = false;
						results[3] = true;
					}

					RAM[0x2700 + sprite] = 0;
					for (uint_fast8_t i = 0; i < 4; i++) {
						RAM[0x2700 + sprite] += results[i] << i;
					}

					//Death kick
					if (RAM[0x2880 + sprite] & 4) {
						RAM[0x2B00 + sprite] = 1;
					}

					//Powerups
					if (RAM[0x2000 + sprite] >= 5 && RAM[0x2000 + sprite] <= 8) {
						uint_fast8_t NEW_STATE = new_state_lookup[RAM[0x2000 + sprite] - 5];
						if (NEW_STATE < 0x80) { //Isn't special state. 
							if (!isClient) {
								if (NEW_STATE > min(1, STATE)) {
									if (STATE != 0) {
										RAM[0x1DFC] = 0xB;
										reserve_item = min(3, STATE);
									}
									player_netcommand[player_index] = 0xCF + (NEW_STATE << 4);
									RAM[0x1DF9] = NEW_STATE == 2 ? 0xD : 0xA;
								}
								else {
									RAM[0x1DFC] = 0xB;
									reserve_item = min(3, NEW_STATE);
								}

								addScore(1000);
								RAM[0x2000 + sprite] = 0;
							}
						}
						else {
							if (!isClient) {
								if (RAM[0x1DFB] != 0xB && RAM[0x1DFB] != 0xA) {
									RAM[0x0DDA] = RAM[0x1DFB];
									RAM[0x1DFB] = 0xA;
								}

								RAM[0x1DF9] = 0xA;
								INVINCIBILITY_FRAMES_STAR = 0xFF;
								RAM[0x1490] = 0xFF;
								RAM[0x2000 + sprite] = 0;
							}
						}
					}

					//Grabbing
					if (GRABBED_SPRITE == 0xFF && !isClient) {
						if (RAM[0x2000 + sprite] == 2 && RAM[0x2E00 + sprite] == 0) {
							if (p_pad[button_y]) {
								RAM[0x2000 + sprite] = 3;
								GRABBED_SPRITE = sprite;
							}
							else {
								if (RAM[0x2080 + sprite] != 0x3E) {
									if ((RAM[0x2880 + sprite] & 2) && jump_is_spin && RAM[0x2080 + sprite] != 0x53) {
										RAM[0x2A00 + sprite] = 0x01;
										RAM[0x2000 + sprite] = 0x01;
										Y_SPEED = Calculate_Speed(128);
										RAM[0x1DF9] = 0x8;
									}
									else {
										RAM[0x2680 + sprite] = uint_fast8_t(X_SPEED > 0 ? 1 : -1);
										RAM[0x2400 + sprite] = uint_fast8_t((X_SPEED > 0 ? 1 : -1) * 0x40);

										RAM[0x2000 + sprite] = 4;
										RAM[0x2E00 + sprite] = 0x10;

										RAM[0x1DF9] = 0x3;
									}
								}
							}
						}
					}

				}
			}
		}

		//Don't do block collision if we died from a sprite
		if (DEAD) {
			return false;
		}

		//Shitty block collision. Surprised this doesn't lag.
		for (int_fast16_t xB = startX; xB < PosXBlock + TotalBlocksCollisionCheck; xB++) {
			for (int_fast16_t yB = startY; yB < PosYBlock + TotalBlocksCollisionCheck + (STATE > 0); yB++) {
				map16_handler.update_map_tile(xB, yB);
				uint_fast8_t new_s = map16_handler.get_slope();
				double f_h = map16_handler.ground_y(NewPositionX + 8.0 - (xB * 16));
				double f_w = map16_handler.ground_s();
				double BelowBlock = double(yB * 16) + (f_h - 16.0) - height;
				double AboveBlock = double(yB * 16) + f_h;
				double RightBlock = double(xB * 16) + f_w;
				double LeftBlock = double(xB * 16) - f_w;

				bool checkLeft = map16_handler.logic[3];
				bool checkRight = map16_handler.logic[2];
				bool checkBottom = map16_handler.logic[1];
				bool checkTop = map16_handler.logic[0];

				bool playHitSound = true;

				if (IN_WT && yMove > 0.0 && !RAM[0x85]) {
					if (map16_handler.get_tile(xB, yB) == 0x25 && NewPositionY > WaterLevel) {
						if (p_pad[button_up] && (p_pad[button_b] || p_pad[button_a])) {
							JumpOutOfWater();
						}
						else {
							checkBottom = true;
							playHitSound = false;
						}
					}
				}

				if (NewPositionX < RightBlock && NewPositionX > LeftBlock && NewPositionY < AboveBlock && NewPositionY > BelowBlock) {
					if (map16_handler.logic[5]) {
						Die();
					}
					else {
						if (map16_handler.logic[4]) {
							Hurt();
						}
					}
					map16_handler.process_block(xB, yB, inside);

					if (xMove < 0.0 && checkRight && NewPositionX < RightBlock && x >= RightBlock) {
						NewPositionX = RightBlock;
						willreturn = false;
						map16_handler.process_block(xB, yB, right, pressed_y);
						if (p_pad[button_left] && map16_handler.get_tile(xB, yB) == 0x13F && ON_FL) {
							EnterPipe(-2, 0);
						}
					}
					if (xMove > 0.0 && checkLeft && NewPositionX > LeftBlock && x <= LeftBlock) {
						NewPositionX = LeftBlock;
						willreturn = false;
						map16_handler.process_block(xB, yB, left, pressed_y);
						if (p_pad[button_right] && map16_handler.get_tile(xB, yB) == 0x13F && ON_FL) {
							EnterPipe(2, 0);
						}
					}
					double AboBlockCheck = AboveBlock - ((new_s || SLOPE_TYPE) ? (3.0 + abs(X_SPEED)) : 0);
					if (yMove < 0.0 && checkTop && NewPositionY < AboveBlock && y >= AboBlockCheck) {
						NewPositionY = AboveBlock;
						willreturn = false;
						uint_fast8_t new_s = map16_handler.get_slope();
						if (new_s != 0 && SLOPE_TYPE == 0) { SLOPE_TYPE = new_s; }
						if ((jump_is_spin && map16_handler.get_tile(xB, yB) == 0x11E) && STATE > 0) {
							map16_handler.process_block(xB, yB, top, pressed_y, true);
							Y_SPEED = Calculate_Speed(768); willreturn = true;
						}
						else {
							map16_handler.process_block(xB, yB, top, pressed_y);
						}
						if (p_pad[button_down] && map16_handler.get_tile(uint_fast16_t((NewPositionX + 4.0) / 16.0), yB) == 0x137 && map16_handler.get_tile(uint_fast16_t((NewPositionX + 12.0) / 16.0), yB) == 0x138) {
							EnterPipe(0, -2);
						}
					}
					if (yMove > 0.0 && checkBottom && NewPositionY > BelowBlock && y <= BelowBlock) {
						NewPositionY = BelowBlock;
						willreturn = false;
						if (playHitSound) {
							writeToRam(0x1DF9, 0x1, 1);
						}
						map16_handler.process_block(xB, yB, bottom, false, false, STATE);
						if (p_pad[button_up] && map16_handler.get_tile(uint_fast16_t((NewPositionX + 4.0) / 16.0), yB) == 0x137 && map16_handler.get_tile(uint_fast16_t((NewPositionX + 12.0) / 16.0), yB) == 0x138) {
							EnterPipe(0, 2);
						}
					}

					if (spawned_grabbable != 0xFF && !isClient && GRABBED_SPRITE == 0xFF) {
						GRABBED_SPRITE = spawned_grabbable;
						RAM[0x2000 + GRABBED_SPRITE] = 0x03;
					}
					spawned_grabbable = 0xFF;
				}
			}
		}

		if (!do_change)
		{
			x = NewPositionX;
			y = NewPositionY;
		}
		return willreturn;
	}

	//State Rendering
	int state_str() {
		if (powerup_anim) {
			if (STATE == 3) {
				return 1 + (((powerup_anim & 0x3F) >> 2) & 1);
			}
			else {
				uint_fast8_t FRMD = powerup_anim & 0x3F;
				uint_fast8_t TYP = powerup_anim >> 6; //0 - small->big 1 - any-> cape  2 - any->flower
				return TYP == 3 ? (FRMD >= 16) : (FRMD < 16);
			}
		}
		else {
			return (STATE == 3 ? 2 : min(STATE, 1));
		}
	}

	//Get player sprite/state for rendering. Runs simulations and previous objects.
	void Get_Sprite() {
		Cape_Cloth_Simulation();
		pose = p_pad[button_up] ? POSE_LOOKUP : POSE_STAND;
		if (GRABBED_SPRITE != 0xFF) {
			pose = pose == POSE_LOOKUP ? POSE_GRABLOOKUP : POSE_GRAB;
		}
		if (in_pipe) { pose = POSE_FRONT; return; }
		if (SLIDING) { pose = POSE_SLIDE; return; }
		if ((jump_is_spin && !ON_FL) || cape_spin_timer) {
			uint_fast8_t frIndex = (global_frame_counter >> 1) & 3;
			to_scale = PlayerSpinScale[frIndex];
			pose = PlayerSpinFrames[frIndex];
			return;
		}
		if (climbing) { pose = POSE_CLIMB; }
		else
		{
			if (fire_anim_timer > 0) {
				to_scale = WALKING_DIR > 0 ? -1 : 1;
				pose = POSE_POWER_FIRE + ON_FL;
			}
			else {
				if (!CROUCH) {
					if (!ON_FL) {
						if (IN_WT) {
							pose = POSE_SWIM;
						}
						else {
							if (GRABBED_SPRITE != 0xFF) {
								pose = POSE_GRAB + (STATE > 0 ? 2 : 1);
							}
							else {
								if (CAN_SPRINT) {
									pose = POSE_JUMPB;
								}
								else {
									pose = Y_SPEED > 0.0 ? POSE_JUMP : POSE_FALL;
								}
							}
						}
					}
					else {
						if (SKIDDING == 0) {
							if (X_SPEED != 0) {
								FRM += X_SPEED / 5;
								int Frame = abs(int(FRM) % (2 + (STATE > 0)));
								if (GRABBED_SPRITE != 0xFF) {
									pose = POSE_GRAB + Frame;
								}
								else {
									if (CAN_SPRINT) {
										pose = POSE_RUN + Frame;
									}
									else {
										pose = POSE_WALK + Frame;
									}
								}
							}
						}
						else {
							pose = GRABBED_SPRITE == 0xFF ? POSE_SKID : POSE_FRONT;
						}
					}
				}
				else {
					pose = GRABBED_SPRITE == 0xFF ? POSE_CROUCH : POSE_GRABCROUCH;
				}
			}
		}
		if (WALKING_DIR != 0) { to_scale = WALKING_DIR > 0 ? -1 : 1; }
	}

	//Main Process
	int Process() {
		if (RAM[0x3F11] == 2) {
			Respawn(false);
			CAMERA_X = x;
			CAMERA_Y = y;
			invisible = true;
			return 1;
		}

		//this will cause 1 frame of delay oh well, it fixes MP atleast
		if (!PlayerControlled && DEAD != OLD_DEAD && isClient) {
			OLD_DEAD = DEAD;
			if (DEAD) {
				RAM[0x1DFC] = 100;
			}
		}

		getInput();

		//lol
		if (!RAM[0x9D] || powerup_anim) {
			if (DEAD) {
				ProcessGrabbed();
				return DeathProcess();
			}
			if (powerup_anim) {
				uint_fast8_t FRMD = powerup_anim & 0x3F;
				uint_fast8_t TYP = powerup_anim >> 6; //0 - small->big 1 - any-> cape  2 - any->flower
				invisible = false;
				if (TYP == 0 || TYP == 3) { pose = ((FRMD >> 2) & 1) ? POSE_STAND : POSE_POWER_TRANSITION; } else { Get_Sprite(); }
				if (TYP == 1) { invisible = FRMD != 31; }
				if (FRMD > 0) { FRMD--; }
				if (!FRMD && !networking) { player_netcommand[player_index] = 0; RAM[0x9D] = 1; }
				powerup_anim = !FRMD ? 0 : (FRMD + (TYP << 6));
			}
			return 1;
		}
		SLOPE_TYPE = 0;

		if (p_pad[button_y] != old_y) {
			old_y = p_pad[button_y];
			if (old_y) {
				pressed_y = true;
			}
		}

		if (enemyjump_cooldown) {
			enemyjump_cooldown--;
		}

		if (y < -32.0 && !DEAD) {
			Die();
		}

		if (RAM[0x1412] == 0 && !(RAM[0x3F1F] & 4)) {
			if (y < (CAMERA_Y - 192.0) && !DEAD) {
				Die();
			}
		}

		if (DEAD) {
			ProcessGrabbed();
			return DeathProcess();
		}

		if (INVINCIBILITY_FRAMES > 0) {INVINCIBILITY_FRAMES--;}
		if (INVINCIBILITY_FRAMES_STAR > 0 && !isClient) {
			bool doStar = INVINCIBILITY_FRAMES_STAR < 64 ? !(ingame_frame_counter & (INVINCIBILITY_FRAMES_STAR < 32 ? (INVINCIBILITY_FRAMES_STAR < 16 ? 7 : 3) : 1)) : true;
			if (doStar) {
				createParticle(0x4B, 0x00, 0x8C, 9, x - 4 + (rand() % 16), y - 28 + (rand() % (14 + (STATE > 0) * 14)), 0, 0, 0);
			}
			if (!(ingame_frame_counter & 3)) {
				INVINCIBILITY_FRAMES_STAR -= 1;
			}
		}
		invisible = INVINCIBILITY_FRAMES > 0 ? !invisible : false;

		double GRAV = (cape_ascent_timer && (jump_is_spin ? p_pad[button_a] : true)) ? 0.0 : -double(RAM[0x7C]) / (1 + (p_pad[button_a] || p_pad[button_b]));
		bool RUN = p_pad[button_y];
		bool MOV = false;
		bool SLIGHT_HIGH_SPEED = false;

		uint_fast16_t check_x_1 = uint_fast16_t((x + 8) / 16.0);
		uint_fast16_t check_y_1 = uint_fast16_t((y + height - 1) / 16.0);

		IN_WT = RAM[0x85] != 0 || map16_handler.get_tile(check_x_1, check_y_1) < 4;

		if (WaterLevel > 0 && y <= (WaterLevel - height)) {
			IN_WT = true;
		}

		if (IN_WT != OLD_WT) {
			OLD_WT = IN_WT;
			if (IN_WT) {
				Y_SPEED = 0;
				X_SPEED = 0;
				P_METER = 0;

				//Smoke
				createParticle(0x60, 0x11, 0x8, 10, x, y - (STATE == 0 || CROUCH)*16, 0.5, 1, Calculate_Speed(24));
			}
		}
		if (in_pipe) {
			ProcessPipes();
		}
		else {
			//Floor Hit Detection
			ON_FL = false;
			if (!Move(0.0, -1.0, true) && Y_SPEED <= 0) { //Detected a floor below
				ON_FL = true;
				score_streak = 0;
				jump_is_spin = false;
			}
			if (!IN_WT) {
				if (!climbing) {
					if (ON_FL) {
						CAN_SPRINT = false;
					}

					SKIDDING = 0;
					SLIGHT_HIGH_SPEED = abs(X_SPEED) >= Calculate_Speed(576.0);
					if (
						SLIGHT_HIGH_SPEED &&
						(CAN_SPRINT || (ON_FL && !CROUCH)) &&
						p_pad[button_y] && (p_pad[button_left] || p_pad[button_right])
					) {
						P_METER = min(P_METER_REQUIRED, P_METER + 2);
					}
					else {
						if (P_METER > 0) { P_METER--; }
					}
					if (P_METER >= P_METER_REQUIRED && p_pad[button_y]) {
						CAN_SPRINT = true;
					}
					MOV = p_pad[button_left] || p_pad[button_right];
					if (p_pad[button_left]) { WALKING_DIR = -1; }
					if (p_pad[button_right]) { WALKING_DIR = 1; }

					//Slide cancel
					if (SLIDING) {
						if ((MOV || (X_SPEED == 0 && !SLOPE_TYPE)) && ON_FL) {
							SLIDING = false;
						}
					}
					//Crouching
					if (p_pad[button_down] && !SLIDING) {
						if (SLOPE_TYPE != 0) {
							if (GRABBED_SPRITE == 0xFF) {
								if (!MOV && !(p_pad[button_left] || p_pad[button_right])) {
									SLIDING = true;
								}
								else {
									CROUCH = false;
								}
							}
							else {
								WALKING_DIR = 0; MOV = false; CROUCH = true;
							}
						}
						else {
							if (ON_FL) {
								WALKING_DIR = 0; MOV = true; CROUCH = true;
							}
						}
					}
					else {
						if (ON_FL) {
							CROUCH = false;
						}
					}

					bool isJumping = p_pad[button_b] || p_pad[button_a];
					if (isJumping != wasJumping) {
						wasJumping = isJumping;
						if (isJumping && ON_FL) {
							SLIDING = false;
							if (STATE == 2 && CAN_SPRINT) { cape_ascent_timer = 78; }
							if (p_pad[button_a] && GRABBED_SPRITE == 0xFF) {
								//Spinjump
								Y_SPEED = Calculate_Speed(1136.0 + (abs(X_SPEED) * 64.0));
								RAM[0x1DFC] = 0x04;
								jump_is_spin = true;
							}
							else {
								//Normal jump
								Y_SPEED = Calculate_Speed(1232.0 + (abs(X_SPEED) * 64.0));
								RAM[0x1DFA] = 0x01;
							}
						}
					}

					double SLOPE_ADD = 0;
					if (!SLIDING) {
						if (SLOPE_TYPE == 1) {
							if (!MOV || (WALKING_DIR == -1 && MOV)) { SLOPE_ADD = Calculate_Speed(-256); }
							if (MOV) { SLOPE_ADD = Calculate_Speed(-80); }
						}
						if (SLOPE_TYPE == 2) {
							if (!MOV || (WALKING_DIR == 1 && MOV)) { SLOPE_ADD = Calculate_Speed(256); }
							if (MOV) { SLOPE_ADD = Calculate_Speed(80); }
						}
					}

					//Acceleration values
					double SPEED_X_TO_SET = SLOPE_ADD + (Calculate_Speed(320.0 + (RUN * 256.0) + (CAN_SPRINT * 192.0)) * WALKING_DIR) * MOV;
					double SPEED_ACCEL_X = Calculate_Speed(24.0);
					double STOPPING_DECEL = Calculate_Speed(16.0);
					double SKID_ACCEL = Calculate_Speed(16.0 + (24.0 * RUN) + (CAN_SPRINT * 40.0));

					//Sliding down a slope :D
					if (SLIDING) {
						SPEED_X_TO_SET = 0;
						SPEED_ACCEL_X = 0;
						STOPPING_DECEL = Calculate_Speed(0x10);
						SKID_ACCEL = 0;
						if (SLOPE_TYPE == 1) {
							SPEED_X_TO_SET = Calculate_Speed(-0x300);
							SPEED_ACCEL_X = Calculate_Speed(0x18);
						}
						if (SLOPE_TYPE == 2) {
							SPEED_X_TO_SET = Calculate_Speed(0x300);
							SPEED_ACCEL_X = Calculate_Speed(0x18);
						}

						if (SLOPE_TYPE == 3 || SLOPE_TYPE == 4) {
							SPEED_X_TO_SET = Calculate_Speed(-0x2C0);
							SPEED_ACCEL_X = Calculate_Speed(0x10);
						}
						if (SLOPE_TYPE == 5 || SLOPE_TYPE == 6) {
							SPEED_X_TO_SET = Calculate_Speed(0x2C0);
							SPEED_ACCEL_X = Calculate_Speed(0x10);
						}
					}

					//Accel start
					if (MOV) {
						if (X_SPEED > 0.0 && SPEED_X_TO_SET < 0.0 && WALKING_DIR == -1) {
							SKIDDING = -1;
							X_SPEED -= SKID_ACCEL;
						}
						if (X_SPEED < 0.0 && SPEED_X_TO_SET > 0.0 && WALKING_DIR == 1) {
							SKIDDING = 1;
							X_SPEED += SKID_ACCEL;
						}
						if (!ON_FL) {
							SKIDDING = 0;
						}
						if (X_SPEED > SPEED_X_TO_SET) { X_SPEED = max(SPEED_X_TO_SET, X_SPEED - SPEED_ACCEL_X); }
						if (X_SPEED < SPEED_X_TO_SET) { X_SPEED = min(SPEED_X_TO_SET, X_SPEED + SPEED_ACCEL_X); }
					}
					else {
						if (ON_FL) {
							if (!SLOPE_TYPE) {
								if (X_SPEED > 0.0) { X_SPEED = max(0.0, X_SPEED - STOPPING_DECEL); }
								if (X_SPEED < 0.0) { X_SPEED = min(0.0, X_SPEED + STOPPING_DECEL); }
							}
							else {
								if (X_SPEED > SPEED_X_TO_SET) { X_SPEED = max(SPEED_X_TO_SET, X_SPEED - SPEED_ACCEL_X); }
								if (X_SPEED < SPEED_X_TO_SET) { X_SPEED = min(SPEED_X_TO_SET, X_SPEED + SPEED_ACCEL_X); }
							}
						}
					}
					//Accel end

					//Slide particles
					if ((SKIDDING || SLIDING) && !(global_frame_counter & 3) && ON_FL) {
						createParticle(0x3C, 0x00, 0x8, 3, x + 5.0, y - 27.0, 0, 0, 0, 0);
					}

					//Gravity & max falling Y Speed
					if (!ON_FL || SLOPE_TYPE) {
						Y_SPEED += Calculate_Speed(GRAV);
					}
					else {
						LAST_X_SPEED_ON_FL = X_SPEED;
					}
					double max_fall = (p_pad[button_a] || p_pad[button_b]) ? Calculate_Speed(-1072.0) : Calculate_Speed(-1120.0);
					Y_SPEED = max(max_fall, Y_SPEED);

					//Climb Test
					if ((p_pad[button_up] || p_pad[button_down]) && GRABBED_SPRITE == 0xFF) {
						uint_fast16_t check_x_1 = uint_fast16_t((x + 2) / 16.0);
						uint_fast16_t check_x_2 = uint_fast16_t((x + 14) / 16.0);
						uint_fast16_t check_y_1 = uint_fast16_t((y + height - 1) / 16.0);
						uint_fast16_t check_y_2 = uint_fast16_t((y + 1) / 16.0);
						if (
							map16_handler.check_climbable(check_x_1, check_y_1) &&
							map16_handler.check_climbable(check_x_2, check_y_1) &&
							map16_handler.check_climbable(check_x_1, check_y_2) &&
							map16_handler.check_climbable(check_x_2, check_y_2)
						) {
							climbing = true;
						}
					}
				}
				else {
					//Climb Physics
					ON_FL = false;
					jump_is_spin = false;
					SLIDING = false;
					CROUCH = false;
					P_METER = 0;
					X_SPEED = p_pad[button_right] - p_pad[button_left];
					Y_SPEED = p_pad[button_up] - p_pad[button_down];
					if (X_SPEED != 0) {
						uint_fast16_t check_x_1 = uint_fast16_t((x + (X_SPEED > 0 ? 14 : 2)) / 16.0);
						uint_fast16_t check_y_1 = uint_fast16_t((y + height - 1) / 16.0);
						uint_fast16_t check_y_2 = uint_fast16_t((y + 1) / 16.0);
						if (!map16_handler.check_climbable(check_x_1, check_y_1) && !map16_handler.check_climbable(check_x_1, check_y_2)) {
							X_SPEED = 0;
						}
					}
					if (Y_SPEED != 0) {
						uint_fast16_t check_x_1 = uint_fast16_t((x + 2) / 16.0);
						uint_fast16_t check_x_2 = uint_fast16_t((x + 14) / 16.0);
						uint_fast16_t check_y_1 = uint_fast16_t((y + (Y_SPEED > 0 ? (height - 1) : 8)) / 16.0);
						if (!map16_handler.check_climbable(check_x_1, check_y_1) && !map16_handler.check_climbable(check_x_2, check_y_1)) {
							if (Y_SPEED < 0)
							{
								climbing = false;
							}
							Y_SPEED = 0;
						}
					}
					if (WALKING_DIR == 0) {
						WALKING_DIR = 1;
					}
					if (X_SPEED != 0 || Y_SPEED != 0) {
						FRM += 1;
						if (FRM > 8) {
							FRM = 0;
							WALKING_DIR *= -1;
						}
					}
					//Climb Test
					uint_fast16_t check_x_1 = uint_fast16_t((x + 8) / 16.0);
					uint_fast16_t check_y_1 = uint_fast16_t((y + height / 2) / 16.0);
					if (!map16_handler.check_climbable(check_x_1, check_y_1)) {
						climbing = false;
					}

					if (p_pad[button_b] != wasJumping) {
						wasJumping = p_pad[button_b];
						if (wasJumping) {
							climbing = false;
							Y_SPEED = Calculate_Speed(1232.0);
							RAM[0x1DFA] = 0x01;
						}
					}
				}
				//End Land Physics. Movement
				
				//Force YSPD and XSPD according to 7B/7D values
				if (RAM[0x7B] != 0) { X_SPEED = double(int_fast8_t(RAM[0x7B])) / 16.0; }
				if (RAM[0x7D] != 0) { Y_SPEED = double(int_fast8_t(RAM[0x7D])) / 16.0; }

				if (!Move(X_SPEED, 0.0)) { X_SPEED = 0.0; }
				if (!Move(0.0, Y_SPEED)) {
					if (SLOPE_TYPE) {
						Y_SPEED = max(Calculate_Speed(-768), min(0, Y_SPEED));
					}
					else {
						Y_SPEED = 0;
					}
				}
			}
			else {
				//SWIMCODE
				climbing = false;

				if (!(global_frame_counter & 0x7F) && WaterLevel == 0) {
					createParticle(0x21, 0x00, 0x88, 7, x + 4 + to_scale * -6, y - 16.0, 0, 0, 0);
				}
				SLIDING = false;
				jump_is_spin = false;

				CAN_SPRINT = false;
				CROUCH = false;

				MOV = p_pad[button_left] || p_pad[button_right];
				if (p_pad[button_left]) { WALKING_DIR = -1; }
				if (p_pad[button_right]) { WALKING_DIR = 1; }

				if (p_pad[button_down]) {
					if (ON_FL && GRABBED_SPRITE == 0xFF) {
						WALKING_DIR = 0;
						MOV = true;
						CROUCH = true;
					}
				}

				bool isJumping = p_pad[button_b] || p_pad[button_a];
				if (isJumping != wasJumping) {
					wasJumping = isJumping;
					if (isJumping && GRABBED_SPRITE == 0xFF) {
						RAM[0x1DF9] = 0x0E;
						Y_SPEED += Calculate_Speed(384);
						jump_is_spin = false;
					}
				}

				double SPEED_X_TO_SET = Calculate_Speed(256.0 / (1.0 + double(ON_FL))) * WALKING_DIR;
				double STOP_SPEED = 0;
				if (WaterLevel > 0 && y <= (WaterLevel - height)) {
					SPEED_X_TO_SET -= Calculate_Speed(192.0);
					STOP_SPEED -= Calculate_Speed(256.0);
				}
				double SPEED_ACCEL_X = Calculate_Speed(24.0);
				double STOPPING_DECEL = Calculate_Speed(24.0);
				double SKID_ACCEL = Calculate_Speed(40.0);
				if (GRABBED_SPRITE == 0xFF) {
					Y_SPEED = Y_SPEED + Calculate_Speed(-8);
					Y_SPEED = max(Calculate_Speed(-1024), min(Y_SPEED, Calculate_Speed(384.0 - (p_pad[button_down] * 256.0) + (p_pad[button_up] * 384.0))));
				}
				else {
					Y_SPEED = min(1.0, Y_SPEED + Calculate_Speed(8));
					MOV = true;
					double sp = 256 + (p_pad[button_right] || p_pad[button_left]) * 256;
					SPEED_ACCEL_X = Calculate_Speed(16.0);
					SPEED_X_TO_SET = Calculate_Speed(sp / (1.0 + double(ON_FL)))* WALKING_DIR;

					if (p_pad[button_down]) {
						Y_SPEED = -1;
						if (!(global_frame_counter % 8)) {
							RAM[0x1DF9] = 0x0E;
						}
					}
				}
				//Accel start
				if (MOV) {
					if (X_SPEED > SPEED_X_TO_SET) { X_SPEED = max(SPEED_X_TO_SET, X_SPEED - SPEED_ACCEL_X); }
					if (X_SPEED < SPEED_X_TO_SET) { X_SPEED = min(SPEED_X_TO_SET, X_SPEED + SPEED_ACCEL_X); }
					if (X_SPEED > 0.0 && SPEED_X_TO_SET < 0.0 && WALKING_DIR == -1) { X_SPEED -= SKID_ACCEL; }
					if (X_SPEED < 0.0 && SPEED_X_TO_SET > 0.0 && WALKING_DIR == 1) { X_SPEED += SKID_ACCEL; }
				}
				else {
					if (X_SPEED > STOP_SPEED) { X_SPEED = max(STOP_SPEED, X_SPEED - STOPPING_DECEL); }
					if (X_SPEED < STOP_SPEED) { X_SPEED = min(STOP_SPEED, X_SPEED + STOPPING_DECEL); }
				}
				//Accel end
				//Force YSPD and XSPD according to 7B/7D values
				if (RAM[0x7D] != 0) { Y_SPEED = double(int_fast8_t(RAM[0x7D])) / 16.0; }
				if (!Move(X_SPEED + double(int_fast8_t(RAM[0x7B])) / 16.0, 0.0)) { X_SPEED = 0.0; }
				if (!Move(0.0, Y_SPEED)) { Y_SPEED = 0.0; }
				if (WaterLevel > 0) {
					if (y > (WaterLevel - height) && !ON_FL && Y_SPEED > 0) {
						if (p_pad[button_up] && (p_pad[button_b] || p_pad[button_a])) {
							JumpOutOfWater();
						}
						else {
							y = WaterLevel - height;
							Y_SPEED = 0.0;
						}
					}
				}
			}
			//Swimming code end, close of brackets
		}

		//Process Normal stuff, rest is history.
		ProcessCape();
		ProcessFireballs();
		ProcessReserve();
		ProcessGrabbed();
		Get_Sprite();

		pressed_y = false;
		if (x < 8.0) { x = 8.0; X_SPEED = 0; }
		if (x > double(-24 + mapWidth * 16)) { x = double(-24 + mapWidth * 16); }
		return 1;
	}

	//Camera Script Variables Local
	double camera_focus_y = 0;
	double camera_snap_dist = -16;
	double camera_distance_from_player = 0;
	uint_fast8_t camera_state = 0;
	int_fast8_t WALKING_DIR_OLD = -1;
	int_fast8_t WALKING_DIR_CAMERA = 0;
	void ResetCamera() {
		camera_focus_y = 0;
		camera_snap_dist = 0;
		camera_distance_from_player = 0;
		camera_state = 0;
	}

	void SMWCameraX() {
		if(X_SPEED != 0) {
			WALKING_DIR_CAMERA = X_SPEED > 0 ? 1 : -1;
		}
		if (WALKING_DIR_CAMERA != WALKING_DIR_OLD) {
			WALKING_DIR_OLD = WALKING_DIR_CAMERA;
			camera_state = 0; //Set Camera State to Wait For Move
		}
		if (abs(camera_distance_from_player) > 128) {
			camera_state = 1;
		}

		if (camera_state == 0) {
			double DISTANCE_FROM_CAMERA = x - CAMERA_X;
			if (abs(DISTANCE_FROM_CAMERA) > 32) {
				camera_state = 1;
				camera_snap_dist = DISTANCE_FROM_CAMERA > 0 ? -12 : 12;
				camera_distance_from_player = x - (CAMERA_X + camera_snap_dist);
			}
		}

		if (camera_state == 1) {
			int camera_focus_speed = abs(camera_distance_from_player) > 128 ? 16 : 2;
			if (camera_distance_from_player > 0) {
				camera_distance_from_player = max(0.0, camera_distance_from_player - camera_focus_speed);
			}
			if (camera_distance_from_player < 0) {
				camera_distance_from_player = min(0.0, camera_distance_from_player + camera_focus_speed);
			}
			CAMERA_X = x - camera_snap_dist - camera_distance_from_player;
		}
	}

	void SMWCameraY() {
		double new_focus = y + 16;
		if (((ON_FL && Y_SPEED <= 0) || (new_focus > (camera_focus_y + 96) || IN_WT)) || ((P_METER >= P_METER_REQUIRED && Y_SPEED > 0) || climbing)) {
			camera_focus_y = new_focus;
		}
		if (y < (CAMERA_Y - 44)) { CAMERA_Y = max(y + 44, CAMERA_Y - 4); camera_focus_y = CAMERA_Y; }
		if (CAMERA_Y < camera_focus_y) { CAMERA_Y = min(camera_focus_y, CAMERA_Y + 3); }
	}

	//Def Camera
	void ProcessCamera() {
		if (RAM[0x3F11] || !RAM[0x9D]) {
			return;
		}

		bool sm_mode = smooth_camera || local_multiplayer;
		if (!RAM[0x1493] && !DEAD) {
			if (!RAM[0x1411]) {
				int min_x = (RAM[0x1462] + RAM[0x1463] * 256) + 8;
				if (!(RAM[0x3F1F] & 4)) {
					x = min(double(min_x + 224), max(double(min_x), x));
				}
				CAMERA_X = double(120 + RAM[0x1462] + RAM[0x1463] * 256);
				if (!sm_mode) {
					camera_snap_dist = 0;
					camera_distance_from_player = x - CAMERA_X;
				}
			}
			else {
				if (sm_mode) {
					CAMERA_X += (x - CAMERA_X) / smooth_camera_speed;
				}
				else {
					SMWCameraX();
				}
			}
			if (!RAM[0x1412]) {
				CAMERA_Y = double(104 + RAM[0x1464] + (RAM[0x1465] << 8));
			}
			else {
				if (sm_mode) {
					CAMERA_Y += ((y + 16) - CAMERA_Y) / smooth_camera_speed;
				}
				else {
					SMWCameraY();
				}
			}
		}
	}

#ifndef DISABLE_NETWORK
	//NETWORKING: Pack variables
	//You may add new entries here, but have byte sizes in minds and change player_expected_packet_size in globalgamevariables.h
	void NetPackVariables(bool include_special)
	{
		CurrentPacket << float(x); CurrentPacket << float(y);
		CurrentPacket << int_fast8_t(X_SPEED * 16.0); CurrentPacket << int_fast8_t(Y_SPEED * 16.0);

		uint_fast8_t bools_1 = ON_FL;
		bools_1 += DEAD << 1;
		bools_1 += jump_is_spin << 2;
		bools_1 += in_pipe << 3;
		bools_1 += SLIDING << 4;
		bools_1 += (uint_fast8_t(FRM) & 3) << 5;
		bools_1 += (climbing) << 7;
		CurrentPacket << bools_1;

		if (include_special) {
			CurrentPacket << GRABBED_SPRITE; CurrentPacket << fire_anim_timer; CurrentPacket << cape_spin_timer;
			CurrentPacket << INVINCIBILITY_FRAMES_STAR;
			CurrentPacket << KO_counter;
		}
		CurrentPacket << INVINCIBILITY_FRAMES; CurrentPacket << DEATH_TIMER; CurrentPacket << powerup_anim;

		CurrentPacket << to_scale; CurrentPacket << SKIDDING; CurrentPacket << P_METER;
		CurrentPacket << WALKING_DIR;
		CurrentPacket << cape_ascent_timer;

		CurrentPacket << STATE;
		CurrentPacket << int_fast16_t(CAMERA_X); CurrentPacket << int_fast16_t(CAMERA_Y);

		CurrentPacket << SLOPE_TYPE;
		CurrentPacket << WO_counter;

		CurrentPacket << skin;
		CurrentPacket << pipe_speed_x; CurrentPacket << pipe_speed_y;

		//Mouse
		uint_fast16_t m_state_1 = (mouse_x & 0x3FFF) + (mouse_state[0] << 15) + (mouse_state[2] << 14);
		uint_fast16_t m_state_2 = (mouse_y & 0x3FFF) + (mouse_state[1] << 15) + (mouse_state[3] << 14);
		CurrentPacket << m_state_1; CurrentPacket << m_state_2;

		//Input
		uint_fast8_t input_d = 0;
		for (int inputs = 0; inputs < total_inputs; inputs++) {
			input_d |= (p_pad[inputs] << inputs);
		}
		CurrentPacket << input_d;
		for (int plr_name = 0; plr_name < player_name_size; plr_name++) {
			CurrentPacket << uint_fast8_t(player_name_cut[plr_name]);
		}
	}

	//NETWORKING: Unpack variables
	void NetUnpackVariables(bool include_special)
	{
		float xN, yN; CurrentPacket >> xN; CurrentPacket >> yN;
		int_fast8_t X_SPEED_N, Y_SPEED_N; CurrentPacket >> X_SPEED_N; CurrentPacket >> Y_SPEED_N;
		X_SPEED = double(X_SPEED_N) / 16.0; Y_SPEED = double(Y_SPEED_N) / 16.0; x = xN; y = yN;

		uint_fast8_t bools; CurrentPacket >> bools;
		ON_FL = bools & 1;
		DEAD = bools & 2;
		jump_is_spin = bools & 4;
		in_pipe = bools & 8;
		SLIDING = bools & 16;
		FRM = (bools >> 5) & 3;
		climbing = bools >> 7;
		if (include_special) {
			CurrentPacket >> GRABBED_SPRITE; CurrentPacket >> fire_anim_timer; CurrentPacket >> cape_spin_timer;
			CurrentPacket >> INVINCIBILITY_FRAMES_STAR;
			CurrentPacket >> KO_counter;
		}
		CurrentPacket >> INVINCIBILITY_FRAMES; CurrentPacket >> DEATH_TIMER; CurrentPacket >> powerup_anim;

		CurrentPacket >> to_scale; CurrentPacket >> SKIDDING; CurrentPacket >> P_METER;
		CurrentPacket >> WALKING_DIR;
		CurrentPacket >> cape_ascent_timer;

		CurrentPacket >> STATE;

		int_fast16_t CAMERA_X_N, CAMERA_Y_N; CurrentPacket >> CAMERA_X_N; CurrentPacket >> CAMERA_Y_N;
		CAMERA_X = CAMERA_X_N; CAMERA_Y = CAMERA_Y_N;

		CurrentPacket >> SLOPE_TYPE;
		CurrentPacket >> WO_counter;

		CurrentPacket >> skin;
		CurrentPacket >> pipe_speed_x; CurrentPacket >> pipe_speed_y;

		//Mouse
		uint_fast16_t m_state_1, m_state_2;
		CurrentPacket >> m_state_1; CurrentPacket >> m_state_2;
		mouse_state[0] = (m_state_1 >> 15) & 1;
		mouse_state[1] = (m_state_2 >> 15) & 1;
		mouse_state[2] = (m_state_1 >> 14) & 1;
		mouse_state[3] = (m_state_2 >> 14) & 1;
		mouse_x = m_state_1 & 0x3FFF;
		mouse_y = m_state_2 & 0x3FFF;

		//Input
		uint_fast8_t input_d = 0;
		CurrentPacket >> input_d;
		for (int inputs = 0; inputs < total_inputs; inputs++) {
			p_pad[inputs] = (input_d >> inputs) & 1;
		}

		for (int plr_name = 0; plr_name < player_name_size; plr_name++) {
			uint_fast8_t new_l = 0;
			CurrentPacket >> new_l;
			player_name_cut[plr_name] = char(new_l);
		}
	}

	//NETWORKING: Send Specifics
	void NetPackSpecificVariables() {
		CurrentPacket << GRABBED_SPRITE;
		CurrentPacket << fire_anim_timer;
		CurrentPacket << cape_spin_timer;
		CurrentPacket << reserve_item;
		CurrentPacket << INVINCIBILITY_FRAMES_STAR;
		CurrentPacket << KO_counter;
		bool hasToSyncPos = server_position_sync_s != server_position_sync_c;
		bool received_mouse_status = mouse_state[2] || mouse_state[3];
		uint_fast8_t Flags = (hasToSyncPos + (received_mouse_status << 1)) + (pvp << 2); CurrentPacket << Flags;
		if (hasToSyncPos) {
			CurrentPacket << server_position_sync_x; CurrentPacket << server_position_sync_y; CurrentPacket << server_position_sync_s;
		}
		if (gamemode == GAMEMODE_OVERWORLD) {
			CurrentPacket << ow_level_name;
		}
	}

	//NETWORKING: Receive Specifics
	void NetUnpackSpecificVariables() {
		CurrentPacket >> GRABBED_SPRITE;
		CurrentPacket >> fire_anim_timer;
		CurrentPacket >> cape_spin_timer;
		CurrentPacket >> reserve_item;
		CurrentPacket >> INVINCIBILITY_FRAMES_STAR;
		CurrentPacket >> KO_counter;
		uint_fast8_t Flags; CurrentPacket >> Flags;
		bool hasSyncPosition = (Flags & 1);
		bool received_mouse_status = (Flags & 2);
		pvp = (Flags & 4);
		if (hasSyncPosition) {
			CurrentPacket >> x; CurrentPacket >> y; CurrentPacket >> server_position_sync_c;
		}
		if (received_mouse_status) {
			mouse_w_up = false;
			mouse_w_down = false;
		}
		if (gamemode == GAMEMODE_OVERWORLD) {
			CurrentPacket >> ow_level_name;
		}
	}
#endif
};

//Player management
vector<MPlayer> Players;
void AddNewPlayer() { Players.push_back(MPlayer(LevelManager.start_x, LevelManager.start_y)); }
void RemovePlayer() { Players.pop_back(); }
void CheckForPlayers() //Have to be careful when fucking with this. Or else memory might leak.
{
	while (Players.size() > PlayerAmount) { RemovePlayer(); }
	while (Players.size() < PlayerAmount) { AddNewPlayer(); }
}

// Get Player
MPlayer& GetPlayerByNumber(uint_fast8_t number) {
	CheckForPlayers();
	return number < Players.size() ? Players[number] : Players[0];
}

//player interaction with other players, for now it's just player combat
void PlayerInteraction()
{
	if (!pvp) {
		return;
	}
	for (int i = 0; i < Players.size(); i++) {
		MPlayer& CurrPlayer = Players[i];
		if (CurrPlayer.DEAD) {
			continue;
		}
		else {
			for (int e = 0; e < Players.size(); e++) {
				MPlayer& PlrInteract = Players[e];
				if ((&PlrInteract == &CurrPlayer || PlrInteract.DEAD) || PlrInteract.INVINCIBILITY_FRAMES > 0) {
					continue;
				}
				else {
					double LeftBound = PlrInteract.x - 16.0;
					double RightBound = PlrInteract.x + 16.0;
					double UpBound = PlrInteract.y + (PlrInteract.STATE == 0 ? 16.0 : 32.0);
					double DownBound = PlrInteract.y - (CurrPlayer.STATE == 0 ? 16.0 : 32.0);
					if (CurrPlayer.x > LeftBound && CurrPlayer.x < RightBound && CurrPlayer.y < UpBound && CurrPlayer.y > DownBound && (CurrPlayer.Y_SPEED < -2.0 || CurrPlayer.INVINCIBILITY_FRAMES_STAR > 0)) {
						if (!CurrPlayer.INVINCIBILITY_FRAMES_STAR) {
							if (CurrPlayer.jump_is_spin) {
								CurrPlayer.Enemy_Jump_Spin();
							}
							else {
								CurrPlayer.Enemy_Jump();
							}
						}
						if (!isClient) {
							if (PlrInteract.STATE == 0) {
								player_netcommand[e] = 0xF;
								if (CurrPlayer.KO_counter != 255) {
									CurrPlayer.KO_counter++;
								}
							}
							else {
								player_netcommand[e] = 0x1F;
							}
						}
					}
				}
			}
		}
	}
}