#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_HONY()
{
	Identifier = "DEFAULT_PT_HONY";
	Name = "Honey";
	Colour = 0xFFB300_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.1f;
	AirDrag   = 0.02f * CFDS;
	AirLoss   = 0.97f;
	Loss      = 0.55f;
	Collision = 0.0f;
	Gravity   = 0.05f;
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 2;

	Flammable = 10;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 5;

	Weight = 40;

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 25;
	Description = "Honey. Very viscous. Ferments with yeast into mead. Dilutes in water. Caramelizes when heated.";

	Properties = TYPE_LIQUID;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 459.0f;
	HighTemperatureTransition  = PT_CARL; //@ HONY -> CARL (caramelizes)

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

		if (TYP(r) == PT_YEST
			&& parts[i].temp > 283.0f && parts[i].temp < 360.0f
			&& sim->rng.chance(1, 250))
		{
			//@ HONY + YEST -> ALCH + CO2 (mead fermentation)
			sim->part_change_type(i, x, y, PT_ALCH);
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_CO2);
			return 1;
		}
		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 300))
		{
			//@ HONY + WATR/DSTW -> 2xSGWT (dilutes into sugar water)
			sim->part_change_type(i, x, y, PT_SGWT);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_SGWT);
			return 1;
		}
	}
	return 0;
}
