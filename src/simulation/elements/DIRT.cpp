#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_DIRT()
{
	Identifier = "DEFAULT_PT_DIRT";
	Name = "Dirt";
	Colour = 0x6B4226_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.94f;
	Loss = 0.95f;
	Collision = -0.1f;
	Gravity = 0.3f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 1;

	Flammable = 2;
	Explosive = 0;
	Meltable = 0;
	Hardness = 3;

	Weight = 80;

	HeatConduct = 70;
	Description = "Dirt. Mix with water for mud. Enriched by yeast into humus. Plants can grow in it.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = 900.0f;
	HighTemperatureTransition = PT_STNE; //@ DIRT -> STNE (baked hard)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 30))
		{
			//@ DIRT + WATR/DSTW -> 2xMUD
			sim->part_change_type(i, x, y, PT_MUD);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_MUD);
			return 1;
		}
		if (TYP(r) == PT_YEST && sim->rng.chance(1, 150))
		{
			//@ DIRT + YEST -> HUMS (yeast decomposes organic matter into humus)
			sim->part_change_type(i, x, y, PT_HUMS);
			return 1;
		}
		if (TYP(r) == PT_PLNT && sim->rng.chance(1, 600))
		{
			//@ DIRT -> PLNT (plant roots grow into soil)
			sim->part_change_type(i, x, y, PT_PLNT);
			return 1;
		}
	}
	return 0;
}
