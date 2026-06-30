#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ZINC()
{
	Identifier = "DEFAULT_PT_ZINC";
	Name = "Zinc";
	Colour = 0x7A9BAD_rgb;
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
	Hardness  = 3;

	Weight = 100;

	HeatConduct = 180;
	Description = "Zinc. Sacrificially corrodes to protect adjacent iron. Combine with copper for brass.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 693.0f;
	HighTemperatureTransition  = PT_LAVA; //@ ZINC -> LAVA(ZINC)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool ironNearby = false;

	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		if (abs(rx) == 2 && abs(ry) == 2) continue; // diamond shape
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;
		if (TYP(r) == PT_IRON)
		{
			ironNearby = true;
			break;
		}
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if ((TYP(r) == PT_SLTW || TYP(r) == PT_SALT) && sim->rng.chance(1, ironNearby ? 40 : 200))
		{
			//@ ZINC + SLTW/SALT -> BMTL (galvanic corrosion; faster when protecting iron)
			sim->part_change_type(i, x, y, PT_BMTL);
			parts[i].tmp = sim->rng.between(20, 29);
			return 1;
		}
		if (TYP(r) == PT_ACID && sim->rng.chance(1, 15))
		{
			//@ ZINC + ACID -> (dissolved, zinc reacts readily with acid)
			sim->kill_part(i);
			return 1;
		}
	}
	return 0;
}
