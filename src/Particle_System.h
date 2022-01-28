#pragma once
//Particle System Implementation. Only uses OAM so this doesnt need to sync
class Particle
{
public:
	uint_fast8_t spr_tile = 0;
	uint_fast8_t spr_size = 0x11;
	uint_fast16_t pal_props = 0;
	uint_fast8_t special_animation_type = 0;
	double spr_x = 0;
	double spr_y = 0;
	double spr_sx = 0;
	double spr_sy = 0;
	double spr_grav = 0;
	int t = 0;
	bool to_del = false;
	int time_limit = 0;
	double max_speed_y = 0;

	void draw()
	{
		switch (special_animation_type)
		{
			/*
				Turn Block Tile animation
			*/
		case 1:
			switch ((t >> 2) % 6)
			{
			case 0:
				spr_tile = 0x3D;
				pal_props = 0x108;
				break;
			case 1:
				spr_tile = 0x3C;
				pal_props = 0x108;
				break;
			case 2:
				spr_tile = 0x3C;
				pal_props = 0x118;
				break;
			case 3:
				spr_tile = 0x3C;
				pal_props = 0x138;
				break;
			case 4:
				spr_tile = 0x3C;
				pal_props = 0x128;
				break;
			case 5:
				spr_tile = 0x3D;
				pal_props = 0x128;
				break;
			}
			break;
			/*
				Coin spark
			*/
		case 2:
			pal_props = 0x108;
			if (t < 0) {
				spr_tile = 0;
			}
			else {
				if ((t >> 3) == 0) {
					spr_tile = 0x7D;
				}
				if ((t >> 3) == 1) {
					spr_tile = 0x7C;
				}
				if ((t >> 3) == 2) {
					spr_tile = 0x76;
				}
				if (t > 15) {
					to_del = true;
				}
			}
			break;
			/*
				Player skid
			*/
		case 3:
			pal_props = 0x108;
			spr_tile = 0x62 + ((t >> 2) << 1);
			if (t > 10) {
				to_del = true;
			}

			break;
			/*
				Smoke
			*/
		case 4:
			pal_props = 0x108;
			spr_tile = 0x60 + ((t >> 3) << 1);
			spr_size = 0x11;

			if (t > 30)
			{
				to_del = true;
			}

			break;
			/*
				Hit spark
			*/
		case 5:
			spr_tile = 0x44;
			spr_size = 0x11;
			pal_props = 0x108 | (((t / 2) % 2) * 0x10);

			if (t > 8) {
				to_del = true;
			}

			break;
			/*
				Throwblock Tile animation
			*/
		case 6:
			switch ((t >> 2) % 6)
			{
			case 0:
				spr_tile = 0x3D;
				pal_props = 0x100;
				break;
			case 1:
				spr_tile = 0x3C;
				pal_props = 0x100;
				break;
			case 2:
				spr_tile = 0x3C;
				pal_props = 0x110;
				break;
			case 3:
				spr_tile = 0x3C;
				pal_props = 0x130;
				break;
			case 4:
				spr_tile = 0x3C;
				pal_props = 0x120;
				break;
			case 5:
				spr_tile = 0x3D;
				pal_props = 0x120;
				break;
			}
			pal_props += 8 + (t & 7);
			break;
			/*
				Player Bubble
			*/
		case 7:
			spr_x += ((t >> 2) & 3) >= 2 ? -0.25 : 0.25;
			spr_y += (((t >> 4) & 3) >= 3) ? 0 : 1;
			spr_tile = 0x21;
			if (t > 128) {
				to_del = true;
			}
			break;
			/*
				ZZZZZ Bubble
			*/
		case 8:
			spr_x += ((t >> 3) & 3) >= 2 ? -0.25 : 0.25;
			spr_x += 0.125;
			spr_y += 0.25;
			spr_tile = 0xE0 + ((t >> 4) & 1) + (((t >> 4) >> 1) << 4);
			if (t > 64)
			{
				to_del = true;
			}
			break;
			/*
				Stars
			*/
		case 9:
			pal_props = 0x10B;
			spr_tile = 0x4C + ((t >> 2) << 4);

			if (t > 10)
			{
				to_del = true;
			}

			break;
		/*
			Smoke
		*/
		case 10:
			pal_props = 0x108;
			spr_tile = 0x62 + ((t >> 3) << 1);
			spr_size = 0x11;
			spr_sx = spr_sx * -1.1;

			if (t > 22)
			{
				to_del = true;
			}

			break;
		}

		if (time_limit)
		{
			if (t >= time_limit)
			{
				to_del = true;
			}
		}
		spr_sy -= spr_grav;
		if (max_speed_y != 0) {
			if (spr_sy < max_speed_y) {
				spr_sy = max_speed_y;
			}
		}
		spr_x += spr_sx;
		spr_y += spr_sy;
		t++;

		if (!spr_tile) {
			return;
		}
		int_fast16_t s_x = int_fast16_t(spr_x);
		int_fast16_t s_y = int_fast16_t(spr_y);

		Create_OAMTile(spr_tile, spr_size, s_x, s_y, pal_props, 0);
	}
};
vector<Particle> particles;

void createParticle(uint_fast8_t t, uint_fast8_t size, uint_fast16_t prop, uint_fast8_t anim_type, double x, double y, double sx, double sy, double grav, int tt = 0, int t_del = 0, double max_y_speed = 0)
{
	if (isClient) { return; } //Only server and singleplayer can create particles.
	particles.push_back(Particle{ t, size, prop, anim_type, x, y, sx, sy, grav, tt, false, t_del, max_y_speed});
}

void processParticles()
{
	if (isClient) { return; }
	for (int i = 0; i < particles.size(); i++)
	{
		Particle& b = particles[i];
		b.draw();
		if (b.spr_y < -32.0 || b.to_del)
		{
			particles.erase(particles.begin() + i);
			i--;
		}
	}
}