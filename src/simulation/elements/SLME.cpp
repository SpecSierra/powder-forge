#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_SLME()
{
	Identifier = "DEFAULT_PT_SLME";
	Name = "Slime";
	Colour = 0x30B830_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag = 0.00f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.00f;
	Collision = 0.0f;
	Gravity = 0.0f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 5;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 50;
	Description = "Slime. Grows on plants and wood near water. Killed by salt or acid. Mildly corrosive.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 450.0f;
	HighTemperatureTransition = PT_DSTW; //@ SLME -> DSTW (melts to dirty water)

	Update = &update;
	Graphics = &graphics;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool near_water = false;

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_WATR || TYP(r) == PT_DSTW)
			near_water = true;

		if ((TYP(r) == PT_SALT || TYP(r) == PT_SLTW) && sim->rng.chance(1, 20))
		{
			//@ SLME + SALT -> kills slime (osmosis)
			sim->kill_part(i);
			return 1;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 5))
		{
			//@ SLME + ACID -> dies
			sim->kill_part(i);
			return 1;
		}
		// Weak acid: slowly corrode nearby stone
		if ((TYP(r) == PT_STNE || TYP(r) == PT_SAND) && sim->rng.chance(1, 3000))
		{
			//@ SLME slowly corrodes STNE/SAND (mild acid)
			sim->kill_part(ID(r));
		}
	}

	// Growth near water: spread to adjacent organic material
	if (near_water && sim->rng.chance(1, 400))
	{
		int gx = x + sim->rng.between(-1, 1);
		int gy = y + sim->rng.between(-1, 1);
		if (gx != x || gy != y)
		{
			auto r = pmap[gy][gx];
			if (r && (TYP(r) == PT_WOOD || TYP(r) == PT_PLNT || TYP(r) == PT_VINE || TYP(r) == PT_SAWD))
			{
				//@ SLME (near WATR) spreads to WOOD/PLNT/VINE/SAWD
				sim->part_change_type(ID(r), gx, gy, PT_SLME);
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Wet sheen
	*colg = 0xB8 + int((cpart->temp - 295.15f) * 0.05f);
	if (*colg > 255) *colg = 255;
	if (*colg < 0x80) *colg = 0x80;
	*firea = 30;
	*firer = 0x10;
	*fireg = 0xA0;
	*fireb = 0x10;
	*pixel_mode |= FIRE_ADD;
	return 0;
}
