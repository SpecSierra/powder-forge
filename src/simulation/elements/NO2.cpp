#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_NO2()
{
	Identifier = "DEFAULT_PT_NO2";
	Name = "Nitrogen Dioxide";
	Colour = 0xB04010_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = 0.10f;
	Diffusion = 0.8f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 60;
	Description = "Nitrogen Dioxide. Brown toxic gas (smog). Dissolves in water to form acid. Decomposes at high heat.";

	Properties = TYPE_GAS | PROP_DEADLY;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 900.0f;
	HighTemperatureTransition  = PT_N2GS; //@ NO2 -> N2GS (thermal decomposition, O2 also released but approximated)

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
			//@ NO2 + WATR/DSTW -> ACID (nitric acid: 3NO2 + H2O -> 2HNO3 + NO)
			sim->kill_part(i);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_ACID);
			return 1;
		}
	}
	return 0;
}
