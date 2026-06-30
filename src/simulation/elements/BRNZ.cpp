#include "simulation/ElementCommon.h"

static int graphics(GRAPHICS_FUNC_ARGS);

void Element::Element_BRNZ()
{
	Identifier = "DEFAULT_PT_BRNZ";
	Name = "Bronze";
	Colour = 0xA07040_rgb;
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
	Hardness  = 1;

	Weight = 100;

	HeatConduct = 190;
	Description = "Bronze. Alloy of copper and tin. Harder than pure copper, historically the first alloy.";

	Properties = TYPE_SOLID | PROP_CONDUCTS | PROP_LIFE_DEC | PROP_HOT_GLOW;

	LowPressure            = IPL;
	LowPressureTransition  = NT;
	HighPressure           = IPH;
	HighPressureTransition = NT;
	LowTemperature            = ITL;
	LowTemperatureTransition  = NT;
	HighTemperature            = 1173.0f;
	HighTemperatureTransition  = PT_LAVA; //@ BRNZ -> LAVA(BRNZ)

	Graphics = &graphics;
}

static int graphics(GRAPHICS_FUNC_ARGS)
{
	int rndstore = gfctx.rng.gen();
	*colr += (rndstore % 8) - 4;
	rndstore >>= 3;
	*colg += (rndstore % 6) - 3;
	return 0;
}
