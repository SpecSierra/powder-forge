#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_SGWT()
{
	Identifier = "DEFAULT_PT_SGWT";
	Name = "Sugar Water";
	Colour = 0x4878D0_rgb;
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

	Flammable = 0;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 20;

	Weight = 32; // slightly heavier than water due to dissolved sugar

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 30;
	Description = "Sugar water. Ferments with yeast into alcohol and CO2.";

	Properties = TYPE_LIQUID | PROP_NEUTPASS | PROP_PHOTPASS;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = 271.0f;  // depressed freezing point
	LowTemperatureTransition  = PT_ICEI; //@ SGWT -> ICEI
	HighTemperature            = 375.0f; // elevated boiling point
	HighTemperatureTransition  = PT_WTRV; //@ SGWT -> WTRV

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
			&& sim->rng.chance(1, 300))
		{
			//@ SGWT + YEST -> ALCH + CO2 (fermentation)
			sim->part_change_type(i, x, y, PT_ALCH);
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_CO2);
			return 1;
		}
	}
	return 0;
}
