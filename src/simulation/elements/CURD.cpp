#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CURD()
{
	Identifier = "DEFAULT_PT_CURD";
	Name = "Curds";
	Colour = 0xFFF5C0_rgb;
	MenuVisible = 1;
	MenuSection = SC_SOLIDS;
	Enabled = 1;

	Advection = 0.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.90f;
	Loss      = 0.00f;
	Collision = 0.0f;
	Gravity   = 0.0f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 5;
	Explosive = 0;
	Meltable  = 3;
	Hardness  = 5;

	Weight = 100;

	HeatConduct = 40;
	Description = "Cheese curds. Formed when milk curdles with acid or yeast. Melts when heated.";

	Properties = TYPE_SOLID;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 430.0f;
	HighTemperatureTransition  = PT_MILK; //@ CURD -> MILK (melts back into whey)

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

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 30))
		{
			//@ CURD + FIRE -> SMKE (smoulders when burning)
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_SMKE);
		}
	}
	return 0;
}
