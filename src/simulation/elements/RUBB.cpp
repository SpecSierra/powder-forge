#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_RUBB()
{
	Identifier = "DEFAULT_PT_RUBB";
	Name = "Rubber";
	Colour = 0x1A1A1A_rgb;
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

	Flammable = 15;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 2;

	Weight = 100;

	HeatConduct = 10;
	Description = "Rubber. Electrical insulator. Burns with black smoke. OIL polymerizes into rubber under pressure.";

	// No PROP_CONDUCTS — rubber is a natural electrical insulator
	Properties = TYPE_SOLID;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 500.0f;
	HighTemperatureTransition  = PT_GOO; //@ RUBB -> GOO (melts into sticky substance)

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

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 20))
		{
			//@ RUBB + FIRE -> SMKE (burns slowly with thick black smoke)
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_SMKE);
		}
		if (TYP(r) == PT_OIL && sim->pv[y/CELL][x/CELL] > 5.0f && sim->rng.chance(1, 50))
		{
			//@ OIL + RUBB (pressure > 5) -> RUBB (vulcanization — oil polymerizes into rubber)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_RUBB);
		}
	}
	return 0;
}
