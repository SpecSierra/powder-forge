#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_N2GS()
{
	Identifier = "DEFAULT_PT_N2GS";
	Name = "Nitrogen";
	Colour = 0xB0C0E8_rgb;
	MenuVisible = 1;
	MenuSection = SC_GAS;
	Enabled = 1;

	Advection = 2.0f;
	AirDrag   = 0.00f * CFDS;
	AirLoss   = 0.99f;
	Loss      = 0.30f;
	Collision = -0.1f;
	Gravity   = 0.0f;
	Diffusion = 2.0f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 0;

	Flammable = 0;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 0;

	Weight = 1;

	HeatConduct = 70;
	Description = "Nitrogen gas. Inert. Smothers fire. Produced when liquid nitrogen evaporates. Combines with oxygen at extreme temperatures.";

	Properties = TYPE_GAS;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = 77.0f;
	LowTemperatureTransition  = PT_LNTG; //@ N2GS -> LNTG (condenses back to liquid nitrogen)
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

		if (TYP(r) == PT_FIRE && sim->rng.chance(1, 80))
		{
			//@ N2GS + FIRE -> (nitrogen smothers fire, reduces its life)
			parts[ID(r)].life -= 2;
		}
		if (TYP(r) == PT_O2 && parts[i].temp > 1500.0f && sim->rng.chance(1, 2000))
		{
			//@ N2GS + O2 (>1500K) -> 2xNO2 (nitrogen fixation at extreme heat)
			sim->part_change_type(i, x, y, PT_NO2);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_NO2);
			return 1;
		}
	}
	return 0;
}
