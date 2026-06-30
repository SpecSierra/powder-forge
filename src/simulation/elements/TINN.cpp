#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_TINN()
{
	Identifier = "DEFAULT_PT_TINN";
	Name = "Tin";
	Colour = 0xD0D0D8_rgb;
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

	Flammable = 0;
	Explosive = 0;
	Meltable  = 1;
	Hardness  = 2;

	Weight = 100;

	HeatConduct = 160;
	Description = "Tin. Very low melting point. Heat with lead for solder. Heat with copper for bronze.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 505.0f;
	HighTemperatureTransition  = PT_LAVA; //@ TINN -> LAVA(TINN)

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

		if (TYP(r) == PT_LEAD && parts[i].temp > 450.0f && sim->rng.chance(1, 150))
		{
			//@ TINN + LEAD (>450K) -> 2xSOLD (eutectic solder alloy)
			sim->part_change_type(i, x, y, PT_SOLD);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_SOLD);
			return 1;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 40))
		{
			//@ TINN + ACID -> (tin dissolves slowly in acid)
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}
