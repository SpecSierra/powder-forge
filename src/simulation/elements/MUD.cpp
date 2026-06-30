#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_MUD()
{
	Identifier = "DEFAULT_PT_MUD";
	Name = "Mud";
	Colour = 0x4A3018_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.2f;
	AirDrag = 0.02f * CFDS;
	AirLoss = 0.95f;
	Loss = 0.60f;
	Collision = 0.0f;
	Gravity = 0.08f;
	Diffusion = 0.00f;
	HotAir = 0.000f * CFDS;
	Falldown = 2;

	Flammable = 0;
	Explosive = 0;
	Meltable = 0;
	Hardness = 5;

	Weight = 42;

	HeatConduct = 55;
	Description = "Mud. Dirty water and soil. Dries into dirt, freezes solid. Plants and seeds thrive in it.";

	Properties = TYPE_LIQUID;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 268.0f;
	LowTemperatureTransition = PT_STNE; //@ MUD -> STNE (frozen solid)
	HighTemperature = 355.0f;
	HighTemperatureTransition = PT_DIRT; //@ MUD -> DIRT (dries out)

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

		if (TYP(r) == PT_PLNT && sim->rng.chance(1, 300))
		{
			//@ MUD -> PLNT (plant roots spread easily through mud)
			sim->part_change_type(i, x, y, PT_PLNT);
			return 1;
		}
		if (TYP(r) == PT_SEED && sim->rng.chance(1, 60))
		{
			//@ SEED + MUD -> PLNT (rapid germination in wet mud)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_PLNT);
		}
		if (TYP(r) == PT_GRVL && sim->rng.chance(1, 2000))
		{
			//@ MUD + GRVL -> DIRT + GRVL (mud dries on gravel)
			sim->part_change_type(i, x, y, PT_DIRT);
			return 1;
		}
	}
	return 0;
}
