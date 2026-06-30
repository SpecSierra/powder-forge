#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_ACET()
{
	Identifier = "DEFAULT_PT_ACET";
	Name = "Acetylene";
	Colour = 0xF0F0D8_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = -0.02f; // slightly lighter than air
	Diffusion = 1.5f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 800;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 70;
	Description = "Acetylene. Extremely flammable. Burns with oxygen at over 3500K — used for welding.";

	Properties = TYPE_GAS | PROP_NEUTPASS;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 573.0f;
	HighTemperatureTransition  = PT_FIRE; //@ ACET -> FIRE (auto-ignites when very hot)

	Update = &update;
}

static int update(UPDATE_FUNC_ARGS)
{
	bool oxygenNearby = false;

	for (auto rx = -2; rx <= 2; rx++)
	for (auto ry = -2; ry <= 2; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;
		if (TYP(r) == PT_O2)
		{
			oxygenNearby = true;
			break;
		}
	}

	for (auto rx = -1; rx <= 1; rx++)
	for (auto ry = -1; ry <= 1; ry++)
	{
		if (!rx && !ry) continue;
		auto r = pmap[y+ry][x+rx];
		if (!r) continue;

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 5))
		{
			if (oxygenNearby)
			{
				//@ ACET + FIRE + O2 -> super-hot FIRE (oxy-acetylene welding flame, >3500K)
				parts[ID(r)].temp = 3773.15f;
				parts[ID(r)].tmp |= 0x3;
				sim->create_part(i, x, y, PT_FIRE);
				parts[i].temp = 3773.15f;
				parts[i].tmp |= 0x3;
			}
			else
			{
				//@ ACET + FIRE -> FIRE (standard combustion, still very hot)
				parts[ID(r)].temp += 500.0f;
				sim->create_part(i, x, y, PT_FIRE);
				parts[i].temp += 500.0f;
			}
			return 1;
		}
	}
	return 0;
}
