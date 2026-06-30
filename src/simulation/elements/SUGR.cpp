#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SUGR()
{
	Identifier = "DEFAULT_PT_SUGR";
	Name = "Sugar";
	Colour = 0xFFFAE0_rgb;
	MenuVisible = 1;
	MenuSection = SC_POWDERS;
	Enabled = 1;

	Advection = 0.4f;
	AirDrag   = 0.04f * CFDS;
	AirLoss   = 0.94f;
	Loss      = 0.95f;
	Collision = -0.1f;
	Gravity   = 0.3f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 1;

	Flammable = 20;
	Explosive = 0;
	Meltable  = 5;
	Hardness  = 1;

	Weight = 80;

	HeatConduct = 120;
	Description = "Sugar. Dissolves in water. Caramelises when heated. Flammable.";

	Properties = TYPE_PART;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 459.0f;
	HighTemperatureTransition  = PT_CARL; //@ SUGR -> CARL (caramelises)

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

		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 200))
		{
			//@ SUGR + WATR/DSTW -> 2xSGWT (dissolves)
			sim->part_change_type(i, x, y, PT_SGWT);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_SGWT);
			return 1;
		}
	}
	return 0;
}
