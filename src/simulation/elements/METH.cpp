#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_METH()
{
	Identifier = "DEFAULT_PT_METH";
	Name = "Methane";
	Colour = 0x90C0D8_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 0.7f;
	AirDrag = 0.01f * CFDS;
	AirLoss = 0.99f;
	Loss = 0.30f;
	Collision = 0.0f;
	Gravity = -0.1f; // lighter than air, rises
	Diffusion = 1.5f;
	HotAir = 0.000f * CFDS;
	Falldown = 0;

	Flammable = 70;
	Explosive = 2;
	Meltable = 0;
	Hardness = 0;

	Weight = 1; // very light gas

	HeatConduct = 100;
	Description = "Methane. Natural gas. Produced by peat and organic decay. Extremely flammable — ignites to fire and CO2.";

	Properties = TYPE_GAS;

	LowPressure = IPL;
	LowPressureTransition = NT;
	HighPressure = IPH;
	HighPressureTransition = NT;
	LowTemperature = 111.0f; // methane condenses at 111K
	LowTemperatureTransition = PT_OIL; //@ METH -> OIL (liquefied natural gas approximation)
	HighTemperature = 600.0f;
	HighTemperatureTransition = PT_FIRE; //@ METH -> FIRE (auto-ignites)

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

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 8))
		{
			//@ METH + FIRE -> FIRE + CO2 (combustion: CH4 + 2O2 -> CO2 + 2H2O)
			parts[i].temp = 1400.0f; // very hot combustion
			sim->part_change_type(i, x, y, PT_FIRE);
			sim->create_part(-1, x + sim->rng.between(-1,1), y + sim->rng.between(-1,1), PT_CO2);
			return 1;
		}
		if (TYP(r) == PT_PLSM && sim->rng.chance(1, 4))
		{
			//@ METH + PLSM -> explosive combustion (unburnt gas ignition)
			parts[i].temp = 2000.0f;
			sim->part_change_type(i, x, y, PT_FIRE);
			return 1;
		}
		if (TYP(r) == PT_LIGH)
		{
			//@ METH + LIGH -> FIRE (lightning ignition)
			sim->part_change_type(i, x, y, PT_FIRE);
			return 1;
		}
	}
	return 0;
}
