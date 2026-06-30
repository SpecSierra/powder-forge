#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_GRVL()
{
	Identifier = "DEFAULT_PT_GRVL";
	Name = "Gravel";
	Colour = 0x9A9090_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.3f;
	AirDrag = 0.04f * CFDS;
	AirLoss = 0.90f;
	Loss = 0.90f;
	Collision = -0.1f;
	Gravity = 0.4f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable = 1;
	Hardness = 0;

	Weight = 95; // heavier than SAND (90) — sinks below it

	HeatConduct = 60;
	Description = "Gravel. Coarse heavy rock fragments. Acid erosion produces sand. High pressure compresses into stone.";

	Properties = TYPE_PART;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = 10.0f;
	HighPressureTransition = PT_STNE; //@ GRVL -> STNE (compressed into rock)
	LowTemperature = ITL;
	LowTemperatureTransition = NT;
	HighTemperature = ITH;
	HighTemperatureTransition = NT;

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

		if (TYP(r) == PT_ACID && sim->rng.chance(1, 25))
		{
			//@ GRVL + ACID -> SAND (acid erodes gravel)
			sim->part_change_type(i, x, y, PT_SAND);
			return 1;
		}
	}
	return 0;
}
