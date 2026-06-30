#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_H2S()
{
	Identifier = "DEFAULT_PT_H2S";
	Name = "Hydrogen Sulfide";
	Colour = 0xD8D870_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = 0.06f;
	Diffusion = 1.0f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 1;

	Flammable = 60;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 60;
	Description = "Hydrogen Sulfide. Toxic volcanic gas. Flammable — burns to sulfur dioxide and steam.";

	Properties = TYPE_GAS | PROP_DEADLY;

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

		if ((TYP(r) == PT_FIRE || TYP(r) == PT_O2) && sim->rng.chance(1, 30))
		{
			//@ H2S + FIRE/O2 -> SO2 + WTRV (combustion: 2H2S + 3O2 -> 2SO2 + 2H2O)
			sim->part_change_type(i, x, y, PT_SO2);
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_WTRV);
			return 1;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 60))
		{
			//@ H2S + ACID -> SO2 + WATR (acid reacts with sulfide)
			sim->part_change_type(i, x, y, PT_SO2);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_WATR);
			return 1;
		}
	}
	return 0;
}
