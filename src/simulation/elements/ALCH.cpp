#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ALCH()
{
	Identifier = "DEFAULT_PT_ALCH";
	Name = "Alcohol";
	Colour = 0xC0DCFF_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.6f;
	AirDrag   = 0.01f * CFDS;
	AirLoss   = 0.98f;
	Loss      = 0.95f;
	Collision = 0.0f;
	Gravity   = 0.1f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 2;

	Flammable = 40;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 2;

	Weight = 25; // ethanol is less dense than water

	HeatConduct = 42;
	Description = "Alcohol. Very flammable. Produced by yeast fermenting sugar water.";

	Properties = TYPE_LIQUID | PROP_NEUTPASS;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL; // freezes at -114 C, never in-game
	LowTemperatureTransition  = NT;
	HighTemperature            = 351.15f; // 78 C, ethanol boiling point
	HighTemperatureTransition  = PT_WTRV; //@ ALCH -> WTRV (evaporates)

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

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 15))
		{
			//@ ALCH + FIRE -> CO2 (combustion byproduct)
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_CO2);
		}
	}
	return 0;
}
