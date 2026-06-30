#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_NH3()
{
	Identifier = "DEFAULT_PT_NH3";
	Name = "Ammonia";
	Colour = 0xDCEEFF_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = -0.05f; // lighter than air — rises
	Diffusion = 1.8f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 20;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 70;
	Description = "Ammonia gas. Rises (lighter than air). Neutralises acid. Burns to nitrogen and steam.";

	Properties = TYPE_GAS;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = ITH;
	HighTemperatureTransition  = NT;

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

		if ((TYP(r) == PT_ACID || TYP(r) == PT_CAUS) && sim->rng.chance(1, 30))
		{
			//@ NH3 + ACID/CAUS -> SALT + WATR (acid-base neutralisation)
			sim->part_change_type(i, x, y, PT_WATR);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_SALT);
			return 1;
		}
		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 50))
		{
			//@ NH3 + FIRE -> N2GS + WTRV (combustion: 4NH3 + 3O2 -> 2N2 + 6H2O)
			sim->part_change_type(i, x, y, PT_N2GS);
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_WTRV);
			return 1;
		}
	}
	return 0;
}
