#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CL2()
{
	Identifier = "DEFAULT_PT_CL2";
	Name = "Chlorine";
	Colour = 0x88CC20_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = 0.12f;
	Diffusion = 0.8f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 1;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 60;
	Description = "Chlorine gas. Toxic. Bleaches water. Reacts with hydrogen to form acid. Accelerates iron corrosion.";

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

		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 15))
		{
			//@ CL2 + WATR/DSTW -> CAUS (hypochlorous acid / bleach solution)
			sim->kill_part(i);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_CAUS);
			return 1;
		}
		if (TYP(r) == PT_H2 && sim->rng.chance(1, 15))
		{
			//@ CL2 + H2 -> ACID (HCl gas, approximated as acid on contact)
			sim->part_change_type(i, x, y, PT_ACID);
			sim->kill_part(ID(r));
			return 1;
		}
		if (TYP(r) == PT_IRON && sim->rng.chance(1, 40))
		{
			//@ CL2 + IRON -> BMTL (iron chloride — chlorine accelerates corrosion)
			sim->part_change_type(ID(r), x+rx, y+ry, PT_BMTL);
			parts[ID(r)].tmp = sim->rng.between(10, 25);
		}
		if (TYP(r) == PT_NH3 && sim->rng.chance(1, 15))
		{
			//@ CL2 + NH3 -> CAUS (chloramine, toxic reaction product)
			sim->part_change_type(i, x, y, PT_CAUS);
			sim->kill_part(ID(r));
			return 1;
		}
	}
	return 0;
}
