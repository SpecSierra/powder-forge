#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);
static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_MUSH()
{
	Identifier = "DEFAULT_PT_MUSH";
	Name = "Mushroom";
	Colour = 0xC8A878_rgb;
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

	Flammable = 15;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 100;

	HeatConduct = 40;
	Description = "Mushroom. Slowly spreads onto wood and plants. Killed by salt. Burns with colorful spores.";

	Properties = TYPE_SOLID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 500.0f;
	HighTemperatureTransition = PT_FIRE; //@ MUSH -> FIRE (burns)

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

		if ((TYP(r) == PT_SALT || TYP(r) == PT_SLTW) && sim->rng.chance(1, 30))
		{
			//@ MUSH + SALT/SLTW -> dies (osmotic death)
			sim->kill_part(i);
			return 1;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 10))
		{
			//@ MUSH + ACID -> dies
			sim->kill_part(i);
			return 1;
		}
	}

	// Growth: spread to adjacent organic material
	int grow_chance = near_water ? 300 : 600;
	if (sim->rng.chance(1, grow_chance))
	{
		int gx = x + sim->rng.between(-1, 1);
		int gy = y + sim->rng.between(-1, 1);
		if (gx != x || gy != y)
		{
			auto r = pmap[gy][gx];
			if (r && (TYP(r) == PT_WOOD || TYP(r) == PT_SAWD || TYP(r) == PT_PLNT || TYP(r) == PT_VINE))
			{
				//@ MUSH spreads to WOOD/SAWD/PLNT/VINE
				sim->part_change_type(ID(r), gx, gy, PT_MUSH);
			}
		}
	}
	return 0;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	// Warm spore glow when hot
	if (cpart->temp > 350.0f)
	{
		*firea = 50;
		*firer = 0xFF;
		*fireg = 0xA0;
		*fireb = 0x20;
		*pixel_mode |= FIRE_ADD;
	}
	return 0;
}
