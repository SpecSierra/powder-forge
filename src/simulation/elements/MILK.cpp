#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_MILK()
{
	Identifier = "DEFAULT_PT_MILK";
	Name = "Milk";
	Colour = 0xF0F0F8_rgb;
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
	Hardness  = 0; // handle acid manually — ACID produces CURD, not dissolve

	Weight = 31;

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 30;
	Description = "Milk. Curdles with acid or yeast to form cheese curds. Freezes and boils like water.";

	Properties = TYPE_LIQUID;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = 271.0f;
	LowTemperatureTransition  = PT_ICEI; //@ MILK -> ICEI
	HighTemperature            = 375.0f;
	HighTemperatureTransition  = PT_WTRV; //@ MILK -> WTRV (evaporates)

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

		if ((TYP(r) == PT_ACID || TYP(r) == PT_CAUS) && sim->rng.chance(1, 5))
		{
			//@ MILK + ACID/CAUS -> CURD (curdles near-instantly)
			sim->part_change_type(i, x, y, PT_CURD);
			return 1;
		}
		if (TYP(r) == PT_YEST && sim->rng.chance(1, 40))
		{
			//@ MILK + YEST -> CURD + CO2 (fermentation)
			sim->part_change_type(i, x, y, PT_CURD);
			sim->create_part(-1, x + sim->rng.between(-1, 1), y + sim->rng.between(-1, 1), PT_CO2);
			return 1;
		}
	}
	return 0;
}
