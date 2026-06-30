#include "simulation/ElementCommon.h"

static int update(UPDATE_FUNC_ARGS);

void Element::Element_CARL()
{
	Identifier = "DEFAULT_PT_CARL";
	Name = "Caramel";
	Colour = 0xC05A0A_rgb;
	MenuVisible = 1;
	MenuSection = SC_LIQUID;
	Enabled = 1;

	Advection = 0.2f;
	AirDrag   = 0.02f * CFDS;
	AirLoss   = 0.95f;
	Loss      = 0.60f;
	Collision = 0.0f;
	Gravity   = 0.07f;  // flows slowly — thick, viscous
	Diffusion = 0.00f;
	HotAir    = 0.000f * CFDS;
	Falldown  = 2;

	Flammable = 15;
	Explosive = 0;
	Meltable  = 0;
	Hardness  = 2;

	Weight = 45;  // denser than water

	DefaultProperties.temp = R_TEMP + 273.15f;
	HeatConduct = 50;
	Description = "Caramel. Formed by heating sugar. Dissolves in water. Flammable.";

	Properties = TYPE_LIQUID;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 630.0f;
	HighTemperatureTransition  = PT_FIRE; //@ CARL -> FIRE (chars and ignites)

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

		if ((TYP(r) == PT_WATR || TYP(r) == PT_DSTW) && sim->rng.chance(1, 400))
		{
			//@ CARL + WATR/DSTW -> 2xSGWT (dissolves into sugar water)
			sim->part_change_type(i, x, y, PT_SGWT);
			sim->part_change_type(ID(r), x+rx, y+ry, PT_SGWT);
			return 1;
		}
	}
	return 0;
}
